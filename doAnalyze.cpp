//
// Created by ilker on 3/22/20.
//

#include "doAnalyze.h"

struct Data{

    string FilePath        = "";
    //string OutPath         = "/media/ilker/DATA/Analysis/Mother/100EventsSingle";
    string OutPath         = "output"
    string sipmGainPath    = "files/sipmGains.txt";
    string pixelPath       = "production/10mm/pixelization.txt";
    string TablePath       = "production/10mm/64_sipm/opRefTable.txt";
    string TrueFile        = "truePositions.txt";

    bool Analyze           = 1;
    bool Reconst           = 1;
    bool SaveFile          = 1;
    bool Cut               = 1;
    bool AveragedEvents    = false;

    Double_t Radius        = 50.5619;
    UShort_t Th            = 210;
    Int_t NofCoinc         = 4;
    unsigned ledTriggers   = 132000;

    std::vector<bool> Condition={1,0,0}; //1 is on, 0 is off; Gain,LEDFilter,DarkCount
    std::vector<bool> FilterCondition={1,0,0}; //1 is on, 0 is off; Gain,LEDFilter,DarkCount


};

int main()
{
    struct Data MotherShip;

    doAnalyze Analyze;
    struct Analyze.Data d;
    d=MotherShip;
    Analyze.fdoAnalyze(&MotherShip);
    return 0;

}
