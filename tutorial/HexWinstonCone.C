// $Id: SC.C 28 2010-11-30 00:43:38Z oxon $
// Author: Akira Okumura 2011/3/6

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

// An example of hexagonal Winston cone

// define useful units
static const Double_t cm = AOpticsManager::cm();
static const Double_t mm = AOpticsManager::mm();
static const Double_t um = AOpticsManager::um();
static const Double_t nm = AOpticsManager::nm();
static const Double_t  m = AOpticsManager::m();

void HexWinstonCone()
{
  AOpticsManager* manager = new AOpticsManager("manager", "SC");
  //manager->SetNsegments(100);

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 30*m, 30*m, 30*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);

  // Top volume 
  TGeoBBox* topbox = new TGeoBBox("topbox", 30*m, 30*m, 30*m);
  AOpticalComponent* top = new AOpticalComponent("top", topbox);

  // Make the primary mirror
  Double_t rin = 20*mm;
  Double_t rout = 10*mm;
  AGeoWinstonCone2D* coneV = new AGeoWinstonCone2D("coneV", rin, rout, rin*1.733);
  TGeoRotation* rot30 = new TGeoRotation("rot30", 30, 0, 0);
  rot30->RegisterYourself();
  TGeoRotation* rot60 = new TGeoRotation("rot60", 60, 0, 0);
  rot60->RegisterYourself();
  TGeoRotation* rot120 = new TGeoRotation("rot120", 120, 0, 0);
  rot120->RegisterYourself();

  TGeoPgon* pgon = new TGeoPgon("pgon", 0, 360, 6, 4);
  pgon->DefineSection(0, -coneV->GetDZ()*0.999, 0, rout*1.001);
  pgon->DefineSection(1, -coneV->GetDZ()*0.5, 0, rin*0.9);
  pgon->DefineSection(2, -coneV->GetDZ()*0., 0, rin*0.99);
  pgon->DefineSection(3,  coneV->GetDZ()*0.999, 0, rin*1.001);

  TGeoCompositeShape* coneComp1 = new TGeoCompositeShape("coneComp1", "coneV*(coneV:rot60)*(coneV:rot120)");
  TGeoCompositeShape* coneComp2 = new TGeoCompositeShape("coneComp2", "pgon:rot30 - coneComp1");
  //  TGeoCompositeShape* coneComp2 = new TGeoCompositeShape("coneComp2", "pgon:rot30 - coneV");
  AMirror* coneMirror = new AMirror("coneMirror", coneComp2);
  top->AddNode(coneMirror, 1);

  TGeoPgon* pgonPMT = new TGeoPgon("pgonPMT", 0, 360, 6, 2);
  pgonPMT->DefineSection(0, -coneV->GetDZ() - 0.01*mm, 0, rout*1.01);
  pgonPMT->DefineSection(1, -coneV->GetDZ()       , 0, rout*1.01);
  AFocalSurface* pmt = new AFocalSurface("pmt", pgonPMT);
  top->AddNode(pmt, 1, rot30);

  world->AddNode(top, 1);

  manager->CloseGeometry();

  TCanvas* can1 = new TCanvas("can1", "can1");
  top->Draw();

  TGraph* graAeff = new TGraph;

  for(Double_t deg = 0.; deg < 40.; deg += 0.1){
    TGeoTranslation* raytr = new TGeoTranslation("raytr", 100*mm*TMath::Sin(deg*TMath::DegToRad()), 0, 100*mm*TMath::Cos(deg*TMath::DegToRad()));
    TGeoRotation* rayrot = new TGeoRotation("rayrot", 90, 180 + deg, 0);

    TVector3 dir(0, 0, 1);
    Double_t lambda = 400*nm; // does not affect the results because we have no lens
    // 1 photon per 0.0625 mm^2
    ARayArray* array = ARayShooter::Square(lambda, 100*mm, 101, rayrot, raytr, &dir);
    //    ARayArray* array = ARayShooter::Rectangle(lambda, 1*mm, 100*mm, 2, 501, rayrot, raytr, &dir);
    manager->TraceNonSequential(*array);
    TObjArray* focused = array->GetFocused();

    Double_t Aeff = 0.;
    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      if(!ray) continue;

      // Calculate the effective area from the number of focused photons
      Aeff += 0.0625; // 0.0625 (mm^2)

      if(graAeff->GetN() == 290){
	ray->Draw(); // high CPU load!
      } // if
    } // j

    graAeff->SetPoint(graAeff->GetN(), deg, Aeff);

    delete array;
  } // deg

  TCanvas* can2 = new TCanvas("can2", "can2");
  graAeff->Draw("ap");

  //  coneMirror->Raytrace();
}
