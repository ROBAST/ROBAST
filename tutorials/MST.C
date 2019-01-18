// Author: Akira Okumura
/*******************************************************************************
 * This script shows how to simulate a MST by ROBAST.
 ******************************************************************************/

#include "TCanvas.h"
#include "TGeoBBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoPgon.h"
#include "TGeoSphere.h"
#include "TGeoTube.h"
#include "TH2.h"
#include "TThread.h"

#include "AFocalSurface.h"
#include "AGeoUtil.h"
#include "AMirror.h"
#include "AObscuration.h"
#include "AOpticsManager.h"
#include "ARayShooter.h"

// define useful units
const double km = AOpticsManager::km();
const double m = AOpticsManager::m();
const double cm = AOpticsManager::cm();
const double mm = AOpticsManager::mm();
const double um = AOpticsManager::um();
const double nm = AOpticsManager::nm();

// optics parameters
// taken from CTA-PROD4-MST-optics.cfg
const double kF = 16.00 * m;                // focal length
const double kDishShapeLength = 19.20 * m;  // dish shape length (rad. of curv)
const double kMirroF = 16.07 * m;           // mirror focal length
const double kMirrorR = kMirroF * 2;        // the radius of curvature
const double kMirrorD = 1.2 * m;            // facet diameter, hexagonal mirror

const double kMirrorT = 0.01 * mm;  // mirror thickness, intentionally use a
                                    // very thin thickness to avoid unnecessary
                                    // reflection on the edges

void AddMirrors(AOpticalComponent* opt);
void AddCamera(AOpticalComponent* opt);
void AddMasts(AOpticalComponent* opt);
void RayTrace(AOpticsManager* manager, TCanvas* can3D);

void MST() {
  TThread::Initialize();  // call this first when you use the multi-thread mode

  AOpticsManager* manager = new AOpticsManager("manager", "MST");
  // Ignore Fresnel reflection in the camera window
  manager->DisableFresnelReflection(kTRUE);
  // Make the OpenGL objects more smooth
  manager->SetNsegments(50);
  // Make the world of 40-km cube
  TGeoBBox* boxWorld = new TGeoBBox("boxWorld", 20 * km, 20 * km, 20 * km);
  AOpticalComponent* world = new AOpticalComponent("world", boxWorld);
  TGeoBBox* boxSmallWorld =
      new TGeoBBox("boxSmallWorld", 20 * m, 20 * m, 20 * m);
  AOpticalComponent* small_world =
      new AOpticalComponent("small_world", boxSmallWorld);
  manager->SetTopVolume(world);
  world->AddNode(small_world, 1);

  AddMirrors(small_world);
  AddCamera(small_world);
  manager->CloseGeometry();  // finalize the geometry construction
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 2, 0)
  manager->SetMultiThread(kTRUE);  // enable multi threading
#endif
  manager->SetMaxThreads(8);  // 8 threads

  TCanvas* can = new TCanvas("can3D", "can3D", 800, 800);
  small_world->Draw("ogl");

  RayTrace(manager, can);
}

