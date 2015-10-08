#include "AOpticsManager.h"
#include "AMirror.h"
#include "ALens.h"
#include "AOpticalComponent.h"
#include "AGeoWinstonConePoly.h"
#include "AGeoBezierPgon.h"
#include "AGeoBezierPcon.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TGeoTube.h"
#include "TGeoCompositeShape.h"
#include "TGeoSphere.h"
#include "TMath.h"
#include "TFile.h"
#include "TH2D.h"
#include "TRandom.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TROOT.h"
#include "TPad.h"
#include "TVector3.h"
#include "TCanvas.h"
#include <iostream>
#include <string.h>

const Double_t nm = AOpticsManager::nm();
const Double_t um = AOpticsManager::um();
const Double_t mm = AOpticsManager::mm();
const Double_t cm = AOpticsManager::cm();
const Double_t m  = AOpticsManager::m();

const Double_t kSmaller = 0.99999;
const Double_t kLarger  = 1.00001;

const Int_t kNedges = 6;

class Cone
{
protected:
  Double_t fRin;
  Double_t fRout;
  Double_t fDz;

  AOpticsManager* fManager;
  AMirror*        fCone;
  AFocalSurface*  fPMT;

public:
  Cone(Double_t rin, Double_t rout);
  virtual ~Cone();

  virtual void            BuildGeometry() = 0;
  virtual void            BuildPMT();
  virtual AOpticsManager* GetManager() {return fManager;}
  virtual Double_t        GetArea() = 0;
  virtual AMirror*        GetCone() {return fCone;}
  virtual AFocalSurface*  GetPMT() {return fPMT;}
  virtual ARayArray*      Trace(Double_t theta, Double_t phi, UInt_t n);
  virtual ARayArray*      Trace(Double_t theta, Double_t phi1, Double_t phi2, UInt_t n);
  virtual Double_t        WinstonDz();
  virtual Double_t        WinstonTheta();
};

//______________________________________________________________________________
Cone::Cone(Double_t rin, Double_t rout)
{
  fRin = rin;
  fRout = rout;

  //TThread::Initialize();
  fManager = new AOpticsManager("manager", "manager");

  TGeoBBox* topBox = new TGeoBBox("topBox", 10.*cm, 10.*cm, 10.*cm);
  AOpticalComponent* topVolume = new AOpticalComponent("topVolume", topBox);
  fManager->SetTopVolume(topVolume);
}

//______________________________________________________________________________
Cone::~Cone()
{
  delete fManager;
  fManager = 0;
}

//______________________________________________________________________________
void Cone::BuildPMT()
{
  // Build a PMT with a flat input window
  // We assume that the PMT does not have an input glass window
  TGeoTube* tube = new TGeoTube("tube", 0*mm, fRout, 1*mm);
  fPMT = new AFocalSurface("pmt", tube);

  TGeoTranslation* translation = new TGeoTranslation(0, 0, -fDz - 1*mm);

  fManager->GetTopVolume()->AddNodeOverlap(fPMT, 1, translation);
}

//______________________________________________________________________________
ARayArray* Cone::Trace(Double_t theta, Double_t phi, UInt_t n)
{
  Double_t z = ((TGeoBBox*)fCone->GetShape())->GetDZ();
  Double_t rmax = fRin/TMath::Cos(TMath::Pi()/kNedges);

  Double_t dx = - TMath::Sin(theta)*TMath::Cos(phi);
  Double_t dy = - TMath::Sin(theta)*TMath::Sin(phi);
  Double_t dz = - TMath::Cos(theta);

  ARayArray* array = new ARayArray();

  for(UInt_t i = 0; i < n; i++){
    Double_t x = rmax;
    Double_t y = rmax;
    while(x*x + y*y > rmax*rmax){
      x = gRandom->Uniform(-1, 1)*rmax;
      y = gRandom->Uniform(-1, 1)*rmax;
    } // if
    ARay* ray = new ARay(0, 470*nm, x, y, z, 0, dx, dy, dz);
    array->Add(ray);
  } // i

  fManager->TraceNonSequential(*array);

  return array;
}

