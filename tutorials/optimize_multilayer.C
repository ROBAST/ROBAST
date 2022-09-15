// Example to optimize a multilayer design using MINUIT
// See Fig. 12 on page 300 of this book.
// https://books.google.com/books?id=ldT5DwAAQBAJ&pg=PA300&lpg=PA300

#include "AFilmetrixDotCom.h"
#include "AMultilayer.h"
#include "AOpticsManager.h"
#include "ARefractiveIndex.h"

#include "TCanvas.h"
#include "TLegend.h"
#include "Math/Functor.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"

static const Double_t nm = AOpticsManager::nm();
static const Double_t mm = AOpticsManager::mm();
static const Double_t deg = AOpticsManager::deg();

// std::make_shared is a C++11 feature that provide a kind of smart pointers
// Rrefractive index objects must be pointers in ROBAST internally but
// deconstruction of these objects should be done automatically
auto air = std::make_shared<ARefractiveIndex>(1., 0.);
auto glass = std::make_shared<ARefractiveIndex>(1.52, 0.);
auto high = std::make_shared<ARefractiveIndex>(2.36, 0.);
auto low = std::make_shared<ARefractiveIndex>(1.46, 0.);

static std::shared_ptr<ROOT::Math::Minimizer> minimizer;

const std::size_t kNlayers = 41;

AMultilayer multi(air, air);

double min_func_all(const double* par) {

  for (std::size_t i = 0; i < kNlayers; ++i) {
    multi.ChangeThickness(i + 1, par[i]);
  }

  double chi2 = 0;

  for (int wl = 900; wl <= 1700; ++wl) {
    if (1180 < wl and wl < 1220) {
      continue;
    }

    double reflectance, transmittance;
    multi.IncoherentTMMP(0 * deg, wl * nm, reflectance, transmittance);

    if (wl <= 1180) {
      double merit =  0.;
      chi2 += TMath::Power(transmittance - merit, 2);
    } else {
      double merit =  1;
      if (transmittance < 0.96) {
      chi2 += TMath::Power(transmittance - 0.96, 2);
      }
    }
  }

  return chi2;
}

void optimize_multilayer() {
  // the target wavelength
  Double_t lambda = 1000. * nm;

  // Initial Design
  // Air | H/2 | L | H | L | ... | H/2 | Glass
  Double_t n_low = low->GetRefractiveIndex(lambda);
  Double_t n_high = high->GetRefractiveIndex(lambda);
  Double_t d_low = lambda / n_low / 4.;
  Double_t d_high = lambda / n_high / 4.;

  bool coherent = kFALSE;
  multi.AddLayer(glass, 2 * mm, coherent);

  multi.AddLayer(high, d_high / 2.);
  for (std::size_t i = 0; i < (kNlayers - 3)/2; ++i) {
    multi.AddLayer(low, d_low);
    multi.AddLayer(high, d_high);
  }
  multi.AddLayer(low, d_low);
  multi.AddLayer(high, d_high / 2.);

  auto graT0 = new TGraph;
  auto graD0 = multi.MakeIndexGraph(lambda);

  for (Int_t i = 900; i <= 1650; ++i) {
    Double_t lambda = i * nm;
    Double_t angle = 0. * deg;
    Double_t reflectance, transmittance;
    // Calculate reflectance and transmittance for S polarization
    // but in this normal-incident case, either P (TMMP) or S (TMMS) is OK
    // You can also use TMMMixed for mixed polarization
    multi.IncoherentTMMS(angle, lambda, reflectance, transmittance);
    graT0->SetPoint(graT0->GetN(), i, transmittance);
  }
  
  auto can = new TCanvas("can", "can", 800, 600);
  auto frame = can->DrawFrame(900, 0, 1650, 1);
  frame->SetTitle(";Wavelength (nm);Transmittance");
  graT0->Draw("l");
  graT0->SetLineStyle(7);

  minimizer.reset(ROOT::Math::Factory::CreateMinimizer("Minuit2"));
  minimizer->SetMaxFunctionCalls(10000);
  minimizer->SetTolerance(1e-0);
  minimizer->SetPrintLevel(1);

  auto f = new ROOT::Math::Functor(&min_func_all, kNlayers);
  minimizer->SetFunction(*f);

  for (std::size_t i = 0; i < kNlayers; ++i) {
    minimizer->SetVariable(i, Form("layer%lu", i), multi.GetThickness(i + 1), 1 * nm);
    minimizer->SetVariableLimits(i, 10 * nm, multi.GetThickness(i + 1) * 2);
  }
  
  minimizer->Minimize();
  for (std::size_t i = 0; i < kNlayers; ++i) {
    multi.ChangeThickness(i + 1, minimizer->X()[i]);
  }
  multi.PrintLayers(lambda);

  auto graT1 = new TGraph;
  auto graD1 = multi.MakeIndexGraph(lambda);

  for (Int_t i = 900; i <= 1650; ++i) {
    Double_t lambda = i * nm;
    Double_t angle = 0. * deg;
    Double_t reflectance, transmittance;
    multi.IncoherentTMMS(angle, lambda, reflectance, transmittance);
    graT1->SetPoint(graT1->GetN(), i, transmittance);
  }
  graT1->Draw("l same");

  auto leg = new TLegend(0.5, 0.3, 0.85, 0.5);
  leg->AddEntry(graT0, "Initial Design", "l");
  leg->AddEntry(graT1, "Optimized Design", "l");
  leg->Draw();

  auto can2 = new TCanvas("can2", "can2", 800, 600);
  can2->Divide(1, 2, 1e-10, 1e-10);
  can2->cd(1);
  frame = gPad->DrawFrame(0., 1., 6000, 2.5);
  frame->SetTitle(";Thickness (nm);Refractive Index");
  graD0->Draw("l same");

  can2->cd(2);
  frame = gPad->DrawFrame(0., 1, 6000, 2.5);
  frame->SetTitle(";Thickness (nm);Refractive Index");
  graD1->Draw("l same");
}