void AddMirrors(AOpticalComponent* opt) {
  double theta =
      TMath::ASin(kMirrorD / TMath::Sqrt(3) / kMirrorR) * TMath::RadToDeg();
  TGeoSphere* mirSphere = new TGeoSphere(
      "mirSphere", kMirrorR, kMirrorR + kMirrorT, 180. - theta, 180.);

  // copied from cfg/CTA/mirror_CTA-100_1.20-86-0.04.dat
  const int kNMirror = 86;
  double xy[kNMirror][2] =
      // clang-format off
    {{-620.80,    0.00}, {-558.72,  107.53}, {-496.64,  215.05},
     {-434.56,  322.58}, {-372.48,  430.10}, {-558.72, -107.53},
     {-496.64,    0.00}, {-434.56,  107.53}, {-372.48,  215.05},
     {-310.40,  322.58}, {-248.32,  430.10}, {-186.24,  537.63},
     {-496.64, -215.05}, {-434.56, -107.53}, {-372.48,    0.00},
     {-310.40,  107.53}, {-248.32,  215.05}, {-186.24,  322.58},
     {-124.16,  430.10}, { -62.08,  537.63}, {-434.56, -322.58},
     {-372.48, -215.05}, {-310.40, -107.53}, {-248.32,    0.00},
     {-186.24,  107.53}, {-124.16,  215.05}, { -62.08,  322.58},
     {   0.00,  430.10}, {  62.08,  537.63}, {-372.48, -430.10},
     {-310.40, -322.58}, {-248.32, -215.05}, {-186.24, -107.53},
     {-124.16,    0.00}, { -62.08,  107.53}, {   0.00,  215.05},
     {  62.08,  322.58}, { 124.16,  430.10}, { 186.24,  537.63},
     {-248.32, -430.10}, {-186.24, -322.58}, {-124.16, -215.05},
     { -62.08, -107.53}, {  62.08,  107.53}, { 124.16,  215.05},
     { 186.24,  322.58}, { 248.32,  430.10}, {-186.24, -537.63},
     {-124.16, -430.10}, { -62.08, -322.58}, {   0.00, -215.05},
     {  62.08, -107.53}, { 124.16,    0.00}, { 186.24,  107.53},
     { 248.32,  215.05}, { 310.40,  322.58}, { 372.48,  430.10},
     { -62.08, -537.63}, {   0.00, -430.10}, {  62.08, -322.58},
     { 124.16, -215.05}, { 186.24, -107.53}, { 248.32,    0.00},
     { 310.40,  107.53}, { 372.48,  215.05}, { 434.56,  322.58},
     {  62.08, -537.63}, { 124.16, -430.10}, { 186.24, -322.58},
     { 248.32, -215.05}, { 310.40, -107.53}, { 372.48,    0.00},
     { 434.56,  107.53}, { 496.64,  215.05}, { 186.24, -537.63},
     { 248.32, -430.10}, { 310.40, -322.58}, { 372.48, -215.05},
     { 434.56, -107.53}, { 496.64,    0.00}, { 558.72,  107.53},
     { 372.48, -430.10}, { 434.56, -322.58}, { 496.64, -215.05},
     { 558.72, -107.53}, { 620.80,    0.00}};
  // clang-format on
  for (int i = 0; i < kNMirror; i++) {
    double x = xy[i][0] * cm;
    double y = xy[i][1] * cm;
    double r2d = TMath::RadToDeg();
    double r2 = TMath::Power(x, 2) + TMath::Power(y, 2);
    double z =
        kDishShapeLength - TMath::Sqrt(TMath::Power(kDishShapeLength, 2) - r2);

    TGeoPgon* cut = new TGeoPgon("mirCut", 0., 360., 6, 2);
    cut->DefineSection(0, -kMirrorR + 1 * m, 0, kMirrorD / 2.);
    cut->DefineSection(1, -kMirrorR - 1 * m, 0, kMirrorD / 2.);

    double phi = TMath::ATan2(y, x) * r2d;
    TGeoRotation* cutrot = new TGeoRotation(Form("cutrot%d", i), -phi, 0., 0.);
    cutrot->RegisterYourself();

    TGeoCompositeShape* mir_cs = new TGeoCompositeShape(
        Form("mir_cs%d", i), Form("mirSphere*(mirCut:cutrot%d)", i));

    AMirror* mirror = new AMirror(Form("mirror%d", i), mir_cs);

    // each mirror center is relocated from the origin (0, 0, 0) to (x, y, z)
    TGeoTranslation* trans =
        new TGeoTranslation(Form("mirTrans%d", i), x, y, z);

    // and is rotated to compose a DC optics
    theta = TMath::ATan2(TMath::Sqrt(r2), 2 * kF - z) * r2d;
    TGeoRotation* rot = new TGeoRotation("", phi - 90., theta, 0);

    // make a matrix from translation and rotation matrices
    TGeoTranslation* transZ = new TGeoTranslation(0, 0, kMirrorR);
    TGeoCombiTrans* combi = new TGeoCombiTrans(*trans, *rot);
    TGeoHMatrix* hmat = new TGeoHMatrix((*combi) * (*transZ));

    // finally add this mirror to the world
    opt->AddNode(mirror, i + 1, hmat);
  }  // i
}

void AddCamera(AOpticalComponent* opt) {
  // parameters taken from sim_telarray/cfg/hess/hess_masts.dat
  const double kCameraD = 2.5 * m;     // the camera diameter (N/A in cfg)
  const double kCameraBoxD = 3.0 * m;  // the camera box diameter
  const double kCameraBoxH = 1.5 * m;  // the camera box height (N/A in cfg)
  const double kCameraOffset = -2.56 * cm;

  // Make a disk focal plane
  TGeoTube* tubeCamera = new TGeoTube("tubeCamera", 0, kCameraD / 2., 1 * mm);
  AFocalSurface* focalPlane = new AFocalSurface("focalPlane", tubeCamera);
  opt->AddNode(focalPlane, 1,
               new TGeoTranslation(0, 0, kF + kCameraOffset + 1 * mm));

  // Make a camera box
  TGeoBBox* tubeCameraBox = new TGeoBBox("tubeCameraBox", kCameraBoxD / 2.,
                                         kCameraBoxD / 2., kCameraBoxH / 2.);
  double t = 1 * cm;
  TGeoBBox* tubeCameraBox2 =
      new TGeoBBox("tubeCameraBox2", kCameraBoxD / 2. - t, kCameraBoxD / 2. - t,
                   kCameraBoxH / 2. - t);

  TGeoTranslation* transZ1 = new TGeoTranslation(
      "transZ1", 0, 0, kF + kCameraOffset + kCameraBoxH / 2.);
  transZ1->RegisterYourself();
  TGeoTranslation* transZ2 = new TGeoTranslation(
      "transZ2", 0, 0, kF + kCameraOffset + kCameraBoxH / 2. - t - 1 * mm);
  transZ2->RegisterYourself();

  TGeoCompositeShape* boxComposite = new TGeoCompositeShape(
      "boxComposite", "tubeCameraBox:transZ1-tubeCameraBox2:transZ2");

  AObscuration* cameraBox = new AObscuration("cameraBox", boxComposite);
  opt->AddNode(cameraBox, 1);
}

