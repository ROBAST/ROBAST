// Author: Akira Okumura 2011/5/1

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

const Double_t kMirrorRad = 1.5*m;
const Double_t kFocalLength = 3*m;
const Double_t kMirrorSag = kMirrorRad*kMirrorRad/4./kFocalLength;
const Double_t kFocalRad = 20*cm;

AOpticsManager* MakeGeometry()
{
  // Make the geometry of a simple parabolic telescope

  AOpticsManager* manager = new AOpticsManager("manager", "SimpleParabolicTelescope");
  manager->SetNsegments(100); // Set the smoothness of surface drawing. Display use only.

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 10*m, 10*m, 10*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);


  // Define a paraboloid for the mirror
  TGeoParaboloid* para = new TGeoParaboloid("mirror_para", 0, kMirrorRad, kMirrorSag/2.);

  TGeoTranslation* mirror_tr1 = new TGeoTranslation("mirror_tr1", 0, 0, kMirrorSag/2.);
  mirror_tr1->RegisterYourself();
  TGeoTranslation* mirror_tr2 = new TGeoTranslation("mirror_tr2", 0, 0, kMirrorSag/2. - 1*um);
  mirror_tr2->RegisterYourself();

  // Composite two TGeoParaboloid to make a thin parabolic surface
  TGeoCompositeShape* mirror_comp = new TGeoCompositeShape("mirror_comp", "mirror_para:mirror_tr2 - mirror_para:mirror_tr1");

  // Make a parabolic mirror
  AMirror* mirror = new AMirror("mirror", mirror_comp);
  world->AddNode(mirror, 1);

  // Define a tube for the forcal surface
  TGeoTube* focal_tube = new TGeoTube("focal_tube", 0, kFocalRad, 10*um);

  TGeoTranslation* focal_tr = new TGeoTranslation("focal_tr", 0, 0, kFocalLength + 10*um);
  focal_tr->RegisterYourself();

  // Make a focal surface
  AFocalSurface* focal = new AFocalSurface("focal", focal_tube);
  world->AddNode(focal, 1, focal_tr);

  TGeoTube* obs_tube1 = new TGeoTube("obs_tube1", 0, kFocalRad + 10*um, 10*um);
  TGeoTranslation* obs_tr1 = new TGeoTranslation("obs_tr1", 0, 0, kFocalLength + 30*um);
  obs_tr1->RegisterYourself();

  // Make a dummy obscuration behind the focal plane
  AObscuration* obs1 = new AObscuration("obs1", obs_tube1);
  world->AddNode(obs1, 1, obs_tr1);

  TGeoTube* obs_tube2 = new TGeoTube("obs_tube2", kFocalRad, kFocalRad + 10*um, 10*um);
  TGeoTranslation* obs_tr2 = new TGeoTranslation("obs_tr2", 0, 0, kFocalLength + 10*um);
  obs_tr2->RegisterYourself();

  // Make one more obscuration surrounding the focal plane
  AObscuration* obs2 = new AObscuration("obs2", obs_tube2);
  world->AddNode(obs2, 1, obs_tr2);

  manager->CloseGeometry();

  world->Draw();

  return manager;
}

void SimpleParabolicTelescope()
{
  AOpticsManager* manager = MakeGeometry();

  const Int_t kN = 30;
  TH2D* hist[kN];

  for(Int_t i = 0; i < kN; i++){
    Double_t deg = i*0.1;
    Double_t rad = deg*TMath::DegToRad();

    hist[i] = new TH2D(Form("hist%d", i), Form("#it{#theta} = %.1f (deg);X (cm);Y (cm)", deg), 300, -3, 3, 300, -3, 3);

    TGeoTranslation* raytr = new TGeoTranslation("raytr", -kFocalLength*2*TMath::Sin(rad), 0, kFocalLength*2*TMath::Cos(rad));
    TVector3 dir;
    dir.SetMagThetaPhi(1, TMath::Pi() - rad, 0);
    Double_t lambda = 400*nm; // does not affect the results because we have no lens
    ARayArray* array = ARayShooter::Square(lambda, 5*m, 201, 0, raytr, &dir);

    manager->TraceNonSequential(*array);
    TObjArray* focused = array->GetFocused();

    // Get the mean <x> and <y>
    TH2D mean("", "", 1, -10*m, 10*m, 1, -10*m, 10*m);
    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      Double_t p[4];
      ray->GetLastPoint(p);
      mean.Fill(p[0], p[1]);
    } // j

    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      const Double_t* first = ray->GetFirstPoint();
      Double_t last[4];
      ray->GetLastPoint(last);

      Double_t x = deg*10*cm;
      hist[i]->Fill(last[0] - mean.GetMean(1), last[1] - mean.GetMean(2));

      // Draw only some selected photons in 3D
      if(((i == 0) || (i == kN - 1)) && (TMath::Abs(first[0]) < 1*cm || TMath::Abs(first[1]) < 1*cm)){
        TPolyLine3D* pol = ray->MakePolyLine3D();
        if(i == 0){
          pol->SetLineColor(3);
        }
        pol->Draw();
      } // ij
    } // if
  } // i

  TCanvas* can_spot = new TCanvas("can_spot", "can_spot", 1200, 1000);
  can_spot->Divide(6, 5, 1e-10, 1e-10);

  TLegend* leg = new TLegend(0.15, 0.6, 0.5, 0.85);
  leg->SetFillStyle(0);
  char title[3][100] = {"#sigma_{x}", "#sigma_{y}", "#sigma"};

  TGraph* gra[3];
  for(Int_t i = 0; i < 3; i++){
    gra[i] = new TGraph();
    gra[i]->SetLineStyle(i + 1);
    gra[i]->SetMarkerStyle(24 + i);
    leg->AddEntry(gra[i], title[i], "lp");
  } // i

  for(Int_t i = 0; i < kN; i++){
    Double_t deg = i*0.1;
    can_spot->cd(i + 1);
    hist[i]->Draw("colz");
    
    Double_t rmsx = hist[i]->GetRMS(1);
    Double_t rmsy = hist[i]->GetRMS(2);

    gra[0]->SetPoint(i, deg, rmsx);
    gra[1]->SetPoint(i, deg, rmsy);
    gra[2]->SetPoint(i, deg, TMath::Sqrt(rmsx*rmsx + rmsy*rmsy));
  } // i

  TCanvas* can_sigma = new TCanvas("can_sigma", "can_sigma");
  gra[2]->Draw("apl");
  gra[2]->GetXaxis()->SetTitle("Incident Angle (deg)");
  gra[2]->GetYaxis()->SetTitle("Spot Size (cm)");
  gra[0]->Draw("pl same");
  gra[1]->Draw("pl same");
  leg->Draw();
}
