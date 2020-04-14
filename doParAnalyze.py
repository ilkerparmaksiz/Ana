#!/bin/python
import sys
import os
import time

sys.path.insert(1,'Tools/')
import functions as fun
from multiprocessing import Process
from multiprocessing import Pool
def info (title):
    print (title)
    print ("parent Process:",os.getppid())
    print ("process id: ",os.getpid())

#1st Data
FilePath="/home/ilker/Desktop/DATA/Mothership_02_24_2020"

#2nd Data
#FilePath="/home/ilker/Desktop/DATA/MotherShip_02_25_2020_2atm/"

#3rd Data
#FilePath="/home/ilker/Desktop/DATA/MotherShip_02_27_2020_2atm/"

#4th Data
#FilePath="/home/ilker/Desktop/DATA/MotherShip_03_06_2020_6atm/"

#5th Data
#FilePath="/home/ilker/Desktop/DATA/MotherShip_03_06_2020_v2/"

#6th data
#FilePath="/home/ilker/Desktop/DATA/MotherShip_03_10_2020_10atm_v2/"

#7th data
#FilePath="/media/ilker/DATA/Mothership/data/8arg/"

#Small Wheel Data
SFilePath="/home/ilker/Desktop/Analysis/Second_Data/Nov_21_2019"
def info(title):
    print title
    print 'module name:', __name__
    if hasattr(os, 'getppid'):  # only available on Unix
        print 'parent process:', os.getppid()
    print 'process id:', os.getpid()
def doParalel(DataPath,MotherShip,procesess):
    DataPath    = DataPath + "/"
    Files=fun.ReadFiles(DataPath,"root") # Collects the files to analyze with root extension
    for Path in Files :
        print ("Total of  " + str(len(Files[Path])) + " Root Files Will be Analyzed!")
        cn=0
        for file in Files[Path]:
            info("Process for "+file)
            SingleFile= DataPath + file +".root"
            MotherShip["FilePath"]=SingleFile
            p=Process(target=fun.Analyze,kwargs=MotherShip)
            p.start()
            print "Starting to Analyze " + file +".root"
            if(cn==procesess):
                p.join()
                cn=0
            cn+=1
        if(cn<procesess):
            p.join()
def main():
    ''' FilePath            =>  Either a path to root file or to root files
        ThePath             =>  This is the Path to store analyzed and reconstructed files
        Analyze             =>  Takes 1 or 0 (Activates or Deactivates Analysis)
        Reconst             =>  Takes 1 or 0 (Activates or Deactivates Reconstruction)
        pixelPath           =>  The Path to pixelization.txt
        TablePath           =>  The Path to opRefTable.txt or splinedOpRefTable.txt
        Radius              =>  Radius of the disk
        sipmGainPath        =>  Path to the Gains of the SIPMs
        SaveFile            =>  1 Saves the results to file, 0 is off
        Cut                 =>  1 Applys the Cut
        Th                  =>  ADC Value to apply the cut 220 around 1 PE
        NofCoinc            =>  Max Number of SIPM coincidences
        TrueFile            =>  TruePositions of the Files
    '''
    MotherShip = {                                          # Variables to Use for Mothership Data
        "FilePath"          :   "/home/ilker/Desktop/Analysis/Second_Data/Nov_21_2019",
        #"OutPath"           :   "/media/ilker/DATA/Analysis/Mother/100EventsSingle",
        "OutPath"           :   "output2",
        "Analyze"           :   1,
        "Reconst"           :   1,
        "pixelPath"         :   "production/10mm/pixelization.txt",
        "TablePath"         :   "production/10mm/64_sipm/opRefTable.txt",
        "Radius"            :   50.5619,
        "sipmGainPath"      :   "files/sipmGains.txt",
        "SaveFile"          :   1,
        "Cut"               :   1,
        "Th"                :   210,
        "NofCoinc"          :   4,
        "TrueFile"          :  "truePositions.txt"
    }
    SmallWheel = {                                          # Variables to Use for SmallWheel Data
        "FilePath"          :   "",
        "OutPath"           :   "output",
        #"OutPath"           :   "/media/ilker/DATA/Analysis/Small_Wheel/SingleEvents",
        "Analyze"           :   1,
        "Reconst"           :   1,
        "pixelPath"         :   "production/5mm/pixelization.txt",
        "TablePath"         :   "production/5mm/16_sipm/splinedOpRefTable.txt",
        "Radius"            :   15,
        "sipmGainPath"      :   "files/SmallWheelGains.txt",
        "SaveFile"          :   1,
        "Cut"               :   1,
        "Th"                :   210,
        "NofCoinc"          :   4,
        "TrueFile"          :   "truePositions.txt"
    }

    # For Analyzing Multiple or Single Files
    #fun.Analyze(**SmallWheel)
    #fun.Analyze(**MotherShip)

    Home="/media/ilker/DATA/Analysis/Mother/DATA/"
    pro=15
    FilePath=Home+"Mothership_02_24_2020"
    doParalel(FilePath,MotherShip,pro)

    FilePath=Home+"MotherShip_02_25_2020_2atm"
    doParalel(FilePath,MotherShip,pro)

    #3rd Data
    FilePath=Home+"MotherShip_02_27_2020_2atm"
    doParalel(FilePath,MotherShip,pro)


    #4rd Data
    FilePath=Home+"Mothership_03_05_2020_6atm"
    doParalel(FilePath,MotherShip,pro)


    #5th Data
    FilePath=Home+"MotherShip_03_06_2020_v2"
    doParalel(FilePath,MotherShip,pro)


    #6th Data
    FilePath=Home+"MotherShip_03_10_2020_10atm"
    doParalel(FilePath,MotherShip,pro)



    #6th Data
    FilePath=Home+"MotherShip_03_10_2020_10atm_v2"
    doParalel(FilePath,MotherShip,pro)


    FilePath=Home+"MotherShip_03_06_2020_6atm"
    doParalel(FilePath,MotherShip,pro)


if __name__=="__main__":
    main()