void RayTrace(AOpticsManager* manager, TCanvas* can3D) {
  const int kNangle = 9;
  TH2D* h2[kNangle];
  TCanvas* can = new TCanvas("can", "can", 900, 900);
  can->Divide(3, 3, 1e-10, 1e-10);

  TGraph* graphX = new TGraph;

  for (int i = 0; i < kNangle; i++) {
    const double dist = 10 * km;
    double angle = i * 0.5;
    TGeoRotation rayrot("rayrot", 90, 180 - angle, 0);
    TGeoTranslation raytr("raytr",
                          -dist * TMath::Sin(angle * TMath::DegToRad()), 0,
                          dist * TMath::Cos(angle * TMath::DegToRad()));

    double lambda = 400 * nm;  // dummy

    // Rmax = 6.90 m so 7.50 m is large enough
    ARayArray* array = ARayShooter::RandomCone(lambda, 7.5 * m, dist, 1000000,
                                               &rayrot, &raytr);

    manager->TraceNonSequential(*array);

    TObjArray* focused = array->GetFocused();

    const double platescale = 28.65;  // (mm/deg)

    TH1D htmp("", "", 1, -1e10, 1e10);
    for (Int_t k = 0; k <= focused->GetLast(); k++) {
      ARay* ray = (ARay*)(*focused)[k];
      Double_t p[4];
      ray->GetLastPoint(p);
      htmp.Fill(p[0]);
    }

    double meanx = htmp.GetMean();
    h2[i] = new TH2D(Form("h%d", i),
                     Form("#it{#theta} = %3.1f#circ;x (mm); y (mm)", angle),
                     300, meanx / mm - 50, meanx / mm + 150, 300, -100, 100);
    h2[i]->GetXaxis()->SetNdivisions(110);
    h2[i]->GetYaxis()->SetNdivisions(110);

    for (Int_t k = 0; k <= focused->GetLast(); k++) {
      ARay* ray = (ARay*)(*focused)[k];
      Double_t p[4];
      ray->GetLastPoint(p);
      h2[i]->Fill(p[0] / mm, p[1] / mm);

      if (i == kNangle - 1 && k < 30) {
        TPolyLine3D* pol = ray->MakePolyLine3D();
        pol->SetLineColor(2);
        pol->SetLineWidth(2);
        can3D->cd();
        // pol->Draw("same");
      }  // if
    }    // k

    delete array;
    can->cd(i + 1)->SetLogz();
    h2[i]->Draw("colz");
    h2[i]->SetMaximum(h2[0]->GetEntries() / 100.);
    can->Update();

    graphX->SetPoint(graphX->GetN(), angle, meanx / mm);
  }  // i

  TCanvas* can2 = new TCanvas("can2", "can2", 800, 600);
  can2->DrawFrame(0, 0, 4, 1200, ";Field Angle (deg);<#it{x}> (mm);");
  graphX->SetMarkerStyle(20);
  graphX->Draw("p same");

  Double_t pscale = graphX->GetY()[graphX->GetN() - 1] * mm /
                    graphX->GetX()[graphX->GetN() - 1];  // plate scale

  TGraph* graphStdX = new TGraph;
  TGraph* graphStdY = new TGraph;

  for (int i = 0; i < kNangle; i++) {
    Double_t stdx = h2[i]->GetStdDev(1);
    Double_t stdy = h2[i]->GetStdDev(2);
    double angle = i * 0.5;
    graphStdX->SetPoint(i, angle, stdx * mm / pscale);
    graphStdY->SetPoint(i, angle, stdy * mm / pscale);
  }  // i

  TCanvas* can3 = new TCanvas("can3", "can3", 800, 600);
  can3->DrawFrame(0, 0, 4, 0.11, ";Field Angle (deg);Std. Dev. (deg);");
  graphStdX->Draw("p same");
  graphStdX->SetMarkerStyle(20);
  graphStdY->Draw("p same");
  graphStdY->SetMarkerStyle(21);

  TLegend* leg = new TLegend(0.15, 0.6, 0.5, 0.85);
  leg->AddEntry(graphStdX, "Std. Dev. along X", "p");
  leg->AddEntry(graphStdY, "Std. Dev. along Y", "p");
  leg->Draw();
}
