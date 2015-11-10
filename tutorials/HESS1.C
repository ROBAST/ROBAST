// Author: Akira Okumura
/*******************************************************************************
 * This script shows how to simulate a HESS CT3 by ROBAST.
 ******************************************************************************/

// define useful units
const double cm = AOpticsManager::cm();
const double mm = AOpticsManager::mm();
const double um = AOpticsManager::um();
const double nm = AOpticsManager::nm();
const double  m = AOpticsManager::m();

// optics parameters
const double kF = 15.00*m; // focal length
const double kMirrorR = kF*2; // the radius of curvature
const double kMirrorD = 0.6*m; // facet diameter, circular mirror
const double kMirrorT = 0.01*mm; // mirror thickness, intentionally use a very thin thickness to avoid unnecessary reflection on the edges

void AddMirrors(AOpticalComponent* opt);
void AddCamera(AOpticalComponent* opt);
void AddMasts(AOpticalComponent* opt);
void RayTrace(AOpticsManager* manager, TCanvas* can3D);

void HESS1()
{
  TThread::Initialize(); // call this first when you use the multi-thread mode

  AOpticsManager* manager = new AOpticsManager("manager", "HESS CT3 System");
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
  double theta = TMath::ASin(kMirrorD/2./kMirrorR)*TMath::RadToDeg();
  TGeoSphere* mirSphere = new TGeoSphere("mirSphere", kMirrorR, kMirrorR + kMirrorT, 180. - theta, 180.);
  AMirror* mirror = new AMirror("mirror", mirSphere);

  // copied from cfg/hess/hess_mirrors_ct3.dat in sim_telarray
  const int kNMirror = 380;
  double xy[kNMirror][2] =
    {{ -31.0, 117.8}, {  31.0, 117.8}, { -62.0, 171.5}, {   0.0, 171.5},
     {  62.0, 171.5}, { -93.0, 225.2}, { -31.0, 225.2}, {  31.0, 225.2},
     {  93.0, 225.2}, {-124.0, 278.9}, { -62.0, 278.9}, {   0.0, 278.9},
     {  62.0, 278.9}, { 124.0, 278.9}, {-155.0, 332.6}, { -93.0, 332.6},
     { -31.0, 332.6}, {  31.0, 332.6}, {  93.0, 332.6}, { 155.0, 332.6},
     {-186.0, 386.3}, {-124.0, 386.3}, { -62.0, 386.3}, {   0.0, 386.3},
     {  62.0, 386.3}, { 124.0, 386.3}, { 186.0, 386.3}, {-217.0, 440.0},
     {-155.0, 440.0}, { -93.0, 440.0}, { -31.0, 440.0}, {  31.0, 440.0},
     {  93.0, 440.0}, { 155.0, 440.0}, { 217.0, 440.0}, {-248.0, 493.7},
     {-186.0, 493.7}, {-124.0, 493.7}, { -62.0, 493.7}, {   0.0, 493.7},
     {  62.0, 493.7}, { 124.0, 493.7}, { 186.0, 493.7}, { 248.0, 493.7},
     {-279.0, 547.3}, {-217.0, 547.3}, {-155.0, 547.3}, { -93.0, 547.3},
     { -31.0, 547.3}, {  31.0, 547.3}, {  93.0, 547.3}, { 155.0, 547.3},
     { 217.0, 547.3}, { 279.0, 547.3}, {-248.0, 601.0}, {-186.0, 601.0},
     {-124.0, 601.0}, { -62.0, 601.0}, {   0.0, 601.0}, {  62.0, 601.0},
     { 124.0, 601.0}, { 186.0, 601.0}, { 248.0, 601.0}, {  86.5,  85.7},
     { 117.5,  32.1}, { 117.5, 139.4}, { 148.5,  85.7}, { 179.5,  32.1},
     { 148.5, 193.1}, { 179.5, 139.4}, { 210.5,  85.7}, { 241.5,  32.1},
     { 179.5, 246.8}, { 210.5, 193.1}, { 241.5, 139.4}, { 272.5,  85.7},
     { 303.5,  32.1}, { 210.5, 300.5}, { 241.5, 246.8}, { 272.5, 193.1},
     { 303.5, 139.4}, { 334.5,  85.7}, { 365.5,  32.1}, { 241.5, 354.2},
     { 272.5, 300.5}, { 303.5, 246.8}, { 334.5, 193.1}, { 365.5, 139.4},
     { 396.5,  85.7}, { 427.5,  32.1}, { 272.5, 407.9}, { 303.5, 354.2},
     { 334.5, 300.5}, { 365.5, 246.8}, { 396.5, 193.1}, { 427.5, 139.4},
     { 458.5,  85.7}, { 489.5,  32.1}, { 303.5, 461.6}, { 334.5, 407.9},
     { 365.5, 354.2}, { 396.5, 300.5}, { 427.5, 246.8}, { 458.5, 193.1},
     { 489.5, 139.4}, { 520.5,  85.7}, { 551.5,  32.1}, { 334.5, 515.3},
     { 365.5, 461.6}, { 396.5, 407.9}, { 427.5, 354.2}, { 458.5, 300.5},
     { 489.5, 246.8}, { 520.5, 193.1}, { 551.5, 139.4}, { 582.5,  85.7},
     { 613.5,  32.1}, { 396.5, 515.3}, { 427.5, 461.6}, { 458.5, 407.9},
     { 489.5, 354.2}, { 520.5, 300.5}, { 551.5, 246.8}, { 582.5, 193.1},
     { 613.5, 139.4}, { 644.5,  85.7}, { 675.5,  32.1}, { 117.5, -32.1},
     {  86.5, -85.7}, { 179.5, -32.1}, { 148.5, -85.7}, { 117.5,-139.4},
     { 241.5, -32.1}, { 210.5, -85.7}, { 179.5,-139.4}, { 148.5,-193.1},
     { 303.5, -32.1}, { 272.5, -85.7}, { 241.5,-139.4}, { 210.5,-193.1},
     { 179.5,-246.8}, { 365.5, -32.1}, { 334.5, -85.7}, { 303.5,-139.4},
     { 272.5,-193.1}, { 241.5,-246.8}, { 210.5,-300.5}, { 427.5, -32.1},
     { 396.5, -85.7}, { 365.5,-139.4}, { 334.5,-193.1}, { 303.5,-246.8},
     { 272.5,-300.5}, { 241.5,-354.2}, { 489.5, -32.1}, { 458.5, -85.7},
     { 427.5,-139.4}, { 396.5,-193.1}, { 365.5,-246.8}, { 334.5,-300.5},
     { 303.5,-354.2}, { 272.5,-407.9}, { 551.5, -32.1}, { 520.5, -85.7},
     { 489.5,-139.4}, { 458.5,-193.1}, { 427.5,-246.8}, { 396.5,-300.5},
     { 365.5,-354.2}, { 334.5,-407.9}, { 303.5,-461.6}, { 613.5, -32.1},
     { 582.5, -85.7}, { 551.5,-139.4}, { 520.5,-193.1}, { 489.5,-246.8},
     { 458.5,-300.5}, { 427.5,-354.2}, { 396.5,-407.9}, { 365.5,-461.6},
     { 334.5,-515.3}, { 675.5, -32.1}, { 644.5, -85.7}, { 613.5,-139.4},
     { 582.5,-193.1}, { 551.5,-246.8}, { 520.5,-300.5}, { 489.5,-354.2},
     { 458.5,-407.9}, { 427.5,-461.6}, { 396.5,-515.3}, {  31.0,-117.8},
     { -31.0,-117.8}, {  62.0,-171.5}, {   0.0,-171.5}, { -62.0,-171.5},
     {  93.0,-225.2}, {  31.0,-225.2}, { -31.0,-225.2}, { -93.0,-225.2},
     { 124.0,-278.9}, {  62.0,-278.9}, { -62.0,-278.9}, {-124.0,-278.9},
     { 155.0,-332.6}, {  93.0,-332.6}, { -31.0,-332.6}, { -93.0,-332.6},
     {-155.0,-332.6}, { 186.0,-386.3}, { 124.0,-386.3}, {  62.0,-386.3},
     {   0.0,-386.3}, { -62.0,-386.3}, {-124.0,-386.3}, {-186.0,-386.3},
     { 217.0,-440.0}, { 155.0,-440.0}, {  93.0,-440.0}, {  31.0,-440.0},
     { -31.0,-440.0}, { -93.0,-440.0}, {-155.0,-440.0}, {-217.0,-440.0},
     { 248.0,-493.7}, { 186.0,-493.7}, { 124.0,-493.7}, {  62.0,-493.7},
     {   0.0,-493.7}, { -62.0,-493.7}, {-124.0,-493.7}, {-186.0,-493.7},
     {-248.0,-493.7}, { 279.0,-547.3}, { 217.0,-547.3}, { 155.0,-547.3},
     {  93.0,-547.3}, {  31.0,-547.3}, { -31.0,-547.3}, { -93.0,-547.3},
     {-155.0,-547.3}, {-217.0,-547.3}, {-279.0,-547.3}, { 248.0,-601.0},
     { 186.0,-601.0}, { 124.0,-601.0}, {  62.0,-601.0}, {   0.0,-601.0},
     { -62.0,-601.0}, {-124.0,-601.0}, {-186.0,-601.0}, {-248.0,-601.0},
     { -86.5, -85.7}, {-117.5, -32.1}, {-117.5,-139.4}, {-148.5, -85.7},
     {-179.5, -32.1}, {-148.5,-193.1}, {-179.5,-139.4}, {-210.5, -85.7},
     {-241.5, -32.1}, {-179.5,-246.8}, {-210.5,-193.1}, {-241.5,-139.4},
     {-272.5, -85.7}, {-303.5, -32.1}, {-210.5,-300.5}, {-241.5,-246.8},
     {-272.5,-193.1}, {-303.5,-139.4}, {-334.5, -85.7}, {-365.5, -32.1},
     {-241.5,-354.2}, {-272.5,-300.5}, {-303.5,-246.8}, {-334.5,-193.1},
     {-365.5,-139.4}, {-396.5, -85.7}, {-427.5, -32.1}, {-272.5,-407.9},
     {-303.5,-354.2}, {-334.5,-300.5}, {-365.5,-246.8}, {-396.5,-193.1},
     {-427.5,-139.4}, {-458.5, -85.7}, {-489.5, -32.1}, {-303.5,-461.6},
     {-334.5,-407.9}, {-365.5,-354.2}, {-396.5,-300.5}, {-427.5,-246.8},
     {-458.5,-193.1}, {-489.5,-139.4}, {-520.5, -85.7}, {-551.5, -32.1},
     {-334.5,-515.3}, {-365.5,-461.6}, {-396.5,-407.9}, {-427.5,-354.2},
     {-458.5,-300.5}, {-489.5,-246.8}, {-520.5,-193.1}, {-551.5,-139.4},
     {-582.5, -85.7}, {-613.5, -32.1}, {-396.5,-515.3}, {-427.5,-461.6},
     {-458.5,-407.9}, {-489.5,-354.2}, {-520.5,-300.5}, {-551.5,-246.8},
     {-582.5,-193.1}, {-613.5,-139.4}, {-644.5, -85.7}, {-675.5, -32.1},
     {-117.5,  32.1}, { -86.5,  85.7}, {-179.5,  32.1}, {-148.5,  85.7},
     {-117.5, 139.4}, {-241.5,  32.1}, {-210.5,  85.7}, {-179.5, 139.4},
     {-148.5, 193.1}, {-303.5,  32.1}, {-272.5,  85.7}, {-241.5, 139.4},
     {-210.5, 193.1}, {-179.5, 246.8}, {-365.5,  32.1}, {-334.5,  85.7},
     {-303.5, 139.4}, {-272.5, 193.1}, {-241.5, 246.8}, {-210.5, 300.5},
     {-427.5,  32.1}, {-396.5,  85.7}, {-365.5, 139.4}, {-334.5, 193.1},
     {-303.5, 246.8}, {-272.5, 300.5}, {-241.5, 354.2}, {-489.5,  32.1},
     {-458.5,  85.7}, {-427.5, 139.4}, {-396.5, 193.1}, {-365.5, 246.8},
     {-334.5, 300.5}, {-303.5, 354.2}, {-272.5, 407.9}, {-551.5,  32.1},
     {-520.5,  85.7}, {-489.5, 139.4}, {-458.5, 193.1}, {-427.5, 246.8},
     {-396.5, 300.5}, {-365.5, 354.2}, {-334.5, 407.9}, {-303.5, 461.6},
     {-613.5,  32.1}, {-582.5,  85.7}, {-551.5, 139.4}, {-520.5, 193.1},
     {-489.5, 246.8}, {-458.5, 300.5}, {-427.5, 354.2}, {-396.5, 407.9},
     {-365.5, 461.6}, {-334.5, 515.3}, {-675.5,  32.1}, {-644.5,  85.7},
     {-613.5, 139.4}, {-582.5, 193.1}, {-551.5, 246.8}, {-520.5, 300.5},
     {-489.5, 354.2}, {-458.5, 407.9}, {-427.5, 461.6}, {-396.5, 515.3}};

  for(int i = 0; i < kNMirror; i++){
    double x = xy[i][0]*cm;
    double y = xy[i][1]*cm;
    double r2d = TMath::RadToDeg();
    double r2 = TMath::Power(x, 2) + TMath::Power(y, 2);
    double z = kF - TMath::Sqrt(TMath::Power(kF, 2) - r2);

    // each mirror center is relocated from the origin (0, 0, 0) to (x, y, z)
    TGeoTranslation* trans = new TGeoTranslation(Form("mirTrans%d", i), x, y, z);

    // and is rotated to compose a DC optics
    double phi = TMath::ATan2(y, x)*r2d;
    theta = TMath::ATan2(TMath::Sqrt(r2), 2*kF - z)*r2d;
    TGeoRotation* rot = new TGeoRotation("", phi - 90., theta, 0);
    
    // make a matrix from translation and rotation matrices
    TGeoTranslation* transZ = new TGeoTranslation(0, 0, kMirrorR);
    TGeoCombiTrans* combi = new TGeoCombiTrans(*trans, *rot);
    TGeoHMatrix* hmat = new TGeoHMatrix((*combi)*(*transZ));

    // finally add this mirror to the world
    opt->AddNode(mirror, i + 1, hmat);
  } // i
}

