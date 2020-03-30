

// We need the resulting plot from reco
  TFile f("recoanatree.root", "READ");
  if (f.IsOpen()) {
    gStyle->SetPalette(kDarkBodyRadiator);
  }    TH2F *recoHist = nullptr;
    f.GetObject("histFinal", recoHist);
    if (recoHist) {
      recoHist->SetTitle("Reconstructed Image");
      recoHist->GetXaxis()->SetTitle("X [cm]");
      recoHist->GetYaxis()->SetTitle("Y [cm]");
      recoHist->Draw("colz");
      c1->cd();
      c1->Update();
