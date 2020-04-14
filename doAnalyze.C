#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include "TTree.h"
#include <boost/algorithm/string.hpp>
using namespace std;

vector<unsigned> sipmDead;
vector<unsigned> febChannelsVec ;
vector<float> sipmGains;
vector<float> DarkCounts;


typedef map<UInt_t,vector<UShort_t>> ts0ChgMap ;
typedef const pair<const UInt_t,vector<UShort_t>> ts0ChgPair ;

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
string DeadSipmPath            = "files/DEAD_SIPMS.txt";
string DarkPath                = "files/Dark.txt";
string ThePath;
string theOutputPath;
string theTreePath;
string sipmGainPath;
vector<bool> Condition         = {1, 0, 0}; //1 is on, 0 is off; Gain,LEDFilter,DarkCount
vector<bool> FilterCondition   = {1, 0, 0}; //1 is on, 0 is off; Gain,LEDFilter,DarkCount
bool WriteToTextFile                = false; // Allows to Write Number of Entries to text File
UInt_t MaxTimeDifference            = 200000000; //200 us
Int_t  EventLimit                   = 0; // will limit events to 5

//Applying Some Conditions such as Gain, LEDFilter and DarkCount Substraction. 0 is off
struct sAna {
    // Get the ADC integrals
    vector<unsigned> theCounts;
    vector<unsigned> FilteredCounts;
    vector<unsigned> NPairCounts;

    vector<unsigned> sipmDead;
    vector<unsigned> ActualCounts;
    vector<unsigned> DiffTime;

    string FileName;
    string ThePath;
    string theOutputPath;
    string theTreePath;
    string sipmGainPath;
    string theEvdPath              = ThePath + "data.txt";

    int NSIPMs;
    unsigned nentries;
    bool SaveFile;                              //Saving Results to File
    bool Cut;                                   //Threshold Cut is Active
    bool WriteToTextFile;                        // Allows to Write Number of Entries to text File
    bool MotShipData=true;                          // if you analyze the mothership
    int Th ;                                    // Thresshold to filter some events
    int NofCoinc ;                              // Max # of SIPMs in Coincidence  With Specific Th
    vector<UInt_t> Board1ts0;
    vector<UInt_t> Board2ts0;
    vector<int> ts0Substract;

    UShort_t chg[32];
    vector<UShort_t> Board1chg;
    vector<UShort_t> Board2chg;
    UChar_t  mac5;
    UInt_t   ts0;
    UInt_t   count=0;
    bool TimedEvents=true;
    bool   AveragedEvents=true;
    int n                               = 2;



};



// Variables to pass for Update
struct sUpdateHelper{
    string theOutputPath;
    bool MotShipData;

};

struct TTreeHelp{
    Int_t eventID;
    //Int_t nsipms;

    vector<unsigned>ActualCounts;
    vector<unsigned>AnaCounts;
    vector<unsigned>FilteredCounts;
    vector<unsigned>DiffTime;
    vector<unsigned> NPairCounts;

    /*vector<unsigned>DeadSIPM;
    vector<int> ts0sub;
    vector<UInt_t>Board1ts0;
    vector<UInt_t>Board2ts0;
     */
};

sUpdateHelper hUp;
TTreeHelp     htr;
/* Save Analyzed data to a file */
void fUpdate(sUpdateHelper *help,TTreeHelp *htr)
{

    TFile f(help->theOutputPath.c_str(), "UPDATE");
    TTree* t = (TTree*)f.Get("ana");
    if (!t)
    {
        t = new TTree("ana", "ana");
        t->Branch("TTreeHelp",&htr);
    }
    else
        t->SetBranchAddress("TTreeHelp", &htr);


    t->Fill();
    t->Write(t->GetName(), TObject::kWriteDelete);
    t->Reset();

}


// Getting the gains from the file
void fgetSIPMGains (string Path,vector<float> &Gains)
{
    ifstream file(Path);
    string str;
    while (getline(file,str))
        Gains.push_back(stof(str));
    if(Gains.size()>0)
        cout<<"SIPM gains are obtained from the file"<<endl;
    else
        cout<<"SIPM gains could not obtained from the file"<<endl;
}
//Reading the Dead Channels
void fDeadChs (string Path, vector<unsigned> &Dead)
{
    ifstream file(Path);
    string str;
    while (getline(file,str)) Dead.push_back(stoi(str));

    if(Dead.size()>0) cout<<"Dead SIPM are obtained from the file"<<endl;
    else {
        cout << "No Dead SIPMs" << endl;
        Dead.push_back(-9999);
    }

}
void Clear(struct sAna *h)
{

    h->DiffTime.clear();
    fill(h->FilteredCounts.begin(), h->FilteredCounts.end(), 0.0);
    fill(h->theCounts.begin(), h->theCounts.end(), 0.0);
    fill(h->ActualCounts.begin(), h->ActualCounts.end(), 0.0);
    fill(h->NPairCounts.begin(), h->NPairCounts.end(), 0.0);



}
bool fisSIPMGood(int SIPM,vector<unsigned>DeadSipm)
{
    if (find(DeadSipm.begin(),DeadSipm.end(),SIPM)==DeadSipm.end())
        return true;
    else return false;
}

