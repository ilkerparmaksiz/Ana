#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include "TTree.h"
#include <boost/algorithm/string.hpp>
std::vector<unsigned> sipmDead;
std::vector<unsigned> febChannelsVec ;
std::vector<float> sipmGains;
std::vector<float> DarkCounts;

typedef std::map<UInt_t,std::vector<UShort_t>> ts0ChgMap ;
typedef const std::pair<const UInt_t,std::vector<UShort_t>> ts0ChgPair ;

Int_t eventID                       = 0;
UChar_t board1Mac                   = 0x15;
UChar_t board2Mac                   = 0x55;



struct info{
    UChar_t mac5;
    ts0ChgMap DataMap;

};

ts0ChgMap QualBoard1Data;
ts0ChgMap QualBoard2Data;
// Adjustable Variables
const unsigned ledTriggers          = 132000;
std::string DeadSipmPath            = "files/DEAD_SIPMS.txt";
std::string DarkPath                = "files/Dark.txt";
std::string ThePath;
std::string theOutputPath;
std::string theTreePath;
std::string sipmGainPath;
std::vector<bool> Condition         = {1, 0, 0}; //1 is on, 0 is off; Gain,LEDFilter,DarkCount
std::vector<bool> FilterCondition   = {1, 0, 0}; //1 is on, 0 is off; Gain,LEDFilter,DarkCount
bool WriteToTextFile                = false; // Allows to Write Number of Entries to text File


//Applying Some Conditions such as Gain, LEDFilter and DarkCount Substraction. 0 is off
struct sAna {
    // Get the ADC integrals
    std::vector<unsigned> theCounts;
    std::vector<unsigned> FilteredCounts;
    std::vector<unsigned> TimeCounts;
    std::vector<unsigned> sipmDead;
    std::vector<unsigned> ActualCounts;
    std::string FileName;
    std::string ThePath;
    std::string theOutputPath;
    std::string theTreePath;
    std::string sipmGainPath;
    std::string theEvdPath              = ThePath + "data.txt";

    int NSIPMs;
    unsigned nentries;
    bool SaveFile;                              //Saving Results to File
    bool Cut;                                   //Threshold Cut is Active
    bool WriteToTextFile;                        // Allows to Write Number of Entries to text File
    bool MotShipData ;                          // if you analyze the mothership
    int Th ;                                    // Thresshold to filter some events
    int NofCoinc ;                              // Max # of SIPMs in Coincidence  With Specific Th
    std::vector<UInt_t> Board1ts0;
    std::vector<UInt_t> Board2ts0;
    std::vector<int> ts0Substract;

    UShort_t chg[32];
    UChar_t  mac5;
    UInt_t   ts0;
};



// Variables to pass for Update
struct sUpdateHelper{
    std::string theOutputPath;
    bool MotShipData;

};

struct TTreeHelp{
    Int_t eventID;
    Int_t nsipms;
    std::vector<unsigned>ActualCounts;
    std::vector<unsigned>AnaCounts;
    std::vector<unsigned>FilteredCounts;
    std::vector<unsigned>TimeCounts;
    std::vector<unsigned>DeadSIPM;
    std::vector<int> ts0sub;
    std::vector<UInt_t>Board1ts0;
    std::vector<UInt_t>Board2ts0;
};

