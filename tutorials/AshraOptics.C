// Author: Akira Okumura 2007/10/04
/******************************************************************************
 * This script simulates the Ashra optical system first proposed in Sasaki et *
 * al. (2012). The final optimization of the lens parameters was done by      *
 * Akira Okumura using ZEMAX EE (2003 ver.). The main difference from the     *
 * paper is that bottom surfaces of the collector lenses are flat in this     *
 * script. The mechanical structure built with TGeo* classes is based on the  *
 * actual design assembled on Mauna Loa, Hawawii.                             *
 *                                                                            *
 * This will help you to understand how a complex telescop is realized in     *
 * ROBAST with a dirty script. At the current moment, there is not an easy    *
 * way to convert a 3D CAD file into a ROBAST script. If you want to simulate *
 * your optical system in a 3D CAD file, please consider using ZEMAX or       *
 * Geant4 (CAD -> STEP -> GDML) instead.                                      *
 *                                                                            *
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#include "AFocalSurface.h"
#include "AGeoAsphericDisk.h"
#include "ALens.h"
#include "AMirror.h"
#include "AOpticalComponent.h"
#include "AOpticsManager.h"
#include "ARayShooter.h"
#include "ASchottFormula.h"

#include "TGeoArb8.h"
#include "TGeoBBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoMatrix.h"
#include "TGeoPgon.h"
#include "TGeoSphere.h"
#include "TGeoTube.h"
#include "TGeoXtru.h"
#include "TNtuple.h"

#include "TCanvas.h"
#include "TGLViewer.h"
#include "TH2D.h"
#include "TMath.h"
#include "TStyle.h"

// define useful units
const double cm = AOpticsManager::cm();
const double mm = AOpticsManager::mm();
const double um = AOpticsManager::um();
const double nm = AOpticsManager::nm();
const double  m = AOpticsManager::m();

const Double_t kInputZ = 1.05625*mm; // shift of 20" I.I.

// Input window parameters
const Double_t kInputR[2] = {714*mm, 722*mm};

void AddMirror(AOpticalComponent* opt);
void AddLens(AOpticalComponent* opt);
void AddLensRing(AOpticalComponent* opt);
void AddLensFrame(AOpticalComponent* opt);
void AddPipeline(AOpticalComponent* opt);
void AddInputWindow(AOpticalComponent* opt);
void AddStewart(AOpticalComponent* opt);
void Add30Frame(AOpticalComponent* opt);

void AshraOptics()
{
  AOpticsManager* manager = new AOpticsManager("manager", "Ashra Optics");
  manager->DisableFresnelReflection(kTRUE);
  manager->SetNsegments(50);
  // Make the world
  TGeoBBox* box1 = new TGeoBBox("box1", 2*m, 2*m, 2*m);
  AOpticalComponent *top = new AOpticalComponent("top", box1);
  manager->SetTopVolume(top);

  // Top volume
  TGeoBBox* optbox = new TGeoBBox("optbox", 2*m, 2*m, 2*m);
  AOpticalComponent* opt = new AOpticalComponent("opt", optbox);

  AddLens(opt);
  AddLensRing(opt);
  AddLensFrame(opt);
  AddMirror(opt);
  AddPipeline(opt);
  AddInputWindow(opt);
  AddStewart(opt);
  Add30Frame(opt);

  // Dummy Obscuration
  TGeoTube* tube = new TGeoTube("tube", 500*mm, 1500*mm, 5*mm);
  AObscuration* stop = new AObscuration("stop", tube);
  stop->SetVisibility(kFALSE); // not shown in views
  opt->AddNodeOverlap(stop, 1);

  // Rotate all optical system
  TGeoRotation* r1 = new TGeoRotation;
  r1->SetAngles(90, 90, 0);
  top->AddNode(opt, 1, r1);

  manager->CloseGeometry();

  TCanvas* can = new TCanvas("can", "", 640+4, 480+28);
  top->Draw("ogl");

  // Start ray-tracing

  const Int_t kLambdaN = 30;
  // weight map                                     eff x flat eff x Cere
  const Double_t kSpectrum[kLambdaN][3] = {{300*nm, 0.0810219, 0.0900244},
                                           {310*nm, 0.0832656, 0.0866447},
                                           {320*nm, 0.1530340, 0.149447},
                                           {330*nm, 0.1561810, 0.143417},
                                           {340*nm, 0.1729140, 0.14958},
                                           {350*nm, 0.1680870, 0.137214},
                                           {360*nm, 0.1820270, 0.140453},
                                           {370*nm, 0.1840770, 0.134461},
                                           {380*nm, 0.1891030, 0.130958},
                                           {390*nm, 0.1877240, 0.123421},
                                           {400*nm, 0.1899490, 0.118718},
                                           {410*nm, 0.1869390, 0.111207},
                                           {420*nm, 0.1886550, 0.106947},
                                           {430*nm, 0.1865470, 0.100891},
                                           {440*nm, 0.1838270, 0.0949519},
                                           {450*nm, 0.1745720, 0.0862083},
                                           {460*nm, 0.1605160, 0.0758582},
                                           {470*nm, 0.1463170, 0.0662367},
                                           {480*nm, 0.0438607, 0.0190368},
                                           {490*nm, 0.00256585, 0.00106866},
                                           {500*nm, 0.00165472, 0.000661888},
                                           {510*nm, 0.00218826, 0.000841315},
                                           {520*nm, 0.000492465, 0.000182125},
                                           {530*nm, 0.000677054, 0.00024103},
                                           {540*nm, 0.000597908, 0.000205044},
                                           {550*nm, 0.000212673, 7.03052e-05},
                                           {560*nm, 0.000374296, 0.000119355},
                                           {570*nm, 0.00026141, 8.04586e-05},
                                           {580*nm, 0.000148344, 4.40976e-05},
                                           {590*nm, 0.000224703, 6.45514e-05}};

  const int kN = 22; // 0 to 22 [deg]
  TH2D* hist[kN][3];

  TNtuple* nt = new TNtuple("nt", "", "x:y:z");

  for(Int_t n=0; n<kN; n++){
    for(Int_t j=0; j<3; j++){
      hist[n][j] = new TH2D(Form("hist%d_%d", n, j), "", 100, -1, 1,
                            100, -1, 1);
    } // j

    TGeoRotation* rayrot = new TGeoRotation;
    rayrot->SetAngles(90, 90-n, 0);
    TGeoTranslation* raytr = new TGeoTranslation("raytr", -500*mm*TMath::Cos(n*TMath::Pi()/180.), 0, -500*mm*TMath::Sin(n*TMath::Pi()/180.));

    for(Int_t i=0; i<kLambdaN; i++){
      ARayArray* array = ARayShooter::Rectangle(kSpectrum[i][0], 1200*mm, 1200*mm, 20, 20, rayrot, raytr);

      manager->TraceNonSequential(*array);
      TObjArray* focused = array->GetFocused();

      for(Int_t j=0; j<=focused->GetLast(); j++){
        ARay* ray = (ARay*)(*focused)[j];
        if(!ray) continue;
        Double_t p[4];
        ray->GetLastPoint(p);
        ray->SetLineWidth(1);

        if(n == 0 && i == 0 && j%3 == 0){
          TPolyLine3D* pol = ray->MakePolyLine3D();
          pol->SetLineColor(2);
          pol->SetLineWidth(2);
          pol->Draw();
        } // if

        Double_t y = kInputR[0]*TMath::Sin(n*TMath::Pi()/180);
        hist[n][0]->Fill(p[1]/mm, p[2]/mm - y/mm);
        hist[n][1]->Fill(p[1]/mm, p[2]/mm - y/mm, kSpectrum[i][1]);
        hist[n][2]->Fill(p[1]/mm, p[2]/mm - y/mm, kSpectrum[i][2]);
        nt->Fill(p[0], p[1], p[2]);
      } // j

      delete array;
    } // i

    delete raytr;
    delete rayrot;
  } // n

  // Rotate the camera angles in OpenGL View
  can->Update();
  TGLViewer* gl = (TGLViewer*)can->GetViewer3D();
  gl->SetPerspectiveCamera(TGLViewer::kCameraPerspXOZ, 25., 0, 0, -75*TMath::DegToRad(), 210.*TMath::DegToRad());

  TCanvas* can2 = new TCanvas("can2", "", 600+4, 600+28);
  can2->Divide(5, 5);
  gStyle->SetPalette(1);

  for(Int_t i=0; i<kN; i++){
    can2->cd(i+1);
    hist[i][1]->Draw("col");
  } // i

  TGraph* rms[3];
  for(Int_t i=0; i<3; i++){
    rms[i] = new TGraph;
    rms[i]->SetLineColor(i+1);
    for(Int_t j=0; j<kN; j++){
      Double_t val = TMath::Sqrt(TMath::Power(hist[j][i]->GetRMS(1), 2)
                                 + TMath::Power(hist[j][i]->GetRMS(2)/TMath::Cos(TMath::Pi()*j/180), 2));
      rms[i]->SetPoint(j, j, val*mm/kInputR[0]/TMath::Pi()*180*60);
    } // j
  } // i

  TCanvas* can3 = new TCanvas("can3", "", 640+4, 480+4);
  TH2D* frameRMS = new TH2D("frameRMS", ";Incident Angle [deg];RMS [arcmin];",
                            1, 0, 21, 1, 0, 2);
  frameRMS->Draw();

  for(Int_t i=0; i<3; i++){
    rms[i]->Draw("l same");
  } // i
}

//_____________________________________________________________________________
void AddMirror(AOpticalComponent* opt)
{
  // Mirror parameters
  const Double_t kMirrorT = 8*mm; // thickness
  const Double_t kMirrorRad = 425*mm; // radius
  const Int_t kMirrorN = 7;
  const Double_t kMirrorR[kMirrorN] = {1363*mm, 1363*mm, 1363*mm, 1363*mm,
                                       1363*mm, 1363*mm, 1363*mm};

  // Mirror
  TGeoSphere* sphere[kMirrorN];
  TGeoPgon* cut1;
  TGeoArb8* cut2[kMirrorN];
  TGeoArb8* cut3[kMirrorN];
  TGeoCompositeShape* composite[kMirrorN];
  AMirror* mirror[kMirrorN];
  TGeoTranslation* tr[kMirrorN];
  TGeoRotation* rt[kMirrorN];

  // Cut of mirror
  for(Int_t i=0; i<kMirrorN; i++){
    Double_t R = kMirrorR[i];
    Double_t R_ = kMirrorR[i] + kMirrorT;
    Double_t r = R_*TMath::Tan(TMath::ASin(kMirrorRad/R));
    Double_t deg = TMath::DegToRad();

    if(i==0){
      cut1 = new TGeoPgon("mir_cut1", 0, 360, 6, 2);
      cut1->DefineSection(0, 0, 0, 1*nm); // 1*nm is a hack to avoid OpenGL bug
      cut1->DefineSection(1, R_, 0, r*TMath::Sqrt(3)/2);
    } else {

      cut2[i] = new TGeoArb8(Form("mir_cut2%d", i), R_/2.);
      cut3[i] = new TGeoArb8(Form("mir_cut3%d", i), R_/2.);
      for(Int_t j=0; j<2; j++){
        Double_t r_;
        if(j==0) r_ = 1*nm; else r_ = r; // hack again
        cut2[i]->SetVertex(0 + j*4, r_*TMath::Cos(60*deg), r_*TMath::Sin(60*deg));
        cut2[i]->SetVertex(1 + j*4, r_*TMath::Cos(-8.6*deg), r_*TMath::Sin(-8.6*deg));
        cut2[i]->SetVertex(2 + j*4, r_*TMath::Cos(188.6*deg), r_*TMath::Sin(188.6*deg));
        cut2[i]->SetVertex(3 + j*4, r_*TMath::Cos(120*deg), r_*TMath::Sin(120*deg));

        cut3[i]->SetVertex(0 + j*4, r_*TMath::Cos(351.4*deg), r_*TMath::Sin(351.4*deg));
        cut3[i]->SetVertex(1 + j*4, 0, r_/r*-10000*mm);
        cut3[i]->SetVertex(2 + j*4, r_*TMath::Cos(188.6*deg), r_*TMath::Sin(188.6*deg));
        cut3[i]->SetVertex(3 + j*4, 0, 0);
      } // j
    } // if

    tr[i] = new TGeoTranslation(Form("mir_tr%d", i), 0, 0, R_/2.);
    tr[i]->RegisterYourself();

    if(i==0){
      rt[i] = new TGeoRotation(Form("mir_rt%d", i), 60*i, 0, 0);
    } else {
      rt[i] = new TGeoRotation(Form("mir_rt%d", i), 60*i, 32, 0);
    } // if
    rt[i]->RegisterYourself();

    sphere[i] = new TGeoSphere(Form("mir_sph%d", i), R, R_,
                               0, TMath::ASin(kMirrorRad/R)/deg);
    if(i==0){
      composite[i] = new TGeoCompositeShape(Form("mir_cs%d", i), Form("(mir_sph%d*mir_cut1):mir_tr%d", i, i));
    } else {
      composite[i] = new TGeoCompositeShape(Form("mir_cs%d", i), Form("mir_sph%d*((mir_cut2%d + mir_cut3%d):mir_tr%d)", i, i, i, i));
    } // if

    mirror[i] = new AMirror(Form("mirror%d", i), composite[i]);
    opt->AddNode(mirror[i], 1, rt[i]);
  } // i
}

//_____________________________________________________________________________
void AddLens(AOpticalComponent* opt)
{
  // Lens parameters
  const Double_t kLensZ[3][2] = {{-184.0*mm, -174.0*mm},
                                 {  -5.0*mm,    4.5*mm},
                                 { 174.0*mm,  184.0*mm}};
  const Double_t kLensR[3] = {-10739.5*mm, 29819.0*mm, -11148.2*mm};
  const Double_t kLensRadius[3][2] = {{590*mm,  85*mm},
                                      {500*mm, 140*mm},
                                      {590*mm, 205*mm}};
  const Double_t kLensPol[3][4] = {{0,
                                    2.50657e-10*TMath::Power(mm, -3),
                                    -3.26591e-16*TMath::Power(mm, -5),
                                    -2.30099e-22*TMath::Power(mm, -7)},
                                   {0,
                                    -3.16481e-10*TMath::Power(mm, -3),
                                    6.23315e-16*TMath::Power(mm, -5),
                                    8.71537e-22*TMath::Power(mm, -7)},
                                   {0,
                                    2.48098e-10*TMath::Power(mm, -3),
                                    -3.44020e-16*TMath::Power(mm, -5),
                                    -1.99402e-22*TMath::Power(mm, -7)}};

  // Acrylite data measured with V-Block method
  ASchottFormula* acrylite = new ASchottFormula(+2.192728e-0, -5.320698e-3,
                                                +7.215869e-3, +1.657987e-3,
                                                -2.122694e-4, +1.173515e-5);

  AGeoAsphericDisk* a[3];
  ALens* lens[3];
  for(Int_t i=0; i<3; i++){
    a[i] = new AGeoAsphericDisk(Form("lens_a%d", i+1), kLensZ[i][0], 0,
                                kLensZ[i][1], 1/kLensR[i],
                                kLensRadius[i][0], kLensRadius[i][1]);
    a[i]->SetPolynomials(0, 0, 4, &kLensPol[i][0]);
    lens[i] = new ALens(Form("lens%d", i+1), a[i]);
    lens[i]->SetRefractiveIndex(acrylite);
    opt->AddNode(lens[i], 1);
  } // i

  TGeoBBox* box1 = new TGeoBBox("lens_box1", 610*mm, 610*mm, 5*mm);
  TGeoTube* tube1 = new TGeoTube("lens_tube1", 0*mm, 590*mm, 5.1*mm);
  TGeoTube* tube2 = new TGeoTube("lens_tube2", 0*mm, 500*mm, 5.1*mm);
  TGeoCompositeShape* composite1 = new TGeoCompositeShape("lens_cs1", "lens_box1-lens_tube1");
  TGeoCompositeShape* composite2 = new TGeoCompositeShape("lens_cs2", "lens_box1-lens_tube2");
  AObscuration* obs1 = new AObscuration("lens_obs1", composite1);
  AObscuration* obs2 = new AObscuration("lens_obs2", composite2);
  opt->AddNode(obs1, 1, new TGeoTranslation(0, 0, -179*mm));
  opt->AddNode(obs2, 1);
  opt->AddNode(obs1, 2, new TGeoTranslation(0, 0, 179*mm));
}

//_____________________________________________________________________________
void AddPipeline(AOpticalComponent* opt)
{
  // Haube parameters (ref. 20Haube5-1.dwg)
  const Double_t kOffset = 714*mm - 20.35*mm + kInputZ;
  const Int_t kHaubeN = 14;
  const Double_t kHaubeZ[kHaubeN] = {  0*mm,   4*mm,   4*mm, 194*mm, 194*mm,
                                       202*mm, 202*mm, 298*mm, 613*mm, 717*mm,
                                       717*mm, 720*mm, 720*mm, 730*mm};
  const Double_t kHaubeRmax[kHaubeN] = {320*mm, 320*mm, 315*mm, 315*mm, 315*mm,
                                        315*mm, 291*mm, 291*mm, 119*mm, 119*mm,
                                        119*mm, 119*mm,  93*mm,  93*mm};
  const Double_t kHaubeRmin[kHaubeN] = {310*mm, 310*mm, 310*mm, 310*mm, 287*mm,
                                        287*mm, 287*mm, 287*mm, 115*mm, 115*mm,
                                        0*mm,     0*mm,   0*mm,   0*mm};

  TGeoPcon* pcon1 = new TGeoPcon("pip_pcon1", 0, 360, kHaubeN);
  for(Int_t i=0; i<kHaubeN; i++){
    pcon1->DefineSection(i, kOffset-kHaubeZ[i], kHaubeRmin[i], kHaubeRmax[i]);
  } // i
  AObscuration* haube = new AObscuration("haube", pcon1);
  opt->AddNode(haube, 1);
  /*
    const Int_t kPipeN = 14;
    const Double_t kHaubeZ[kHaubeN] = {};
    const Double_t kHaubeZ[kHaubeN] = {-516.94375*mm, -206.943475*mm,
    -206.943475*mm, 103.056525*mm,
    103.056525*mm, 405.056525*mm,
    479.056525*mm, 703.056525*mm};
    const Double_t kHaubeRmax[kHaubeN] = {50*mm, 50*mm, 81*mm, 81*mm,
    128*mm, 227*mm, 315*mm, 320*mm};
    const Double_t kHaubeRmin[kHaubeN] = { 0*mm,  0*mm,  0*mm, 0*mm,
    0*mm, 0*mm, 0*mm, 319*mm};
  */

  // Stewart
  TGeoBBox* box1 = new TGeoBBox("pip_box1", 20*mm, 14*mm, 10*mm);

  TGeoArb8* arb1 = new TGeoArb8("pip_arb1", 10*mm+1*nm);
  arb1->SetVertex(0, 20*mm+1*nm,  8.0936*mm);
  arb1->SetVertex(1, 20*mm+1*nm, -14*mm-1*nm);
  arb1->SetVertex(2, 20*mm+1*nm, -14*mm-1*nm);
  arb1->SetVertex(3, 4.66*mm, -14*mm-1*nm);
  arb1->SetVertex(4, 20*mm+1*nm, -6.7987*mm);
  arb1->SetVertex(5, 20*mm+1*nm, -14*mm-1*nm);
  arb1->SetVertex(6, 20*mm+1*nm, -14*mm-1*nm);
  arb1->SetVertex(7, 15*mm, -14*mm-1*nm);

  TGeoArb8* arb2 = new TGeoArb8("pip_arb2", 10*mm+1*nm);
  arb2->SetVertex(0, -20*mm-1*nm,  8.0936*mm);
  arb2->SetVertex(1, -4.66*mm, -14*mm-1*nm);
  arb2->SetVertex(2, -20*mm-1*nm, -14*mm-1*nm);
  arb2->SetVertex(3, -20*mm-1*nm, -14*mm-1*nm);
  arb2->SetVertex(4, -20*mm-1*nm, -6.7987*mm);
  arb2->SetVertex(5, -15*mm, -14*mm-1*nm);
  arb2->SetVertex(6, -20*mm-1*nm, -14*mm-1*nm);
  arb2->SetVertex(7, -20*mm-1*nm, -14*mm-1*nm);

  TGeoArb8* arb3 = new TGeoArb8("pip_arb3", 10*mm+1*nm);
  arb3->SetVertex(0, -20*mm-1*nm, 14*mm+1*nm);
  arb3->SetVertex(1, -14.0757*mm, 14*mm+1*nm);
  arb3->SetVertex(2, -14.0757*mm, -14*mm-1*nm);
  arb3->SetVertex(3, -20*mm-1*nm, -14*mm-1*nm);
  arb3->SetVertex(4, -20*mm-1*nm, 14*mm+1*nm);
  arb3->SetVertex(5, -20*mm-1*nm, 14*mm+1*nm);
  arb3->SetVertex(6, -20*mm-1*nm, -14*mm-1*nm);
  arb3->SetVertex(7, -20*mm-1*nm, -14*mm-1*nm);

  TGeoArb8* arb4 = new TGeoArb8("pip_arb4", 10*mm+1*nm);
  arb4->SetVertex(0, 20*mm+1*nm, 14*mm+1*nm);
  arb4->SetVertex(1, 20*mm+1*nm, -14*mm-1*nm);
  arb4->SetVertex(2, 14.0757*mm, -14*mm-1*nm);
  arb4->SetVertex(3, 14.0757*mm, 14*mm+1*nm);
  arb4->SetVertex(4, 20*mm+1*nm, 14*mm+1*nm);
  arb4->SetVertex(5, 20*mm+1*nm, -14*mm-1*nm);
  arb4->SetVertex(6, 20*mm+1*nm, -14*mm-1*nm);
  arb4->SetVertex(7, 20*mm+1*nm, 14*mm+1*nm);

  TGeoSphere* sph1 = new TGeoSphere("pip_sph1", 0, 4*mm);
  TGeoTranslation* tr1 = new TGeoTranslation("pip_tr1", 15*mm, -6.7987*mm, 0);
  tr1->RegisterYourself();
  TGeoTranslation* tr2 = new TGeoTranslation("pip_tr2", -15*mm, -6.7987*mm, 0);
  tr2->RegisterYourself();
  TGeoTranslation* tr3 = new TGeoTranslation("pip_tr3", -17.03785*mm, 0, 0);
  tr3->RegisterYourself();
  TGeoTranslation* tr4 = new TGeoTranslation("pip_tr4", 17.03785*mm, 0, 0);
  tr4->RegisterYourself();

  TGeoCompositeShape* composite1 = new TGeoCompositeShape("pip_cs1", "pip_box1-(pip_arb1+pip_arb2+pip_sph1:pip_tr1+pip_sph1:pip_tr2)");
  TGeoCompositeShape* composite2 = new TGeoCompositeShape("pip_cs2", "(pip_box1-(pip_arb1+pip_arb3))-(pip_sph1:pip_tr1+pip_sph1:pip_tr3)");
  TGeoCompositeShape* composite3 = new TGeoCompositeShape("pip_cs3", "(pip_box1-(pip_arb2+pip_arb4))-(pip_sph1:pip_tr2+pip_sph1:pip_tr4)");
  AObscuration* Buhin1_1 = new AObscuration("Buhin1_1", composite1);
  AObscuration* Buhin1_2 = new AObscuration("Buhin1_2", composite2);
  AObscuration* Buhin1_3 = new AObscuration("Buhin1_3", composite3);

  opt->AddNode(Buhin1_1, 1, new TGeoCombiTrans(0, 309*mm, kOffset - 212*mm, new TGeoRotation("pip_rot1", 180, 0, 0)));
  opt->AddNode(Buhin1_2, 1, new TGeoCombiTrans(-309*mm*sqrt(3)/2, -309*mm/2, kOffset - 212*mm, new TGeoRotation("pip_rot2", -60, 0, 0)));
  opt->AddNode(Buhin1_3, 1, new TGeoCombiTrans(309*mm*sqrt(3)/2, -309*mm/2, kOffset - 212*mm, new TGeoRotation("pip_rot3", 60, 0, 0)));
}

