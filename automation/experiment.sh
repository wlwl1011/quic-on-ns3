
#QUIC - loss
for count in 0 1 2 3 4 5
do
    for cca in cubic bbr
    do
        for trial in 1 2 3 4 5
        do
            ./waf --run "scratch/quic_loss --topo=dumbbell --lo=${count} --cc1=${cca} --folder=loss/${count}/QUIC/${cca}/${trial}" | tee loss_${count}_QUIC_${cca}_${trial}.out
            cd /home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces/loss/${count}/QUIC/${cca}/${trial}
            chmod 777 auto_cat_goodput.sh
            chmod 777 auto_cat_owd.sh
            chmod 777 auto_cat_inflight.sh
            chmod 777 auto_cat_lost.sh
            chmod 777 auto_cat_cwnd.sh
            ./auto_cat_goodput.sh
            ./auto_cat_owd.sh
            ./auto_cat_inflight.sh
            ./auto_cat_lost.sh
            ./auto_cat_cwnd.sh
            cd ../../../../../../
        done
    done
done  

# #QUIC - delay
for count in 0 50 100 150 200 250
do
    for cca in cubic bbr
    do
        for trial in 1 2 3 4 5
        do
    

            ./waf --run "scratch/quic_loss --topo=dumbbell --de=${count} --cc1=${cca} --folder=delay/${count}/QUIC/${cca}/${trial}" | tee delay_${count}_QUIC_${cca}_${trial}.out
            cd /home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces/delay/${count}/QUIC/${cca}/${trial}
            chmod 777 auto_cat_goodput.sh
            chmod 777 auto_cat_owd.sh
            chmod 777 auto_cat_inflight.sh
            chmod 777 auto_cat_lost.sh
            chmod 777 auto_cat_cwnd.sh
            ./auto_cat_goodput.sh
            ./auto_cat_owd.sh
            ./auto_cat_inflight.sh
            ./auto_cat_lost.sh
            ./auto_cat_cwnd.sh
            cd ../../../../../../
        done
    done
done  

#QUIC - node
for count in 1 10 20 30 40 50 60 70 80 90 100
do
    for cca in cubic bbr
    do
        for trial in 1 2 3 4 5
        do
            ./waf --run "scratch/quic_loss --topo=dumbbell --i=${count} --cc1=${cca} --folder=node/${count}/QUIC/${cca}/${trial}" | tee node_${count}_QUIC_${cca}_${trial}.out
            cd /home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces/node/${count}/QUIC/${cca}/${trial}
            chmod 777 auto_cat_goodput.sh
            chmod 777 auto_cat_owd.sh
            chmod 777 auto_cat_inflight.sh
            chmod 777 auto_cat_lost.sh
            chmod 777 auto_cat_cwnd.sh
            ./auto_cat_goodput.sh
            ./auto_cat_owd.sh
            ./auto_cat_inflight.sh
            ./auto_cat_lost.sh
            ./auto_cat_cwnd.sh
            cd ../../../../../../
        done
    done
done  


#TCP - loss
for count in 0 1 2 3 4 5
do
    for cca in cubic bbr
    do
        for trial in 1 2 3 4 5
        do
    

            ./waf --run "scratch/tcp_loss --lo=${count} --cc1=${cca} --folder=loss/${count}/TCP/${cca}/${trial}" | tee loss_${count}_TCP_${cca}_${trial}.out
            cd /home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces/loss/${count}/TCP/${cca}/${trial}
            chmod 777 auto_cat_goodput.sh
            chmod 777 auto_cat_owd.sh
            chmod 777 auto_cat_inflight.sh
            chmod 777 auto_cat_lost.sh
            chmod 777 auto_cat_cwnd.sh
            ./auto_cat_goodput.sh
            ./auto_cat_owd.sh
            ./auto_cat_inflight.sh
            ./auto_cat_lost.sh
            ./auto_cat_cwnd.sh
            cd ../../../../../../
        done
    done
done  

#TCP - delay
for count in 0 50 100 150 200 250
do
    for cca in cubic bbr
    do
        for trial in 1 2 3 4 5
        do
    

            ./waf --run "scratch/tcp_loss --de=${count} --cc1=${cca} --folder=delay/${count}/TCP/${cca}/${trial}" | tee delay_${count}_TCP_${cca}_${trial}.out
            cd /home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces/delay/${count}/TCP/${cca}/${trial}
            chmod 777 auto_cat_goodput.sh
            chmod 777 auto_cat_owd.sh
            chmod 777 auto_cat_inflight.sh
            chmod 777 auto_cat_lost.sh
            chmod 777 auto_cat_cwnd.sh
            ./auto_cat_goodput.sh
            ./auto_cat_owd.sh
            ./auto_cat_inflight.sh
            ./auto_cat_lost.sh
            ./auto_cat_cwnd.sh
            cd ../../../../../../
        done
    done
done  

#TCP - node
for count in 1 10 20 30 40 50 60 70 80 90 100
do
    for cca in cubic bbr
    do
        for trial in 1 2 3 4 5
        do
            ./waf --run "scratch/quic_loss --i=${count} --cc1=${cca} --folder=node/${count}/TCP/${cca}/${trial}" | tee node_${count}_TCP_${cca}_${trial}.out
            cd /home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces/node/${count}/TCP/${cca}/${trial}
            chmod 777 auto_cat_goodput.sh
            chmod 777 auto_cat_owd.sh
            chmod 777 auto_cat_inflight.sh
            chmod 777 auto_cat_lost.sh
            chmod 777 auto_cat_cwnd.sh
            ./auto_cat_goodput.sh
            ./auto_cat_owd.sh
            ./auto_cat_inflight.sh
            ./auto_cat_lost.sh
            ./auto_cat_cwnd.sh
            cd ../../../../../../
        done
    done
done  