/* Save Analyzed data to a file */
void fUpdate(sUpdateHelper help,TTreeHelp *htr)
{
    htr->nsipms=htr->ActualCounts.size();


    for (unsigned i = 0; i < htr->nsipms; i++)
    {
        if(std::find(htr->DeadSIPM.begin(),htr->DeadSIPM.end(),i)!=htr->DeadSIPM.end())
            htr->AnaCounts.push_back(0);
        else
            htr->AnaCounts.push_back(htr->ActualCounts.at(i));


    }


    TFile f(help.theOutputPath.c_str(), "UPDATE");
    TTree* t = (TTree*)f.Get("ana");
    if (!t)
    {
        t = new TTree("ana", "ana");
        /*t->Branch("eventID",&eventID,"eventID/I");
        t->Branch("nsipms",&nsipms,"nsipms/I");
        t->Branch("AnaCounts",AnaCounts,"AnaCounts[nsipms]/I");
        t->Branch("ActCounts",ActCounts,"ActCounts[nsipms]/I");
        t->Branch("ThCounts",&help.FilteredCounts);
        t->Branch("DeadSIPMs",&help.DeadSIPM);
        t->Branch("ts0_Board1", &help.Board1ts0);
        t->Branch("ts0_Board2", &help.Board2ts0);
        t->Branch("ts0Sub", &help.ts0sub);
        */
        t->Branch("TTreeHelp",&htr);
    }
    else
    {
        /*t->SetBranchAddress("eventID", &eventID);
        t->SetBranchAddress("nsipms", &nsipms);
        t->SetBranchAddress("AnaCounts",AnaCounts);
        t->SetBranchAddress("ActCounts",ActCounts);
        t->SetBranchAddress("ThCounts",&help.FilteredCounts);
        t->SetBranchAddress("DeadSIPMs",&help.DeadSIPM);
        t->SetBranchAddress("ts0_Board1", &help.Board1ts0);
        t->SetBranchAddress("ts0_Board2", &help.Board2ts0);
        t->SetBranchAddress("ts0Sub", &help.ts0sub);
        */
        t->SetBranchAddress("TTreeHelp", &htr);
    }

    t->Fill();
    t->Write(t->GetName(), TObject::kWriteDelete);
    t->Reset();
}


// Getting the gains from the file
void fgetSIPMGains (std::string Path,std::vector<float> &Gains)
{
    std::ifstream file(Path);
    std::string str;
    while (std::getline(file,str))
        Gains.push_back(std::stof(str));
    if(Gains.size()>0)
        std::cout<<"SIPM gains are obtained from the file"<<std::endl;
    else
        std::cout<<"SIPM gains could not obtained from the file"<<std::endl;
}
//Reading the Dead Channels
void fDeadChs (std::string Path, std::vector<unsigned> &Dead)
{
    std::ifstream file(Path);
    std::string str;
    while (std::getline(file,str)) Dead.push_back(std::stoi(str));

    if(Dead.size()>0) std::cout<<"Dead SIPM are obtained from the file"<<std::endl;
    else {
        std::cout << "No Dead SIPMs" << std::endl;
        Dead.push_back(-9999);
    }

}
bool fisSIPMGood(int SIPM,std::vector<unsigned>DeadSipm)
{
    if (std::find(DeadSipm.begin(),DeadSipm.end(),SIPM)==DeadSipm.end())
        return true;
    else return false;

}

bool is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

void fDark (std::string Path, std::vector<float> &Dark)
{
    std::ifstream file(Path);
    std::string str;
    while (std::getline(file,str)) {
        Dark.push_back(std::stof(str));
    }
    if(Dark.size()>0) {
        std::cout << "Dark counts are obtained from the file" << std::endl;
    }
    else
        std::cout<<"Dark counts  could not obtained from the file"<<std::endl;

}
std::vector<int>fSubTs0(std::vector<UInt_t>B1ts0,std::vector<UInt_t> B2ts0)
{
    int Size=0;
    std::vector<int> v;
    if(B1ts0.size()>B2ts0.size()) Size=B2ts0.size();else Size=B1ts0.size();
    Int_t Result;
    for (int i =0; i<Size;i++) {
        Result=B2ts0.at(i) - B1ts0.at(i);
        v.push_back(Result);
    }
    return v;
}

void fCoinThreshold(std::vector<unsigned> *SIPMPairs,struct sAna &h)
{
    std::vector<unsigned> ThPassed;
    // Get the SIPMs pass the Threshold
    for (int i=0;i<SIPMPairs->size();i++)
        if((h.chg[SIPMPairs->at(i)]>=h.Th))
            ThPassed.push_back(SIPMPairs->at(i));


    //Apply the Values to Counts
    if((ThPassed.size()==SIPMPairs->size() || ThPassed.size()==(h.Th-1)))
    {
        for (int i=0;i<ThPassed.size();i++)
            h.FilteredCounts[SIPMPairs->at(i)]+=h.chg[SIPMPairs->at(i)];
    }

    SIPMPairs->clear();

}

