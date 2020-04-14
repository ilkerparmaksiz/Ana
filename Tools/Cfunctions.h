//
// Created by ilker on 3/27/20.
//

#ifndef ANA_CFUNCTIONS_H
#define ANA_CFUNCTIONS_H

#include <cmath>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;
class Cfunctions {
    public:
        string Title;
        TFile * f;
        string FileName;
        string TrueFilePath;
        string FilePath;
        Double_t    R;
        Double_t dif = 30;
        Double_t TrueX;
        Double_t TrueY;
        Double_t ExpX;
        Double_t ExpY;
        TH1I *exp;
        TH2F *chi2;
        TH2F *mli;
        bool CDraw=true;
        bool SaveImg=true;
        bool SaveFile=true;


    struct TTreeHelp{

        vector<double>AnaExpY;
        vector<double>AnaExpX;
        vector<double>AnaErrX;
        vector<double>AnaErrY;

        vector<double>ActExpY;
        vector<double>ActExpX;
        vector<double>ActErrX;
        vector<double>ActErrY;

        vector<double>FiltExpY;
        vector<double>FiltExpX;
        vector<double>FiltErrX;
        vector<double>FiltErrY;

        vector<double>NPairExpY;
        vector<double>NPairExpX;
        vector<double>NPairErrX;
        vector<double>NPairErrY;
    };
    void CreateTree(TTreeHelp *htr)
    {
        TTree* t = (TTree*)f->Get("reco");
        if (!t)
        {
            t = new TTree("reco", "reco");
            t->Branch("Histos",&htr);
        }
        else
            t->SetBranchAddress("Histos",&htr);


        t->Fill();
        t->Write(t->GetName(), TObject::kWriteDelete);
        t->Reset();
    }


    void DrawCircle(){
            TGraph *TruePos = new TGraph(1);
            TGraph *ExpPos = new TGraph(1);
            TGraph *gr;
            TMultiGraph *mg= new TMultiGraph();


            if(!f) return;
            struct TTreeHelp htr;

            Int_t n = 360;
            Double_t x, y;
            string sdev;

            Double_t pR = R + dif;
            Double_t nR = -R - dif;
            vector<string> Error;
            string ErrX;
            string ErrY;

            Error=ErroEst();
            ErrX="ErrX= " + (Error[0]) ;
            ErrY="ErrY= " + (Error[1])  ;

            TCanvas *c1 = new TCanvas(Title.c_str(),Title.c_str(), 800, 800);
            c1->SetGrid();

            //Draw The circle
            gr = new TGraph(n);
            gr->SetMarkerColor(kBlack);
            gr->SetMarkerStyle(21);
            gr->SetLineWidth(1);
            gr->SetMarkerSize(1);


            TruePos->SetMarkerColor(kGreen);
            TruePos->SetMarkerStyle(20);
            TruePos->SetMarkerSize(2);

            ExpPos->SetMarkerColor(kRed);
            ExpPos->SetMarkerStyle(20);
            ExpPos->SetMarkerSize(2);


            for (Int_t i = 0; i < n; i++) {
                x = R * cos(i);
                y = R * sin(i);
                gr->SetPoint(i, x, y);
            }

            TruePos->SetPoint(0,TrueX,TrueY);
            ExpPos->SetPoint(0,ExpX, ExpY);
            if(Title.find("Ana")!=string::npos){
                htr.AnaExpY.push_back(ExpY);
                htr.AnaExpX.push_back(ExpX);
                htr.AnaErrX.push_back(stod(Error[0]));
                htr.AnaErrY.push_back(stod(Error[1]));
            }else if(Title.find("Fill")!=string::npos){
                htr.FiltExpY.push_back(ExpY);
                htr.FiltExpX.push_back(ExpX);
                htr.FiltErrX.push_back(stod(Error[0]));
                htr.FiltErrY.push_back(stod(Error[1]));
            }else if(Title.find("Act")!=string::npos){
                htr.ActExpY.push_back(ExpY);
                htr.ActExpX.push_back(ExpX);
                htr.ActErrX.push_back(stod(Error[0]));
                htr.ActErrY.push_back(stod(Error[1]));
            }else if(Title.find("NPair")!=string::npos){
                htr.NPairExpY.push_back(ExpY);
                htr.NPairExpX.push_back(ExpX);
                htr.NPairErrX.push_back(stod(Error[0]));
                htr.NPairErrY.push_back(stod(Error[1]));
            }
            if(SaveFile)
                this->CreateTree(&htr);


            string TrueLabel;
            string ExpLabel;
            TrueLabel = "TrueX= " + to_string(TrueX) + " , " + "TrueY= " + to_string(TrueY);
            ExpLabel = "ExpX= " + to_string(ExpX) + " , " + "ExpY= " + to_string(ExpY);
            TLegend *leg;
            leg = new TLegend(0.1,0.75,0.9,0.9);
            leg->SetHeader("Results");
            leg->AddEntry(TruePos, TrueLabel.c_str(), "lep");
            leg->AddEntry(ExpPos, ExpLabel.c_str(), "lep");
            leg->AddEntry("ErrX", ErrX.c_str(), "l");
            leg->AddEntry("ErrY", ErrY.c_str(), "l");
            leg->SetEntrySeparation(0.2);
            leg->SetTextSize(0.03);

            mg->Add(gr);
            mg->Add(TruePos);
            mg->Add(ExpPos);

            string MTitle=Title+"_TrueVsExp";
            c1->DrawFrame(nR, nR, pR, pR)->SetTitle(MTitle.c_str());
            mg->Draw("p");
            leg->Draw();

            // Just to Draw Some Plots not all
            if((abs(stof(Error[0]))<150 && abs(stof(Error[1]))<150) || CDraw)
            {
                this->CombinedTCanvas(c1);

            }
    }
    vector<string> ErroEst()
    {
        Double_t ErrX;
        Double_t ErrY;
        ReadTrueFile();
        vector<string> Result;
        if(TrueX==0){ TrueX+=1;ExpX+=1; }
        if(TrueY==0){ TrueY+=1;ExpY+=1; }


        ErrX=abs(ExpX-TrueX/TrueX)*100;
        ErrY=abs(ExpY-TrueY/TrueY)*100;

        Result.push_back(to_string(round(ErrX * 100) / 100.0));
        Result.push_back(to_string(round(ErrY * 100) / 100.0));
        return Result;

    }