//_____________________________________________________________________________
void AddInputWindow(AOpticalComponent* opt)
{
  // Kovar glass
  ASchottFormula* kovar = new ASchottFormula(+2.176400e-0, -5.051050e-3,
                                             +1.379077e-2, -9.428276e-4,
                                             +1.304423e-4, -5.956672e-6);

  // Input window of 20" II
  TGeoSphere* sphere1 = new TGeoSphere("in_sphere1", kInputR[0], kInputR[1],
                                       0, 24);
  ALens* input = new ALens("input", sphere1);
  input->SetLineColor(42);
  input->SetRefractiveIndex(kovar);
  TGeoTranslation* tr1 = new TGeoTranslation(0, 0, kInputZ);
  opt->AddNode(input, 1, tr1);

  // Focal plane
  TGeoSphere* sphere2 = new TGeoSphere("in_sphere2", kInputR[0]-.1*mm,
                                       kInputR[0], 0, 24);
  AFocalSurface* focal = new AFocalSurface("focal", sphere2);
  opt->AddNode(focal, 1, tr1);
}

//_____________________________________________________________________________
void AddLensRing(AOpticalComponent* opt)
{
  // LR-003
  TGeoTube* tube1 = new TGeoTube("lring_tube1", 597.5*mm, 599.5*mm, 19*mm);
  TGeoTube* tube2 = new TGeoTube("lring_tube2", 597.5*mm, 607.5*mm, 1*mm);
  TGeoTranslation* tr1 = new TGeoTranslation("lring_tr1", 0, 0, 20*mm);
  tr1->RegisterYourself();

  TGeoCompositeShape* composite1 = new TGeoCompositeShape("lring_cs1", "lring_tube1+lring_tube2:lring_tr1");
  AObscuration* LR003 = new AObscuration("LR003", composite1);

  opt->AddNode(LR003, 1, new TGeoTranslation(0, 0, -206*mm));

  TGeoRotation* rot1 = new TGeoRotation("lring_rot1", 0, 180, 0);

  opt->AddNode(LR003, 2, new TGeoCombiTrans(0, 0, 206*mm, rot1));

  // LR-001
  TGeoTube* tube3 = new TGeoTube("lring_tube3", 597.5*mm, 599.5*mm, 81.5*mm);
  TGeoTube* tube4 = new TGeoTube("lring_tube4", 0, 45*mm, 700*mm);
  TGeoTranslation* tr2 = new TGeoTranslation("lring_tr2", 0, 0, 82.5*mm);
  tr2->RegisterYourself();
  TGeoTranslation* tr3 = new TGeoTranslation("lring_tr3", 0, 0, -82.5*mm);
  tr3->RegisterYourself();
  TGeoRotation* rot2 = new TGeoRotation("lring_rot2", 0, 90, 0);
  rot2->RegisterYourself();
  TGeoRotation* rot3 = new TGeoRotation("lring_rot3", 90, 90, 0);
  rot3->RegisterYourself();
  TGeoCompositeShape* composite2 = new TGeoCompositeShape("lring_cs2", "(lring_tube3+lring_tube2:lring_tr2+lring_tube2:lring_tr3)-(lring_tube4:lring_rot2+lring_tube4:lring_rot3)");
  AObscuration* LR001 = new AObscuration("LR001", composite2);

  opt->AddNode(LR001, 1, new TGeoTranslation(0, 0, 89.5*mm));
  opt->AddNode(LR001, 2, new TGeoTranslation(0, 0, -89.5*mm));
}