void AddCamera(AOpticalComponent* opt)
{
  // parameters taken from sim_telarray/cfg/hess/hess_masts.dat
  const double kCameraD = 1.57*m; // the camera diameter
  const double kCameraBoxD = 1.6*m; // the camera box diameter
  const double kCameraBoxH = 1.55*m; // the camera box height

  // Make a disk focal plane
  TGeoTube* tubeCamera = new TGeoTube("tubeCamera", 0, kCameraD/2., 1*mm);
  AFocalSurface* focalPlane = new AFocalSurface("focalPlane", tubeCamera);
  opt->AddNode(focalPlane, 1, new TGeoTranslation(0, 0, kF + 1*mm));

  // Make a camera box
  TGeoTube* tubeCameraBox = new TGeoTube("tubeCameraBox", 0, kCameraBoxD/2., kCameraBoxH/2.);
  double t = 1*cm;
  TGeoTube* tubeCameraBox2 = new TGeoTube("tubeCameraBox2", 0, kCameraBoxD/2. - t, kCameraBoxH/2. - t);

  TGeoTranslation* transZ1 = new TGeoTranslation("transZ1", 0, 0, kF + kCameraBoxH/2.);
  transZ1->RegisterYourself();
  TGeoTranslation* transZ2 = new TGeoTranslation("transZ2", 0, 0, kF + kCameraBoxH/2. - t - 1*mm);
  transZ2->RegisterYourself();

  TGeoCompositeShape* boxComposite = new TGeoCompositeShape("boxComposite", "tubeCameraBox:transZ1-tubeCameraBox2:transZ2");

  AObscuration* cameraBox = new AObscuration("cameraBox", boxComposite);
  opt->AddNode(cameraBox, 1);

  const double kLidOffset = 92.0*cm;
  const double kLidAngle = 270.; // deg
  const double kLidOpening = -110.; // deg
  TGeoTube* tubeLid = new TGeoTube("tubeLid", 0, kCameraD/2., 0.1*mm);
  AObscuration* cameraLid = new AObscuration("cameraLid", tubeLid);

  double d2r = TMath::DegToRad();
  TGeoTranslation* trans = new TGeoTranslation("", 0, -kLidOffset + kCameraD/2.*TMath::Cos(kLidOpening*d2r), kF + kCameraD/2.*TMath::Sin(kLidOpening*d2r));
  TGeoRotation* rot = new TGeoRotation("", 0, kLidOpening, 0);
  TGeoCombiTrans* combi = new TGeoCombiTrans(*trans, *rot);
  opt->AddNode(cameraLid, 1, combi);
  
}

