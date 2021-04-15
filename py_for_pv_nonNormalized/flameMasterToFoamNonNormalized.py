import os
import re
import linecache
import csv
import numpy as np
f=os.listdir(os.getcwd()) 
print f  
def is_specie(x):
    return x.split('_',2)[0] == 'CH4'
def out_chi(x):
    return re.findall(r"\d+\.?\d*\e?\-?\d*",x)[3]
f = filter(is_specie, f) 
f.sort(key=lambda x: re.findall(r"\d+\.?\d*",x)[3])     
print f
for c in range(len(f)):
     g1=open('Table'+'_'+out_chi(f[c])+'.csv','w')
     f4=open('list.txt','a')
     f4.write(out_chi(f[c])+'\n')
     f1=open(f[c],'r')
     lines=f1.readlines()
     i=34
     j=34
     h=0
     l=0
     m=0
     flame=np.random.random([181,62])
     FPV=np.random.random([181,14])
     on=[2]*2851
     off=[2]*2851
     list1=['.']
     for line in lines:   
         for list1_element in list1:
             for list2_element in lines[i:i+1]:
                 for list3_element in lines[i+1:i+2]:
                     if list1_element not in list2_element and list1_element in list3_element: 
                        on[i]=1
                     if list1_element in list2_element and list1_element not in list3_element:
                        off[i]=0  
         i=i+1
         if i>2850:
            break
     f1.close
     for j in range(2130):
         if on[j]==1 and on[j+1]==2:
            f2=open('col','a')
            f2.write(linecache.getline(f[c],j+1))
            k=j+1
            m=0
            while k<2130:
                lin=re.findall(r"\d+\.?\d*\e\+?\-?\d*",linecache.getline(f[c],k+1))
                for h in range(len(lin)):
                    flame[m,l]=lin[h]
                    m+=1
                if off[k]==0 and off[k+1]==2:
                   break
                k+=1
            l+=1
     f2.close
     f4.close
     j=2429
     l=0
     while j<2660:
         if on[j]==1 and on[j+1]==2:
            f2=open('col','a')
            f2.write(linecache.getline(f[c],j+1))
            k=j+1
            m=0
            while k<2660:
                lin=re.findall(r"\-?\d+\.?\d*\e\+?\-?\d*",linecache.getline(f[c],k+1))
                for h in range(len(lin)):
                    FPV[m,l]=lin[h]
                    m+=1
                if off[k]==0 and off[k+1]==2:
                   break
                k+=1
            l+=1
         j+=1

     j=2733
     l=6
     while j<2850:
         if on[j]==1 and on[j+1]==2:
            f2=open('col','a')
            f2.write(linecache.getline(f[c],j+1))
            k=j+1
            m=0
            while k<2850:
                lin=re.findall(r"\-?\d+\.?\d*\e\+?\-?\d*",linecache.getline(f[c],k+1))
                for h in range(len(lin)):
                    FPV[m,l]=lin[h]
                    m+=1
                if off[k]==0 and off[k+1]==2:
                   break
                k+=1
            l+=1
         j+=1

     j=2239
     while j<2280:
         if on[j]==1 and on[j+1]==2:
            f2=open('col','a')
            f2.write(linecache.getline(f[c],j+1))
            k=j+1
            m=0
            while k<2280:
                lin=re.findall(r"\-?\d+\.?\d*\e\+?\-?\d*",linecache.getline(f[c],k+1))
                for h in range(len(lin)):
                    FPV[m,13]=lin[h]#FPV 13 is destiny
                    m+=1
                if off[k]==0 and off[k+1]==2:
                   break
                k+=1
         j+=1
         #FPV 7 is ProdRatePos-NO [kg/m^3s],8 is ProdRateNeg-NO [kg/m^3s]
     m=0
     for m in range(181):
         FPV[m, 9] = flame[m, 18] + flame[m, 30]
         FPV[m, 10] = (FPV[m, 0] + FPV[m, 1]) / FPV[m, 13]
         FPV[m, 11] = flame[m, 35] #35 is NO
         FPV[m, 12] = (FPV[m, 6]) / FPV[m, 13] #6 is NO
         
         flame[m, 55] = FPV[m, 9]
         flame[m, 56] = FPV[m, 10]
         flame[m, 57] = FPV[m, 11]
         flame[m, 58] = FPV[m, 12]
         flame[m, 59] = FPV[m, 7] / FPV[m, 13]#FPV 7 is ProdRatePos-NO [kg/m^3s],8 is ProdRateNeg-NO [kg/m^3s]
         flame[m, 60] = FPV[m, 8] / FPV[m, 13]
         flame[m, 61] = flame[m, 55]
     f2.close
     
     g1.write('Z,T,AR,O,O2,H,OH,H2,HO2,H2O2,CH,CO,CH2,HCO,CH2(S),CH3,CH2O,CH4,CO2,CH2OH,CH3O,CH3OH,C2H,C2H2,HCCO,C2H3,CH2CO,C2H4,C2H5,C2H6,H2O,N2,C,HCCOH,N,NO,N2O,NO2,NH,HNO,NH2,NNH,CN,NCO,HCN,HOCN,HNCO,H2CN,HCNN,HCNO,NH3,CH2CHO,CH3CHO,C3H8,C3H7,PVmajor,OMGmajor,PVNO,OMGNO, OMGNOTransportPos, OMGNOTransportNeg, PVmajornonNormalized'+'\n')
     np.savetxt('FPV.csv',FPV,fmt='%e')
     np.savetxt('1.csv',flame,fmt='%e') 
     f3=open('1.csv','r')
     for line in f3.readlines():
         p=','.join(line.split())
         g1.write(p+'\n')
     f3.close
     g1.close