//_____________________________________________________________________________
void AddLensFrame(AOpticalComponent* opt)
{
  // LF-150
  TGeoBBox* box1 = new TGeoBBox("lframe_box1", 662.5*mm, 662.5*mm, 300*mm);
  TGeoBBox* box2 = new TGeoBBox("lframe_box2", 657.5*mm, 657.5*mm, 295*mm);
  TGeoBBox* box3 = new TGeoBBox("lframe_box3", 662.6*mm, 612.5*mm, 250*mm);
  TGeoBBox* box4 = new TGeoBBox("lframe_box4", 612.5*mm, 662.6*mm, 250*mm);
  TGeoBBox* box5 = new TGeoBBox("lframe_box5", 612.6*mm, 612.6*mm, 300.1*mm);
  TGeoCompositeShape* composite1 = new TGeoCompositeShape("lframe_cs1", "lframe_box1-(lframe_box2+lframe_box3+lframe_box4+lframe_box5)");

  AObscuration* LF150 = new AObscuration("LF150", composite1);

  opt->AddNode(LF150, 1, new TGeoTranslation(0, 0, 25*mm));

  //LF-151
  Double_t r2 = TMath::Sqrt2();
  TGeoBBox* box6 = new TGeoBBox("lframe_box6", 340*mm, 50*mm, 25*mm);
  TGeoBBox* box7 = new TGeoBBox("lframe_box7", 340*mm, 45*mm, 22.51*mm);
  TGeoBBox* box8 = new TGeoBBox("lframe_box8", 340/r2*mm, 340/r2*mm, 30*mm);
  TGeoTranslation* tr1 = new TGeoTranslation("lframe_tr1", 0, 0, 2.5*mm);
  tr1->RegisterYourself();
  TGeoTranslation* tr2 = new TGeoTranslation("lframe_tr2", 50/r2*mm, 50/r2*mm, 0);
  tr2->RegisterYourself();
  TGeoRotation* rot1 = new TGeoRotation("lframe_rot1", -45, 0, 0);
  rot1->RegisterYourself();
  TGeoCompositeShape* composite2 = new TGeoCompositeShape("lframe_cs2", "((lframe_box6-lframe_box7:lframe_tr1):lframe_rot1)*(lframe_box8:lframe_tr2)");
  AObscuration* LF151 = new AObscuration("LF151", composite2);
  TGeoRotation* rot2 = new TGeoRotation("lframe_rot2", 90, 0, 0);
  TGeoRotation* rot3 = new TGeoRotation("lframe_rot3", 180, 0, 0);
  TGeoRotation* rot4 = new TGeoRotation("lframe_rot4", 270, 0, 0);

  opt->AddNode(LF151, 1, new TGeoCombiTrans(-(657.5-145*r2)*mm, -(657.5-145*r2)*mm, 295*mm, 0));
  opt->AddNode(LF151, 2, new TGeoCombiTrans(+(657.5-145*r2)*mm, -(657.5-145*r2)*mm, 295*mm, rot2));
  opt->AddNode(LF151, 3, new TGeoCombiTrans(+(657.5-145*r2)*mm, +(657.5-145*r2)*mm, 295*mm, rot3));
  opt->AddNode(LF151, 4, new TGeoCombiTrans(-(657.5-145*r2)*mm, +(657.5-145*r2)*mm, 295*mm, rot4));

  TGeoRotation* rot5 = new TGeoRotation("lframe_rot5", 0, 180, 270);
  TGeoRotation* rot6 = new TGeoRotation("lframe_rot6", 90, 180, 270);
  TGeoRotation* rot7 = new TGeoRotation("lframe_rot7", 180, 180, 270);
  TGeoRotation* rot8 = new TGeoRotation("lframe_rot8", 270, 180, 270);

  opt->AddNode(LF151, 5, new TGeoCombiTrans(-(657.5-145*r2)*mm, -(657.5-145*r2)*mm, -300*mm, rot5));
  opt->AddNode(LF151, 6, new TGeoCombiTrans(+(657.5-145*r2)*mm, -(657.5-145*r2)*mm, -300*mm, rot6));
  opt->AddNode(LF151, 7, new TGeoCombiTrans(+(657.5-145*r2)*mm, +(657.5-145*r2)*mm, -300*mm, rot7));
  opt->AddNode(LF151, 8, new TGeoCombiTrans(-(657.5-145*r2)*mm, +(657.5-145*r2)*mm, -300*mm, rot8));

}