void AddMasts(AOpticalComponent* opt)
{
  // Main camera masts
  const int kNMasts = 32;
  double pos[kNMasts][7] = {
    { 342.3,  593.0,  122.4,   57.7,  100.0, 1555.5, 16.83},
    {-342.3,  593.0,  122.4,  -57.7,  100.0, 1555.5, 16.83},
    { 342.3, -593.0,  122.4,   57.7, -100.0, 1555.5, 16.83},
    {-342.3, -593.0,  122.4,  -57.7, -100.0, 1555.5, 16.83},
    // Interconnecting masts
    { 205.3,  355.6,  805.4,  205.3, -355.6,  805.4, 12.7},
    { 205.3, -355.6,  805.4, -205.3, -355.6,  805.4, 12.7},
    {-205.3, -355.6,  805.4, -205.3,  355.6,  805.4, 12.7},
    {-205.3,  355.6,  805.4,  205.3,  355.6,  805.4, 12.7},
    // Stabilizing strings (lower part, sort)
    { 342.3,  593.0,  122.4, -205.3,  355.6,  805.4, 2.4},
    {-342.3,  593.0,  122.4,  205.3,  355.6,  805.4, 2.4},
    { 342.3, -593.0,  122.4, -205.3, -355.6,  805.4, 2.4},
    {-342.3, -593.0,  122.4,  205.3, -355.6,  805.4, 2.4},
    // Stabilizing strings (lower part, long)
    { 342.3,  593.0,  122.4,  205.3, -355.6,  805.4, 1.2},
    { 342.3, -593.0,  122.4,  205.3,  355.6,  805.4, 1.2},
    {-342.3,  593.0,  122.4, -205.3, -355.6,  805.4, 1.2},
    {-342.3, -593.0,  122.4, -205.3,  355.6,  805.4, 1.2},
    // Stabilizing strings (upper part, short)
    { 205.3,  355.6,  805.4,  -57.7,  100.0, 1555.5, 2.4},
    {-205.3,  355.6,  805.4,   57.7,  100.0, 1555.5, 2.4},
    { 205.3, -355.6,  805.4,  -57.7, -100.0, 1555.5, 2.4},
    {-205.3, -355.6,  805.4,   57.7, -100.0, 1555.5, 2.4},
    // Stabilizing strings (upper part, long)
    { 205.3,  355.6,  805.4,   57.7, -100.0, 1555.5, 1.2},
    { 205.3, -355.6,  805.4,   57.7,  100.0, 1555.5, 1.2},
    {-205.3,  355.6,  805.4,  -57.7, -100.0, 1555.5, 1.2},
    {-205.3, -355.6,  805.4,  -57.7,  100.0, 1555.5, 1.2},
    // Camera mounting frame
    { 100.0,   57.7, 1555.5,  100.0,  -57.7, 1555.5, 20.0},
    { 100.0,  -57.7, 1555.5,   57.7, -100.0, 1555.5, 20.0},
    {  57.7, -100.0, 1555.5,  -57.7, -100.0, 1555.5, 20.0},
    { -57.7, -100.0, 1555.5, -100.0,  -57.7, 1555.5, 20.0},
    {-100.0,  -57.7, 1555.5, -100.0,   57.7, 1555.5, 20.0},
    {-100.0,   57.7, 1555.5,  -57.7,  100.0, 1555.5, 20.0},
    { -57.7,  100.0, 1555.5,   57.7,  100.0, 1555.5, 20.0},
    {  57.7,  100.0, 1555.5,  100.0,   57.7, 1555.5, 20.0}};

  for(int i = 0; i < kNMasts; i++){
    double x1 = pos[i][0]*cm;
    double y1 = pos[i][1]*cm;
    double z1 = pos[i][2]*cm;

    double x2 = pos[i][3]*cm;
    double y2 = pos[i][4]*cm;
    double z2 = pos[i][5]*cm;

    double r = pos[i][6]*cm/2.;

    TVector3 v1(x1, y1, z1);
    TVector3 v2(x2, y2, z2);
    TGeoTube* tube;
    TGeoCombiTrans* combi;
    AGeoUtil::MakePointToPointTube(Form("mast%d", i), v1, v2, r, &tube, &combi);
    AObscuration* obs = new AObscuration(Form("obsMast%d", i), tube);
    opt->AddNode(obs, 1, combi);
  }
}

