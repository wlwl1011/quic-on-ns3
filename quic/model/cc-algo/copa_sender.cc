#include "copa_sender.h"
#include "../model/ns3-quic-congestion-factory.h"
#include <limits>
#include <stdexcept>
#include <sstream>
#include <string>

#include "gquiche/quic/core/congestion_control/rtt_stats.h"
#include "gquiche/quic/core/quic_time.h"
#include "gquiche/quic/core/quic_time_accumulator.h"
#include "gquiche/quic/platform/api/quic_bug_tracker.h"
#include "gquiche/quic/platform/api/quic_flag_utils.h"
#include "gquiche/quic/platform/api/quic_flags.h"
#include "gquiche/quic/platform/api/quic_logging.h"

namespace quic{
namespace{
    const QuicByteCount kDefaultMinimumCongestionWindow = 4 *kDefaultTCPMSS;
    const QuicTime::Delta kMinRTTWindowLength = QuicTime::Delta::FromSeconds(10);
}
template <class T1>
void addAndCheckOverflow(T1& value, const T1& toAdd) {
  if (std::numeric_limits<T1>::max() - toAdd < value) {
    // TODO: the error code is CWND_OVERFLOW but this function can totally be
    // used for inflight bytes.
    throw std::runtime_error("Overflow bytes in flight");
  }
  value +=(toAdd);
}
template <class T1>
void subtractAndCheckUnderflow(T1& value, const T1& toSub) {
  if (value < toSub) {
    // TODO: wrong error code
    throw std::runtime_error("Underflow bytes in flight");
  }
  value -=(toSub);
}


CopaSender::CopaSender(QuicTime now,
            const RttStats* rtt_stats,
            const QuicUnackedPacketMap* unacked_packets,
            QuicPacketCount initial_tcp_congestion_window,
            QuicPacketCount max_tcp_congestion_window,
            QuicRandom* random,
            QuicConnectionStats* stats):
rtt_stats_(rtt_stats),
unacked_packets_(unacked_packets),
random_(random),
stats_(stats),
congestion_window_(initial_tcp_congestion_window * kDefaultTCPMSS),
initial_congestion_window_(initial_tcp_congestion_window *kDefaultTCPMSS),
max_congestion_window_(max_tcp_congestion_window * kDefaultTCPMSS),
min_congestion_window_(kDefaultMinimumCongestionWindow),
pacing_rate_(QuicBandwidth::Zero()),
isSlowStart_(true),
lastCwndDoubleTime_(QuicTime::Zero()),
minRTTFilter_(kMinRTTWindowLength.ToMicroseconds(),QuicTime::Delta::Zero(),0),
standingRTTFilter_(100000/*100 ms*/,QuicTime::Delta::Zero(),0){
  if (stats_) {
    // Clear some startup stats if |stats_| has been used by another sender,
    // which happens e.g. when QuicConnection switch send algorithms.
    stats_->slowstart_count = 0;
    stats_->slowstart_duration = QuicTimeAccumulator();
  }
}
CopaSender::~CopaSender(){}

bool CopaSender::InSlowStart() const{
    return isSlowStart_;
}
bool CopaSender::InRecovery() const{
  return largest_acked_packet_number_.IsInitialized() &&
         largest_sent_at_last_cutback_.IsInitialized() &&
         largest_acked_packet_number_ <= largest_sent_at_last_cutback_;
}
bool CopaSender::ShouldSendProbingPacket() const{
    return false;
}
void CopaSender::SetInitialCongestionWindowInPackets(QuicPacketCount congestion_window){
    if(isSlowStart_){
        initial_congestion_window_ = congestion_window * kDefaultTCPMSS;
        congestion_window_ = congestion_window * kDefaultTCPMSS;
    }
}

void CopaSender::OnCongestionEvent(bool rtt_updated,
                         QuicByteCount prior_in_flight,
                         QuicTime event_time,
                         const AckedPacketVector& acked_packets,
                         const LostPacketVector& lost_packets){
  for (const LostPacket& lost_packet : lost_packets) {
    OnPacketLost(lost_packet.packet_number, lost_packet.bytes_lost,
                 prior_in_flight);
  }
  OnPacketAcked(acked_packets,
                prior_in_flight, event_time);
}
void CopaSender::OnPacketSent(QuicTime sent_time,
                    QuicByteCount bytes_in_flight,
                    QuicPacketNumber packet_number,
                    QuicByteCount bytes,
                    HasRetransmittableData is_retransmittable){
  QUICHE_DCHECK(!largest_sent_packet_number_.IsInitialized() ||
         largest_sent_packet_number_ < packet_number);
  largest_sent_packet_number_ = packet_number;     
}
bool CopaSender::CanSend(QuicByteCount bytes_in_flight){
    return bytes_in_flight<GetCongestionWindow();
}
QuicBandwidth CopaSender::PacingRate(QuicByteCount bytes_in_flight) const{
  QuicTime::Delta srtt = rtt_stats_->SmoothedOrInitialRtt();
  const QuicBandwidth bandwidth =
      QuicBandwidth::FromBytesAndTimeDelta(GetCongestionWindow(), srtt);
      return bandwidth * (InSlowStart() ? 2 : (InRecovery() ? 1 : 1.25));
}
QuicBandwidth CopaSender::BandwidthEstimate() const {
  QuicTime::Delta srtt = rtt_stats_->smoothed_rtt();
  if (srtt.IsZero()) {
    // If we haven't measured an rtt, the bandwidth estimate is unknown.
    return QuicBandwidth::Zero();
  }
  return QuicBandwidth::FromBytesAndTimeDelta(GetCongestionWindow(), srtt);
}
QuicByteCount CopaSender::GetCongestionWindow() const {
    return congestion_window_;
}
QuicByteCount CopaSender::GetSlowStartThreshold() const{
    return 0;
}
CongestionControlType CopaSender::GetCongestionControlType() const{
    return (CongestionControlType)kCopa;
}
std::string CopaSender::GetDebugState() const{
    return "copa";
}
void CopaSender::OnPacketLost(QuicPacketNumber packet_number,
                                       QuicByteCount lost_bytes,
                                       QuicByteCount prior_in_flight){
    if (largest_sent_at_last_cutback_.IsInitialized() &&packet_number <= largest_sent_at_last_cutback_) {
        return;
    }
    largest_sent_at_last_cutback_=largest_sent_packet_number_;
}
void CopaSender::OnPacketAcked(const AckedPacketVector&acked_packets,
                                        QuicByteCount prior_in_flight,
                                        QuicTime event_time){
  for (const AckedPacket acked_packet : acked_packets){
      largest_acked_packet_number_.UpdateMax(acked_packet.packet_number);
  }
  QuicTime::Delta wall_time=event_time-QuicTime::Zero();
  QuicTime::Delta lrtt=rtt_stats_->latest_rtt();
  QuicTime::Delta srtt = rtt_stats_->smoothed_rtt();
  minRTTFilter_.Update(lrtt,wall_time.ToMicroseconds());
  auto rttMin = minRTTFilter_.GetBest();
  standingRTTFilter_.SetWindowLength(srtt.ToMicroseconds()/ 2);
  standingRTTFilter_.Update(lrtt,wall_time.ToMicroseconds());
  auto rttStanding= standingRTTFilter_.GetBest();


  int64_t delayInMicroSec =lrtt.ToMicroseconds()- rttMin.ToMicroseconds();
  if (delayInMicroSec < 0) {
    //LOG(ERROR) << __func__
    //           << "delay negative, lrtt=" << conn_.lossState.lrtt.count()
    //           << " rttMin=" << rttMin.count() << " " << conn_;
    QUICHE_CHECK(0);
    return;
  }
  if (rttStanding.IsZero()) {
    //LOG(ERROR) << __func__ << "rttStandingMicroSec zero, lrtt = "
    //           << conn_.lossState.lrtt.count() << " rttMin=" << rttMin.count()
     //          << " " << conn_;
     QUICHE_CHECK(0);
    return;
  }

  bool increaseCwnd = false;
  if (delayInMicroSec == 0) {
    // taking care of inf targetRate case here, this happens in beginning where
    // we do want to increase cwnd
    increaseCwnd = true;
  } else {
    int64_t targetRate = (1.0 * kDefaultTCPMSS * 1000000) /
        (latencyFactor_ * delayInMicroSec);
    int64_t currentRate = (1.0 * GetCongestionWindow() * 1000000) / rttStanding.ToMicroseconds();

    //VLOG(10) << __func__ << " estimated target rate=" << targetRate
    //         << " current rate=" << currentRate << " " << conn_;
    increaseCwnd = targetRate >= currentRate;
  }

  if (!(increaseCwnd && isSlowStart_)) {
    // Update direction except for the case where we are in slow start mode,
    CheckAndUpdateDirection(event_time);
  }

  if (increaseCwnd) {
    if (isSlowStart_) {
      // When a flow starts, Copa performs slow-start where
      // cwnd doubles once per RTT until current rate exceeds target rate".
      if (!lastCwndDoubleTime_.IsInitialized()) {
        lastCwndDoubleTime_ =event_time;
      } else if ((event_time - lastCwndDoubleTime_)>srtt) {
        QUICHE_DLOG(INFO)<< __func__ << " doubling cwnd per RTT from=" << congestion_window_
                 << " due to slow start";
        addAndCheckOverflow(congestion_window_, congestion_window_);
        lastCwndDoubleTime_ =event_time;
      }
    } else {
      if (velocityState_.direction != VelocityState::Direction::Up &&
          velocityState_.velocity > 1.0) {
        // if our current rate is much different than target, we double v every
        // RTT. That could result in a high v at some point in time. If we
        // detect a sudden direction change here, while v is still very high but
        // meant for opposite direction, we should reset it to 1.
        ChangeDirection(VelocityState::Direction::Up, event_time);
      }
      uint64_t addition = (acked_packets.size()*kDefaultTCPMSS *
                            kDefaultTCPMSS* velocityState_.velocity) /
          (latencyFactor_ * congestion_window_);
      QUICHE_DLOG(INFO)<< __func__ << " increasing cwnd from=" << congestion_window_ << " by "
               << addition;
      addAndCheckOverflow(congestion_window_, addition);
    }
  } else {
    if (velocityState_.direction != VelocityState::Direction::Down &&
        velocityState_.velocity > 1.0) {
      // if our current rate is much different than target, we double v every
      // RTT. That could result in a high v at some point in time. If we detect
      // a sudden direction change here, while v is still very high but meant
      // for opposite direction, we should reset it to 1.
      ChangeDirection(VelocityState::Direction::Down, event_time);
    }
    uint64_t reduction = (acked_packets.size() *kDefaultTCPMSS*
                          kDefaultTCPMSS * velocityState_.velocity) /
        (latencyFactor_ * congestion_window_);
    QUICHE_DLOG(INFO)<< __func__ << " decreasing cwnd from=" << congestion_window_ << " by "
             << reduction;
    isSlowStart_ = false;
    subtractAndCheckUnderflow(
        congestion_window_,
        std::min<uint64_t>(
            reduction,
            congestion_window_ -min_congestion_window_));
  }                                            
}
void CopaSender::CheckAndUpdateDirection(const QuicTime ackTime){
  if (!velocityState_.lastCwndRecordTime.IsInitialized()) {
    velocityState_.lastCwndRecordTime = ackTime;
    velocityState_.lastRecordedCwndBytes = GetCongestionWindow();
    return;
  }
  QuicTime::Delta srtt = rtt_stats_->smoothed_rtt();
  auto elapsed_time = ackTime - velocityState_.lastCwndRecordTime;


  if (elapsed_time >= srtt) {
    auto newDirection = GetCongestionWindow() > velocityState_.lastRecordedCwndBytes
        ? VelocityState::Direction::Up
        : VelocityState::Direction::Down;
    if (newDirection != velocityState_.direction) {
      // if direction changes, change velocity to 1
      velocityState_.velocity = 1;
      velocityState_.numTimesDirectionSame = 0;
    } else {
      velocityState_.numTimesDirectionSame++;
      if (velocityState_.numTimesDirectionSame >= 3) {
        velocityState_.velocity = 2 * velocityState_.velocity;
      }
    }
    /*VLOG(10) << __func__ << " updated direction from "
             << velocityState_.direction << " to " << newDirection
             << " velocityState_.numTimesDirectionSame "
             << velocityState_.numTimesDirectionSame << " velocity "
             << velocityState_.velocity << " " << conn_;*/
    velocityState_.direction = newDirection;
    velocityState_.lastCwndRecordTime = ackTime;
    velocityState_.lastRecordedCwndBytes = GetCongestionWindow();
  }
}
void CopaSender::ChangeDirection(
    VelocityState::Direction newDirection,
    const QuicTime ackTime) {
  if (velocityState_.direction == newDirection) {
    return;
  }
  //VLOG(10) << __func__ << " Suddenly direction change to " << newDirection
  //         << " " << conn_;
  velocityState_.direction = newDirection;
  velocityState_.velocity = 1;
  velocityState_.numTimesDirectionSame = 0;
  velocityState_.lastCwndRecordTime = ackTime;
  velocityState_.lastRecordedCwndBytes =GetCongestionWindow() ;
}
}