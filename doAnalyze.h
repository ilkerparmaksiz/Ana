//
// Created by ilker on 3/22/20.
//

#ifndef ANA_DOANALYZE_H
#define ANA_DOANALYZE_H
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include "TTree.h"

class doAnalyze {

public:

    std::vector<unsigned> sipmDead;
    std::vector<unsigned> febChannelsVec = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    std::vector<float> sipmGains;
    std::vector<float> DarkCounts;
    std::vector<bool> Condition={1,0,0}; //1 is on, 0 is off; Gain,LEDFilter,DarkCount
    std::vector<bool> FilterCondition={1,0,0}; //1 is on, 0 is off; Gain,LEDFilter,DarkCount

    std::string DarkPath="files/Dark.txt";
    std::string sipmGainPath="files/sipmGains.txt";
    std::string DeadSipmPath="files/DEAD_SIPMS.txt";
    unsigned ledTriggers = 132000;
    UChar_t board1Mac = 0x15;
    UChar_t board2Mac = 0x55;
    Int_t eventID=0;
    bool SaveFile=true;
    bool Cut=true;
    int NofCoinc=4;
    int Th =260;
    std::string theTreePath;
    std::string theEvdPath
    std::string theOutputPath;

    // Variables For Update
    struct UpdateHelper{
        std::vector<unsigned>*ActualCounts;
        std::vector<unsigned>*FilteredCounts;
        std::vector<unsigned>*DeadSIPM;
        std::vector<int> *ts0sub;
        std::vector<int>*Board1ts0;
        std::vector<int>*Board2ts0;
    };

/* Save Analyzed data to a file */
    void Update(UpdateHelper help,Int_t eventID)
    {
        Int_t nsipms=help.ActualCounts->size();
        Int_t ActCounts[nsipms];
        Int_t AnaCounts[nsipms];
        for (unsigned i = 0; i < nsipms; i++)
        {
            if(std::find(help.DeadSIPM->begin(),help.DeadSIPM->end(),i)==help.DeadSIPM->end())
                AnaCounts[i]=help.ActualCounts->at(i);
            else
                AnaCounts[i]=-1;
            ActCounts[i]=help.ActualCounts->at(i);
        }


        TFile f(theOutputPath.c_str(), "UPDATE");
        TTree* t = (TTree*)f.Get("ana");
        if (!t)
        {
            t = new TTree("ana", "ana");
            t->Branch("eventID",&eventID,"eventID/I");
            t->Branch("nsipms",&nsipms,"nsipms/I");
            t->Branch("AnaCounts",AnaCounts,"AnaCounts[nsipms]/I");
            t->Branch("ActCounts",ActCounts,"ActCounts[nsipms]/I");
            t->Branch("ThCounts",&help.FilteredCounts);
            t->Branch("DeadSIPMs",&help.DeadSIPM);
            t->Branch("ts0_Board1",&help.Board1ts0);
            t->Branch("ts0_Board2",&help.Board2ts0);
            t->Branch("ts0Sub",&help.ts0sub);

        }
        else
        {
            t->SetBranchAddress("eventID", &eventID);
            t->SetBranchAddress("nsipms", &nsipms);
            t->SetBranchAddress("AnaCounts",&AnaCounts);
            t->SetBranchAddress("ActCounts",&ActCounts);
            t->SetBranchAddress("ThCounts",&help.FilteredCounts);
            t->SetBranchAddress("DeadSIPMs",&help.DeadSIPM);
            t->SetBranchAddress("ts0_Board1",&help.Board1ts0);
            t->SetBranchAddress("ts0_Board2",&help.Board2ts0);
            t->SetBranchAddress("ts0Sub",&help.ts0sub);

        }

        t->Fill();
        t->Write(t->GetName(), TObject::kWriteDelete);
        t->Reset();
    }


// Getting the gains from the file
    void getSIPMGains (std::string Path,std::vector<float> &Gains)
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
    void DeadChs (std::string Path, std::vector<unsigned> &Dead)
    {
        std::ifstream file(Path);
        std::string str;
        while (std::getline(file,str)) {
            Dead.push_back(std::stoi(str));
            std::cout<< "DeadChannels -> " + str<< std::endl;
        }
        if(Dead.size()>0)
            std::cout<<"Dead SIPM are obtained from the file"<<std::endl;
        else {
            std::cout << "No Dead SIPMs" << std::endl;
            Dead.push_back(-9999);
        }

    }
    bool isSIPMGood(int SIPM,std::vector<unsigned>DeadSipm)
    {

        if (std::find(DeadSipm.begin(),DeadSipm.end(),SIPM)==DeadSipm.end())
            return true;
        else return false;

    }

    bool is_empty(std::ifstream& pFile)
    {
        return pFile.peek() == std::ifstream::traits_type::eof();
    }

    void Dark (std::string Path, std::vector<float> &Dark)
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
    std::vector<int> SubTs0(std::vector<int>B1ts0,std::vector<int> B2ts0)
    {
        int Size=0;
        std::vector<int> v;
        if(B1ts0.size()>B2ts0.size()) Size=B2ts0.size();else Size=B1ts0.size();
        for (int i =0; i<Size;i++)
            v.push_back((B2ts0.at(i)-B1ts0.at(i)));

        return v;
    }

