// Author: Akira Okumura
/*******************************************************************************
 * This script shows how to simulate a Davies-Cotton telescope by ROBAST.
 ******************************************************************************/

// define useful units
const double cm = AOpticsManager::cm();
const double mm = AOpticsManager::mm();
const double um = AOpticsManager::um();
const double nm = AOpticsManager::nm();
const double  m = AOpticsManager::m();

// optics parameters
const double kF = 16*m; // focal length
const double kMirrorR = kF*2; // the radius of curvature
const double kMirrorD = 1.2*m; // facet diameter, hexagonal, flat-to-flat
const double kMirrorT = 0.1*mm; // mirror thickness, intentionally use a very thin thickness to avoid unnecessary reflection on the edges
const double kCameraD = 2.2*m; // the camera diameter
const double kCameraBoxD = 2.5*m; // the camera box diameter
const double kCameraBoxH = 1*m; // the camera box height

void AddMirrors(AOpticalComponent* opt);
void AddCamera(AOpticalComponent* opt);
void AddMasts(AOpticalComponent* opt);
void RayTrace(AOpticsManager* manager, TCanvas* can3D);

void DaviesCotton()
{
  TThread::Initialize(); // call this first when you use the multi-thread mode

  AOpticsManager* manager = new AOpticsManager("manager", "Davies-Cotton System");
  // Ignore Fresnel reflection in the camera window
  manager->DisableFresnelReflection(kTRUE);
  // Make the OpenGL objects more smooth
  manager->SetNsegments(50);
  // Make the world of 40-m cube
  TGeoBBox* boxWorld = new TGeoBBox("boxWorld", 20*m, 20*m, 20*m);
  AOpticalComponent* world = new AOpticalComponent("world", boxWorld);
  manager->SetTopVolume(world);

  AddMirrors(world);
  AddCamera(world);
  AddMasts(world);
  manager->CloseGeometry(); // finalize the geometry construction
  manager->SetMultiThread(kTRUE); // enable multi threading
  manager->SetMaxThreads(8); // 8 threads

  TCanvas* can = new TCanvas("can3D", "can3D", 800, 800);
  world->Draw("ogl");

  RayTrace(manager, can);
}

