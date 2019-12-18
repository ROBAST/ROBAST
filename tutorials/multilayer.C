// Examples to simulate multilayer coating
// UV-cut and IR-cut filter examples shown this page is tlanslated into ROBAST
// See https://www.tokaioptical.com/jp/technology01/

#include "AFilmetrixDotCom.h"
#include "AMultilayer.h"
#include "AOpticsManager.h"
#include "ARefractiveIndex.h"

#include "TCanvas.h"

static const Double_t nm = AOpticsManager::nm();
static const Double_t deg = AOpticsManager::deg();

void multilayer() {
  // std::make_shared is a C++11 feature that provide a kind of smart pointers
  // Rrefractive index objects must be pointers in ROBAST internally but
  // deconstruction of these objects should be done automatically
  auto air = std::make_shared<ARefractiveIndex>(1., 0.);
  auto glass = std::make_shared<ARefractiveIndex>(1.52, 0.);
  // Read text files downloaded from filmetrix.com
  // See https://www.filmetrics.com/refractive-index-database
  auto SiO2 = std::make_shared<AFilmetrixDotCom>("SiO2.txt");
  auto TiO2 = std::make_shared<AFilmetrixDotCom>("TiO2.txt");
  // You can also use data from https://refractiveindex.info

  AMultilayer uv_cut(air, glass);

  // The filter design is optimized for 350 nm photons
  Double_t lambda = 350. * nm;

  // Air
  // --------------------------
  // n = 2.40 | 0.5 H (0.5 x d_high)
  // n = 1.46 | 1.0 L (1.0 x d_low)
  // n = 2.40 | 1.0 H
  // ...
  // n = 2.40 | 1.0 H
  // n = 1.46 | 1.0 L
  // n = 2.40 | 0.5 H
  // --------------------------
  // Glass
  Double_t n_low = SiO2->GetRefractiveIndex(lambda);
  Double_t n_high = TiO2->GetRefractiveIndex(lambda);
  Double_t d_low = lambda / n_low / 4.;
  Double_t d_high = lambda / n_high / 4.;

  uv_cut.InsertLayer(TiO2, d_high / 2.);
  for (Int_t i = 0; i < 8; ++i) {
    uv_cut.InsertLayer(SiO2, d_low);
    uv_cut.InsertLayer(TiO2, d_high);
  }
  uv_cut.InsertLayer(SiO2, d_low);
  uv_cut.InsertLayer(TiO2, d_high / 2.);

  // Note InsertLayer insterts a new layer at the boundary between the bottom
  // medium and the last layer

  // Top Medium                  Top Medium
  // ----------                  ----------
  // Layer A       InsertLayer   Layer A
  // ----------    ==========>   ----------
  // Layer B                     Layer B
  // ----------                  ----------
  // Bottom Medium               Layer C
  //                             ----------
  //                             Bottom Medium

  AMultilayer ir_cut(air, glass);

  // The filter design is optimized for 800 nm photons
  lambda = 800. * nm;

  // Air
  // --------------------------
  // n = 1.46 | 0.5 L (0.5 x d_low)
  // n = 2.40 | 1.0 H (1.0 x d_high)
  // n = 1.46 | 1.0 L
  // ...
  // n = 1.46 | 1.0 L
  // n = 2.40 | 1.0 H
  // n = 1.46 | 0.5 L
  // --------------------------
  // Glass
  n_low = SiO2->GetRefractiveIndex(lambda);
  n_high = TiO2->GetRefractiveIndex(lambda);
  d_low = lambda / n_low / 4.;
  d_high = lambda / n_high / 4.;

  ir_cut.InsertLayer(SiO2, d_low / 2.);
  for (Int_t i = 0; i < 8; ++i) {
    ir_cut.InsertLayer(TiO2, d_high);
    ir_cut.InsertLayer(SiO2, d_low);
  }
  ir_cut.InsertLayer(TiO2, d_high);
  ir_cut.InsertLayer(SiO2, d_low / 2.);

  auto graIR = new TGraph;
  auto graUV = new TGraph;

  for (Int_t i = 300; i <= 800; ++i) {
    Double_t lambda = i * nm;
    Double_t angle = 0. * deg;
    Double_t reflectance, transmittance;
    // Calculate reflectance and transmittance for S polarization
    // but in this normal-incident case, either P (TMMP) or S (TMMS) is OK
    // You can also use TMMMixed for mixed polarization
    uv_cut.CoherentTMMS(angle, lambda, reflectance, transmittance);
    graUV->SetPoint(graUV->GetN(), i, transmittance * 100);

    ir_cut.CoherentTMMS(angle, lambda, reflectance, transmittance);
    graIR->SetPoint(graIR->GetN(), i, transmittance * 100);
  }
  auto can = new TCanvas("can", "can", 800, 600);
  auto frame = can->DrawFrame(350, 0, 800, 100);
  frame->SetTitle(";Wavelength (nm);Transmittance (%)");
  graUV->Draw("l");
  graUV->SetLineColor(2);
  graIR->Draw("l");
  graIR->SetLineColor(4);
}