void fApplyGain(std::vector<unsigned> &theCounts, std::vector<bool>Condition)
{
    // Normalize to one trigger
    // And N = (1/G) * integral
    size_t counter(0);
    for (auto& i : theCounts)
    {

        if(Condition[0]) i/= sipmGains[counter]; // Activates the Appliacation of SIPM Gains
        if(Condition[1]) i /= ledTriggers;       // Use LED Filter
        if(Condition[2]) i=i-DarkCounts[counter];
        counter++;
    }

}
void fNormalize (std::string FileName,TTree *t,Int_t Size)
{

    std::string Board1Cond="mac5==" + std::to_string(board1Mac);
    std::string Board2Cond="mac5==" + std::to_string(board2Mac);
    unsigned Board1Nent=t->GetEntries(Board1Cond.c_str());
    unsigned Board2Nent=t->GetEntries(Board2Cond.c_str());
    std::vector<TH1F*> Scalled;

    std::string RFileName="output/Norm_"+FileName+".root";
    TFile *result=TFile::Open(RFileName.c_str(),"RECREATE");
    std::string CombinedName;
    std::string Name;

    for (Int_t i=0; i<Size;i++)
    {

        CombinedName= "ch_" + std::to_string(i)+"_can";
        TCanvas *c2=new TCanvas(CombinedName.c_str());

        CombinedName= "ch_" + std::to_string(i)+"_"+ std::to_string(board1Mac);

        Name="chg[" + std::to_string(i) +"]";

        TH1F *Board1 = new TH1F(CombinedName.c_str(),CombinedName.c_str(),100,0,2500);
        Board1->SetLineWidth(3);
        Board1->SetLineColor(kBlack);
        Name = Name + ">>" + CombinedName;
        t->Draw(Name.c_str(), Board1Cond.c_str()); // Get histograms for board one


        CombinedName= "ch_" + std::to_string(i)+"_"+ std::to_string(board2Mac);
        Name="chg[" + std::to_string(i) +"]";
        TH1F *Board2 = new TH1F(CombinedName.c_str(),CombinedName.c_str(),100,0,2500);
        Board2->SetLineWidth(3);
        Board2->SetLineColor(kRed);
        Name = Name + ">>" + CombinedName;
        t->Draw(Name.c_str(), Board2Cond.c_str()); // Get histograms for board one
        if(Board1Nent>Board2Nent)
        {
            Double_t Norm=Board1Nent;
            Double_t scale=Norm/(Board2->Integral());
            Board2->Scale(scale);
        }else
        {
            Double_t Norm=Board2Nent;
            Double_t scale=Norm/(Board1->Integral());
            Board1->Scale(scale);

        }
        Board1->Draw();
        Board2->Draw("HISTSAME");
        c2->Write();
        gPad->Update();

    }


}

void fTimeLoopAna(struct sAna &h)
{
    size_t sipmId(0);
    unsigned count;
    for (const auto& relId : febChannelsVec)
    {
        int sipmId = relId;
        if (h.mac5 == board2Mac && h.MotShipData) sipmId += 32;
        if(!h.MotShipData) sipmId -=6;
        if(sipmId==18 && h.MotShipData)
            h.theCounts[sipmId] += h.chg[15];
        else if (sipmId==15 && h.MotShipData)
            h.theCounts[sipmId] += h.chg[18];
        else h.theCounts[sipmId] += h.chg[relId];
    }
}

void fLoopAna(struct sAna &h)
{
    size_t sipmId(0);
    unsigned count;
    for (const auto& relId : febChannelsVec)
    {
        int sipmId = relId;
        //std::cout<<"RelID "<<relId<<std::endl;
        if (h.mac5 == board2Mac && h.MotShipData) sipmId += 32;
        if(!h.MotShipData) sipmId -=6;
        if(sipmId==18 && h.MotShipData)
            h.theCounts[sipmId] += h.chg[15];
        else if (sipmId==15 && h.MotShipData)
            h.theCounts[sipmId] += h.chg[18];
        else h.theCounts[sipmId] += h.chg[relId];
        //std::cout<<"sipmID "<<sipmId<<std::endl;
    }
}

