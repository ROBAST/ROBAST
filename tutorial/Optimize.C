// This tutorial shows how to optimize a lens shape by using the ROOT Minuite2
// optimization engine.

#include "TCanvas.h"
#include "TH2D.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TGeoPhysicalNode.h"
#include "Minuit2/Minuit2Minimizer.h"
#include "Math/Functor.h"

#include "AOpticsManager.h"
#include "AGeoAsphericDisk.h"
#include "ARayArray.h"
#include "ARayShooter.h"
#include "AGlassCatalog.h"
#include "AMirror.h"
#include "ALens.h"
#include "AFocalSurface.h"

static const Double_t mm = AOpticsManager::mm();
static const Double_t um = AOpticsManager::um();
static const Double_t nm = AOpticsManager::nm();

static AOpticsManager* gOpticsManager = 0;
static const int kNtheta = 3;
static ARayArray* gArray[kNtheta] = {0, 0, 0};
static double kTheta[kNtheta] = {0., 2.5, 5.0};
static const int kNlambda = 3;
static const double kRadius = 5.0*mm;

double GetSpotSize(ARayArray* array)
{
  // Returns standard deviation of a spot
  TObjArray* focused = array->GetFocused();
  TH2D h("", "", 1, 0, 0, 1, 0, 0);
  
  for(int i = 0; i < focused->GetLast(); i++){
    ARay* ray = (ARay*)(*focused)[i];
    double p[4];    
    ray->GetLastPoint(p);
    h.Fill(p[0], p[1]);
  } // i

  double sx = h.GetStdDev(1);
  double sy = h.GetStdDev(2);

  return TMath::Sqrt(sx*sx + sy*sy);
}

int Nphotons(ARayArray* array)
{
  int total = 0;
  total += array->GetFocused()->GetLast() + 1;
  total += array->GetStopped()->GetLast() + 1;
  total += array->GetSuspended()->GetLast() + 1;
  total += array->GetAbsorbed()->GetLast() + 1;

  return total;
}

double Func(const double* par)
{
  static bool initialized = kFALSE;
  static TGeoRotation rot[kNtheta];
  static TGeoTranslation tr("tr", 0, 0, -3*mm);

  if(!initialized){
    for(int i = 0; i < kNtheta; i++){
      rot[i] = TGeoRotation();
      rot[i].SetAngles(0, kTheta[i], 0.);
    } // i
    initialized = kTRUE;
  } // if
  
  if(gOpticsManager){
    delete gOpticsManager;
  } // if
  gOpticsManager = new AOpticsManager("manager", "manager");
  gOpticsManager->DisableFresnelReflection(kTRUE);

  // Make the world
  TGeoBBox* box = new TGeoBBox("box", 100*mm, 100*mm, 100*mm);
  AOpticalComponent* top = new AOpticalComponent("top", box);
  gOpticsManager->SetTopVolume(top);

  AGeoAsphericDisk* disk = new AGeoAsphericDisk("disk", 0*mm, 0., par[0], par[1], kRadius, 0.*mm);
  disk->SetConicConstants(0, par[2]);
  double f1 = disk->CalcF1(kRadius);
  double f2 = disk->CalcF2(kRadius);
  if(f2 - f1 < 0){ // negative edge thickness
    return 1e100;
  } // if

  ALens* lens = new ALens("lens", disk);
  lens->SetRefractiveIndex(AGlassCatalog::GetRefractiveIndex("N-BK7"));
  gOpticsManager->GetTopVolume()->AddNode(lens, 1);
  
  double origin[3] = {0, 0, 50*mm + 1*um};
  TGeoBBox* box2 = new TGeoBBox("box2", 10*mm, 10*mm, 1*um, origin);
  AFocalSurface* screen = new AFocalSurface("screen", box2);
  top->AddNode(screen, 1);

  AGeoAsphericDisk* disk2 = new AGeoAsphericDisk("disk2", 0*mm, 0., par[0], 0., kRadius*1.2, kRadius);
  AObscuration* obs = new AObscuration("obs", disk2);
  gOpticsManager->GetTopVolume()->AddNode(obs, 1);
  
  gOpticsManager->CloseGeometry();

  double total = 0.;

  for(int i = 0; i < kNtheta; i++){
    if(gArray[i]){
      delete gArray[i];
      gArray[i] = 0;
    } // if

    const double kLambda = 587.6*nm;
    gArray[i] = ARayShooter::Circle(kLambda, kRadius*1.1, 20, 10, &rot[i], &tr);
    gOpticsManager->TraceNonSequential(gArray[i]);
    
    total += TMath::Power(GetSpotSize(gArray[i]), 2);
  } // i

  if(total == 0){
    return 1e100;
  } // if

  return total;
}

void DrawRays()
{
 for(int i = 0; i < kNtheta; i++){
   TObjArray* focused = gArray[i]->GetFocused();
   
    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      TPolyLine3D* pol = ray->MakePolyLine3D();
      pol->SetLineColor(i + 2);
      pol->Draw();
    } // j
  } // i
}

void DrawPSF()
{
  TCanvas* can = new TCanvas("can", "can", 1200, 400);
  can->Divide(3, 1, 1e-10, 1e-10);
  TGraph* graph[kNtheta];
  for(int i = 0; i < kNtheta; i++){
    TH2D tmp("", "", 1, 0, 0, 1, 0, 0);
    
    TObjArray* focused = gArray[i]->GetFocused();
    
    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      Double_t p[4];    
      ray->GetLastPoint(p);
      tmp.Fill(p[0], p[1]);
    } // j
    
    double meany = tmp.GetMean(2);
    
    graph[i] = new TGraph();
    
    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      Double_t p[4];    
      ray->GetLastPoint(p);
      graph[i]->SetPoint(j, p[0]/um, (p[1] - meany)/um);
    } // j
    
    can->cd(i + 1);
    gPad->DrawFrame(-300, -300, 300, 300, Form("%.1f (deg);X (#it{#mu}m);Y (#it{#mu}m)", kTheta[i]));
    graph[i]->SetMarkerColor(2);
    graph[i]->SetMarkerStyle(5);
    graph[i]->SetMarkerSize(0.5);
    graph[i]->Draw("p same");
  } // i
}

void Optimize()
{
  ROOT::Minuit2::Minuit2Minimizer min2;
  min2.SetMaxFunctionCalls(1000000);
  min2.SetMaxIterations(100000);
  min2.SetTolerance(0.001);
 
  ROOT::Math::Functor f(&Func, 3); 
  double step[3] = {0.01, 0.01, 0.01};
  double par[3] = {3*mm, -1/(50*mm), 0.};
  double pmin[3] = {1*mm, -1/(20*mm), -3};
  double pmax[3] = {5*mm, -1/(100*mm), 1};
 
  min2.SetFunction(f);
  min2.SetLimitedVariable(0, "thickness", par[0], step[0], pmin[0], pmax[0]);
  min2.SetLimitedVariable(1, "curvature", par[1], step[1], pmin[1], pmax[1]);
  min2.SetLimitedVariable(2, "conic constant", par[2], step[2], pmin[2], pmax[2]);
  min2.Minimize();
  
  const double* x = min2.X();
  std::cout << "Thickness = " << x[0]/mm << " (mm)\n";
  std::cout << "Raduis    = " << (1/x[1])/mm << " (mm)\n";
  std::cout << "Conic     = " << x[2] << "\n";

  DrawPSF();
  gPad = 0;
  gOpticsManager->GetTopVolume()->Draw("ogl");
  DrawRays();
}