bool is_empty(ifstream& pFile)
{
    return pFile.peek() == ifstream::traits_type::eof();
}
bool LimitEvents(unsigned int EventID)
{
    if(EventLimit!=0) {
        if (EventID == EventLimit){
            cout<<"Cutting the Events short at " <<EventID<<endl;
            return true;
        }
    }

    return false;

}
void fRunUpdate(sAna *hAna)
{
    struct TTreeHelp htr={0};

    if(hAna->SaveFile ) {
        cout << "Updating the file ...\n";

        htr.ActualCounts      = hAna->ActualCounts;
        htr.FilteredCounts    = hAna->FilteredCounts;
        htr.DiffTime          = hAna->DiffTime;
        htr.NPairCounts       = hAna->NPairCounts;
        /*htr.DeadSIPM          = hAna->sipmDead;
        htr.ts0sub            = hAna->ts0Substract;
        htr.Board1ts0         = hAna->Board1ts0;
        htr.Board2ts0         = hAna->Board2ts0;
         */
        htr.eventID            = hAna->count;
        hUp.MotShipData       = hAna->MotShipData;
        hUp.theOutputPath     = hAna->theOutputPath;


        const int nsipms=hAna->NSIPMs;
        int counter=0;
        for (unsigned spm = 0; spm < nsipms; spm++)
        {
            if(find(hAna->sipmDead.begin(),hAna->sipmDead.end(),spm)!=hAna->sipmDead.end())
                htr.AnaCounts.push_back(0);
            else
                htr.AnaCounts.push_back(htr.ActualCounts.at(spm));

        }
        fUpdate(&hUp,&htr);
        Clear(hAna);


    }

}
void fDark (string Path, vector<float> &Dark)
{
    ifstream file(Path);
    string str;
    while (getline(file,str)) {
        Dark.push_back(stof(str));
    }
    if(Dark.size()>0) {
        cout << "Dark counts are obtained from the file" << endl;
    }
    else
        cout<<"Dark counts  could not obtained from the file"<<endl;

}
vector<int>fSubTs0(vector<UInt_t>B1ts0,vector<UInt_t> B2ts0)
{
    int Size=0;
    vector<int> v;
    if(B1ts0.size()>B2ts0.size()) Size=B2ts0.size();else Size=B1ts0.size();
    Int_t Result;
    for (int i =0; i<Size;i++) {
        Result=B2ts0.at(i) - B1ts0.at(i);
        v.push_back(Result);
    }
    B1ts0.clear();
    B2ts0.clear();
    return v;
}

void fCoinThreshold(vector<unsigned> *SIPMPairs,struct sAna *h,unsigned int &PassCnt) {

    // Get the SIPMs pass the Threshold
    Int_t SipmId = 0;
    unsigned int cnt=0;
    for (int i = 0; i < SIPMPairs->size(); i++) {
        SipmId = SIPMPairs->at(i);
        if (SipmId > 31) SipmId -= 32;
        if (!h->MotShipData) SipmId += 6;
        if(h->TimedEvents) {
            if (SIPMPairs->at(i) < 32 && h->Board1chg.at(SipmId) >= h->Th) cnt++;
            else if (SIPMPairs->at(i) > 31 && h->Board2chg.at(SipmId) >= h->Th ) cnt++;
        }
        else if (!h->TimedEvents && h->chg[SipmId] >= h->Th) cnt++;
    }
    if(cnt>=h->NofCoinc-1)
        PassCnt++;

}


