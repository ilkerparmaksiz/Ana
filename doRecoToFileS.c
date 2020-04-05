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


/**
 * @brief
 *
 */

// Collects all the variables needed for the reconstruction
struct RecoHelper
{
    typedef std::shared_ptr<std::vector<majutil::Pixel>> pixelTablePtr_t;
    typedef std::map<size_t, size_t> counts_t;

    // Some useful variables
    std::string     theMethod;
    std::string     thePixelPath;
    std::string     theOpRefPath;
    std::string     FileName;
    std::string     ImageName;
    double          theDiskRadius;
    double          theGamma;
    double          thePixelSpacing;
    size_t          theDoPenalized;
    size_t          thePenalizedIter;
    size_t          theUnpenalizedIter;
    pixelTablePtr_t thePixelVec = nullptr;
    counts_t        theData;
    Int_t           NSIPMs;
    std::string     ThePath;

};

struct Data{
    std::map<size_t, size_t> AnaCounts;
    std::map<size_t, size_t> ActCounts;
    std::map<size_t,size_t> FilteredMap;
};

struct TTreeHelp{
    Int_t eventID;
    //Int_t nsipms;
    std::vector<unsigned>ActualCounts;
    std::vector<unsigned>AnaCounts;
    std::vector<unsigned>FilteredCounts;
    std::vector<unsigned>DiffTime;
    /*std::vector<unsigned>DeadSIPM;
    std::vector<int> ts0sub;
    std::vector<UInt_t>Board1ts0;
    std::vector<UInt_t>Board2ts0;
     */
};

// Protos
void LoadPixelization(RecoHelper& recoHelper);
void LoadOpRefTable(RecoHelper& recoHelper);
void Reconstruct(RecoHelper *recoHelper,Cfunctions *fun);
    std::string dataPath;

/**
 * @brief Method to load pixelization scheme.
 *
 */
