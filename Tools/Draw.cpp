//
// Created by ilker on 4/7/20.
//
#include <iostream>
#include <string>
#include "Cfunctions.h"
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>
using namespace std;
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

void CreateTree(string FileName,TTreeHelp *htr)
{

    TFile *f =TFile::Open(FileName.c_str(),"RECREATE");
    TTree* t = (TTree*)f->Get("reco");
    if (!t)
    {
        t = new TTree("reco", "reco");
        t->Branch("Histos",&htr);
        t->Fill();
        t->Write(t->GetName(), TObject::kWriteDelete);
        t->Reset();
    }
    else
        return;
    f->Close();
}

void ReadDataTree(string DataPath,Cfunctions::TTreeHelp *htr,string FileName)
{
    if(!gSystem->AccessPathName(FileName.c_str()))
    {
        cout<<FileName<< " Exists! Skipping Correction.."<<endl;
        return;
    }

    struct TTreeHelp cphtr;
    auto f  = TFile::Open(DataPath.c_str(),"READ");


    TTree* tr = (TTree*)f->Get("reco");
    if (!tr){
        cout<<"Could not Find the Tree"<<endl;
    }
    else
        tr->SetBranchAddress("Histos",&htr);


    Long64_t nentries = tr->GetEntries();
    for (Long64_t jentry=0; jentry<nentries; jentry++)
    {
        tr->GetEvent(jentry);
        if (htr->AnaExpX.size()!=0)
            cphtr.AnaExpX.push_back(htr->AnaExpX.at(0));
        if (htr->AnaExpY.size()!=0)
            cphtr.AnaExpY.push_back(htr->AnaExpY.at(0));
        if (htr->AnaErrX.size()!=0)
            cphtr.AnaErrX.push_back(abs(htr->AnaErrX.at(0)));
        if (htr->AnaErrY.size()!=0)
            cphtr.AnaErrY.push_back(abs(htr->AnaErrY.at(0)));

        if (htr->ActExpX.size()!=0)
            cphtr.ActExpX.push_back(htr->ActExpX.at(0));
        if (htr->ActExpY.size()!=0)
            cphtr.ActExpY.push_back(htr->ActExpY.at(0));
        if (htr->ActErrX.size()!=0)
            cphtr.ActErrX.push_back(abs(htr->ActErrX.at(0)));
        if (htr->ActErrY.size()!=0)
            cphtr.ActErrY.push_back(abs(htr->ActErrY.at(0)));

        if (htr->FiltExpX.size()!=0)
            cphtr.FiltExpX.push_back(htr->FiltExpX.at(0));
        if (htr->FiltExpY.size()!=0)
            cphtr.FiltExpY.push_back(htr->FiltExpY.at(0));
        if (htr->FiltErrX.size()!=0)
            cphtr.FiltErrX.push_back(abs(htr->FiltErrX.at(0)));
        if (htr->FiltErrY.size()!=0)
            cphtr.FiltErrY.push_back(abs(htr->FiltErrY.at(0)));

        if (htr->NPairExpX.size()!=0)
            cphtr.NPairExpX.push_back(htr->NPairExpX.at(0));
        if (htr->NPairExpY.size()!=0)
            cphtr.NPairExpY.push_back(htr->NPairExpY.at(0));
        if (htr->NPairErrX.size()!=0)
            cphtr.NPairErrX.push_back(abs(htr->NPairErrX.at(0)));
        if (htr->NPairErrY.size()!=0)
            cphtr.NPairErrY.push_back(abs(htr->NPairErrY.at(0)));
    }


    CreateTree(FileName,&cphtr);

}

