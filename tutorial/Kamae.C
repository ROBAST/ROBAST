// Author: Akira Okumura 2011/3/6

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

// Tutorial for a hexagonal tube mirror

// define useful units
static const Double_t cm = AOpticsManager::cm();
static const Double_t mm = AOpticsManager::mm();
static const Double_t um = AOpticsManager::um();
static const Double_t nm = AOpticsManager::nm();
static const Double_t  m = AOpticsManager::m();

void Kamae()
{
  const Double_t kRmin = 10*cm; // inner radius
  const Double_t kRmax = 11*cm; // outer radius
  const Double_t kLength = 200*cm; // length

  AOpticsManager* manager = new AOpticsManager("manager", "HexTubeMirror");
  manager->SetNsegments(100); // make the GL visualization smoother

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 1*m, 1*m, 1*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);

  // Make the hexagonal mirror
  TGeoPgon* hexV = new TGeoPgon("hexV", 0, 360, 6, 2);
  hexV->DefineSection(0, -kLength/2., kRmin, kRmax);
  hexV->DefineSection(1, +kLength/2., kRmin, kRmax);

  AMirror* mirror = new AMirror("coneMirror", hexV);
  world->AddNode(mirror, 1);

  // Make the focal plane
  TGeoBBox* focalV = new TGeoBBox("focalV", kRmax*3, kRmax*3, 1*mm);
  AFocalSurface* focal = new AFocalSurface("focal", focalV);

  TGeoTranslation* trans = new TGeoTranslation("trans", 0, 0, -kLength/2.);

  world->AddNode(focal, 1, trans);

  manager->CloseGeometry();

  TCanvas* can1 = new TCanvas("can1", "can1");
  world->Draw("ogl");

  TH2D* hist = new TH2D("hist", "PSF;X (cm);Y (cm)", 100, -10, 10, 100, -10, 10);

  TGeoTranslation* ray_trans = new TGeoTranslation("ray_trans", 0, 0, 1.1*kLength/2.);
  ARayArray* array = ARayShooter::RandomSphere(400*nm, 10000, ray_trans);
  manager->TraceNonSequential(*array);
  
  TObjArray* focused = array->GetFocused();

  for(Int_t i = 0; i <= focused->GetLast(); i++){
    ARay* ray = (ARay*)(*focused)[i];

    Double_t x[4];
    ray->GetLastPoint(x);
    TPolyLine3D* pol = ray->MakePolyLine3D();
    pol->Draw();
    hist->Fill(x[0]/cm, x[1]/cm);
  } // i

  TCanvas* can = new TCanvas("can", "can", 800, 800);
  hist->Draw("colz");
}