void LoadPixelization(RecoHelper *recoHelper)
{
    if (!recoHelper->thePixelVec) recoHelper->thePixelVec = std::make_shared<std::vector<majutil::Pixel>>();
    recoHelper->thePixelVec->clear();

    // Make pixels for each position
    std::ifstream f(recoHelper->thePixelPath.c_str());
    if (!f.is_open())
    {
        std::cerr << "PixelTable::Initialize() Error! Cannot open pixelization file!\n";
        exit(1);
    }

    std::cout << "Reading pixelization file...\n";

    // Table must be:
    //
    //   pixelID x y
    //

    // First read top line
    std::string string1, string2, string3;
    std::getline(f, string1, ' ');
    std::getline(f, string2, ' ');
    std::getline(f, string3);
    if(string1 != "pixelID" ||
    string2 != "x"       ||
    string3 != "y")
    {
        std::cout << "PixelTable::Initialize() Error! ReferenceTable must have "
        << "\'pixelID mppcID probability\' as header.\n";
        exit(1);
    }

    // For computing the size
    unsigned thePixelCount(0);
    float aPixelPos(0);
    float min = std::numeric_limits<float>::max();
    while (std::getline(f, string1, ' '))
    {
        std::getline(f, string2, ' ');
        std::getline(f, string3);

        unsigned pixelID = std::stoi(string1);
        float    x       = std::stof(string2);
        float    y       = std::stof(string3);
        thePixelCount++;
        if (thePixelCount == 1) aPixelPos = x;
        else min = std::abs(aPixelPos-x) < min && std::abs(aPixelPos-x) > 0 ? std::abs(aPixelPos-x) : min;

        // Get r and theta just in case we need it
        float r     = std::sqrt(x*x + y*y);
        float thetaDeg(0);
        if (r > 0.01) thetaDeg = std::asin(std::abs(y/r))*180/M_PI;
        // Handle theta convention
        if (x <  0 && y >= 0) thetaDeg = 180 - thetaDeg;
        if (x <  0 && y <  0) thetaDeg = 180 + thetaDeg;
        if (x >= 0 && y <  0) thetaDeg = 360 - thetaDeg;

        recoHelper->thePixelVec->emplace_back(pixelID, x, y, r, thetaDeg);
        recoHelper->thePixelVec->back().SetSize(recoHelper->thePixelSpacing);
    }

    f.close();

    // Sort
    std::sort( recoHelper->thePixelVec->begin(), recoHelper->thePixelVec->end(), [](const majutil::Pixel& left, const majutil::Pixel& right) { return left.ID() < right.ID(); } );
    std::cout << "Initialized " << recoHelper->thePixelVec->size() << " " << min << "x" << min << "cm2 pixels...\n";

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
    std::ifstream f(recoHelper->theOpRefPath.c_str());
    if (!f.is_open())
    {
        std::cout << "PixelTable::LoadReferenceTable() Error! Cannot open reference table file! " << recoHelper->theOpRefPath.c_str() << "\n";
        exit(1);
    }
    std::cout << "Reading reference table file...\n";

    // Table must be:
    //
    //    pixelID mppcID probability
    //
    std::string string1, string2, string3;
    std::getline(f, string1, ' ');
    std::getline(f, string2, ' ');
    std::getline(f, string3);

    if (string1 != "pixelID" ||
    string2 != "mppcID"  ||
    string3 != "probability")
    {
        std::cout << "PixelTable::LoadReferenceTable() Error! ReferenceTable must have "
        << "\'pixelID mppcID probability\' as header.\n";
        exit(1);
    }

    while (std::getline(f, string1, ' '))
    {
        std::getline(f, string2, ' ');
        std::getline(f, string3);

        unsigned pixelID = std::stoi(string1);
        unsigned mppcID  = std::stof(string2);
        float    prob    = std::stof(string3);

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
        std::vector<Double_t> EstValues;
        EstValues=theReconstructor.EstimatedValuesD();




        // Write the reconstructed image
        std::string FileName;
        FileName=recoHelper->ThePath+"Reco_"+recoHelper->FileName+".root";
        std::cout <<FileName<<std::endl;
        TFile f(FileName.c_str(), "UPDATE");
        fun->f=&f;
        std::string Chi2Name=recoHelper->ImageName+"_Chi2";
        std::string MLIName=recoHelper->ImageName+"_MLI";
        std::string expDataName=recoHelper->ImageName+"_expData";
        fun->mli=theReconstructor.MLImage();
        //theReconstructor.MLImage()->Write(MLIName.c_str());
        fun->chi2=theReconstructor.Chi2Image();
        //theReconstructor.Chi2Image()->Write(Chi2Name.c_str());



        //gROOT->cd("Rint:/");

        // Write the expected data
        auto expdata = theReconstructor.ExpectedCounts();
        TH1I h(expDataName.c_str(), expDataName.c_str(), expdata.size(), 0.5, expdata.size()+0.5);
        for (const auto& d : expdata) h.SetBinContent(d.first, d.second);
        fun->exp=&h;
        //h.Write();
        cout << "Finished!" << endl;

        //TrueVsReco Graph
        fun->Title=recoHelper->ImageName;
        fun->ExpX=EstValues.at(0);
        fun->ExpY=EstValues.at(1);
        fun->DrawCircle();

        f.Close();
}

void SetVariables(RecoHelper *reco,
                  Cfunctions *fun,
                  std::string pixelizationPath,
                  std::string opRefTablePath,
                  std::string ThePath,
                  std::string FileName,
                  std::string TrueFile,
                  std::string Combined)
                  {
                        std::vector<std::string> splitLine;
                        boost::split(splitLine,Combined,boost::is_any_of("_"));

                        reco->theMethod          = "chi2";
                        reco->theDiskRadius      = std::stof(splitLine[2]);//50.5619;
                        reco->theGamma           = 0.5;
                        reco->theDoPenalized     = true;
                        reco->thePenalizedIter   = 100;
                        reco->theUnpenalizedIter = 100;
                        reco->thePixelSpacing    = std::stof(splitLine[1]); // in cm
                        reco->NSIPMs             = std::stoi(splitLine[0]);

                        reco->theOpRefPath = opRefTablePath;
                        reco->thePixelPath = pixelizationPath;
                        reco->FileName     = FileName;
                        reco->ThePath      = ThePath;



                        fun->R                   = std::stof(splitLine[2]);
                        if(reco->NSIPMs>32) fun->dif                 = 35;
                        else fun->dif                 = 10;
                        fun->FileName            = FileName;
                        fun->TrueFilePath        = TrueFile;
                        LoadPixelization(reco); //This comes before the LoadOpRefTable(reco)
                        LoadOpRefTable(reco);


                  }

/*void ReadDataTree(std::string DataPath,struct TTreeHelp *htr,struct Data *d1)
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
void ReadMultiData(std::string DataPath,struct TTreeHelp *htr,struct Data *d1, struct RecoHelper *recoHelper, struct Cfunctions *fun)
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
        d1->ActCounts.clear();
        std::cout << "Event ID " << htr->eventID << std::endl;
        for (unsigned i = 0; i < recoHelper->NSIPMs; i++) {
            d1->ActCounts.emplace(i, htr->ActualCounts.at(i));
            d1->FilteredMap.emplace(i, htr->FilteredCounts.at(i));
            d1->AnaCounts.emplace(i, htr->AnaCounts.at(i));
        }


        if (d1->ActCounts.size() > 0) {
            recoHelper->ImageName = "Act_" + std::to_string(htr->eventID) + "_";
            recoHelper->theData = d1->ActCounts;
            Reconstruct(recoHelper, fun);
            d1->ActCounts.clear();
            htr->ActualCounts.clear();

        }
        if (d1->AnaCounts.size() > 0) {
            recoHelper->ImageName = "Ana" + std::to_string(htr->eventID) + "_";;
            recoHelper->theData = d1->AnaCounts;
            Reconstruct(recoHelper,fun);
            d1->AnaCounts.clear();
            htr->AnaCounts.clear();
        }
        if (d1->FilteredMap.size() > 0) {
            recoHelper->ImageName = "FillTered" + std::to_string(htr->eventID) + "_";
            recoHelper->theData = d1->FilteredMap;
            Reconstruct(recoHelper, fun);
            d1->FilteredMap.clear();
            htr->FilteredCounts.clear();

        }


    }

}

/********************************/
void doRecoToFileS(std::string pixelizationPath, std::string opRefTablePath,std::string ThePath,std::string FileName,std::string TrueFile,std::string Combined)
{
    RecoHelper recoHelper;
    Cfunctions fun;
    struct TTreeHelp htr;
    Data d1;
    dataPath                = ThePath+"ana_"+FileName+".root";
    SetVariables(&recoHelper,&fun,pixelizationPath,opRefTablePath,ThePath,FileName,TrueFile,Combined);// Get other Variables
    ReadMultiData( dataPath,&htr,&d1,&recoHelper,&fun);

}