//_____________________________________________________________________________
void AddStewart(AOpticalComponent* opt)
{
  // M8 nut
  TGeoPgon* pgon1 = new TGeoPgon("st_pgon1", 0, 360, 6, 2);
  pgon1->DefineSection(0, -3.75*mm, 0, 6.5*2/sqrt(3)*mm);
  pgon1->DefineSection(1, 3.75*mm, 0, 6.5*2/sqrt(3)*mm);

  // SW-006
  TGeoPgon* pgon2 = new TGeoPgon("st_pgon2", 0, 360, 6, 2);
  pgon2->DefineSection(0, -50*mm, 0, 6.5*2/sqrt(3)*mm);
  pgon2->DefineSection(1,  50*mm, 0, 6.5*2/sqrt(3)*mm);

  // SW-002
  TGeoTube* tube1 = new TGeoTube("st_tube1", 0, 4*mm, 135*mm);
  TGeoTranslation* tr1 = new TGeoTranslation("st_tr1", 0, 0, 12.5*mm);
  tr1->RegisterYourself();
  TGeoCompositeShape* composite1 = new TGeoCompositeShape("st_cs1", "st_tube1+st_pgon1:st_tr1");

  // SW-001
  TGeoTube* tube2 = new TGeoTube("st_tube2", 0, 4*mm, 205*mm);
  TGeoTranslation* tr2 = new TGeoTranslation("st_tr2", 0, 0, 97.5*mm);
  tr2->RegisterYourself();
  TGeoCompositeShape* composite2 = new TGeoCompositeShape("st_cs2", "st_tube2+st_pgon1:st_tr2");

  // SW-004
  TGeoPcon* pcon1 = new TGeoPcon("st_pcon1", 0, 360, 10);
  pcon1->DefineSection(0, -21*mm, 0, 4*mm);
  pcon1->DefineSection(1, -16*mm, 0, 4*mm);
  pcon1->DefineSection(2, -16*mm, 0, 6*mm);
  pcon1->DefineSection(3, -11.6*mm, 0, 6*mm);
  pcon1->DefineSection(4, -11.6*mm, 0, 12.679*mm);
  pcon1->DefineSection(5, 0*mm, 0, 12.679*mm);
  pcon1->DefineSection(6, 15.585*mm, 0, 6.252*mm);
  pcon1->DefineSection(7, 27*mm, 0, 6.252*mm);
  pcon1->DefineSection(8, 27*mm, 0, 8*mm);
  pcon1->DefineSection(9, 32*mm, 0, 8*mm);

  TGeoRotation* rot1 = new TGeoRotation("st_rot1", 0, 180, 0);
  rot1->RegisterYourself();
  TGeoTranslation* tr3 = new TGeoTranslation("st_tr3", 0, 0, -519.92*mm);
  tr3->RegisterYourself();
  TGeoTranslation* tr4 = new TGeoTranslation("st_tr4", 0, 0, -82*mm);
  tr4->RegisterYourself();
  TGeoTranslation* tr5 = new TGeoTranslation("st_tr5", 0, 0, -296.92*mm);
  tr5->RegisterYourself();
  TGeoTranslation* tr6 = new TGeoTranslation("st_tr6", 0, 0, -198.17*mm);
  tr6->RegisterYourself();
  TGeoTranslation* tr7 = new TGeoTranslation("st_tr7", 0, 0, -484.17*mm);
  tr7->RegisterYourself();

  TGeoCompositeShape* composite3 = new TGeoCompositeShape("st_cs3", "st_pcon1:st_tr3+st_pcon1:st_rot1+st_pgon2:st_tr4+st_tube2:st_tr5+st_pgon1:st_tr6+st_pgon1:st_tr7");
  AObscuration* BE = new AObscuration("BE", composite3);
  BE->SetLineColor(16);
  opt->AddNode(BE, 1, new TGeoCombiTrans(-550*mm, 303*mm, 348.5*mm, new TGeoRotation("st_rot2", 32, -104.8, 0)));
  opt->AddNode(BE, 2, new TGeoCombiTrans(550*mm, 303*mm, 348.5*mm, new TGeoRotation("st_rot3", -32, -104.8, 0)));

  TGeoTranslation* tr8 = new TGeoTranslation("st_tr8", 0, 0, -379.28*mm);
  tr8->RegisterYourself();
  TGeoTranslation* tr9 = new TGeoTranslation("st_tr9", 0, 0, -226.28*mm);
  tr9->RegisterYourself();
  TGeoTranslation* tr10 = new TGeoTranslation("st_tr10", 0, 0, -217.53*mm);
  tr10->RegisterYourself();
  TGeoTranslation* tr11 = new TGeoTranslation("st_tr11", 0, 0, -343.53*mm);
  tr11->RegisterYourself();

  TGeoCompositeShape* composite4 = new TGeoCompositeShape("st_cs4", "st_pcon1:st_tr8+st_pcon1:st_rot1+st_pgon2:st_tr4+st_tube1:st_tr9+st_pgon1:st_tr10+st_pgon1:st_tr11");
  AObscuration* CD = new AObscuration("CD", composite4);
  CD->SetLineColor(16);
  opt->AddNode(CD, 1, new TGeoCombiTrans(-304*mm, 551*mm, 349*mm, new TGeoRotation("st_rot4", 51, -109.5, 0)));
  opt->AddNode(CD, 2, new TGeoCombiTrans(304*mm, 551*mm, 349*mm, new TGeoRotation("st_rot5", -51, -109.5, 0)));

  TGeoTranslation* tr12 = new TGeoTranslation("st_tr12", 0, 0, -372.49*mm);
  tr12->RegisterYourself();
  TGeoTranslation* tr13 = new TGeoTranslation("st_tr13", 0, 0, -219.49*mm);
  tr13->RegisterYourself();
  TGeoTranslation* tr14 = new TGeoTranslation("st_tr14", 0, 0, -210.74*mm);
  tr14->RegisterYourself();
  TGeoTranslation* tr15 = new TGeoTranslation("st_tr15", 0, 0, -336.74*mm);
  tr15->RegisterYourself();

  TGeoCompositeShape* composite5 = new TGeoCompositeShape("st_cs5", "st_pcon1:st_tr12+st_pcon1:st_rot1+st_pgon2:st_tr4+st_tube1:st_tr13+st_pgon1:st_tr14+st_pgon1:st_tr15");
  AObscuration* AF = new AObscuration("AF", composite5);
  AF->SetLineColor(16);
  opt->AddNode(AF, 1, new TGeoCombiTrans(-334*mm, -530*mm, 350*mm, new TGeoRotation("st_rot6", -11, 110, 0)));
  opt->AddNode(AF, 2, new TGeoCombiTrans(334*mm, -530*mm, 350*mm, new TGeoRotation("st_rot7", 11, 110, 0)));

  // SW-003
  TGeoTube* tube3 = new TGeoTube("st_tube3", 0, 30*mm, 40*mm);
  TGeoArb8* arb1 = new TGeoArb8("st_arb1", 15*mm+1*nm);
  arb1->SetVertex(0, 30*mm,  30*mm);
  arb1->SetVertex(1, 30*mm, -30*mm);
  arb1->SetVertex(2, 30*mm, -30*mm);
  arb1->SetVertex(3, 30*mm,  30*mm);
  arb1->SetVertex(4, 30*mm,  30*mm);
  arb1->SetVertex(5, 30*mm, -30*mm);
  arb1->SetVertex(6, 14.5*mm, -30*mm);
  arb1->SetVertex(7, 14.5*mm, 30*mm);
  TGeoTranslation* tr16 = new TGeoTranslation("st_tr16", 0, 0, 25*mm);
  tr16->RegisterYourself();
  TGeoSphere* sph1 = new TGeoSphere("st_sph1", 0, 4*mm);
  TGeoTranslation* tr17 = new TGeoTranslation("st_tr17", (14.5+15/sqrt(30*30+15.5*15.5)*15.5)*mm, 0, (40-15/sqrt(30*30+15.5*15.5)*30)*mm);
  tr17->RegisterYourself();
  TGeoCompositeShape* composite6 = new TGeoCompositeShape("st_cs6", "st_tube3-(st_arb1:st_tr16 + st_sph1:st_tr17)");
  AObscuration* SW003 = new AObscuration("SW003", composite6);
  Double_t r2 = TMath::Sqrt2();
  Double_t x1 = (657.5 - 461.68/r2)*mm;
  Double_t x2 = (657.5 - 118.32/r2)*mm;
  Double_t x3 = (657.5 - 450.87/r2)*mm;
  Double_t y1 = x2;
  Double_t y2 = x1;
  Double_t y3 = -(657.5 - 129.13/r2)*mm;
  Double_t z = 315*mm;
  opt->AddNode(SW003, 1, new TGeoCombiTrans(x1, y1, z, new TGeoRotation("st_rot1", -135, 0, 0)));
  opt->AddNode(SW003, 2, new TGeoCombiTrans(-x1, y1, z, new TGeoRotation("st_rot2", -45, 0, 0)));
  opt->AddNode(SW003, 3, new TGeoCombiTrans(x2, y2, z, new TGeoRotation("st_rot3", -135, 0, 0)));
  opt->AddNode(SW003, 4, new TGeoCombiTrans(-x2, y2, z, new TGeoRotation("st_rot4", -45, 0, 0)));
  opt->AddNode(SW003, 5, new TGeoCombiTrans(x3, y3, z, new TGeoRotation("st_rot5", 90, 0, 0)));
  opt->AddNode(SW003, 6, new TGeoCombiTrans(-x3, y3, z, new TGeoRotation("st_rot6", 90, 0, 0)));
}