    void ReadTrueFile()
    {
        string line;
        ifstream myfile (TrueFilePath);
        vector<string> splitLine;
        unsigned count(0);
        if (myfile.is_open())
        {
            getline (myfile,line);
            while ( getline (myfile,line) )
            {
                boost::split(splitLine,line,boost::is_any_of(" "));
                if(splitLine[0].compare(FileName)==0){

                    TrueX=stof(splitLine[1]);
                    TrueY=stof(splitLine[2]);
                    myfile.close();
                    count++;
                    break;
                }

            }
            if (count==0)
            {
                cout<< "could not find the true position!";
                return;
            }
            myfile.close();

        } else {cout << "Unable to open file True Position File"; return;};
    }

    void CombinedTCanvas(TCanvas *c2)
    {
            string Name=Title+"Combined";
        auto *mc1=new TCanvas(Name.c_str(),Name.c_str(),150,10,800,800);

        mc1->SetTitle(Name.c_str());
        mc1->Divide(2,2,0.01,0.01);
        mc1->SetTopMargin(100);
        mc1->cd(1);
        gPad->SetTickx(2);
        gPad->SetTicky(2);
        chi2->Draw("colz");
        mc1->cd(2);
        gPad->SetTickx(2);
        gPad->SetTicky(2);
        mli->Draw("colz");
        mc1->cd(3);
        gPad->SetTickx(2);
        gPad->SetTicky(2);
        exp->GetYaxis()->SetTitleOffset(1.55);
        exp->Draw();
        mc1->cd(4);
        gPad->SetTickx(2);
        gPad->SetTicky(2);
        c2->DrawClonePad();
        gPad->Update();
        mc1->cd();

        auto *newpad=new TPad("newpad","a transparent pad",1,1,0,0);
        newpad->SetFillStyle(4000);
        newpad->Draw();
        newpad->cd();
        string n=FileName;
        /*auto *title = new TPaveLabel(0.1,0.96,0.9,1,n.c_str());
        title->SetFillColor(16);
        title->SetTextFont(42);
        title->Draw();
         */
        TPaveLabel *title = new TPaveLabel(.4,.95,.60,.99,n.c_str(),"brndc");
        title->Draw();
        gPad->Update();



        /*if(SaveImg && Title.find("Act")!=string::npos){
            string ImageName=FilePath+FileName+Name+".jpg";
            mc1->Print(ImageName.c_str());
        }*/
        string ImageName=FilePath+FileName+Name+".jpg";
        mc1->Print(ImageName.c_str());

        if(SaveFile)
            mc1->Write();

    }


};


#endif //ANA_CFUNCTIONS_H
