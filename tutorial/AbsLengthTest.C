// Author: Akira Okumura 2010/11/28

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

// define useful units
static const Double_t cm = AOpticsManager::cm();
static const Double_t mm = AOpticsManager::mm();
static const Double_t um = AOpticsManager::um();
static const Double_t nm = AOpticsManager::nm();
static const Double_t  m = AOpticsManager::m();

void AbsLengthTest()
{
  gStyle->SetOptStat();

  AOpticsManager* manager = new AOpticsManager("manager", "AbsLengthTest");
  manager->SetNsegments(100);

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 30*m, 30*m, 30*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);

  // Top volume 
  TGeoBBox* topbox = new TGeoBBox("topbox", 30*m, 30*m, 30*m);
  ALens* top = new ALens("top", topbox);
  top->SetConstantAbsorptionLength(10*cm);
  world->AddNode(top, 1);

  manager->CloseGeometry();

  TCanvas* canGeometry = new TCanvas("canGeometry", "canGeometry", 800, 800);
  top->Draw();

  ARayArray* array = ARayShooter::RandomSphere(400*nm, 10000);
  manager->TraceNonSequential(*array);
  TObjArray* absorbed = array->GetAbsorbed();

  TH1D* h1 = new TH1D("h1", "h1", 100, 0, 0);

  for(Int_t i = 0; i <= absorbed->GetLast(); i++){
    ARay* ray = (ARay*)(*absorbed)[i];
    if(!ray) continue;

    Double_t p[4];    
    ray->GetLastPoint(p);
    h1->Fill(TMath::Sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]));
  } // i

  TCanvas* canHist = new TCanvas("canHist", "canHist", 800, 600);
  h1->Draw("e");
  gPad->SetLogy();
}
