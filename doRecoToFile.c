/**
 * @file RecoHelper.C
 * @author H. Sullivan (hsulliva@fnal.gov)
 * @brief Interface to reconstruction algorithm for event display.
 * @date 07-04-2019
 *
 */

#include "Reconstructor.h"
R__LOAD_LIBRARY(libReconstructor.so)
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include "TTree.h"
#include "Pixel.h"
#include <iostream>
#include "Tools/Cfunctions.h"
using namespace std;

/**
 * @brief
 *
 */
// Collects all the variables needed for the reconstruction
struct RecoHelper
{
    typedef shared_ptr<vector<majutil::Pixel>> pixelTablePtr_t;
    typedef map<size_t, size_t> counts_t;

    // Some useful variables
    string     theMethod;
    string     thePixelPath;
    string     theOpRefPath;
    string     FileName;
    string     ImageName;
    double          theDiskRadius;
    double          theGamma;
    double          thePixelSpacing;
    size_t          theDoPenalized;
    size_t          thePenalizedIter;
    size_t          theUnpenalizedIter;
    pixelTablePtr_t thePixelVec = nullptr;
    counts_t        theData;
    Int_t           NSIPMs;
    string     ThePath;

};

struct Data{
    map<size_t, size_t> AnaCounts;
    map<size_t, size_t> ActCounts;
    map<size_t,size_t> FilteredMap;
    map<size_t,size_t> NPairCounts;
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

// Protos
void LoadPixelization(RecoHelper& recoHelper);
void LoadOpRefTable(RecoHelper& recoHelper);
void Reconstruct(RecoHelper *recoHelper,Cfunctions *fun);
    string dataPath;

/**
 * @brief Method to load pixelization scheme.
 *
 */
void LoadPixelization(RecoHelper *recoHelper)
{
    if (!recoHelper->thePixelVec) recoHelper->thePixelVec = make_shared<vector<majutil::Pixel>>();
    recoHelper->thePixelVec->clear();

    // Make pixels for each position
    ifstream f(recoHelper->thePixelPath.c_str());
    if (!f.is_open())
    {
        cerr << "PixelTable::Initialize() Error! Cannot open pixelization file!\n";
        exit(1);
    }

    cout << "Reading pixelization file...\n";

    // Table must be:
    //
    //   pixelID x y
    //

    // First read top line
    string string1, string2, string3;
    getline(f, string1, ' ');
    getline(f, string2, ' ');
    getline(f, string3);
    if(string1 != "pixelID" ||
    string2 != "x"       ||
    string3 != "y")
    {
        cout << "PixelTable::Initialize() Error! ReferenceTable must have "
        << "\'pixelID mppcID probability\' as header.\n";
        exit(1);
    }

    // For computing the size
    unsigned thePixelCount(0);
    float aPixelPos(0);
    float min = numeric_limits<float>::max();
    while (getline(f, string1, ' '))
    {
        getline(f, string2, ' ');
        getline(f, string3);

        unsigned pixelID = stoi(string1);
        float    x;

        if(recoHelper->NSIPMs>32)
            x=stof(string2)*cos(3.14);
        else
            x=stof(string2);
        float    y       = stof(string3);
        thePixelCount++;
        if (thePixelCount == 1) aPixelPos = x;
        else min = abs(aPixelPos-x) < min && abs(aPixelPos-x) > 0 ? abs(aPixelPos-x) : min;

        // Get r and theta just in case we need it
        float r     = sqrt(x*x + y*y);
        float thetaDeg(0);
        if (r > 0.01) thetaDeg = asin(abs(y/r))*180/M_PI;
        // Handle theta convention
        if (x <  0 && y >= 0) thetaDeg = 180 - thetaDeg;
        if (x <  0 && y <  0) thetaDeg = 180 + thetaDeg;
        if (x >= 0 && y <  0) thetaDeg = 360 - thetaDeg;

        recoHelper->thePixelVec->emplace_back(pixelID, x, y, r, thetaDeg);
        recoHelper->thePixelVec->back().SetSize(recoHelper->thePixelSpacing);
        //cout<<"SizeofSIPMs --> " <<recoHelper->NSIPMs<<endl;
    }

    f.close();

    // Sort
    sort( recoHelper->thePixelVec->begin(), recoHelper->thePixelVec->end(), [](const majutil::Pixel& left, const majutil::Pixel& right) { return left.ID() < right.ID(); } );
    cout << "Initialized " << recoHelper->thePixelVec->size() << " " << min << "x" << min << "cm2 pixels...\n";

}

/**
 * @brief Method to load lookup table.
 *
 */
void LoadOpRefTable(RecoHelper *recoHelper)
{
    // Make sure pixels have been initialized
    assert(recoHelper->thePixelVec->size() != 0 && "Pixels have not been initialized!");

    // Read in reference table
    ifstream f(recoHelper->theOpRefPath.c_str());
    if (!f.is_open())
    {
        cout << "PixelTable::LoadReferenceTable() Error! Cannot open reference table file! " << recoHelper->theOpRefPath.c_str() << "\n";
        exit(1);
    }
    cout << "Reading reference table file...\n";

    // Table must be:
    //
    //    pixelID mppcID probability
    //
    string string1, string2, string3;
    getline(f, string1, ' ');
    getline(f, string2, ' ');
    getline(f, string3);

    if (string1 != "pixelID" ||
    string2 != "mppcID"  ||
    string3 != "probability")
    {
        cout << "PixelTable::LoadReferenceTable() Error! ReferenceTable must have "
        << "\'pixelID mppcID probability\' as header.\n";
        exit(1);
    }

    while (getline(f, string1, ' '))
    {
        getline(f, string2, ' ');
        getline(f, string3);

        unsigned pixelID = stoi(string1);
        unsigned mppcID  = stof(string2);
        float    prob    = stof(string3);

        // This assumes the pixels have been ordered
        recoHelper->thePixelVec->at(pixelID-1).AddReference(mppcID, prob);
    }
    f.close();
}

/**
 * @brief Method to start reconstruction algorithm.
 *
 */
void Reconstruct( RecoHelper  *recoHelper, Cfunctions *fun)
{
    cout << "\nIn reconstruct...\n";
    // Initialize the reconstructor
    majreco::Reconstructor theReconstructor(recoHelper->theData, recoHelper->thePixelVec, recoHelper->theDiskRadius);
    if (recoHelper->theMethod == "emml")
    {
        theReconstructor.DoEmMl(recoHelper->theGamma,
        recoHelper->theUnpenalizedIter,
        recoHelper->thePenalizedIter,
        recoHelper->theDoPenalized);
    }
    else

        theReconstructor.DoChi2(recoHelper->theUnpenalizedIter);
        theReconstructor.Dump();
        vector<Double_t> EstValues;
        EstValues=theReconstructor.EstimatedValuesD();




        // Write the reconstructed image
        string FileName;
        FileName=recoHelper->ThePath+"Reco_"+recoHelper->FileName+".root";
        TFile f(FileName.c_str(), "UPDATE");
        fun->f=&f;
        fun->FilePath=recoHelper->ThePath;
        string Chi2Name=recoHelper->ImageName+"_Chi2";
        string MLIName=recoHelper->ImageName+"_MLI";
        string expDataName=recoHelper->ImageName+"_expData";
        fun->mli=theReconstructor.MLImage();
        //theReconstructor.MLImage()->Write(MLIName.c_str());
        fun->chi2=theReconstructor.Chi2Image();
        //theReconstructor.Chi2Image()->Write(Chi2Name.c_str());



        //gROOT->cd("Rint:/");

        // Write the expected data
        auto expdata = theReconstructor.ExpectedCounts();
        string expTitle=expDataName+";SIPM id;Activity";
        TH1I h(expDataName.c_str(), expTitle.c_str(), expdata.size(), 0.5, expdata.size()+0.5);
        unsigned int Tcoun=0;
        for (const auto& d : expdata) {
            h.SetBinContent(d.first, d.second);
            Tcoun+=d.second;
        }
        fun->exp=&h;
        //h.Write();
        cout << "Finished!" << endl;

        //TrueVsReco Graph
        fun->Title=recoHelper->ImageName;
        fun->ExpX=EstValues.at(0);
        fun->ExpY=EstValues.at(1);
        if(Tcoun>0)
            fun->DrawCircle();

        f.Close();
}


void SetVariables(RecoHelper *reco,
                  Cfunctions *fun,
                  string pixelizationPath,
                  string opRefTablePath,
                  string ThePath,
                  string FileName,
                  string TrueFile,
                  string Combined)
                  {
                        vector<string> splitLine;
                        boost::split(splitLine,Combined,boost::is_any_of("_"));

                        reco->theMethod          = "chi2";
                        reco->theDiskRadius      = stof(splitLine[2]);//50.5619;
                        reco->theGamma           = 0.5;
                        reco->theDoPenalized     = true;
                        reco->thePenalizedIter   = 100;
                        reco->theUnpenalizedIter = 100;
                        reco->thePixelSpacing    = stof(splitLine[1]); // in cm
                        reco->NSIPMs             = stoi(splitLine[0]);

                        reco->theOpRefPath = opRefTablePath;
                        reco->thePixelPath = pixelizationPath;
                        reco->FileName     = FileName;
                        reco->ThePath      = ThePath;



                        fun->R                   = stof(splitLine[2]);
                        if(reco->NSIPMs>32) fun->dif                 = 35;
                        else fun->dif                 = 10;
                        fun->FileName            = FileName;
                        fun->TrueFilePath        = TrueFile;
                        LoadPixelization(reco); //This comes before the LoadOpRefTable(reco)
                        LoadOpRefTable(reco);


                  }

/*void ReadDataTree(string DataPath,struct TTreeHelp *htr,struct Data *d1)
{

    auto f  = TFile::Open(DataPath.c_str(),"READ");

    if(!f) return;
    TTree* tr ;
    f->GetObject("ana",tr);
    if (!tr) return;


    tr->SetBranchAddress("TTreeHelp",&htr);
    Long64_t nentries = tr->GetEntries();

    for (Long64_t jentry=0; jentry<nentries; jentry++)
    {
        tr->GetEvent(jentry);

        for(unsigned i=0;i<htr->nsipms;i++)
        {
            if(htr->AnaCounts.at(i)==0)
                d1->AnaCounts.emplace(i,-1);
            else
                d1->AnaCounts.emplace(i,htr->AnaCounts.at(i));

            d1->ActCounts.emplace(i,htr->ActualCounts.at(i));
            d1->FilteredMap.emplace(i,htr->FilteredCounts.at(i));
        }

    }


}
*/

bool isTotalCountsZero(vector< unsigned>a)
{
    unsigned int kTotalCount=0;
    for (unsigned int i=0 ; i<a.size();i++)
        kTotalCount+=a.at(i);

    if (kTotalCount<=0)
        return true ;
    else return false;

}
void ReadMultiData(string DataPath,struct TTreeHelp *htr,struct Data *d1, struct RecoHelper *recoHelper, struct Cfunctions *fun)
{

    auto f  = TFile::Open(DataPath.c_str(),"READ");

    if(!f) return;
    TTree* tr ;
    f->GetObject("ana",tr);
    if (!tr) return;


    tr->SetBranchAddress("TTreeHelp",&htr);
    Long64_t nentries = tr->GetEntries();

    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        tr->GetEvent(jentry);
        for (unsigned i = 0; i < recoHelper->NSIPMs; i++) {
            d1->ActCounts.emplace(i, htr->ActualCounts.at(i));
            d1->FilteredMap.emplace(i, htr->FilteredCounts.at(i));
            d1->AnaCounts.emplace(i, htr->AnaCounts.at(i));
            d1->NPairCounts.emplace(i,htr->NPairCounts.at(i));
        }


        if (d1->ActCounts.size() > 0) {
            recoHelper->ImageName = "Act" + to_string(htr->eventID) + "_";
            recoHelper->theData = d1->ActCounts;
            Reconstruct(recoHelper, fun);
            d1->ActCounts.clear();
            htr->ActualCounts.clear();

        }
        if (d1->AnaCounts.size() > 0) {
            recoHelper->ImageName = "Ana" + to_string(htr->eventID) + "_";;
            recoHelper->theData = d1->AnaCounts;
            Reconstruct(recoHelper,fun);
            d1->AnaCounts.clear();
            htr->AnaCounts.clear();
        }
        if (d1->FilteredMap.size() > 0) {
            recoHelper->ImageName = "FillTered" + to_string(htr->eventID) + "_";
            recoHelper->theData = d1->FilteredMap;
            Reconstruct(recoHelper, fun);
            d1->FilteredMap.clear();
            htr->FilteredCounts.clear();

        }if (d1->NPairCounts.size() > 0) {
            recoHelper->ImageName = "NPair" + to_string(htr->eventID) + "_";
            recoHelper->theData = d1->NPairCounts;
            Reconstruct(recoHelper, fun);
            d1->NPairCounts.clear();
            htr->NPairCounts.clear();
        }


    }

}

/********************************/
void doRecoToFile(string pixelizationPath, string opRefTablePath,string ThePath,string FileName,string TrueFile,string Combined)
{
    RecoHelper recoHelper;
    Cfunctions fun;
    struct TTreeHelp htr;
    Data d1;
    dataPath                = ThePath+"ana_"+FileName+".root";
    SetVariables(&recoHelper,&fun,pixelizationPath,opRefTablePath,ThePath,FileName,TrueFile,Combined);// Get other Variables
    ReadMultiData( dataPath,&htr,&d1,&recoHelper,&fun);

}