void fCoinLoop(struct sAna &h)
{
    int sipmId = 0;
    std::vector<unsigned> SipmPair;
    for (Int_t i = 0; i < febChannelsVec.size() - h.NofCoinc; i += h.NofCoinc) {

        int sipmId = i;
        if (h.mac5 == board2Mac && h.MotShipData) sipmId += 32;
        if(h.MotShipData) h.chg[18] = h.chg[15];


        //Deals With Coincidence
        for (Int_t k = 0; k < h.NofCoinc; k++)
            if (fisSIPMGood(sipmId + k, h.sipmDead)) SipmPair.push_back(sipmId + k);

        if (SipmPair.size() > 1) fCoinThreshold(&SipmPair,h);

    }
}


void fTimeAnalysis(struct info *Board1,struct info *Board2)
{
    struct info *First;
    struct info *Second;

    struct special {
        Int_t Board1ts0;
        Int_t Board2ts0;
        std::vector<UShort_t> Board1chg;
        std::vector<UShort_t> Board2chg;
        Int_t Diffts0;
        info *First;
        info *Second;

        void add(ts0ChgPair item1, ts0ChgPair item2)
        {
            if(board1Mac==First->mac5)
            {
                Board1ts0=item1.first;
                Board2ts0=item2.first;

                Board1chg=item1.second;
                Board2chg=item2.second;

            }else if(board1Mac==Second->mac5){
                Board1ts0=item2.first;
                Board2ts0=item1.first;


                Board1chg=item2.second;
                Board2chg=item1.second;
            }
            Int_t result=item1.first-item2.first;
            Diffts0=std::abs(result);


        }
    };




    struct special temp;
    struct special Current;


    if(Board1->DataMap.size()>Board2->DataMap.size())
    {
        First=Board2;
        Second=Board1;
    }else if (Board2->DataMap.size()>Board1->DataMap.size())
    {
        First=Board1;
        Second=Board2;
    }

    temp.First=First;
    temp.Second=Second;
    Current.First=First;
    Current.Second=Second;

    unsigned int Count(0);
    unsigned int EventID(0);

    if(First->DataMap.size()==0 || Second->DataMap.size()==0) {std::cout<<"Timing Maps are empty"<<std::endl ;return;}

    for (auto const &item1     : First->DataMap) {
        for (auto const &item2 : Second->DataMap)
        {
            if(Count==0) {
                temp.add(item1,item2);
                Count++;
                continue;
            }
            Current.add(item1,item2);
            if(Current.Diffts0<temp.Diffts0)
                temp=Current;
        }
        if(temp.Diffts0<200000000)
        {
            QualBoard1Data.emplace(temp.Diffts0,temp.Board1chg);
            QualBoard2Data.emplace(temp.Diffts0,temp.Board2chg);
            Second->DataMap.erase(Second->DataMap.begin());
        }

    }

    std::cout<<QualBoard1Data.size()<<std::endl;
    std::cout<<QualBoard2Data.size()<<std::endl;

}
void fWriteToFile(struct sAna hAna)
{
    std::fstream outfile(hAna.theEvdPath, std::ios::out | std::ios::in | std::ios::app);
    std::string line;
    std::string title="FileName TotalEntries EntBoard1 EntBoard2";

    if(outfile.is_open())
    {
        outfile.seekg(0, ios::end);
        if(outfile.tellg()==0) outfile<< title <<"\n";
        outfile<< hAna.FileName + " " + hAna.nentries << " " << hAna.Board1ts0.size() << " " << hAna.Board2ts0.size() << "\n";

    }
    outfile.close();
}