void AddMirrors(AOpticalComponent* opt)
{
  // dummy hexagonal prism to cut a spherical mirror
  TGeoPgon* mirCut = new TGeoPgon("mirCut", 0., 360., 6, 2);
  mirCut->DefineSection(0, -100*mm, 0, kMirrorD/2.);
  mirCut->DefineSection(1,  100*mm, 0, kMirrorD/2.);

  double theta = TMath::ASin(kMirrorD/TMath::Sqrt(3)/kMirrorR)*TMath::RadToDeg();
  TGeoSphere* mirSphere = new TGeoSphere("mirSphere", kMirrorR, kMirrorR + kMirrorT, 180. - theta, 180.);
  TGeoTranslation* transZ = new TGeoTranslation("transZ", 0, 0, kMirrorR);
  transZ->RegisterYourself();
  TGeoCompositeShape* mirComposite = new TGeoCompositeShape("mirComposite", "mirSphere:transZ*mirCut");
  AMirror* mirror = new AMirror("mirror", mirComposite);

  const int kNMirror = 88;
  double dx = kMirrorD/TMath::Sqrt(3);
  double dy = kMirrorD/2.;
  double x[kNMirror] = {0, 0, 0, 0, 0, 0, 0, 0,
                        1.5*dx, 1.5*dx, 1.5*dx, 1.5*dx, 1.5*dx,
                        1.5*dx, 1.5*dx, 1.5*dx, 1.5*dx, 1.5*dx,
                        -1.5*dx, -1.5*dx, -1.5*dx, -1.5*dx, -1.5*dx,
                        -1.5*dx, -1.5*dx, -1.5*dx, -1.5*dx, -1.5*dx,
                        3*dx, 3*dx, 3*dx, 3*dx, 3*dx,
                        3*dx, 3*dx, 3*dx, 3*dx,
                        -3*dx, -3*dx, -3*dx, -3*dx, -3*dx,
                        -3*dx, -3*dx, -3*dx, -3*dx,
                        4.5*dx, 4.5*dx, 4.5*dx, 4.5*dx,
                        4.5*dx, 4.5*dx, 4.5*dx, 4.5*dx,
                        -4.5*dx, -4.5*dx, -4.5*dx, -4.5*dx,
                        -4.5*dx, -4.5*dx, -4.5*dx, -4.5*dx,
                        6*dx, 6*dx, 6*dx, 6*dx,
                        6*dx, 6*dx, 6*dx,
                        -6*dx, -6*dx, -6*dx, -6*dx,
                        -6*dx, -6*dx, -6*dx,
                        7.5*dx, 7.5*dx, 7.5*dx,
                        7.5*dx, 7.5*dx, 7.5*dx,
                        -7.5*dx, -7.5*dx, -7.5*dx,
                        -7.5*dx, -7.5*dx, -7.5*dx};

  double y[kNMirror] = {2*dy, 4*dy, 6*dy, 8*dy, -2*dy, -4*dy, -6*dy, -8*dy,
                        1*dy, 3*dy, 5*dy, 7*dy, 9*dy,
                        -1*dy, -3*dy, -5*dy, -7*dy, -9*dy,
                        1*dy, 3*dy, 5*dy, 7*dy, 9*dy,
                        -1*dy, -3*dy, -5*dy, -7*dy, -9*dy,
                        0*dy, 2*dy, 4*dy, 6*dy, 8*dy,
                        -2*dy, -4*dy, -6*dy, -8*dy,
                        0*dy, 2*dy, 4*dy, 6*dy, 8*dy,
                        -2*dy, -4*dy, -6*dy, -8*dy,
                        1*dy, 3*dy, 5*dy, 7*dy,
                        -1*dy, -3*dy, -5*dy, -7*dy,
                        1*dy, 3*dy, 5*dy, 7*dy,
                        -1*dy, -3*dy, -5*dy, -7*dy,
                        0*dy, 2*dy, 4*dy, 6*dy,
                        -2*dy, -4*dy, -6*dy,
                        0*dy, 2*dy, 4*dy, 6*dy,
                        -2*dy, -4*dy, -6*dy,
                        1*dy, 3*dy, 5*dy,
                        -1*dy, -3*dy, -5*dy,
                        1*dy, 3*dy, 5*dy,
                        -1*dy, -3*dy, -5*dy};

  for(int i = 0; i < kNMirror; i++){
    double r2d = TMath::RadToDeg();
    double r2 = TMath::Power(x[i], 2) + TMath::Power(y[i], 2);
    double z = kF - TMath::Sqrt(TMath::Power(kF, 2) - r2);

    // each mirror center is relocated from the origin (0, 0, 0) to (x, y, z)
    TGeoTranslation* trans = new TGeoTranslation(Form("mirTrans%d", i), x[i], y[i], z);

    // and is rotated to compose a DC optics
    double phi = TMath::ATan2(y[i], x[i])*r2d;
    TGeoRotation* rot = new TGeoRotation(Form("mirRot%d", i), - phi + 90., 0, 0);
    theta = TMath::ATan2(TMath::Sqrt(r2), 2*kF - z)*r2d;
    TGeoRotation* rot2 = new TGeoRotation("", phi - 90., theta, 0);
    rot->MultiplyBy(rot2, 0);
    
    // make a matrix from translation and rotation matrices
    TGeoCombiTrans* combi = new TGeoCombiTrans(*trans, *rot);

    // finally add this mirror to the world
    opt->AddNode(mirror, i + 1, combi);
  } // i
}

void AddCamera(AOpticalComponent* opt)
{
  // Make a disk focal plane
  TGeoTube* tubeCamera = new TGeoTube("tubeCamera", 0, kCameraD/2., 1*mm);
  AFocalSurface* focalPlane = new AFocalSurface("focalPlane", tubeCamera);
  opt->AddNode(focalPlane, 1, new TGeoTranslation(0, 0, kF + 1*mm));

  // Make a camera box
  TGeoBBox* boxCamera = new TGeoBBox("boxCamera", kCameraBoxD/2., kCameraBoxD/2., kCameraBoxH/2.);
  double t = 10*cm;
  TGeoBBox* boxCamera2 = new TGeoBBox("boxCamera2", kCameraBoxD/2. - t, kCameraBoxD/2. - t, kCameraBoxH/2. - t);

  TGeoTranslation* transZ1 = new TGeoTranslation("transZ1", 0, 0, kF + kCameraBoxH/2.);
  transZ1->RegisterYourself();
  TGeoTranslation* transZ2 = new TGeoTranslation("transZ2", 0, 0, kF + kCameraBoxH/2. - t - 1*mm);
  transZ2->RegisterYourself();

  TGeoCompositeShape* boxComposite = new TGeoCompositeShape("boxComposite", "boxCamera:transZ1-boxCamera2:transZ2");

  AObscuration* cameraBox = new AObscuration("cameraBox", boxComposite);
  opt->AddNode(cameraBox, 1);
}