//_____________________________________________________________________________
void Add30Frame(AOpticalComponent* opt)
{
  Double_t r3 = TMath::Sqrt(3);

  // H steal
  Double_t x[12] = { 50*mm,  50*mm,   4*mm,   4*mm,  50*mm,  50*mm,
                    -50*mm, -50*mm,  -4*mm,  -4*mm, -50*mm, -50*mm,};
  Double_t y[12] = { 50*mm,  42*mm,  42*mm, -42*mm, -42*mm, -50*mm,
                    -50*mm, -42*mm, -42*mm,  42*mm,  42*mm,  50*mm};

  // ASHRA30-013.dwg ASHRA30-015.dwg ASHRA30-030.dwg
  TGeoXtru* xtru1 = new TGeoXtru(2);
  xtru1->SetName("30_xtru1");
  xtru1->DefinePolygon(12, x, y);
  xtru1->DefineSection(0, -594.5*mm);
  xtru1->DefineSection(1, 594.5*mm);
  TGeoBBox* box1 = new TGeoBBox("30_box1", 50*mm, 50*mm, 4.5*mm);
  TGeoTranslation* tr1 = new TGeoTranslation("30_tr1", 0, 0, 599*mm);
  tr1->RegisterYourself();
  TGeoTranslation* tr2 = new TGeoTranslation("30_tr2", 0, 0, -599*mm);
  tr2->RegisterYourself();
  TGeoCompositeShape* composite1 = new TGeoCompositeShape("30_cs1", "30_xtru1+30_box1:30_tr1+30_box1:30_tr2");
  AObscuration* F30_015 = new AObscuration("F30_015", composite1);
  TGeoRotation* rot1 = new TGeoRotation("30_rot1", 90, 90, 0);
  opt->AddNode(F30_015, 1, new TGeoCombiTrans(0, 662.5*mm, 375*mm, rot1));
  opt->AddNode(F30_015, 2, new TGeoCombiTrans(0, -662.5*mm, 375*mm, rot1));

  // ASHRA30-0083.dwg
  TGeoXtru* xtru2 = new TGeoXtru(2);
  xtru2->SetName("30_xtru2");
  xtru2->DefinePolygon(12, x, y);
  xtru2->DefineSection(0, -1482.5*mm);
  xtru2->DefineSection(1, (739.5+100/r3)*mm);
  TGeoBBox* box2 = new TGeoBBox("30_box2", 4.5*mm, 50*mm, 50*mm);
  TGeoTranslation* tr3 = new TGeoTranslation("30_tr3", 54.5*mm, 0, 662.5*mm);
  tr3->RegisterYourself();
  TGeoTranslation* tr4 = new TGeoTranslation("30_tr4", 54.5*mm, 0, -662.5*mm);
  tr4->RegisterYourself();

  TGeoArb8* arb1 = new TGeoArb8("30_arb1", 50/r3*mm+1*nm);
  arb1->SetVertex(0, 50*mm+1*nm, 50*mm+1*nm);
  arb1->SetVertex(1, 50*mm+1*nm, 50*mm+1*nm);
  arb1->SetVertex(2, -50*mm-1*nm, 50*mm+1*nm);
  arb1->SetVertex(3, -50*mm-1*nm, 50*mm+1*nm);
  arb1->SetVertex(4, 50*mm+1*nm, 50*mm+1*nm);
  arb1->SetVertex(5, 50*mm+1*nm, -50*mm-1*nm);
  arb1->SetVertex(6, -50*mm-1*nm, -50*mm-1*nm);
  arb1->SetVertex(7, -50*mm-1*nm, 50*mm+1*nm);

  TGeoArb8* arb2 = new TGeoArb8("30_arb2", 50/r3*mm+1*nm);
  arb2->SetVertex(0, 50*mm+1*nm, 50*mm+1*nm);
  arb2->SetVertex(1, 50*mm+1*nm, -50*mm-1*nm);
  arb2->SetVertex(2, -50*mm-1*nm, -50*mm-1*nm);
  arb2->SetVertex(3, -50*mm-1*nm, 50*mm+1*nm);
  arb2->SetVertex(4, 50*mm+1*nm, -50*mm-1*nm);
  arb2->SetVertex(5, 50*mm+1*nm, -50*mm-1*nm);
  arb2->SetVertex(6, -50*mm-1*nm, -50*mm-1*nm);
  arb2->SetVertex(7, -50*mm-1*nm, -50*mm-1*nm);

  TGeoBBox* box3 = new TGeoBBox("30_box3", 50*mm, 150*mm, 4.5*mm);

  TGeoTranslation* tr5 = new TGeoTranslation("30_tr5", 0, 0, (739.5+50/r3)*mm);
  tr5->RegisterYourself();
  TGeoTranslation* tr6 = new TGeoTranslation("30_tr6", 0, 0, (-1482.5+50/r3)*mm);
  tr6->RegisterYourself();
  TGeoRotation* rot2 = new TGeoRotation("30_rot2", 0, -30, 0);
  TGeoCombiTrans* cm1 = new TGeoCombiTrans("30_cm1", 0, (-150*r3/2+50+4.5/2)*mm, (739.5+150/2.+4.5*r3/2)*mm, rot2);
  cm1->RegisterYourself();
  TGeoCombiTrans* cm2 = new TGeoCombiTrans("30_cm2", 0, (-145*r3/2+50-4.5/2)*mm, (-1482.5+145/2.-4.5*r3/2)*mm, rot2);
  cm2->RegisterYourself();

  TGeoCompositeShape* composite2 = new TGeoCompositeShape("30_cs2", "((30_xtru2+30_box2:30_tr3+30_box2:30_tr4)-(30_arb1:30_tr5+30_arb2:30_tr6))+30_box3:30_cm1+30_box3:30_cm2");
  AObscuration* F30_0083 = new AObscuration("F30_0083", composite2);
  TGeoRotation* rot3 = new TGeoRotation("30_rot3", 0, -90, 0);
  opt->AddNode(F30_0083, 1, new TGeoCombiTrans(-662.5*mm, 0, 375*mm, rot3));

  // ASHRA30-0072.dwg
  // triangle plates are not implemented
  TGeoTranslation* tr7 = new TGeoTranslation("30_tr7", -54.5*mm, 0, 662.5*mm);
  tr7->RegisterYourself();
  TGeoTranslation* tr8 = new TGeoTranslation("30_tr8", -54.5*mm, 0, -662.5*mm);
  tr8->RegisterYourself();
  TGeoCompositeShape* composite3 = new TGeoCompositeShape("30_cs3", "((30_xtru2+30_box2:30_tr7+30_box2:30_tr8)-(30_arb1:30_tr5+30_arb2:30_tr6))+30_box3:30_cm1+30_box3:30_cm2");
  AObscuration* F30_0072 = new AObscuration("F30_0072", composite3);
  opt->AddNode(F30_0072, 1, new TGeoCombiTrans(662.5*mm, 0, 375*mm, rot3));

  TGeoBBox* box4 = new TGeoBBox("30_box4", 1.9*m, 1.9*m, 1.9*m);
  AOpticalComponent* comp = new AOpticalComponent("comp", box4);

  // ASHRA30-001.dwg ASHRA30-002.dwg
  TGeoXtru* xtru3 = new TGeoXtru(2);
  xtru3->SetName("30_xtru3");
  xtru3->DefinePolygon(12, x, y);
  xtru3->DefineSection(0, -603.5*mm);
  xtru3->DefineSection(1, 603.5*mm);
  TGeoTranslation* tr9 = new TGeoTranslation("30_tr9", 0, 0, 608*mm);
  tr9->RegisterYourself();
  TGeoTranslation* tr10 = new TGeoTranslation("30_tr10", 0, 0, -608*mm);
  tr10->RegisterYourself();
  TGeoCompositeShape* composite4 = new TGeoCompositeShape("30_cs4", "30_xtru3+30_box1:30_tr9+30_box1:30_tr10");
  AObscuration* F30_001 = new AObscuration("F30_001", composite4);

  TGeoRotation* rot4 = new TGeoRotation("30_rot4", 90, 90, 90);
  comp->AddNode(F30_001, 1, new TGeoCombiTrans(0, -2050*mm, -1116*mm, rot1));
  comp->AddNode(F30_001, 2, new TGeoCombiTrans(0, -2050*mm, 225*mm, rot4));

  comp->AddNode(F30_015, 3, new TGeoCombiTrans(0, -362*mm, 1214*mm, rot1));
  comp->AddNode(F30_015, 4, new TGeoCombiTrans(0, -2050*mm, 1214*mm, rot1));

  // ASHRA30-009.dwg ASHRA30-010.dwg
  TGeoXtru* xtru4 = new TGeoXtru(2);
  xtru4->SetName("30_xtru4");
  xtru4->DefinePolygon(12, x, y);
  xtru4->DefineSection(0, -1160.5*mm);
  xtru4->DefineSection(1, 1160.5*mm);
  TGeoTranslation* tr11 = new TGeoTranslation("30_tr11", 0, 0, 1165*mm);
  tr11->RegisterYourself();
  TGeoCombiTrans* cm3 = new TGeoCombiTrans("30_cm3", 54.5*mm, 0, -1010.5*mm, rot1);
  cm3->RegisterYourself();
  TGeoCompositeShape* composite5 = new TGeoCompositeShape("30_cs5", "30_xtru4+30_box1:30_tr11+30_box3:30_cm3");
  AObscuration* F30_009 = new AObscuration("F30_009", composite5);
  TGeoRotation* rot5 = new TGeoRotation("30_rot5", 90, 0, 0);
  comp->AddNode(F30_009, 1, new TGeoCombiTrans(662.5*mm, -2050*mm, -5.5*mm, rot5));
  comp->AddNode(F30_009, 2, new TGeoCombiTrans(-662.5*mm, -2050*mm, -5.5*mm, rot5));

  // ASHRA30-004.dwg
  TGeoXtru* xtru5 = new TGeoXtru(2);
  xtru5->SetName("30_xtru5");
  xtru5->DefinePolygon(12, x, y);
  xtru5->DefineSection(0, -602.5*mm);
  xtru5->DefineSection(1, 602.5*mm);
  TGeoTranslation* tr12 = new TGeoTranslation("30_tr12", 0, 0, 607*mm);
  tr12->RegisterYourself();
  TGeoCompositeShape* composite6 = new TGeoCompositeShape("30_cs6", "30_xtru5+30_box1:30_tr12");
  AObscuration* F30_004 = new AObscuration("F30_004", composite6);
  comp->AddNode(F30_004, 1, new TGeoCombiTrans(662.5*mm, 0, 552.5*mm, 0));
  comp->AddNode(F30_004, 2, new TGeoCombiTrans(-662.5*mm, 0, 552.5*mm, 0));

  // ASHRA30-010.dwg ASHRA30-011.dwg
  TGeoXtru* xtru6 = new TGeoXtru(2);
  xtru6->SetName("30_xtru6");
  xtru6->DefinePolygon(12, x, y);
  xtru6->DefineSection(0, -1075*mm);
  xtru6->DefineSection(1, 1075*mm);
  TGeoCombiTrans* cm4 = new TGeoCombiTrans("30_cm4", 54.5*mm, 0, 663*mm, rot4);
  cm4->RegisterYourself();
  TGeoCombiTrans* cm5 = new TGeoCombiTrans("30_cm5", 54.5*mm, 0, -1025*mm, rot4);
  cm5->RegisterYourself();

  TGeoCompositeShape* composite7 = new TGeoCompositeShape("30_cs7", "30_xtru6+30_box1:30_cm4+30_box1:30_cm5");
  AObscuration* F30_010 = new AObscuration("F30_010", composite7);
  TGeoRotation* rot6 = new TGeoRotation("30_rot6", 180, 90, 0);
  comp->AddNode(F30_010, 1, new TGeoCombiTrans(-662.5*mm, -1025*mm, 1214*mm, rot3));
  comp->AddNode(F30_010, 2, new TGeoCombiTrans(662.5*mm, -1025*mm, 1214*mm, rot6));

  // C channel
  Double_t x2[8] = {   75*mm,    75*mm, 68.5*mm, 68.5*mm,
                    -68.5*mm, -68.5*mm,  -75*mm,  -75*mm};
  Double_t y2[8] = {37.5*mm, -37.5*mm, -37.5*mm, 31*mm,
                    31*mm, -37.5*mm, -37.5*mm, 37.5*mm};

  // ASHRA30-0054.dwg ASHRA30-0064.dwg ASHRA30-0121.dwg
  // ASHRA30-001.dwg ASHRA30-0021.dwg
  // 100x150 and 100x210 plates at the ends of 0054/0056 are not implemented
  TGeoXtru* xtru7 = new TGeoXtru(2);
  xtru7->SetName("30_xtru7");
  xtru7->DefinePolygon(8, x2, y2);
  xtru7->DefineSection(0, -814*mm);
  xtru7->DefineSection(1, 1109*mm);

  TGeoBBox* box5 = new TGeoBBox("30_box5", 75*mm, 4.5*mm, 140*mm);

  TGeoArb8* arb3 = new TGeoArb8("30_arb3", 68.5/2*r3*mm+1*nm);
  arb3->SetVertex(0, 75*mm+1*nm, -37.5*mm-1*nm);
  arb3->SetVertex(1, 75*mm+1*nm, -37.5*mm-1*nm);
  arb3->SetVertex(2, -75*mm-1*nm, -37.5*mm-1*nm);
  arb3->SetVertex(3, -75*mm-1*nm, -37.5*mm-1*nm);
  arb3->SetVertex(4, 75*mm+1*nm, 33*mm+1*nm);
  arb3->SetVertex(5, 75*mm+1*nm, -37.5*mm-1*nm);
  arb3->SetVertex(6, -75*mm-1*nm, -37.5*mm-1*nm);
  arb3->SetVertex(7, -75*mm-1*nm, 33*mm+1*nm);

  TGeoArb8* arb4 = new TGeoArb8("30_arb4", 68.5/2/2*mm+1*nm);
  arb4->SetVertex(0, 75*mm+1*nm, 33*mm+1*nm);
  arb4->SetVertex(1, 75*mm+1*nm, -37.5*mm-1*nm);
  arb4->SetVertex(2, -75*mm-1*nm, -37.5*mm-1*nm);
  arb4->SetVertex(3, -75*mm-1*nm, 33*mm+1*nm);
  arb4->SetVertex(4, 75*mm+1*nm, -37.5*mm-1*nm);
  arb4->SetVertex(5, 75*mm+1*nm, -37.5*mm-1*nm);
  arb4->SetVertex(6, -75*mm-1*nm, -37.5*mm-1*nm);
  arb4->SetVertex(7, -75*mm-1*nm, -37.5*mm-1*nm);

  TGeoTranslation* tr13 = new TGeoTranslation("30_tr13", 0, 0, (1109-68.5*r3/2)*mm);
  tr13->RegisterYourself();
  TGeoTranslation* tr14 = new TGeoTranslation("30_tr14", 0, 0, (-814+68.5/2/2)*mm);
  tr14->RegisterYourself();
  TGeoTranslation* tr15 = new TGeoTranslation("30_tr15", 0, 54.5*mm, 533*mm);
  tr15->RegisterYourself();
  TGeoTranslation* tr16 = new TGeoTranslation("30_tr16", 0, 54.5*mm, -533*mm);
  tr16->RegisterYourself();

  TGeoCompositeShape* composite8 = new TGeoCompositeShape("30_cs8", "30_xtru7-(30_arb3:30_tr13+30_arb4:30_tr14)");

  TGeoXtru* xtru8 = new TGeoXtru(2);
  xtru8->SetName("30_xtru8");
  xtru8->DefinePolygon(8, x2, y2);
  xtru8->DefineSection(0, -267.5*mm);
  xtru8->DefineSection(1, 267.5*mm);

  TGeoArb8* arb5 = new TGeoArb8("30_arb5", 75*r3/2*mm+1*nm);
  arb5->SetVertex(0, 75*mm+1*nm, 37.5*mm+1*nm);
  arb5->SetVertex(1, 75*mm+1*nm, -37.5*mm-1*nm);
  arb5->SetVertex(2, -75*mm-1*nm, -37.5*mm-1*nm);
  arb5->SetVertex(3, -75*mm-1*nm, 37.5*mm+1*nm);
  arb5->SetVertex(4, 75*mm+1*nm, -37.5*mm-1*nm);
  arb5->SetVertex(5, 75*mm+1*nm, -37.5*mm-1*nm);
  arb5->SetVertex(6, -75*mm-1*nm, -37.5*mm-1*nm);
  arb5->SetVertex(7, -75*mm-1*nm, -37.5*mm-1*nm);
  TGeoTranslation* tr17 = new TGeoTranslation("30_tr17", 0, 0, (-267.5+75*r3/2)*mm);
  tr17->RegisterYourself();

  TGeoRotation* rot7 = new TGeoRotation("30_rot7", 0, -120, 0);
  TGeoCombiTrans* cm6 = new TGeoCombiTrans("30_cm6", 0, (-75+267.5*r3/2+75/2./2.)*mm, (-4.5-267.5/2+75*r3/2/2)*mm-1*nm, rot7);
  cm6->RegisterYourself();

  TGeoBBox* box6 = new TGeoBBox("30_box6", 150*mm, 5*mm, 150*mm);
  TGeoTranslation* tr18 = new TGeoTranslation("30_tr18", 0, (37.5+5)*mm+1*nm, (267.5-150)*mm);
  tr18->RegisterYourself();
  TGeoTranslation* tr19= new TGeoTranslation("30_tr19", 0, (37.5+5+100)*mm, (267.5-150)*mm);
  tr19->RegisterYourself();

  TGeoCompositeShape* composite9 = new TGeoCompositeShape("30_cs9", "(30_xtru8-30_arb5:30_tr17)+30_box6:30_tr18+30_box6:30_tr19");
  TGeoCompositeShape* composite10 = new TGeoCompositeShape("30_cs10", "(30_box5:30_rot3)+(30_cs9:30_cm6)");

  TGeoRotation* rot8 = new TGeoRotation("30_rot8", 0, 90, 0);
  rot8->RegisterYourself();
  TGeoRotation* rot9 = new TGeoRotation("30_rot9", 0, 90, 180);
  rot9->RegisterYourself();
  TGeoRotation* rot10 = new TGeoRotation("30_rot10", 0, 90, 60);
  rot10->RegisterYourself();
  TGeoRotation* rot11 = new TGeoRotation("30_rot11", 0, 90, 120);
  rot11->RegisterYourself();
  TGeoRotation* rot12 = new TGeoRotation("30_rot12", 0, 90, -60);
  rot12->RegisterYourself();
  TGeoRotation* rot13 = new TGeoRotation("30_rot13", 0, 90, -120);
  rot13->RegisterYourself();

  TGeoCombiTrans* cm7 = new TGeoCombiTrans("30_cm7", 0, (37.5+4.5)*mm, 533*mm, rot8);
  cm7->RegisterYourself();
  TGeoCombiTrans* cm8 = new TGeoCombiTrans("30_cm8", 0, (37.5+4.5)*mm, -533*mm, rot9);
  cm8->RegisterYourself();
  TGeoCombiTrans* cm9 = new TGeoCombiTrans("30_cm9", 0, (37.5+4.5)*mm, 533./2*mm, rot10);
  cm9->RegisterYourself();
  TGeoCombiTrans* cm10 = new TGeoCombiTrans("30_cm10", 0, (37.5+4.5)*mm, -533./2*mm, rot11);
  cm10->RegisterYourself();
  TGeoCombiTrans* cm11 = new TGeoCombiTrans("30_cm11", 0, (37.5+4.5)*mm, 533./2*mm, rot12);
  cm11->RegisterYourself();
  TGeoCombiTrans* cm12 = new TGeoCombiTrans("30_cm12", 0, (37.5+4.5)*mm, -533./2*mm, rot13);
  cm12->RegisterYourself();

  TGeoCompositeShape* composite11 = new TGeoCompositeShape("30_cs11", "30_cs8+(30_cs10:30_cm7)+(30_cs10:30_cm8)");
  TGeoCompositeShape* composite12 = new TGeoCompositeShape("30_cs12", "30_cs8+(30_cs10:30_cm11)+(30_cs10:30_cm12)");
  TGeoCompositeShape* composite13 = new TGeoCompositeShape("30_cs13", "30_cs8+(30_cs10:30_cm9)+(30_cs10:30_cm10)");

  TGeoRotation* rot14 = new TGeoRotation("30_rot14", 0, 300, 0);
  AObscuration* F30_0064 = new AObscuration("F30_0064", composite11);
  AObscuration* F30_0054a = new AObscuration("F30_0054a", composite12);
  AObscuration* F30_0054b = new AObscuration("F30_0054b", composite13);
  comp->AddNode(F30_0064, 1, new TGeoCombiTrans(0*mm, (-363+50-15+6.5/2-1109*r3/2.-37.5/2)*mm, (1214-50-9-1109/2-6.5/2*r3+37.5*r3/2)*mm, rot14));
  comp->AddNode(F30_0054a, 1, new TGeoCombiTrans(462*mm, (-363+50-15+6.5/2-1109*r3/2.-37.5/2)*mm, (1214-50-9-1109/2-6.5/2*r3+37.5*r3/2)*mm, rot14));
  comp->AddNode(F30_0054b, 1, new TGeoCombiTrans(-462*mm, (-363+50-15+6.5/2-1109*r3/2.-37.5/2)*mm, (1214-50-9-1109/2-6.5/2*r3+37.5*r3/2)*mm, rot14));

  opt->AddNodeOverlap(comp, 1, new TGeoCombiTrans(0, (739.5+50/2.+(9+50)*r3/2)*mm, (325-(50+9)/2.+50*r3/2)*mm, rot2));

}
