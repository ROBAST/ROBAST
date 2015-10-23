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

void HexWinstonCone(Int_t mode = 0)
{
  // mode == 0: hex-hex Winston cone built with AGeoWinstonConePoly
  // mode == 1: hex-hex Winston cone built with three AGeoWinstonCone2D
  // mode == 2: circle-circle Winston cone with AGeoWinstonConePoly

  AOpticsManager* manager = new AOpticsManager("manager", "SC");

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 30*m, 30*m, 30*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);

  // Make the primary mirror
  const Double_t kRin = 20*mm;
  const Double_t kRout = 10*mm;
  TGeoRotation* rot30 = new TGeoRotation("rot30", 30, 0, 0);
  rot30->RegisterYourself();
  TGeoRotation* rot60 = new TGeoRotation("rot60", 60, 0, 0);
  rot60->RegisterYourself();
  TGeoRotation* rot120 = new TGeoRotation("rot120", 120, 0, 0);
  rot120->RegisterYourself();

  AGeoWinstonCone2D* coneV = new AGeoWinstonCone2D("coneV", kRin, kRout, kRin*1.733);
  TGeoPgon* pgon = new TGeoPgon("pgon", 0, 360, 6, 4);
  pgon->DefineSection(0, -coneV->GetDZ()*0.999, 0, kRout*1.001);
  pgon->DefineSection(1, -coneV->GetDZ()*0.5, 0, kRin*0.9);
  pgon->DefineSection(2, -coneV->GetDZ()*0., 0, kRin*0.99);
  pgon->DefineSection(3,  coneV->GetDZ()*0.999, 0, kRin*1.001);

  if(mode == 0){
    AGeoWinstonConePoly* hexV = new AGeoWinstonConePoly("hexV", kRin, kRout, 6);
    TGeoCompositeShape* coneComp1 = new TGeoCompositeShape("coneComp1", "pgon:rot30 - hexV");
    AMirror* coneMirror = new AMirror("coneMirror", coneComp1);
    world->AddNode(coneMirror, 1);
  } else if(mode == 1){
    TGeoCompositeShape* coneComp1 = new TGeoCompositeShape("coneComp1", "coneV*(coneV:rot60)*(coneV:rot120)");
    TGeoCompositeShape* coneComp2 = new TGeoCompositeShape("coneComp2", "pgon:rot30 - coneComp1");
    AMirror* coneMirror = new AMirror("coneMirror", coneComp2);
    world->AddNode(coneMirror, 1);
  } else {
    AGeoWinstonConePoly* hexV = new AGeoWinstonConePoly("hexV", kRin, kRout, 100);
    TGeoCompositeShape* coneComp1 = new TGeoCompositeShape("coneComp1", "pgon:rot30 - hexV");
    AMirror* coneMirror = new AMirror("coneMirror", coneComp1);
    world->AddNode(coneMirror, 1);
  } // if

  TGeoPgon* pgonPMT = new TGeoPgon("pgonPMT", 0, 360, 6, 2);
  pgonPMT->DefineSection(0, -coneV->GetDZ() - 0.01*mm, 0, kRout*1.01);
  pgonPMT->DefineSection(1, -coneV->GetDZ()       , 0, kRout*1.01);
  AFocalSurface* pmt = new AFocalSurface("pmt", pgonPMT);
  world->AddNode(pmt, 1, rot30);

  manager->CloseGeometry();

  TCanvas* can1 = new TCanvas("can1", "can1");
  world->Draw();

  TGraph* graAeff = new TGraph;

  for(Double_t deg = 0.; deg < 40.; deg += 0.1){
    TGeoTranslation* raytr = new TGeoTranslation("raytr", 100*mm*TMath::Sin(deg*TMath::DegToRad()), 0, 100*mm*TMath::Cos(deg*TMath::DegToRad()));
    TGeoRotation* rayrot = new TGeoRotation("rayrot", 90, 180 + deg, 0);

    TVector3 dir(0, 0, 1);
    Double_t lambda = 400*nm; // does not affect the results because we have no lens
    // 1 photon per 1 mm^2
    ARayArray* array = ARayShooter::Square(lambda, 100*mm, 101, rayrot, raytr, &dir);
    Double_t dA = 1*mm*mm;
    manager->TraceNonSequential(*array);
    TObjArray* focused = array->GetFocused();

    Double_t Aeff = 0.;
    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      if(!ray) continue;

      // Calculate the effective area from the number of focused photons
      Aeff += dA;

      if(graAeff->GetN() == 0){
        ray->Draw(); // high CPU load!
      } // if
    } // j

    if(mode == 0 || mode == 1){
      graAeff->SetPoint(graAeff->GetN(), deg, Aeff/(2*TMath::Sqrt(3)*kRin*kRin)/TMath::Cos(deg*TMath::DegToRad()));
    } else {
      graAeff->SetPoint(graAeff->GetN(), deg, Aeff/(TMath::Pi()*kRin*kRin)/TMath::Cos(deg*TMath::DegToRad()));
    } // if


    delete array;
  } // deg

  TCanvas* can2 = new TCanvas("can2", "can2");
  graAeff->GetXaxis()->SetTitle("Incident Angle (deg)");
  graAeff->GetYaxis()->SetTitle("Collection Efficiency (%)");
  graAeff->Draw("ap");
}
