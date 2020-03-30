#!/bin/python
import sys
sys.path.insert(1,'Tools/')
import functions as fun

#1st Data
FilePath="/home/ilker/Desktop/Analysis/Tests/Mothership_02_24_2020/"

#2nd Data
#FilePath="/home/ilker/Desktop/Analysis/Mothership/CAEN_Board/evd/Tests/MotherShip_02_25_2020_2atm/"

#3rd Data
#FilePath="/home/ilker/Desktop/Analysis/Mothership/CAEN_Board/evd/Tests/MotherShip_02_27_2020_2atm/"

#4th Data
#FilePath="/home/ilker/Desktop/Analysis/Mothership/CAEN_Board/evd/Tests/MotherShip_03_06_2020_6atm/"

#5th Data
#FilePath="/home/ilker/Desktop/Analysis/Mothership/CAEN_Board/evd/Tests/MotherShip_03_06_2020_v2/"

#6th data
#FilePath="/home/ilker/Desktop/Analysis/Mothership/CAEN_Board/evd/Tests/MotherShip_03_10_2020_10atm_v2/"

#7th data
#FilePath="/media/ilker/DATA/Mothership/data/8arg/"

#Small Wheel Data
FilePath="/home/ilker/Desktop/Analysis/Second_Data/Nov_21_2019"

def main():
    ''' FilePath    =>  Either a path to root file or to root files
        ThePath     =>  This is the Path to store analyzed and reconstructed files
        Analyze     =>  Takes 1 or 0 (Activates or Deactivates Analysis)
        Reconst     =>  Takes 1 or 0 (Activates or Deactivates Reconstruction)
        pixelPath   =>  The Path to pixelization.txt
        TablePath   =>  The Path to opRefTable.txt
    '''
    MotherShip = {
        "FilePath"          :   FilePath,
        "OutPath"           :   "output",
        "Analyze"           :   1,
        "Reconst"           :   1,
        "pixelPath"         :   "production/10mm/pixelization.txt",
        "TablePath"         :   "production/10mm/64_sipm/opRefTable.txt",
        "Radius"            :   50.5619,
        "sipmGainPath"      :   "files/sipmGains.txt",
        "SaveFile"          :   1,
        "Cut"               :   1,
        "Th"                :   260,
        "NofCoinc"          :   4,
        "TrueFile"          :  "truePositions.txt"
    }
    SmallWheel = {
        "FilePath"          :   FilePath,
        "OutPath"           :   "smallWheelOutput",
        "Analyze"           :   1,
        "Reconst"           :   1,
        "pixelPath"         :   "production/5mm/pixelization.txt",
        "TablePath"         :   "production/5mm/16_sipm/splinedOpRefTable.txt",
        "Radius"            :   15,
        "sipmGainPath"      :   "files/SmallWheelGains.txt",
        "SaveFile"          :   1,
        "Cut"               :   1,
        "Th"                :   260,
        "NofCoinc"          :   4,
        "TrueFile"          :   "5s_TrueInfo.txt"
    }

    # For Analyzing Multiple or Single Files
    fun.Analyze(**SmallWheel)


if __name__=="__main__":
    main()

