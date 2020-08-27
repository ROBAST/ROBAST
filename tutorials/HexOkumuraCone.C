/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

// An example script of a hexagonal Okumura cone
// See A. Okumura (2012) Astroparticle Physics 38 18-24

#include "AFocalSurface.h"
#include "AGeoBezierPgon.h"
#include "AGeoWinstonCone2D.h"
#include "AGeoWinstonConePoly.h"
#include "ALens.h"
#include "AMirror.h"
#include "AOpticsManager.h"
#include "ARayShooter.h"

#include "TAxis.h"
#include "TCanvas.h"
#include "TGeoBBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoMatrix.h"
#include "TLegend.h"

// define useful units
static const Double_t cm = AOpticsManager::cm();
static const Double_t mm = AOpticsManager::mm();
static const Double_t um = AOpticsManager::um();
static const Double_t nm = AOpticsManager::nm();
static const Double_t m = AOpticsManager::m();
static const Double_t deg = AOpticsManager::deg();

TGraph* ConeTrace(Int_t mode, bool del) {
  // mode == 0: hex-hex Winston cone built with AGeoWinstonConePoly
  // mode == 1: hex-hex Okumura cone built with AGeoBezierPgon

  AOpticsManager* manager = new AOpticsManager("manager", "manager");

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 10 * cm, 10 * cm, 10 * cm);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);

  const Double_t kRin = 20 * mm;
  const Double_t kRout = 10 * mm;
  TGeoRotation* rot30 = new TGeoRotation("rot30", 30, 0, 0);
  rot30->RegisterYourself();

  AGeoWinstonConePoly* hexWin = new AGeoWinstonConePoly("hexWin", kRin, kRout, 6);
  const Double_t kDZ = hexWin->GetDZ();

  // Make an outer shape of the cone
  // The length is intentionally smaller than kDZ to avoid possible rounding errors
  TGeoPgon* pgon = new TGeoPgon("pgon", 0, 360, 6, 4);
  pgon->DefineSection(0, -kDZ * 0.9999, 0, kRout * 1.1);
  pgon->DefineSection(1, -kDZ * 0.5, 0, kRin * 0.9);
  pgon->DefineSection(2, -kDZ * 0., 0, kRin * 1.001);
  pgon->DefineSection(3, kDZ * 0.9999, 0, kRin * 1.001);

  TGeoCompositeShape* coneComp1 = 0;

  if (mode == 0) {
    coneComp1 = new TGeoCompositeShape("coneComp1", "pgon:rot30 - hexWin");
  } else if (mode == 1) {
    AGeoBezierPgon* hexBez =
        new AGeoBezierPgon("hexBez", 0, 360, 6, 100, kRin, kRout, kDZ);
    hexBez->SetControlPoints(0.39, 0.18, 0.87, 0.36);
    coneComp1 = new TGeoCompositeShape("coneComp1", "pgon:rot30 - hexBez:rot30");
  }

  AMirror* coneMirror = new AMirror("coneMirror", coneComp1);
  world->AddNode(coneMirror, 1);

  // Here we assume a flat hexagonal PMT which is slightly bigger than the exit aperture
  TGeoPgon* pgonPMT = new TGeoPgon("pgonPMT", 0, 360, 6, 2);
  pgonPMT->DefineSection(0, - kDZ - 0.01 * mm, 0, kRout * 1.01);
  pgonPMT->DefineSection(1, - kDZ, 0, kRout * 1.01);
  AFocalSurface* pmt = new AFocalSurface("pmt", pgonPMT);
  world->AddNode(pmt, 1, rot30);

  manager->CloseGeometry();

  if (mode == 1) {
    TCanvas* can1 = new TCanvas("can1", "can1");
    manager->GetTopVolume()->Draw("ogl");
  }  // if

  TGraph* graAeff = new TGraph;

  for (Double_t theta = 0.; theta < 40.; theta += 0.1) {
    Double_t Aeff = 0.;
    for (Double_t phi = 0.; phi < 30.; phi += 0.3) {
      TGeoTranslation* raytr = new TGeoTranslation(
          "raytr", 50 * mm * TMath::Sin(theta * deg), 0,
          50 * mm * TMath::Cos(theta * deg));
      TGeoRotation* rayrot = new TGeoRotation("rayrot", 90 - phi, 180 + theta, 0);

      TVector3 dir(0, 0, 1);
      // does not affect the results because we have no lenses
      Double_t lambda = 400 * nm;
      // 1 photon per 1 mm^2
      const int kNph = 1000;
      const double kD = 100 * mm;
      ARayArray* array =
          ARayShooter::RandomSquare(lambda, kD, kNph, rayrot, raytr, &dir);
      // illuminated small area per input photon
      // i.e., if 3 photons are detected, the effective area is 3 x dA
      Double_t dA = kD * kD / kNph;

      manager->TraceNonSequential(*array);
      TObjArray* focused = array->GetFocused();

      for (Int_t j = 0; j <= focused->GetLast(); j++) {
        ARay* ray = (ARay*)(*focused)[j];
        if (!ray) continue;

        // Calculate the effective area from the number of focused photons
        Aeff += dA;

        if (mode == 1 and TMath::Abs(theta - 20.) < 0.001 and j < 50 and
            phi == 0) {
          TPolyLine3D* pol = ray->MakePolyLine3D();
          pol->SetLineColor(3);
          pol->SetLineWidth(2);
          pol->Draw();
        }
      }

      delete array;
    }  // phi

    Double_t hexA = 2 * TMath::Sqrt(3) * kRin * kRin;  // Area of the inputer aperture
    Double_t eff = Aeff / hexA / TMath::Cos(theta * TMath::DegToRad());
    graAeff->SetPoint(graAeff->GetN(), theta, eff);
  }  // theta

  if (del) {
    delete manager;
    manager = 0;
  }  // if

  return graAeff;
}

void HexOkumuraCone() {
  TGraph* win = ConeTrace(0, true);   // Winston cone
  TGraph* oku = ConeTrace(1, false);  // Okumura cone

  TGraph* gra[2] = {win, oku};
  TCanvas* can2 = new TCanvas("can2", "can2");

  for (int i = 0; i < 2; i++) {
    gra[i]->GetXaxis()->SetTitle("Incident Angle (deg)");
    gra[i]->GetYaxis()->SetTitle("Collection Efficiency (%)");
    gra[i]->SetMarkerSize(0.3);
    gra[i]->SetMarkerStyle(21 + 3 * i);
    gra[i]->Draw(i == 0 ? "ap" : "p same");
    gra[i]->GetXaxis()->SetRangeUser(0, 40.);
  }  // i

  TGraph* ideal = new TGraph;
  ideal->SetPoint(0, 0, 100);
  ideal->SetPoint(1, 30, 100);
  ideal->SetPoint(2, 30, 0);
  ideal->SetPoint(3, 40, 0);
  ideal->Draw("l same");

  TLegend* leg = new TLegend(0.15, 0.15, 0.7, 0.35);
  leg->SetFillStyle(0);
  leg->SetTextFont(132);
  leg->AddEntry(ideal, "Ideal 2D Winston Cone", "l");
  leg->AddEntry(win, "Hexagonal Winston Cone", "p");
  leg->AddEntry(oku, "Hexagonal Okumura Cone", "p");
  leg->Draw();
}