void fApplyGain(vector<unsigned> &theCounts, vector<bool>Condition)
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
void fNormalize (string FileName,TTree *t,Int_t Size)
{

    string Board1Cond="mac5==" + to_string(board1Mac);
    string Board2Cond="mac5==" + to_string(board2Mac);
    unsigned Board1Nent=t->GetEntries(Board1Cond.c_str());
    unsigned Board2Nent=t->GetEntries(Board2Cond.c_str());
    vector<TH1F*> Scalled;

    string RFileName="output/Norm_"+FileName+".root";
    TFile *result=TFile::Open(RFileName.c_str(),"RECREATE");
    string CombinedName;
    string Name;

    for (Int_t i=0; i<Size;i++)
    {

        CombinedName= "ch_" + to_string(i)+"_can";
        TCanvas *c2=new TCanvas(CombinedName.c_str());

        CombinedName= "ch_" + to_string(i)+"_"+ to_string(board1Mac);

        Name="chg[" + to_string(i) +"]";

        TH1F *Board1 = new TH1F(CombinedName.c_str(),CombinedName.c_str(),100,0,2500);
        Board1->SetLineWidth(3);
        Board1->SetLineColor(kBlack);
        Name = Name + ">>" + CombinedName;
        t->Draw(Name.c_str(), Board1Cond.c_str()); // Get histograms for board one


        CombinedName= "ch_" + to_string(i)+"_"+ to_string(board2Mac);
        Name="chg[" + to_string(i) +"]";
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

void fCombinedLoopAna(struct sAna *h) {
    size_t sipmId(0);

    if ((h->Board1chg.size() == 0 && h->Board2chg.size()==0) && !h->TimedEvents) {
        for (const auto &relId : febChannelsVec) {
            int sipmId = relId;
            if (h->mac5 == board2Mac && h->MotShipData) sipmId += 32;
            if (!h->MotShipData) sipmId -= 6;
            if (sipmId == 18 && h->MotShipData)
                h->theCounts[sipmId] += h->chg[15];
            else if (sipmId == 15 && h->MotShipData)
                h->theCounts[sipmId] += h->chg[18];
            else h->theCounts[sipmId] += h->chg[relId];

        }
    } else if(h->TimedEvents && (h->Board1chg.size() > 0 && h->Board2chg.size()>0)) {
        for (const auto &relId : febChannelsVec) {
            int sipmId = relId;
            if (sipmId == 18)
                h->theCounts[sipmId] += h->Board1chg.at(15);
            else if (sipmId == 15)
                h->theCounts[sipmId] += h->Board1chg.at(18);
            else
                h->theCounts[sipmId] += h->Board1chg.at(relId);
            h->theCounts[sipmId + 32] += h->Board2chg.at(relId);

        }
    }else return;
    if(!h->AveragedEvents && h->theCounts.size()!=0) {
        h->ActualCounts = h->theCounts;
        fApplyGain(h->theCounts, Condition);
        fApplyGain(h->FilteredCounts, FilterCondition);
        fApplyGain(h->NPairCounts,FilterCondition);
        fRunUpdate(h);
        cout << "Event " << h->count << " is Done!" << endl;
        h->count++;
        if(LimitEvents(h->count)) return;

    }

}

void fNPair(struct sAna *h)
{
    if (h->TimedEvents && (h->Board1chg.size()==0 && h->Board2chg.size()==0))
        return;

    int satCount=0;

    vector<unsigned> CurrentCount(h->n*febChannelsVec.size(),0.);
    for (unsigned int i = 0; i < h->n*febChannelsVec.size(); i++) {
        int sipmId = i;

        if(h->n==1) {
            if (h->mac5 == board2Mac && h->MotShipData) sipmId += 32;
            if (h->MotShipData) h->chg[18] = h->chg[15];
            if(!h->MotShipData) sipmId+=6;

                if (fisSIPMGood(sipmId, h->sipmDead)){
                    CurrentCount.at(sipmId) += h->chg[i];
                    if (h->chg[i] >= h->Th)
                        satCount++;
                }

        }else {

            h->Board1chg.at(18) = h->Board1chg.at(15);
            if(sipmId>31) sipmId-=32;

            if(i<32){

                if(fisSIPMGood(i,h->sipmDead)) {
                    if (h->Board1chg.at(i) >= h->Th) satCount++;
                    CurrentCount.at(i)+=h->Board1chg.at(i);
                }
            }
            else{

                if(fisSIPMGood(i,h->sipmDead)){
                    if(h->Board2chg.at(sipmId)>=h->Th) satCount++;
                    CurrentCount.at(i)+=h->Board2chg.at(sipmId);
                }
            }

        }

    }
    if(satCount>=h->NofCoinc) {
        for (unsigned int i = 0; i < CurrentCount.size(); i++) {

            h->NPairCounts.at(i) += CurrentCount.at(i);
        }
    }
}

void fCoinLoop(struct sAna *h)
{
    if (h->TimedEvents && (h->Board1chg.size()==0 && h->Board2chg.size()==0))
        return;

    int sipmId = 0;

    unsigned int PassCnt=0;

    for (Int_t i = 0; i <= h->n*febChannelsVec.size() - h->NofCoinc; i ++) {
        vector<unsigned> SipmPair;
        int sipmId = i;
        if(h->n==1) {
            if (h->mac5 == board2Mac && h->MotShipData) sipmId += 32;
            if (h->MotShipData) h->chg[18] = h->chg[15];

        }else h->Board1chg.at(18) = h->Board1chg.at(15);

        //Deals With Coincidence
        for (Int_t k = 0; k < h->NofCoinc; k++){

            if (fisSIPMGood(sipmId + k, h->sipmDead)) SipmPair.push_back(sipmId + k);
        }
        if (SipmPair.size() > 2 ) fCoinThreshold(&SipmPair,h,PassCnt);
    }

    if (PassCnt>0) {
        for (Int_t i = 0; i <= h->n * febChannelsVec.size(); i++) {
            int sipmId = i;
            if (h->n == 1) {
                if (h->mac5 == board2Mac && h->MotShipData) sipmId += 32;
                if (h->MotShipData) h->chg[18] = h->chg[15];

                if(fisSIPMGood(sipmId, h->sipmDead))
                    h->FilteredCounts[sipmId]+=h->chg[i];
            } else{

                h->Board1chg.at(18) = h->Board1chg.at(15);
                if(fisSIPMGood(sipmId, h->sipmDead)) {
                    if (i < 32) h->FilteredCounts[i] += h->Board1chg[i];
                    else {
                        sipmId -= 32;
                        h->FilteredCounts[i] += h->Board2chg[sipmId];
                    }
                }
            }

        }
    }
}





void fTimeAnaLoop(struct sAna *h)
{
    unsigned count(1);
    for (auto const &i : QualBoard1Data){

        auto secondValue = QualBoard2Data.find(i.first)->second;
        h->Board1chg=i.second;
        h->Board2chg=secondValue;
        h->DiffTime.push_back(i.first);
        fCoinLoop(h);
        fNPair(h);
        fCombinedLoopAna(h); // calling the loop to do time analysis
        count++;
    }

}

void fTimeAnalysis(struct info *Board1,struct info *Board2) // Getting the Timed events
{
    struct info *First;
    struct info *Second;

    QualBoard1Data.clear();
    QualBoard2Data.clear();

    struct special { // Special Struct to help to time the Events
        Int_t Board1ts0;
        Int_t Board2ts0;
        vector<UShort_t> Board1chg;
        vector<UShort_t> Board2chg;
        Int_t Diffts0;
        info *First;
        info *Second;

        void add(ts0ChgPair item1, ts0ChgPair item2)
        {
            if (item1.second.size()==0 || item2.second.size()==0)
            {
                cout<< "no Channel info"<<endl;
                return ;
            }
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

            }else {cout<<"else is here"<<endl;}
            Int_t result=item1.first-item2.first;
            Diffts0=abs(result);

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

    unsigned int Count;
    unsigned int EventID(0);

    if(First->DataMap.size()==0 || Second->DataMap.size()==0) {cout<<"Timing Maps are empty"<<endl ;return;}

    cout<<"Analyzing the timed Events..."<<endl;

    for (auto const &item1     : First->DataMap) {
        Count=0;
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
        if(temp.Diffts0<MaxTimeDifference)
        {

            QualBoard1Data.emplace(temp.Diffts0,temp.Board1chg);
            QualBoard2Data.emplace(temp.Diffts0,temp.Board2chg);
            //Second->DataMap.erase(Second->DataMap.begin());
            if(LimitEvents(EventID)) break;
            EventID++;
        }


    }
    cout<<"Total of "<< EventID<<" Qualified Time Events is collected !!"<<endl;
}


void fWriteToFile(struct sAna hAna)
{
    fstream outfile(hAna.theEvdPath, ios::out | ios::in | ios::app);
    string line;
    string title="FileName TotalEntries EntBoard1 EntBoard2";

    if(outfile.is_open())
    {
        outfile.seekg(0, ios::end);
        if(outfile.tellg()==0) outfile<< title <<"\n";
        outfile<< hAna.FileName + " " + hAna.nentries << " " << hAna.Board1ts0.size() << " " << hAna.Board2ts0.size() << "\n";

    }
    outfile.close();
}



//------------------------------------------------------------------------
void doAnalyze( string ptreepath,
                string pFileName,
                string pthePath,
                string psipmGainPath,
                string pSaveFile,
                string pCut,
                string pCombined,
                string pTh,
                string pNofCoinc)
{
    cout << "\nAnalyzing Following File:  " + pFileName + ".root \n";

    struct sAna hAna;
    struct info Board1_info;
    struct info Board2_info;

    vector<string> splitLine;
    boost::split(splitLine,pCombined,boost::is_any_of("_"));

    //Incoming Variables
    hAna.FileName        = pFileName;
    hAna.ThePath         = pthePath;
    hAna.theTreePath     = ptreepath+pFileName+".root" ;
    hAna.theOutputPath   = pthePath+"ana_"+pFileName+".root";
    hAna.sipmGainPath    = psipmGainPath;
    hAna.SaveFile        = stoi(pSaveFile);
    hAna.Cut             = stoi(pCut);
    hAna.Th              = stoi(pTh);
    hAna.NofCoinc        = stoi(pNofCoinc);
    hAna.NSIPMs          = stoi(splitLine[0]);
    if(hAna.NSIPMs>31){
        febChannelsVec       = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

    }else{
        febChannelsVec       = {6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
        hAna.n               = 1;
        hAna.TimedEvents     = false;
        hAna.MotShipData     = false;

    }
    int TheSize              = hAna.n * febChannelsVec.size();

    vector<unsigned> theCounts(TheSize, 0.);
    vector<unsigned> FilteredCounts(TheSize, 0.);
    vector<unsigned> NPairCounts(TheSize,0.);

    TFile theFile(hAna.theTreePath.c_str(), "READ");
    TTree* theTree  = (TTree*)theFile.Get("mppc");


    if (Condition[0])  fgetSIPMGains(hAna.sipmGainPath,sipmGains);
    if (Condition[2])  fDark(DarkPath,DarkCounts);

    if(hAna.MotShipData) fDeadChs(DeadSipmPath,hAna.sipmDead);

    if (!theTree) { cout << "WARNING: Couldn't find mppc tree\n"; return; }

    theTree->SetBranchAddress("chg", &hAna.chg);
    theTree->SetBranchAddress("mac5", &hAna.mac5);
    theTree->SetBranchAddress("ts0", &hAna.ts0);

    hAna.nentries = theTree->GetEntries();

    hAna.theCounts=theCounts;
    hAna.FilteredCounts=FilteredCounts;
    hAna.NPairCounts=NPairCounts;

    //if (hAna.MotShipData) fNormalize (pFileName,theTree,febChannelsVec.size());
    for (Long64_t jentry=0; jentry<hAna.nentries; jentry++)
    {
        theTree->GetEntry(jentry);
        if(LimitEvents(hAna.count)) break;
        if (hAna.Cut)
        {
            fCoinLoop(&hAna);
            fNPair(&hAna);
        } //Fill This Before fCombinedLoopAna

        fCombinedLoopAna(&hAna);



        if(hAna.MotShipData) {
            vector<UShort_t> vchg(begin(hAna.chg),end(hAna.chg));
            if (hAna.mac5 == board2Mac) {

                hAna.Board2ts0.push_back(hAna.ts0);
                Board2_info.DataMap.emplace(ts0ChgPair(hAna.ts0,vchg));
                Board2_info.mac5=board2Mac;

            }
            else
            {
                hAna.Board1ts0.push_back(hAna.ts0);
                Board1_info.DataMap.emplace(ts0ChgPair(hAna.ts0,vchg));
                Board1_info.mac5=board1Mac;

            } // Get The Time info
        }
    }



    // Call the function to substract two vectors
    if(hAna.MotShipData && hAna.TimedEvents) {
        hAna.ts0Substract=fSubTs0(hAna.Board1ts0,hAna.Board2ts0);
        fTimeAnalysis(&Board1_info,&Board2_info);
        fTimeAnaLoop(&hAna);

    }
    if(hAna.AveragedEvents && hAna.theCounts.size()>0){
        hAna.ActualCounts=hAna.theCounts;
        fApplyGain(hAna.theCounts,Condition);
        fApplyGain(hAna.FilteredCounts,FilterCondition);
        fApplyGain(hAna.NPairCounts,FilterCondition);

        fRunUpdate(&hAna);
        hAna.count+=1;
    }


    // Update the file for evd
    if(WriteToTextFile) fWriteToFile(hAna);

    gROOT->cd("Rint:/");
    theTree->Reset();
}

