import pandas as pd
import glob
import os
import openpyxl
from xlsxwriter import Workbook


st = ['loss']
count_loss = [0, 1, 2, 3, 4, 5]
protocol = ['QUIC', 'TCP']
cca = ['bbr']
result_quic = ['goodput']
result_tcp = ['goodput']

# 받아와야하는 데이터 파일 이름 생성하기 !
# loss_0_QUIC_bbr_goodput

txt_name_list = {}



for standard in st:
    if standard == 'loss':
        for count in count_loss:
            for pro in protocol:
                for cc in cca:
                    if pro == 'QUIC': 
                        for re in result_quic:
                            name = '{}_{}_{}_{}_{}'.format(standard, count, pro, cc, re)
                            txt_name_list[name] = [] 
                    elif pro == 'TCP':
                        for re in result_tcp:
                            name = '{}_{}_{}_{}_{}'.format(standard, count, pro, cc, re)
                            txt_name_list[name] = [] 




# 파일 위치가 규칙적으로 여러곳에 흩어져있으니!, 그 때 그 때 필요한 것들을 잘 읽어와야함!
# 각 경로도 리스트로 나타낼 것임.

#실행 위치 "/home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33"


for standard in st:
    if standard == 'loss':
        for count in count_loss:
            for pro in protocol:
                for cc in cca: 
                    for number in range(1,6):
                        if pro == 'QUIC':
                            for re in result_quic:
                                # 파일 경로 생성
                                defualt = '/home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces'
                                file_path = defualt + '/{}/{}/{}/{}/{}/{}.txt'.format(standard, count, pro, cc,number, re)

                                name = '{}_{}_{}_{}_{}'.format(standard, count, pro, cc, re)
                                if os.path.isfile(file_path): 
                                    if os.stat(file_path).st_size == 0:
                                        print(name,'파일이 비어있습니다.')
                                    else:    
                                        if re == 'owd':
                                        # 파일 읽어오기
                                            with open(file_path, 'r') as f:
                                                temp = []
                                                for line in f:
                                                    cols = line.strip().split()
                                                    if cols != '' and cols != ' ' and len(cols)==4: 
                                                        temp.append(float(cols[2]))
                                            
                                                if len(temp) == 0:
                                                    print(file_path,'파일 데이터를 저장한 리스트가 비었습니다.')    
                                                else:
                                                    print(name,"length",len(temp))
                                                    average = sum(temp) / len(temp)
                                                    txt_name_list[name].append(average)
                                        else:
                                            with open(file_path, 'r') as f:
                                                temp = []
                                                for line in f:
                                                    cols = line.strip().split()
                                                    if cols != '' and cols != ' ' and len(cols)==2: 
                                                        temp.append(float(cols[1]))
                                                if len(temp) == 0:
                                                    print(file_path,'파일 데이터를 저장한 리스트가 비었습니다.')    
                                                else:
                                                    #print(name,"length",len(temp))
                                                    average = sum(temp) / len(temp)
                                                    txt_name_list[name].append(average)
                                else:
                                    print(file_path,"파일이 없습니다.")

                        elif pro == 'TCP':
                            for re in result_tcp:
                                # 파일 경로 생성
                                defualt = '/home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces/complete'
                                file_path = defualt + '/{}/{}/{}/{}/{}/{}.txt'.format(standard, count, pro, cc,number, re)

                                name = '{}_{}_{}_{}_{}'.format(standard, count, pro, cc, re)
                                if os.path.isfile(file_path): 
                                    if os.stat(file_path).st_size == 0:
                                        print(name,'파일이 비어있습니다.')
                                    else:    
                                        with open(file_path, 'r') as f:
                                            temp = []
                                            for line in f:
                                                cols = line.strip().split()
                                                if cols != '' and cols != ' ' and len(cols)==2 and float(cols[1]) < 3000000: 
                                                    temp.append(float(cols[1]))
                                            if len(temp) == 0:
                                                print(file_path,'파일 데이터를 저장한 리스트가 비었습니다.')    
                                            else:
                                                #print(name,"sum",sum(temp))
                                               # print(name,"length",len(temp))
                                                average = sum(temp) / len(temp)
                                                txt_name_list[name].append(average)
                                else:
                                    print(file_path,"파일이 없습니다.")  
                                     


# 그전에 Min Max 를 기록 하고,! 그 들을 평균도 있어야합니당

for key, values in txt_name_list.items():
    min_value = 0
    max_value = 0
    avg_value = 0

    min_value = min(values)
    max_value = max(values)
    avg_value = sum(values) / len(values)
    
    txt_name_list[key].append(min_value)
    txt_name_list[key].append(max_value)
    txt_name_list[key].append(avg_value)
    print(key, values, end="\n")


#loss, delay, node 가지 파일로 ! 각각을 나누겠습니다람쥐
# 그전에 Min Max 를 기록 하고,! 그 들을 평균도 있어야합니당

header_list=["name", "1", "2", "3","4","5","min","max","average"]

wb = Workbook("bbr_loss.xlsx")
ws1 = wb.add_worksheet("loss")


row_loss = 1


for header in header_list:
    col=header_list.index(header)
    ws1.write(0,col,header)
    


for name, txt_list in txt_name_list.items():
    if name.find("loss") != -1:
        
        ws1.write(row_loss, 0, name)
        for item in txt_list:
            col=txt_list.index(item)
            col += 1
            ws1.write(row_loss, col, item)
        row_loss += 1    
        




wb.close()
   