//______________________________________________________________________________
ARayArray* Cone::Trace(Double_t theta, Double_t phi1, Double_t phi2, UInt_t n)
{
  Double_t z = ((TGeoBBox*)fCone->GetShape())->GetDZ();
  Double_t rmax = fRin/TMath::Cos(TMath::Pi()/kNedges);

  ARayArray* array = new ARayArray();

  for(UInt_t i = 0; i < n; i++){
    Double_t phi = gRandom->Uniform(phi1, phi2);
    Double_t dx = - TMath::Sin(theta)*TMath::Cos(phi);
    Double_t dy = - TMath::Sin(theta)*TMath::Sin(phi);
    Double_t dz = - TMath::Cos(theta);

    Double_t x = rmax;
    Double_t y = rmax;
    while(x*x + y*y > rmax*rmax){
      x = gRandom->Uniform(-1, 1)*rmax;
      y = gRandom->Uniform(-1, 1)*rmax;
    } // if
    ARay* ray = new ARay(0, 470*nm, x, y, z, 0, dx, dy, dz);
    array->Add(ray);
  } // i

  fManager->TraceNonSequential(*array);

  return array;
}

//______________________________________________________________________________
Double_t Cone::WinstonDz()
{
  Double_t theta = TMath::ASin(fRout/fRin);
  Double_t dz = (fRout + fRin)/TMath::Tan(theta)/2.;

  return dz;
}

//______________________________________________________________________________
Double_t Cone::WinstonTheta()
{
  Double_t theta = TMath::ASin(fRout/fRin);

  return theta;
}

//______________________________________________________________________________
class WinstonCone : public Cone
{
private:

public:
  WinstonCone(Double_t rin, Double_t rout);
  void     BuildGeometry();
  Double_t GetArea();
};

//______________________________________________________________________________
WinstonCone::WinstonCone(Double_t rin, Double_t rout)
  : Cone(rin, rout)
{
  BuildGeometry();
  BuildPMT();
  fManager->CloseGeometry();
  //fManager->SetMaxThreads(4);
  //fManager->SetMultiThread(kTRUE);
}

//______________________________________________________________________________
void WinstonCone::BuildGeometry()
{
  AGeoWinstonConePoly* winston
    = new AGeoWinstonConePoly("winston", fRin, fRout, kNedges);
  fDz = winston->GetDZ();
  AGeoBezierPgon* outer
    = new AGeoBezierPgon("outer", 0, 360, kNedges, 2, fRin*kLarger,
                         fRin*kLarger, fDz*kSmaller);
  outer->SetSections();

  TGeoRotation* rotation = new TGeoRotation("rotation", 180./kNedges, 0, 0);
  rotation->RegisterYourself();
        
  TGeoCompositeShape* composite
    = new TGeoCompositeShape("composite", "outer - winston:rotation");
  fCone = new AMirror("compositeVolume", composite);
  fManager->GetTopVolume()->AddNodeOverlap(fCone, 1);
}

//______________________________________________________________________________
Double_t WinstonCone::GetArea()
{
  Double_t theta = TMath::Pi()/kNedges;
  Double_t area = fRin*fRin/TMath::Tan(theta)*kNedges;

  return area;
}

//______________________________________________________________________________
class BezierCone : public Cone {
protected:
  Double_t fX1;
  Double_t fY1;
  Double_t fX2;
  Double_t fY2;
  Double_t fHeight;
  UInt_t   fNz;

public:
  BezierCone(Double_t rin, Double_t rout, UInt_t nz, Double_t x1, Double_t y1, Double_t x2 = -100, Double_t y2 = -100, Double_t height = -100);
  void     BuildGeometry();
  Double_t GetArea();
};

//______________________________________________________________________________
BezierCone::BezierCone(Double_t rin, Double_t rout, UInt_t nz, Double_t x1, Double_t y1, Double_t x2, Double_t y2, Double_t height)
  : Cone(rin, rout)
{
  fX1 = x1;
  fY1 = y1;
  fX2 = x2;
  fY2 = y2;
  fNz = nz;
  fHeight = height;

  BuildGeometry();
  BuildPMT();
  fManager->CloseGeometry();
}

//______________________________________________________________________________
void BezierCone::BuildGeometry()
{
  fDz = fHeight > 0 ? fHeight/2. : WinstonDz();

  AGeoBezierPgon* bezier
    = new AGeoBezierPgon("bezier", 0, 360, kNedges, fNz, fRin, fRout, fDz);
  AGeoBezierPgon* outer
    = new AGeoBezierPgon("outer", 0, 360, kNedges, 2, fRin*kLarger,
                         fRin*kLarger, fDz*kSmaller);
  
  if(fX2 == -100 and fY2 == -100){
    bezier->SetControlPoints(fX1, fY1);
  } else {
    bezier->SetControlPoints(fX1, fY1, fX2, fY2);
  } // if
  
  outer->SetSections();

  TGeoCompositeShape* composite
    = new TGeoCompositeShape("composite", "outer - bezier");
  fCone = new AMirror("compositeVolume", composite);
  fManager->GetTopVolume()->AddNodeOverlap(fCone, 1);
}

