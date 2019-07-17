#include "AOpticsManager.h"
#include "AMultilayer.h"
#include "AConstantRefractiveIndex.h"
#include "ARefractiveIndex.h"
#include "AFilmetrixDotCom.h"

#include "TCanvas.h"

static const Double_t nm = AOpticsManager::nm();

void multilayer() {
  std::shared_ptr<ARefractiveIndex> air(new AConstantRefractiveIndex(1.));
  std::shared_ptr<ARefractiveIndex> glass(new AConstantRefractiveIndex(1.52));
  std::shared_ptr<ARefractiveIndex> SiO2(new AFilmetrixDotCom("SiO2.txt"));
  std::shared_ptr<ARefractiveIndex> TiO2(new AFilmetrixDotCom("TiO2.txt"));
  //std::shared_ptr<ARefractiveIndex> SiO2(new AConstantRefractiveIndex(1.46));
  //std::shared_ptr<ARefractiveIndex> TiO2(new AConstantRefractiveIndex(2.40));

  AMultilayer uv_cut(air, glass);

  Double_t lambda = 350. * nm;
  Double_t n_low = SiO2->GetRefractiveIndex(lambda);
  Double_t n_high = TiO2->GetRefractiveIndex(lambda);
  Double_t d_low = lambda / n_low / 4.;
  Double_t d_high = lambda / n_high / 4.;

  for(Int_t i = 0; i < 17; ++i) {
    if (i == 0 || i == 16) {
      uv_cut.InsertLayer(TiO2, d_high / 2.);
    } else {
      if (i%2 == 0) {
	uv_cut.InsertLayer(TiO2, d_high);
      } else {
	uv_cut.InsertLayer(SiO2, d_low);
      }
    }
  }

  AMultilayer ir_cut(air, glass);

  lambda = 800. * nm;
  n_low = SiO2->GetRefractiveIndex(lambda);
  n_high = TiO2->GetRefractiveIndex(lambda);
  d_low = lambda / n_low / 4.;
  d_high = lambda / n_high / 4.;

  for(Int_t i = 0; i < 17; ++i) {
    if (i == 0 || i == 16) {
      ir_cut.InsertLayer(SiO2, d_low / 2.);
    } else {
      if (i%2 == 0) {
	ir_cut.InsertLayer(SiO2, d_low);
      } else {
	ir_cut.InsertLayer(TiO2, d_high);
      }
    }
  }

  TGraph* graIR = new TGraph;
  TGraph* graUV = new TGraph;

  for(Int_t i = 300; i <= 800; ++i){
    Double_t lambda = i * nm;
    Double_t angle = 0. * TMath::DegToRad();
    Double_t reflectance, transmittance;
    uv_cut.CoherentTMM(AMultilayer::kS, angle, lambda, reflectance, transmittance);
    graUV->SetPoint(graUV->GetN(), i, transmittance * 100);

    ir_cut.CoherentTMM(AMultilayer::kS, angle, lambda, reflectance, transmittance);
    graIR->SetPoint(graIR->GetN(), i, transmittance * 100);
  }
  TCanvas* can = new TCanvas("can", "can", 800, 600);
  can->DrawFrame(350, 0, 800, 100)->SetTitle(";Wavelength (nm);Transmittance (%)");
  graUV->Draw("l");
  graUV->SetLineColor(2);
  graIR->Draw("l");
  graIR->SetLineColor(4);
}