void DrawHistograms(string FilePath,string FileName,Cfunctions *fun,string Mode,string Path)
{
    if(gSystem->AccessPathName(FilePath.c_str()))
    {
        cout<<FilePath<< " Does not Exists! Skipping Drawing.."<<endl;
        return;
    }

    TFile * f= TFile::Open(FilePath.c_str(),"READ");
    TTree* tr = (TTree*)f->Get("reco");
    if (!tr){
        cout<<"Could not Find the Tree"<<endl;
    }
    string Name;
    vector<string>Act={"ActExpX","ActExpY","ActErrX","ActErrY"};
    vector<string>Ana={"AnaExpX","AnaExpY","AnaErrX","AnaErrY"};
    vector<string>Fill={"FiltExpX","FiltExpY","FiltErrX","FiltErrY"};
    vector<string>NPair={"NPairExpX","NPairExpY","NPairErrX","NPairErrY"};
    string TrueX=to_string(fun->TrueX);
    string TrueY=to_string(fun->TrueY);
    Int_t y;
    Int_t x;
    Int_t ep=300;

    string M;
    if(Mode=="small"){
        x=-20;
        y=20;

        M="S";
    }
    else{
        x=-51;
        y=50;
        M="M";
    }



    string CombinedTitle;

    CombinedTitle="Combined_"+FileName;

    TCanvas *c2= new TCanvas(CombinedTitle.c_str(),CombinedTitle.c_str(),800,800);
    c2->Divide(2,2,0.01,0.01);

    unsigned count(0);
    for (auto &i:NPair)
    {
        string hTitle;
        string Title;
        if (count>1){
            y=ep;
            x=0;
            Title=";Error %;Number of Events";

        }
        Title=";Distance in cm;Number of Events";


        CombinedTitle="Canvas_"+FileName+"_"+to_string(count);
        string CanvasTitle;
        if(count==0)
            CanvasTitle="TrueX= "+ TrueX +";Distance in cm;Number of Events" ;
        else if(count==1)
            CanvasTitle="TrueY= " + TrueY +";Distance in cm;Number of Events" ;
        else if(count==2)
            CanvasTitle="ErrX;Error %;Number of Events";
        else
            CanvasTitle="ErrY;Error %;Number of Events";
        TCanvas *c1= new TCanvas(CombinedTitle.c_str(),CanvasTitle.c_str(),800,800);

        vector<string> Combined;
        c1->Divide(2,2,0.01,0.01);
        c1->cd(1);
        CombinedTitle=fun->FileName + "_" + Act.at(count);
        TH1F *hAct = new TH1F(CombinedTitle.c_str(),CanvasTitle.c_str(),50, x, y);
        hAct->SetLineWidth(2);
        hAct->SetLineColor(kRed);
        Name=Act.at(count) + ">>"+CombinedTitle;
        Combined.push_back(Name);
        tr->Draw(Name.c_str());


        c1->cd(2);

        CombinedTitle=fun->FileName + "_" + Ana.at(count);
        hTitle=CombinedTitle+Title;

        TH1F *hAna = new TH1F(CombinedTitle.c_str(),hTitle.c_str(),50, x, y);
        hAna->SetLineWidth(2);
        hAna->SetLineColor(kBlue);
        Name=Ana.at(count) + ">>"+CombinedTitle;
        Combined.push_back(Name);
        if(Mode!="small") tr->Draw(Name.c_str());


        c1->cd(3);
        CombinedTitle=fun->FileName + "_" + Fill.at(count);
        hTitle=CombinedTitle+Title;
        TH1F *hFill = new TH1F(CombinedTitle.c_str(),hTitle.c_str(),50, x, y);
        hFill->SetLineWidth(2);
        hFill->SetLineColor(kGreen);
        Name=Fill.at(count) + ">>"+CombinedTitle;
        Combined.push_back(Name);
        tr->Draw(Name.c_str());
        c1->cd(4);

        CombinedTitle=fun->FileName + "_" + i;
        hTitle=CombinedTitle+Title;
        TH1F *hNPair = new TH1F(CombinedTitle.c_str(),hTitle.c_str(),50, x, y);
        hNPair->SetLineWidth(2);
        hNPair->SetLineColor(kBlack);
        Name=i + ">>"+CombinedTitle;
        tr->Draw(Name.c_str());

        c1->Update();
        gPad->Update();
        string label;
        label=Path+"/"+M+fun->FileName+"_"+to_string(count)+".jpg";
        c1->SaveAs(label.c_str());


            c2->cd(count+1);
            vector<string> comp={"X= "+TrueX+";cm;Events","Y= "+TrueY+";cm;Events","ErrX;%;Events","ErrY;%;Events"};
            hAct->SetTitle(comp.at(count).c_str());
            hAct->Draw("same");
           if(Mode!="small") hAna->Draw("same");
            hFill->Draw("same");
            hNPair->Draw("same");
            count+=1;

    }
            string label=Path+"/"+M+fun->FileName+"_combined.jpg";
            c2->SaveAs(label.c_str());


}






void Draw()
{
    Cfunctions fun;
    Cfunctions::TTreeHelp htr;
    string Path="/media/ilker/DATA/Histograms/10atm";
    string Data=Path+"/Reco_6arg13.root";
    string TrueFilePath=Path+"/truePositions.txt";
    vector<string> result;
    vector<string> FileNamesplit;
    string FileName;
    string FilePath;

    boost::split(result, Data, boost::is_any_of("/"));
    FilePath=result.at(result.size()-1);
    boost::split(FileNamesplit, FilePath, boost::is_any_of("."));
    FileName=FileNamesplit[0];
    FileName=FileName.erase(0,5);
    fun.FileName=FileName;
    fun.TrueFilePath=TrueFilePath;
    fun.ReadTrueFile();
    if(fun.TrueX==0) fun.TrueX+=1;
    if(fun.TrueY==0) fun.TrueY+=1;


    FilePath="/media/ilker/DATA/Histograms/Hist_"+FilePath;
    ReadDataTree(Data,&htr,FilePath);
    DrawHistograms(FilePath,FileName,&fun,"MOT",Path);
}