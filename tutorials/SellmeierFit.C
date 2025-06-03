#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TROOT.h"

#include "AOpticsManager.h"
#include "ASellmeierFormula.h"

const double nm = AOpticsManager::nm();

void SellmeierFit() {
  // use N-BK7 parameters for the initial values
  ASellmeierFormula* sellmaier =
      new ASellmeierFormula(1.03961212E+00, 2.31792344E-01, 1.01046945E+00,
                            6.00069867E-03, 2.00179144E-02, 1.03560653E+02);
  TF1* f1 = sellmaier->MakeGraph("uvc200b", 300 * nm, 800 * nm);
  f1->SetLineColor(2);

  TFile f("uvc200b.root");
  gROOT->cd();
  TCanvas* canvas = (TCanvas*)f.Get("canvas")->Clone();
  canvas->Draw();

  // Measured refractive indices of UVC-200B
  TGraph* graph = (TGraph*)(canvas->GetListOfPrimitives()->At(2)->Clone());

  for (int i = 0; i < graph->GetN(); i++) {
    // Convert (nm) to the ROBAST unit (cm)
    double x = graph->GetX()[i];
    double y = graph->GetY()[i];
    graph->SetPoint(i, x * nm, y);
  }

  TCanvas* can2 = new TCanvas("can2", "can2");
  graph->Draw("ap");
  graph->GetXaxis()->SetTitle("Wavelength (cm)");  // ROBAST unit
  graph->GetYaxis()->SetTitle("Refractive Index");

  // You will get best-fit parameters: B1, B2, B3, C1, C2, and C3
  sellmaier->FitData(graph, "uvc200b");

  // You can get a refractive index at any wavelength you like
  std::cout << "Refractive Index at 400 nm: "
            << sellmaier->GetRefractiveIndex(400 * nm) << std::endl;
}