void AddMasts(AOpticalComponent* opt)
{
  // Make four-fold symmetric masts with a diameter of 20 cm
  for(int i = 0; i < 4; i++){
    double x1 = 5*m;
    double y1 = 5*m;
    double z1 = 0*m;

    double x2 = kCameraBoxD/2. + 10*cm;
    double y2 = kCameraBoxD/2. + 10*cm;
    double z2 = kF;

    double c = TMath::Cos(TMath::PiOver2()*i);
    double s = TMath::Sin(TMath::PiOver2()*i);

    TVector3 v1(c*x1 - s*y1, s*x1 + c*y1, z1);
    TVector3 v2(c*x2 - s*y2, s*x2 + c*y2, z2);
    TGeoTube* tube;
    TGeoCombiTrans* combi;
    AGeoUtil::MakePointToPointTube(Form("mast%d", i), v1, v2, 10*cm, &tube, &combi);
    AObscuration* obs = new AObscuration(Form("obsMast%d", i), tube);
    opt->AddNode(obs, 1, combi);
  }
}

void RayTrace(AOpticsManager* manager, TCanvas* can3D)
{
  const int kNdeg = 8;
  TH2D* h2[kNdeg];
  TGraph* graph = new TGraph();
  TCanvas* can = new TCanvas("can", "can", 900, 900);
  TCanvas* can2= new TCanvas("can2", "can2", 900, 900);
  can->Divide(3, 3, 1e-10, 1e-10);

  TH2D* hMirror = new TH2D("hMirror", ";X (mm);Y (mm)", 1000, -7, 7, 1000, -7, 7);

  for(int i = 0; i < kNdeg; i++){

    double deg = i*0.5;
    TGeoTranslation raytr("raytr", -2*kF*TMath::Sin(deg*TMath::DegToRad()), 0, 2*kF*TMath::Cos(deg*TMath::DegToRad()));
    TVector3 dir;
    dir.SetMagThetaPhi(1, TMath::Pi() - deg*TMath::DegToRad(), 0);
    double lambda = 400*nm; // dummy
    ARayArray* array = ARayShooter::Square(lambda, 14*m, 401, 0, &raytr, &dir);

    manager->TraceNonSequential(*array);

    h2[i] = new TH2D("", Form("#it{#theta} = %3.1f#circ;x (mm); y (mm)", deg), 200, -40, 100, 200, -70, 70);
    TH2D tmp("", "", 100, -1e5, 1e5, 100, -1e5, 1e5);

    TObjArray* focused = array->GetFocused();

    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      Double_t p[4];
      ray->GetLastPoint(p);
      tmp.Fill(p[0], p[1]);

      if (i == 0) {
        int n = ray->FindNodeNumberStartWith("mirror");
        const double* pn = ray->GetPoint(n);
        hMirror->Fill(pn[0]/m, pn[1]/m);
      } // if

      if (i == kNdeg - 1 && gRandom->Uniform() < 0.001) {
        TPolyLine3D* pol = ray->MakePolyLine3D();
        pol->SetLineColor(2);
        can3D->cd();
        pol->Draw();
      } // if
    } // j

    double meanx = tmp.GetMean();

    for(Int_t j = 0; j <= focused->GetLast(); j++){
      ARay* ray = (ARay*)(*focused)[j];
      Double_t p[4];
      ray->GetLastPoint(p);
      h2[i]->Fill((p[0] - meanx)/mm, p[1]/mm);
    } // j

    can->cd(i + 1);
    h2[i]->Draw("colz");

    if(i == 0){
      can2->cd();
      hMirror->Draw("colz");
    } // i

    delete array;
  } // i
}