//______________________________________________________________________________
Double_t BezierCone::GetArea()
{
  Double_t theta = TMath::Pi()/kNedges;
  Double_t area = fRin*fRin/TMath::Tan(theta)*kNedges;

  return area;
}

//______________________________________________________________________________
void test(Int_t mode, Double_t theta, Double_t phi, UInt_t n)
{
  Cone* cone;
  if(mode == 0){
    cone = new WinstonCone(20*mm, 10*mm);
  } else if(mode == 2){
    cone = new BezierCone(20*mm, 10*mm, 80, 0.9, 0.2);
  } // if

  cone->GetManager()->GetTopVolume()->Draw();

  ARayArray* array = cone->Trace(theta, phi, n);

  //  TObjArray* exited = array->GetExited();
  //  TObjArray* exited = array->GetStopped();
  //  TObjArray* exited = array->GetSuspended();
  //  TObjArray* exited = array->GetAbsorbed();
  TObjArray* focused = array->GetFocused();
  TObjArray* exited = array->GetExited();
  std::cout << array->GetFocused()->GetEntries() << std::endl;

  for(Int_t i = 0; i < focused->GetEntries(); i++){
    ARay* ray = (ARay*)focused->At(i);
    TPolyLine3D* line = ray->MakePolyLine3D();
    line->SetLineColor(2);
    line->Draw();
  } // i

  for(Int_t i = 0; i < exited->GetEntries(); i++){
    ARay* ray = (ARay*)exited->At(i);
    TPolyLine3D* line = ray->MakePolyLine3D();
    line->SetLineColor(3);
    line->Draw();
  } // i
}

//______________________________________________________________________________
void run(Int_t mode, Double_t rin, Double_t rout,
         Double_t x1 = 0.5, Double_t y1 = 0.5,
         Double_t x2 = -100, Double_t y2 = -100, Double_t height = -100)
{
  Cone* cone;

  if(mode == 0){
    // Hexagonal Winston cone
    cone = new WinstonCone(rin*mm, rout*mm);
  } else if(mode == 2){
    // Hexagonal quadratic Bezier cone
    cone = new BezierCone(rin*mm, rout*mm, 80, x1, y1);
  } else if(mode == 4){
    // Hexagonal cubic Bezier cone
    cone = new BezierCone(rin*mm, rout*mm, 80, x1, y1, x2, y2);
  } else if(mode == 6){
    // Hexagonal cubic Bezier cone which has a user-specified cone height
    cone = new BezierCone(rin*mm, rout*mm, 80, x1, y1, x2, y2, height*mm);
  } // if
  
  const Int_t kN = 100000;
  //const Int_t kN = 10000;

  TGraph* graph = new TGraph;

  gROOT->cd();

  for(Int_t i = 0; i <= 150; i++){
    Double_t theta = 30.*i/100.*TMath::DegToRad();

    Double_t phi1 = 0*TMath::DegToRad();
    Double_t phi2 = 60*TMath::DegToRad();

    ARayArray* array = cone->Trace(theta, phi1, phi2, kN);

    TObjArray* focused = array->GetFocused();
    TObjArray* stopped = array->GetStopped();
    TObjArray* exited  = array->GetExited();

    /*
    if(i == 50){
      gGeoManager->GetTopVolume()->Draw("ogl");
      for(Int_t j = 0; j < 100; j++){
        ARay* ray = (ARay*)focused->At(j);
        //ARay* ray = (ARay*)stopped->At(j);
        //ARay* ray = (ARay*)exited->At(j);
        if(ray){
          TPolyLine3D* pol = ray->MakePolyLine3D();
          pol->Draw();
        } // if
      } // i
    } // if
    */
    UInt_t total = 0;

    total = focused->GetEntries();

    delete array;

    std::cout << theta*TMath::RadToDeg() << "\t" << total << std::endl;
    graph->SetPoint(i, theta*TMath::RadToDeg(), total);
    Double_t p = total/(kN + 0.);
    Double_t sigma = TMath::Sqrt(kN*(1 - p)*p);
  } // i
  
  //graph->Draw("ap");
}