//------------------------------------------------------------------------
void doAnalyze( std::string ptreepath,
                std::string pFileName,
                std::string pthePath,
                std::string psipmGainPath,
                std::string pSaveFile,
                std::string pCut,
                std::string pCombined,
                std::string pTh,
                std::string pNofCoinc)
{
    std::cout << "\nAnalyzing Following File:  " + pFileName + ".root \n";

    struct sAna hAna;
    struct info Board1_info;
    struct info Board2_info;

    std::vector<std::string> splitLine;
    boost::split(splitLine,pCombined,boost::is_any_of("_"));

    //Incoming Variables
    hAna.FileName        = pFileName;
    hAna.ThePath         = pthePath;
    hAna.theTreePath     = ptreepath+pFileName+".root" ;
    hAna.theOutputPath   = pthePath+"ana_"+pFileName+".root";
    hAna.sipmGainPath    = psipmGainPath;
    hAna.SaveFile        = std::stoi(pSaveFile);
    hAna.Cut             = std::stoi(pCut);
    hAna.Th              = std::stoi(pTh);
    hAna.NofCoinc        = std::stoi(pNofCoinc);
    hAna.NSIPMs          = std::stoi(splitLine[0]);
    int n                = 2;

    if(hAna.NSIPMs>32){
        hAna.MotShipData     = true;
        febChannelsVec = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

    }else{
        hAna.MotShipData     = false;
        febChannelsVec = {6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
        n=1;
    }
    std::vector<unsigned> theCounts(n * febChannelsVec.size(), 0.);
    std::vector<unsigned> FilteredCounts(n * febChannelsVec.size(), 0.);
    std::vector<unsigned> TimedCounts(n * febChannelsVec.size(), 0.);

    TFile theFile(hAna.theTreePath.c_str(), "READ");
    TTree* theTree  = (TTree*)theFile.Get("mppc");


    if (Condition[0])  fgetSIPMGains(hAna.sipmGainPath,sipmGains);
    if (Condition[2])  fDark(DarkPath,DarkCounts);

    if(hAna.MotShipData) fDeadChs(DeadSipmPath,hAna.sipmDead);

    if (!theTree) { std::cout << "WARNING: Couldn't find mppc tree\n"; return; }

    theTree->SetBranchAddress("chg", &hAna.chg);
    theTree->SetBranchAddress("mac5", &hAna.mac5);
    theTree->SetBranchAddress("ts0", &hAna.ts0);

    hAna.nentries = theTree->GetEntries();

    hAna.theCounts=theCounts;
    hAna.FilteredCounts=FilteredCounts;
    hAna.TimeCounts=TimedCounts;
    //if (hAna.MotShipData) fNormalize (pFileName,theTree,febChannelsVec.size());

    for (Long64_t jentry=0; jentry<hAna.nentries; jentry++)
    {
        theTree->GetEntry(jentry);

        fLoopAna(hAna);

        if(hAna.Cut) fCoinLoop(hAna);

        if(hAna.MotShipData) {
            std::vector<UShort_t> vchg(std::begin(hAna.chg),std::end(hAna.chg));

            if (hAna.mac5 == board2Mac) {

                hAna.Board2ts0.push_back(hAna.ts0);
                Board2_info.DataMap.emplace(ts0ChgPair(hAna.ts0,vchg));
                Board2_info.mac5=board2Mac;

            }
            else{

                hAna.Board1ts0.push_back(hAna.ts0);
                Board1_info.DataMap.emplace(ts0ChgPair(hAna.ts0,vchg));
                Board2_info.mac5=board1Mac;


            } // Get The Time info
            fTimeAnalysis(&Board1_info,&Board2_info);
        }
    }

    hAna.ActualCounts=hAna.theCounts;
    fApplyGain(hAna.theCounts,Condition);
    fApplyGain(hAna.FilteredCounts,FilterCondition);
    fApplyGain(hAna.TimeCounts,FilterCondition);



    // Call the function to substract two vectors
    if(hAna.MotShipData) {
        hAna.ts0Substract=fSubTs0(hAna.Board1ts0,hAna.Board2ts0);
    }


    if(hAna.SaveFile ) {
        std::cout << "Updating the file ...\n";
        sUpdateHelper hUp;
        TTreeHelp     htr;
        htr.ActualCounts      = hAna.ActualCounts;
        htr.DeadSIPM          = hAna.sipmDead;
        htr.ts0sub            = hAna.ts0Substract;
        htr.Board1ts0         = hAna.Board1ts0;
        htr.Board2ts0         = hAna.Board2ts0;
        htr.FilteredCounts    = hAna.FilteredCounts;
        htr.TimeCounts        = hAna.TimeCounts;
        htr.eventID            = eventID;
        hUp.MotShipData       = hAna.MotShipData;
        hUp.theOutputPath     = hAna.theOutputPath;
        fUpdate(hUp,&htr);
    }

    eventID+=1;

    // Update the file for evd
    if(WriteToTextFile) fWriteToFile(hAna);

    gROOT->cd("Rint:/");
    theTree->Reset();
}

