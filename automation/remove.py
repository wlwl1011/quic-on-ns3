import pandas as pd
import glob
import os
import openpyxl
from xlsxwriter import Workbook


st = ['loss', 'delay','node']
count_loss = [0, 1, 2, 3, 4, 5]
count_delay = [0, 50, 100, 150, 200, 250]
count_node = [1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
protocol = ['QUIC', 'TCP']
cca = ['bbr', 'cubic']
result_quic = ['goodput','inflight', 'owd', 'cwnd']
result_tcp = ['goodput','inflight', 'rtt', 'cwnd']


for standard in st:
    if standard == 'loss':
        for count in count_loss:
            for pro in protocol:
                for cc in cca: 
                    for number in range(1,6):
                        defualt = '/home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces'
                        command = defualt + '/{}/{}/{}/{}/'.format(standard, count, pro, cc,number)
                        os.system('sudo find {} -type f -not -name \'*.sh\' -delete'.format(command))          
                
    elif standard == 'delay':
        for count in count_delay:
            for pro in protocol:
                for cc in cca: 
                    for number in range(1,6):
                        defualt = '/home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces'
                        command = defualt + '/{}/{}/{}/{}/'.format(standard, count, pro, cc,number)
                        os.system('sudo find {} -type f -not -name \'*.sh\' -delete'.format(command))       

    elif standard == 'node':
        for count in count_node:
            for pro in protocol:
                for cc in cca: 
                    for number in range(1,6):
                        defualt = '/home/minzzl/networkCongestion/ns-allinone-3.33/ns-3.33/traces'
                        command = defualt + '/{}/{}/{}/{}/'.format(standard, count, pro, cc,number)
                        os.system('sudo find {} -type f -not -name \'*.sh\' -delete'.format(command))           
                        

                                
