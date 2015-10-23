// Author: Akira Okumura 2010/11/28

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

// An example of Schwarzschild-Couder optical system
// Parameters are taken from the OS2 configuration in
// Vassiliev, et al. (2007) Astropart. Phys. 28 10-27

// define useful units
static const Double_t cm = AOpticsManager::cm();
static const Double_t mm = AOpticsManager::mm();
static const Double_t um = AOpticsManager::um();
static const Double_t nm = AOpticsManager::nm();
static const Double_t  m = AOpticsManager::m();

void SchwarzschildCouder()
{
  AOpticsManager* manager = new AOpticsManager("manager", "SC");
  manager->SetNsegments(100);

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 30*m, 30*m, 30*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);

  // Top volume 
  TGeoBBox* topbox = new TGeoBBox("topbox", 30*m, 30*m, 30*m);
  AOpticalComponent* top = new AOpticalComponent("top", topbox);

  // Parameters in Table 2
  const Double_t kDp      = 9.40*m;   // Dp
  const Double_t kDpinner = 4.68*m;   // Dpinner
  const Double_t kFp      = 16.915*m; // Fp
  const Double_t kZs      = 9.980*m;  // Secondary position
  const Double_t kDs      = 6.61*m;   // Ds
  const Double_t kDsinner = 0.50*m;   // Dsinner
  const Double_t kFs      = -3.553*m; // Fs
  const Double_t kZf      = 7.631*m;  // Focal plane position
  const Double_t kFf      = -1.481*m; // Ff
  const Double_t kZi[6] = {TMath::Power(kFp, -1)*0.25, // Zi
                           TMath::Power(kFp, -3)*-0.189377,
                           TMath::Power(kFp, -5)*-0.604706,
                           TMath::Power(kFp, -7)*-4.21374,
                           TMath::Power(kFp, -9)*21.8275,
                           TMath::Power(kFp,-11)*-425.160};
  const Double_t kVi[6] = {TMath::Power(kFs, -1)*0.25, // Vi
                           TMath::Power(kFs, -3)*0.013625,
                           TMath::Power(kFs, -5)*-0.010453,
                           TMath::Power(kFs, -7)*0.014241,
                           TMath::Power(kFs, -9)*-0.012213,
                           TMath::Power(kFs,-11)*0.005184};
  const Double_t kYi[1] = {TMath::Power(kFf, -1)*0.25}; // Yi
  
  // Make the primary mirror
  AGeoAsphericDisk* primaryV = new AGeoAsphericDisk("primaryV", -1*um, 0, 0*m, 0, kDp/2., kDpinner/2.); // curvatures are set to infinity (0 = 1./inf)
  primaryV->SetPolynomials(6, kZi, 6, kZi);
  AMirror* primaryMirror = new AMirror("primaryMirror", primaryV);
  top->AddNode(primaryMirror, 1);

  // Make the secondary mirror
  AGeoAsphericDisk* secondaryV = new AGeoAsphericDisk("secondaryV", kZs, 0, kZs + 1*um, 0, kDs/2., 0*m); // curvatures are set to infinity (0 = 1./inf)
  secondaryV->SetPolynomials(6, kVi, 6, kVi);
  AMirror* secondaryMirror = new AMirror("secondaryMirror", secondaryV);
  top->AddNode(secondaryMirror, 1);

  // Make the focal plane
  AGeoAsphericDisk* focalV = new AGeoAsphericDisk("focalV", kZf - 1*mm, 0, kZf, 0, 10*cm*7, 0.); // curvatures are set to infinity (0 = 1./inf)
  focalV->SetPolynomials(1, kYi, 1, kYi);
  AFocalSurface* focalPlane = new AFocalSurface("focalPlane", focalV);
  top->AddNodeOverlap(focalPlane, 1);

  // Make a dummy obscuration before the focal plane
  // Probably there is a bug in AGeoAsphericDisk::DistFromOutside
  // Therefore, the radius of the obscuration must be larger than that of the
  // focal plane to avoid mis-propagated photons. To be investigated
  AGeoAsphericDisk* focalObsV = new AGeoAsphericDisk("focalObsV", focalV->CalcF1(10*cm*7) - 1*cm, 0, focalV->CalcF1(10*cm*7), 0, 10*cm*7.3, 0.);
  AObscuration* focalObs = new AObscuration("focalObs", focalObsV);
  top->AddNodeOverlap(focalObs, 1);

  world->AddNode(top, 1);

  manager->CloseGeometry();

  TCanvas* canGeometry = new TCanvas("canGeometry", "canGeometry", 800, 800);
  top->Draw();

  // Start ray-tracing
  const int kN = 8; // 0 to 7 [deg]
  TH2D* hist[kN];
  TH1D* histT[kN];
  TGraph* graAeff = new TGraph;
  graAeff->SetTitle(";Field angle (deg);Effective Area (m^{2})");
  TGraph* graRMS = new TGraph;
  graRMS->SetTitle(";Field angle (deg);2 #times max{RMS_{sagital}, RMS_{tangential}} (arcmin)");
  TGraph* graT = new TGraph;
  graT->SetTitle(";Field angle (deg);Photon propagation time spread (RMS) (ns)");

  for(Int_t n = 0; n < kN; n++){
    hist[n] = new TH2D(Form("hist%d", n), Form("#it{#theta} = %d (deg);X (arcmin);Y (arcmin)", n), 1000, -20, 20, 1000, -20, 20);
    histT[n]= new TH1D(Form("histT%d",n), Form("#it{#theta} = %d (deg);Propagation delay (ns);Entries", n), 120, -6, 6);

    Double_t deg = n;

    TGeoTranslation* raytr = new TGeoTranslation("raytr", -1.2*kZs*TMath::Sin(deg*TMath::DegToRad()), 0, 1.2*kZs*TMath::Cos(deg*TMath::DegToRad()));

    TVector3 dir;
    dir.SetMagThetaPhi(1, TMath::Pi() - deg*TMath::DegToRad(), 0);
    Double_t lambda = 400*nm; // does not affect the results because we have no lens
    // 1 photon per 0.0025 m^2
    ARayArray* array = ARayShooter::Square(lambda, 20*m, 401, 0, raytr, &dir);
    manager->TraceNonSequential(*array);
    TObjArray* focused = array->GetFocused();

    Double_t Aeff = 0.;
    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      if(!ray) continue;

      // Calculate the effective area from the number of focused photons
      Aeff += 0.0025; // 0.0025 (m^2)

      Double_t p[4];    
      ray->GetLastPoint(p);
      ray->SetLineWidth(1);
      /* uncomment here if you want to draw all photon trajectories
      TPolyLine3D* pol = ray->MakePolyLine3D();
      pol->Draw();
      */
      Double_t x = deg*10*cm;
      hist[n]->Fill((p[0] - x)/(10*cm)*60, p[1]/(10*cm)*60);
      histT[n]->Fill((p[3] - (3.2*kZs - kZf)/(TMath::C()*m))/1e-9); // ns
    } // j

    graAeff->SetPoint(graAeff->GetN(), deg, Aeff);

    Double_t rmsx = hist[n]->GetRMS(1);
    Double_t rmsy = hist[n]->GetRMS(2);
    
    graRMS->SetPoint(graRMS->GetN(), deg, (rmsx > rmsy ? rmsx : rmsy)*2);

    graT->SetPoint(graT->GetN(), deg, histT[n]->GetRMS());

    delete array;
    delete raytr;
  } // n

  TCanvas* canSpot = new TCanvas("canSpot", "canSpot", 1200, 600);
  canSpot->Divide(4, 2);

  TCanvas* canTime = new TCanvas("canTime", "canTime", 1200, 600);
  canTime->Divide(4, 2);

  for(Int_t i = 0; i < kN; i++){
    canSpot->cd(i + 1);
    hist[i]->Draw("colz");

    canTime->cd(i + 1);
    histT[i]->Draw();
  } // i

  // Figure 5 in the paper
  TCanvas* canFig5 = new TCanvas("canFig5", "canFig5", 1200, 600);
  canFig5->Divide(2, 1);
  canFig5->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  graAeff->Draw("apl");
  graAeff->SetMarkerStyle(25);
  graAeff->GetXaxis()->SetLimits(0, 7);
  graAeff->GetYaxis()->SetRangeUser(0, 60);

  // PSF is not consistent with the original paper, but the spot diagram at
  // 5 (deg) is consistent with each other by eye comparison. There may be a
  // difference between calculations of RMS in my code and the paper
  canFig5->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  graRMS->Draw("apl");
  graRMS->SetMarkerStyle(25);
  graRMS->GetXaxis()->SetLimits(0, 7);
  graRMS->GetYaxis()->SetRangeUser(0, 10);

  // Figure 10 in the paper
  // Time spread is 2 times larger in the original paper. I believe the paper
  // is wrong. You can roughly calculate the spread width by
  // Dp * sin(angle)/c ~ 2.5 (ns)
  TCanvas* canFig10 = new TCanvas("canFig10", "canFig10", 1200, 600);
  canFig10->Divide(2, 1);
  canFig10->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  graT->Draw("apl");
  graT->SetMarkerStyle(25);
  graT->GetXaxis()->SetLimits(0, 7);
  graT->GetYaxis()->SetRangeUser(0, 1.8);

  canFig10->cd(2);
  histT[5]->Draw();
}