void RayTrace(AOpticsManager* manager, TCanvas* can3D)
{
  const int kNdeg = 6;
  TH2D* h2[kNdeg];
  TGraph* graph = new TGraph();
  TCanvas* can = new TCanvas("can", "can", 900, 600);
  can->Divide(3, 2, 1e-10, 1e-10);

  for(int i = 0; i < kNdeg; i++){
    double deg = i*0.5;
    TGeoTranslation raytr("raytr", -1.2*kF*TMath::Sin(deg*TMath::DegToRad()), 0, 1.2*kF*TMath::Cos(deg*TMath::DegToRad()));
    TVector3 dir;
    dir.SetMagThetaPhi(1, TMath::Pi() - deg*TMath::DegToRad(), 0);
    double lambda = 400*nm; // dummy

    h2[i] = new TH2D(Form("h%d", i), Form("#it{#theta} = %3.1f#circ;x (mm); y (mm)", deg), 500, -50, 100, 500, -75, 75);
    
    ARayArray* array = ARayShooter::RandomCircle(lambda, 7.5*m, 100000, 0, &raytr, &dir);
    
    manager->TraceNonSequential(*array);
    
    TObjArray* focused = array->GetFocused();
    
    for(Int_t k = 0; k <= focused->GetLast(); k++){
      ARay* ray = (ARay*)(*focused)[k];
      Double_t p[4];
      ray->GetLastPoint(p);
      h2[i]->Fill((p[0] - 27*deg)/mm, p[1]/mm); // 27 is a rough plate scale
      
      if (i == kNdeg - 1 && k < 30) {
        TPolyLine3D* pol = ray->MakePolyLine3D();
        pol->SetLineColor(2);
        pol->SetLineWidth(2);
        can3D->cd();
        pol->Draw();
      } // if
    } // k
    
    delete array;
    can->cd(i + 1);
    h2[i]->Draw("colz");
    can->Update();
  } // i
}