    void CoinThreshold(std::vector< unsigned> *DeadSipm, std::vector<unsigned> *SIPMPairs,std::vector<unsigned>&FilteredCounts,UShort_t chg[32])
    {
        std::vector<unsigned> ThPassed;
        // Get the SIPMs pass the Threshold
        for (int i=0;i<SIPMPairs->size();i++)
            if((chg[SIPMPairs->at(i)]>=Th))
                ThPassed.push_back(SIPMPairs->at(i));


        //Apply the Values to Counts
        if((ThPassed.size()==SIPMPairs->size() || ThPassed.size()==(Th-1)) && ThPassed.size()!=0)
        {
            for (int i=0;i<ThPassed.size();i++)
                FilteredCounts[SIPMPairs->at(i)]+=chg[SIPMPairs->at(i)];
        }

        SIPMPairs->clear();

    }

    void ApplyGain(std::vector<unsigned> &theCounts, std::vector<bool>Condition)
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
    void Normalize (std::string FileName,TTree *t,Int_t Size)
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


//------------------------------------------------------------------------
    void doAnalyzer(std::string treepath, std::string outputpath, std::string evdpath,std::string FileName)
    {
        std::cout << "Running doAnalyze...\n";

        theTreePath   = treepath;
        theOutputPath = outputpath;
        theEvdPath    = evdpath;

        UShort_t chg[32];
        UChar_t  mac5;
        UInt_t   ts0;
        TFile theFile(theTreePath.c_str(), "READ");
        TTree* theTree = (TTree*)theFile.Get("mppc");
        std::vector<int> Board1ts0;
        std::vector<int> Board2ts0;
        std::vector<int> ts0Substract;
        getSIPMGains(sipmGainPath,sipmGains);
        DeadChs(DeadSipmPath,sipmDead);
        Dark(DarkPath,DarkCounts);

        if (!theTree) { std::cout << "WARNING: Couldn't find mppc tree\n"; return; }
        theTree->SetBranchAddress("chg", chg);
        theTree->SetBranchAddress("mac5", &mac5);
        theTree->SetBranchAddress("ts0", &ts0);
        int Board1Count=0;
        int Board2Count=0;
        unsigned nentries = theTree->GetEntries();
        Normalize (FileName,theTree,febChannelsVec.size());
        // Get the ADC integrals
        std::vector<unsigned> theCounts(2*febChannelsVec.size(), 0.);
        std::vector<unsigned> FilteredCounts(2*febChannelsVec.size(), 0.);

        for (Long64_t jentry=0; jentry<nentries; jentry++)
        {
            theTree->GetEntry(jentry);
            size_t sipmId(0);
            for (const auto& relId : febChannelsVec)
            {
                int sipmId = relId;
                if (mac5 == board2Mac) sipmId += 32;
                if(sipmId==18)
                    theCounts[sipmId] += chg[15];
                else if (sipmId==15)
                    theCounts[sipmId] += chg[18];
                else theCounts[sipmId] += chg[relId];
            }
            if(Cut) {
                sipmId = 0;
                std::vector<unsigned> SipmPair;
                for (Int_t i = 0; i < febChannelsVec.size() - NofCoinc; i += NofCoinc) {
                    int sipmId = i;

                    if (mac5 == board2Mac) sipmId += 32;
                    chg[18] = chg[15];

                    //Deals With Coincidence
                    for (Int_t k = 0; k < NofCoinc; k++)
                        if (isSIPMGood(sipmId + k, sipmDead)) SipmPair.push_back(sipmId + k);

                    if (SipmPair.size() > 0) CoinThreshold(&sipmDead, &SipmPair, FilteredCounts, chg);

                }
            }

            if (mac5 == board2Mac) Board2ts0.push_back(ts0);else Board1ts0.push_back(ts0); // Get The Time info

        }


        std::vector<unsigned> ActualCounts;

        ActualCounts=theCounts;

        ApplyGain(theCounts,Condition);
        ApplyGain(FilteredCounts,FilterCondition);

        // Call the function to substract two vectors
        ts0Substract=(SubTs0(Board1ts0,Board2ts0));

        std::cout << "Updating ...\n";
        if(SaveFile) {
            UpdateHelper h;
            h.ActualCounts=&ActualCounts;
            h.DeadSIPM=&sipmDead;
            h.ts0sub=&ts0Substract;
            h.Board1ts0=&Board1ts0;
            h.Board2ts0=&Board2ts0;
            h.FilteredCounts=&FilteredCounts;
            Update(h,eventID);
        }
        eventID+=1;

        // Update the file for evd

        std::fstream outfile(theEvdPath, std::ios::out | std::ios::in | std::ios::app);
        std::string line;
        std::string title="FileName TotalEntries EntBoard1 EntBoard2";
        if(outfile.is_open())
        {
            outfile.seekg(0, ios::end);
            if(outfile.tellg()==0) outfile<< title <<"\n";
            outfile<< FileName + " " + nentries << " " << Board1ts0.size() << " " << Board2ts0.size() << "\n";

        }
        /*
        if(outfile.is_open())
        {
            //outfile<<FileName+"-";
            for (const auto &c : theCounts)
            {
                outfile << c << "\n";
                cnt++;
            }

        }*/

        outfile.close();



        gROOT->cd("Rint:/");
        theTree->Reset();
    }

};


#endif //ANA_DOANALYZE_H
