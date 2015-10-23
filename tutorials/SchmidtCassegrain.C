// Zemax/Samples/Sequential/Telescopes/Schmidt-Cassegrain spider obscuration.zmz

#include "AOpticsManager.h"
#include "AGlassCatalog.h"
#include "AFocalSurface.h"
#include "ALens.h"
#include "TH2.h"
#include "AGeoAsphericDisk.h"
#include "ARayShooter.h"
#include "TGeoBBox.h"
#include "TGeoTube.h"
#include "TGeoCompositeShape.h"
#include "TGeoMatrix.h"
#include "TCanvas.h"
#include "TText.h"
#include "TArrow.h"
#include "TStopwatch.h"
#include "TMath.h"
#include "TStyle.h"

#include <iostream>

#include "oxon.C"

static const Double_t m = AOpticsManager::m();
static const Double_t cm = AOpticsManager::cm();
static const Double_t mm = AOpticsManager::mm();
static const Double_t um = AOpticsManager::um();
static const Double_t nm = AOpticsManager::nm();
static const Double_t inch = AOpticsManager::inch();

static const Double_t kOffset = 10*cm;

void SchmidtCassegrain()
{
  TThread::Initialize();
  AOpticsManager* manager = new AOpticsManager("Zemax", "Zemax");
  
  // Make the world
  TGeoBBox* box = new TGeoBBox("box", 10*m, 10*m, 10*m);
  AOpticalComponent *top = new AOpticalComponent("top", box);
  manager->SetTopVolume(top);
  manager->SetNsegments(200);

  AGeoAsphericDisk* disk
    = new AGeoAsphericDisk("disk", 0.*inch + kOffset, 0/inch, 0.65*inch + kOffset, -8.721454939626E-005/inch, 12*inch, 0.*inch);
  Double_t coeff[4] = {0, 3.68090959E-7/TMath::Power(inch, 3),
                       2.73643352E-11/TMath::Power(inch, 5),
                       3.20036892E-14/TMath::Power(inch, 7)};
  disk->SetPolynomials(0, 0, 4, coeff);

  ARefractiveIndex* bk7 = AGlassCatalog::GetRefractiveIndex("N-BK7");
  ALens* lens = new ALens("lens", disk);
  lens->SetRefractiveIndex(bk7);
  top->AddNode(lens, 1);

  TGeoTube* tube = new TGeoTube("tube", 12*inch, 18*inch, disk->GetDZ());
  AObscuration* obs = new AObscuration("aperture", tube);
  top->AddNode(obs, 1, new TGeoTranslation(0, 0, disk->GetOrigin()[2]));

  tube = new TGeoTube("tube2", 0*inch, 4.5*inch, 0.01*mm);

  box = new TGeoBBox("box2", 1.210290505556E1/2.*inch, 1*inch, 0.01*mm);
  Double_t dx = box->GetDX();
  Double_t deg = TMath::DegToRad();
  TGeoCombiTrans* tr1
    = new TGeoCombiTrans("tr1", dx*TMath::Cos(90*deg), dx*TMath::Sin(90*deg), 0,
                         new TGeoRotation("", 90, 0, 0));
  TGeoCombiTrans* tr2
    = new TGeoCombiTrans("tr2", dx*TMath::Cos(210*deg), dx*TMath::Sin(210*deg), 0,
                         new TGeoRotation("", 210, 0, 0));
  TGeoCombiTrans* tr3
    = new TGeoCombiTrans("tr3", dx*TMath::Cos(330*deg), dx*TMath::Sin(330*deg), 0,
                         new TGeoRotation("", 330, 0, 0));
  tr1->RegisterYourself();
  tr2->RegisterYourself();
  tr3->RegisterYourself();
  
  TGeoCompositeShape* comp = new TGeoCompositeShape("comp", "box2:tr1 + box2:tr2 + box2:tr3 + tube2");
  obs = new AObscuration("obs", comp);
  
  top->AddNode(obs, 1, new TGeoTranslation(0, 0, (0.65 + 40.0)*inch + kOffset));

  disk
    = new AGeoAsphericDisk("disk2", (0.65 + 40. + 32.)*inch + kOffset, -1.049567394559E-002/inch, (0.65 + 40. + 32. + 0.1)*inch + kOffset, -1.049567394559E-002/inch, 12.183*inch, 4*inch);
  disk->SetConicConstants(0.077235, 0.077235);

  AMirror* mirror = new AMirror("primary", disk);
  top->AddNode(mirror, 1);

  disk = new AGeoAsphericDisk("disk3", (0.65 + 40. + 32. - 30.86635 - 0.1)*inch + kOffset, -2.01270013787E-002/inch, (0.65 + 40. + 32. - 30.86635)*inch + kOffset, -2.01270013787E-002/inch, 4.322385947053*inch, 0);
  mirror = new AMirror("secondary", disk);
  top->AddNode(mirror, 1);

  Double_t origin1[3] = {0, 0, (0.65 + 40. + 32. - 30.86635 + 50.6706488)*inch + kOffset + 5*um};
  TGeoBBox* box1 = new TGeoBBox("box1", 5*inch, 5*inch, 5*um, origin1);
  AFocalSurface* screen1 = new AFocalSurface("screen1", box1);
  top->AddNode(screen1, 1);

  manager->CloseGeometry();

  manager->DisableFresnelReflection(kTRUE);

  manager->SetMultiThread(kTRUE);
  manager->SetMaxThreads(8);

  TCanvas* c1 = new TCanvas("c1");
  top->Draw("ogl");

  TGeoRotation* rot = new TGeoRotation;
  double angle = 0.1;
  rot->SetAngles(0, angle, 0);

  TH2D* h[2];
  const char title[2][100] = {"#it{#theta} = 0.0#circ",
                              "#it{#theta} = 0.1#circ"};

  TCanvas* can = new TCanvas("can", "can", 1250, 625);
  can->Divide(2, 1, 1e-10, 1e-10);

  for(Int_t j = 0; j < 2; j++){
    h[j] = new TH2D(Form("h%d", j), Form("%s;X (#mum);Y (#mum);", title[j]), 500, -1.25e-3*inch/um, 1.25e-3*inch/um, 500, -1.25e-3*inch/um, 1.25e-3*inch/um);
    
    Double_t wave[] = {486.1, 530, 587.6, 610, 656.33};
    for(Int_t k = 0; k < 5; k++){
      ARayArray* array = ARayShooter::RandomCircle(wave[k%5]*nm, 12.5*inch, 200000, j == 0 ? 0 : rot);
      
      manager->TraceNonSequential(*array);
      TObjArray* focused = array->GetFocused();
      c1->cd();
      
      for(Int_t i = 0; i <= focused->GetLast(); i++){
        ARay* ray = (ARay*)(*focused)[i];
        if(j == 0 && i < 5){
          TPolyLine3D* pol = ray->MakePolyLine3D();
          pol->SetLineColor(j == 0 ? 4 : 3);
          pol->SetLineWidth(2);
          pol->Draw();
        } // if
        
        Double_t p[4];    
        ray->GetLastPoint(p);
        
        h[j]->Fill(p[0]/um, -(p[1]/um + (j == 0 ? 0. : 0.253015389*inch/um*angle/0.1)));
      } // i
      delete array;
    } // k

    gPad = can->cd(j + 1);
    h[j]->Draw("colz");

    gPad->SetLogz();
  } // j
}
