#!/usr/bin/env python3

import gzip
import sys
from decimal import Decimal
from tempfile import NamedTemporaryFile
import os
import csv
filename=str(sys.argv[1])

numSM=6
#Active idle 17W
#EPIS = [Decimal('1.0E-11'),Decimal('3.58E-12'),Decimal('3.63E-12'),Decimal('3.15E-10'),Decimal('1.03E-09'),Decimal('4.82E-10'),Decimal('5.45E-12'),Decimal('6.15e-11'),Decimal('5.72e-11'),Decimal('6.87e-11'),Decimal('2.17e-10')] 
#Active idle 4W

#T_DECODE,T_ALU,T_FP,T_DP,T_INT_MUL32,T_SFU,NB_RF,L1,SHD_MEM,L2,MEM
EPIS = { "T_DECODE"    : Decimal('2.5E-11'),
         "T_ALU"       : Decimal('3.42E-11'),
         "T_FP"        : Decimal('3.35E-11'),
         "T_DP"        : Decimal('6.59E-10'),
         "T_INT_MUL32" : Decimal('2.313E-09'),
         "T_SFU"       : Decimal('9.52E-10'),
         "NB_RF"       : Decimal('6.05E-12'),
         "L1"          : Decimal('6.15e-11'),
         "SHD_MEM"     : Decimal('5.72e-11'),
         "L2"          : Decimal('6.87e-11'),
         "MEM"         : Decimal('2.17e-10')  }

numepis = 9
sm_components   = ["EXE_T", "EXE_B","I_CACHE","MEM_T","MEM_B","REG_FILE","SHD_MEM"]
numComponents   = len(sm_components)
chip_components = ["L2", "MC0", "MC1", "MC2"]

fn = 'power_trace.gz'
fz = gzip.open(fn, 'wt')
writer = csv.writer(fz, delimiter=',')
labels = [None] * ((6*len(sm_components))+ len(chip_components));
for i in range(0,numSM):
    sm = 'SM' + str(i)
    labels[(i*numComponents)+0] = (sm + '_' + sm_components[0])
    labels[(i*numComponents)+1] = (sm + '_' + sm_components[1])
    labels[(i*numComponents)+2] = (sm + '_' + sm_components[2])
    labels[(i*numComponents)+3] = (sm + '_' + sm_components[3])
    labels[(i*numComponents)+4] = (sm + '_' + sm_components[4])
    labels[(i*numComponents)+5] = (sm + '_' + sm_components[5])
    labels[(i*numComponents)+6] = (sm + '_' + sm_components[6])

labels[-1] = (chip_components[-1])
labels[-2] = (chip_components[-2])
labels[-3] = (chip_components[-3])
labels[-4] = (chip_components[-4])

writer.writerow(labels)

MAX_LINES = 100000
N = 0
period = Decimal('2.7639579878e-07')
try:
    with gzip.open(filename,'r') as fin:
        next(fin)
        for line in fin:
            if N >= MAX_LINES:
                break
            line = line.decode("utf-8").split(',')
            row = [None] * ((6*len(sm_components))+ len(chip_components));
            for sm in range(0,numSM):
                ic  = Decimal(line[(sm*numepis)+0]) * EPIS["T_DECODE"] 
                alu =(Decimal(line[(sm*numepis)+1]) * EPIS["T_ALU"] +  
                      Decimal(line[(sm*numepis)+2]) * EPIS["T_FP"] +  
                      Decimal(line[(sm*numepis)+3]) * EPIS["T_DP"] +  
                      Decimal(line[(sm*numepis)+4]) * EPIS["T_INT_MUL32"] +  
                      Decimal(line[(sm*numepis)+5]) * EPIS["T_SFU"])
                reg = Decimal(line[(sm*numepis)+6]) * EPIS["NB_RF"]
                ls  = Decimal(line[(sm*numepis)+7]) * EPIS["L1"]
                shd = Decimal(line[(sm*numepis)+8]) * EPIS["SHD_MEM"]

                row[(sm*numComponents)+0]  = (alu / Decimal(2.0)) / period
                row[(sm*numComponents)+1]  = (alu / Decimal(2.0)) / period
                row[(sm*numComponents)+2]  = (ic) / period
                row[(sm*numComponents)+3]  = (ls / Decimal(2.0)) / period
                row[(sm*numComponents)+4]  = (ls / Decimal(2.0)) / period
                row[(sm*numComponents)+5]  = reg / period
                row[(sm*numComponents)+6]  = shd / period
                N+=1

            l2   = (Decimal(line[-2]) * EPIS["L2"])
            dram = (Decimal(line[-1]) * EPIS["MEM"])
            row[-1] = (dram / Decimal(3.0)) / period
            row[-2] = (dram / Decimal(3.0)) / period
            row[-3] = (dram / Decimal(3.0)) / period
            row[-4] = l2 / period
            writer.writerow(row)

except FileNotFoundError:
    print("FileNotFound: "+filename)
fz.close()
