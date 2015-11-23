/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#include "TRandom.h"

#include "ARayShooter.h"

ClassImp(ARayShooter)

//_____________________________________________________________________________
/*
Begin_Html
<p>ARayShooter creates an array of photons to be traced.</p>

<p>Three static member functions create aligned initial photons as below.
By default, the start coordinates is at (x, y, z) = (0, 0, 0) and emitted direction is in parallel to z axis.
Users can change these values using TGeoRotation and TGeoTranslation.
</p>
End_Html
Begin_Macro(source)
{
  Double_t nm = AOpticsManager::nm();
  Double_t  m = AOpticsManager::m();

  ARayArray* rays = ARayShooter::Circle(400*nm, 100*m, 20, 6, 0, 0);
  TGraph* circle = new TGraph;
  circle->SetTitle("Circle(400*nm, 100*m, 20, 6, 0, 0);X (m);Y (m)");
  TObjArray* running = rays->GetRunning();

  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(*running)[i];
    if(!ray) continue;
    Double_t p[4];
    ray->GetLastPoint(p);
    circle->SetPoint(i, p[0], p[1]);
  } // i

  rays = ARayShooter::Rectangle(400*nm, 100*m, 80*m, 20, 10, 0, 0);
  TGraph* rectangle = new TGraph;
  rectangle->SetTitle("Rectangle(400*nm, 100*m, 80*m, 20, 10, 0, 0);X (m);Y (m)");
  running = rays->GetRunning();

  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(*running)[i];
    if(!ray) continue;
    Double_t p[4];
    ray->GetLastPoint(p);
    rectangle->SetPoint(i, p[0], p[1]);
  } // i

  rays = ARayShooter::Square(400*nm, 90*m, 30, 0, 0);
  TGraph* square = new TGraph;
  square->SetTitle("Square(400*nm, 90*m, 30, 0, 0);X (m);Y (m)");
  running = rays->GetRunning();

  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(*running)[i];
    if(!ray) continue;
    Double_t p[4];
    ray->GetLastPoint(p);
    square->SetPoint(i, p[0], p[1]);
  } // i

  AOpticsManager* manager = new AOpticsManager("manager", "manager");
  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 100*m, 100*m, 100*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);

  // Top volume
  TGeoBBox* topbox = new TGeoBBox("topbox", 100*m, 100*m, 100*m);
  AOpticalComponent* top = new AOpticalComponent("top", topbox);

  TGeoBBox* focal_box = new TGeoBBox("focal_box",45*m, 45*m, 1*m);
  AFocalSurface* focal = new AFocalSurface("focal", focal_box);
  top->AddNode(focal, 1, new TGeoTranslation(0, 0, 11*m));
  world->AddNode(top, 1);
  manager->CloseGeometry();

  rays = ARayShooter::RandomCone(400*nm, 45*m, 10*m, 1000, 0);
  TGraph* cone = new TGraph;
  cone->SetTitle("RandomCone(400*nm, 45*m, 10*m, 1000, 0, 0);X (m);Y (m)");
  manager->TraceNonSequential(*rays);
  running = rays->GetFocused();

  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(*running)[i];
    if(!ray) continue;
    Double_t p[4];
    ray->GetLastPoint(p);
    cone->SetPoint(i, p[0], p[1]);
  } // i

  TCanvas* can = new TCanvas("can", "can", 400, 1600);
  can->Divide(1, 4, 1e-10, 1e-10);
  can->cd(1);
  circle->Draw("ap");
  can->cd(2);
  rectangle->Draw("ap");
  can->cd(3);
  square->Draw("ap");
  can->cd(4);
  cone->Draw("ap");

  return can;
}
End_Macro
*/

//_____________________________________________________________________________
ARayShooter::ARayShooter()
{
  // Default constructor
}

//_____________________________________________________________________________
ARayShooter::~ARayShooter()
{
}

//_____________________________________________________________________________
ARayArray* ARayShooter::Circle(Double_t lambda, Double_t rmax, Int_t nr,
                               Int_t nphi, TGeoRotation* rot,
                               TGeoTranslation* tr, TVector3* v)
{
  // Create initial photons aligned in concentric circles
  ARayArray* array = new ARayArray;

  if(0 > rmax or nr < 1 or nphi < 1){
    return array;
  } // if

  Double_t position[3] = {0, 0, 0};
  Double_t new_pos[3];
  Double_t dir[3] = {0, 0, 1};
  if(v){
    dir[0] = v->X();
    dir[1] = v->Y();
    dir[2] = v->Z();
  } // if
  Double_t new_dir[3];

  if(rot){
    rot->LocalToMaster(dir, new_dir);
  } else {
    memcpy(new_dir, dir, 3*sizeof(Double_t));
  } // if

  if(tr) {
    tr->LocalToMaster(position, new_pos);
  } else {
    memcpy(new_pos, position, 3*sizeof(Double_t));
  } // if

  // a photon at the center
  array->Add(new ARay(0, lambda, new_pos[0], new_pos[1], new_pos[2], 0,
                      new_dir[0], new_dir[1], new_dir[2]));

  for(Int_t i = 0; i < nr; i++){
    Double_t r = rmax*(i + 1)/nr;
    for(Int_t j = 0; j < nphi*(i + 1); j++){
      Double_t phi = 2*TMath::Pi()/nphi/(i + 1)*j;
      Double_t x[3] = {r*TMath::Cos(phi), r*TMath::Sin(phi), 0};

      if(rot){
        rot->LocalToMaster(x, new_pos);
      } else {
        memcpy(new_pos, x, 3*sizeof(Double_t));
      } // if

      if(tr) {
        tr->LocalToMaster(new_pos, x);
      } else {
        memcpy(x, new_pos, 3*sizeof(Double_t));
      } // if

      ARay* ray = new ARay(0, lambda, x[0], x[1], x[2], 0,
                           new_dir[0], new_dir[1], new_dir[2]);
      array->Add(ray);
    } // j
  } // i

  return array;
}

//_____________________________________________________________________________
ARayArray* ARayShooter::RandomCircle(Double_t lambda, Double_t rmax, Int_t n,
                                     TGeoRotation* rot,
                                     TGeoTranslation* tr, TVector3* v)
{
  ARayArray* array = new ARayArray;

  if(0 > rmax){
    return array;
  } // if

  Double_t new_pos[3];
  Double_t dir[3] = {0, 0, 1};
  if(v){
    dir[0] = v->X();
    dir[1] = v->Y();
    dir[2] = v->Z();
  } // if
  Double_t new_dir[3];

  if(rot){
    rot->LocalToMaster(dir, new_dir);
  } else {
    memcpy(new_dir, dir, 3*sizeof(Double_t));
  } // if

  for(Int_t i = 0; i < n; i++){
    Double_t r = rmax*10;
    Double_t randx, randy;
    while(r > rmax){
      randx = gRandom->Uniform(-rmax, rmax);
      randy = gRandom->Uniform(-rmax, rmax);
      r = TMath::Sqrt(randx*randx + randy*randy);
    } // while

    Double_t x[3] = {randx, randy, 0};

    if(rot){
      rot->LocalToMaster(x, new_pos);
    } else {
      memcpy(new_pos, x, 3*sizeof(Double_t));
    } //   if

    if(tr) {
      tr->LocalToMaster(new_pos, x);
    } else {
      memcpy(x, new_pos, 3*sizeof(Double_t));
    } // if

    ARay* ray = new ARay(0, lambda, x[0], x[1], x[2], 0,
                         new_dir[0], new_dir[1], new_dir[2]);
    array->Add(ray);
  } // i

  return array;
}

//_____________________________________________________________________________
ARayArray* ARayShooter::RandomCone(Double_t lambda, Double_t r, Double_t d, Int_t n,
                                   TGeoRotation* rot, TGeoTranslation* tr)
{
  // Create initial photons aligned in a cone. Direction is random
  // Start position is at the origin.
  // Arrival position is inside the circle of radius r at z = d
  ARayArray* array = new ARayArray;

  for(Int_t i = 0; i < n; i++){
    // random (x, y) inside a circle
    Double_t x = gRandom->Uniform(-r, r);
    Double_t y = gRandom->Uniform(-r, r);
    if(x*x + y*y > r*r){
      i--;
      continue;
    } // if

    Double_t goal_pos[3] = {x, y, d};
    Double_t tmp_pos[3];
    if(rot){
      rot->LocalToMaster(goal_pos, tmp_pos);
    } else {
        memcpy(tmp_pos, goal_pos, 3*sizeof(Double_t));
    } // if

    Double_t start_pos[3] = {0, 0, 0};

    if(tr) {
      tr->LocalToMaster(tmp_pos, goal_pos);
      tr->LocalToMaster(start_pos, tmp_pos);
    } else {
      memcpy(goal_pos, tmp_pos, 3*sizeof(Double_t));
      memcpy(tmp_pos, start_pos, 3*sizeof(Double_t));
    } // if

    TVector3 start(tmp_pos);
    TVector3 goal(goal_pos);
    TVector3 dir = goal - start;

    ARay* ray = new ARay(0, lambda, start.X(), start.Y(), start.Z(), 0,
                         dir.X(), dir.Y(), dir.Z());
    array->Add(ray);
  } // i

  return array;
}

//_____________________________________________________________________________
ARayArray* ARayShooter::RandomRectangle(Double_t lambda, Double_t dx, Double_t dy,
                                        Int_t n, TGeoRotation* rot,
                                        TGeoTranslation* tr, TVector3* v)
{
  // Create initial photons randomly distributed in a rectangle
  ARayArray* array = new ARayArray;

  if(dx < 0 or dy < 0 or n < 1){
    return array;
  } // if

  Double_t dir[3] = {0, 0, 1};
  if(v){
    dir[0] = v->X();
    dir[1] = v->Y();
    dir[2] = v->Z();
  } // if
  Double_t new_dir[3];

  if(rot){
    rot->LocalToMaster(dir, new_dir);
  } else {
    memcpy(new_dir, dir, 3*sizeof(Double_t));
  } // if

  for(Int_t i = 0; i < n; i++){
    Double_t new_pos[3];
    Double_t x[3] = {gRandom->Uniform(-dx/2., dx/2.), gRandom->Uniform(-dy/2., dy/2.), 0};

    if(rot){
      rot->LocalToMaster(x, new_pos);
    } else {
      memcpy(new_pos, x, 3*sizeof(Double_t));
    } // if

    if(tr) {
      tr->LocalToMaster(new_pos, x);
    } else {
      memcpy(x, new_pos, 3*sizeof(Double_t));
    } // if

    ARay* ray = new ARay(0, lambda, x[0], x[1], x[2], 0,
                         new_dir[0], new_dir[1], new_dir[2]);
    array->Add(ray);
  } // i

  return array;
}

//_____________________________________________________________________________
ARayArray* ARayShooter::RandomSphere(Double_t lambda, Int_t n, TGeoTranslation* tr)
{
  ARayArray* array = new ARayArray;
  for(Int_t i = 0; i < n; i++){
    Double_t dir[3];
    gRandom->Sphere(dir[0], dir[1], dir[2], 1);

    Double_t p[3] = {0, 0, 0};
    Double_t new_pos[3] = {0, 0, 0};
    if(tr){
      tr->LocalToMaster(p, new_pos);
    } // if
    ARay* ray = new ARay(0, lambda, new_pos[0], new_pos[1], new_pos[2], 0,
                         dir[0], dir[1], dir[2]);
    array->Add(ray);
  } // i

  return array;
}

//_____________________________________________________________________________
ARayArray* ARayShooter::RandomSphericalCone(Double_t lambda, Int_t n, Double_t theta, TGeoRotation* rot, TGeoTranslation* tr)
{
  ARayArray* array = new ARayArray;
  for(Int_t i = 0; i < n; i++){
    Double_t dir[3];
    Double_t ran = gRandom->Uniform(TMath::Cos(theta*TMath::DegToRad()), 1);
    Double_t theta_ = TMath::ACos(ran);
    Double_t phi = gRandom->Uniform(0, TMath::TwoPi());
    dir[0] = TMath::Sin(theta_)*TMath::Cos(phi);
    dir[1] = TMath::Sin(theta_)*TMath::Sin(phi);
    dir[2] = TMath::Cos(theta_);

    Double_t new_dir[3];
    if(rot){
      rot->LocalToMaster(dir, new_dir);
    } else {
      memcpy(new_dir, dir, 3*sizeof(Double_t));
    } // if

    Double_t p[3] = {0, 0, 0};
    Double_t new_pos[3] = {0, 0, 0};
    if(tr){
      tr->LocalToMaster(p, new_pos);
    } // if

    ARay* ray = new ARay(0, lambda, new_pos[0], new_pos[1], new_pos[2], 0,
                         new_dir[0], new_dir[1], new_dir[2]);
    array->Add(ray);
  } // i

  return array;
}

//_____________________________________________________________________________
ARayArray* ARayShooter::RandomSquare(Double_t lambda, Double_t d,
                                     Int_t n, TGeoRotation* rot,
                                     TGeoTranslation* tr, TVector3* v)
{
  return RandomRectangle(lambda, d, d, n, rot, tr, v);
}

//_____________________________________________________________________________
ARayArray* ARayShooter::Rectangle(Double_t lambda, Double_t dx, Double_t dy,
                                  Int_t nx, Int_t ny, TGeoRotation* rot,
                                  TGeoTranslation* tr, TVector3* v)
{
  // Create initial photons aligned in rectangles
  ARayArray* array = new ARayArray;

  if(dx < 0 or dy < 0 or nx < 1 or ny < 1){
    return array;
  } // if

  Double_t dir[3] = {0, 0, 1};
  if(v){
    dir[0] = v->X();
    dir[1] = v->Y();
    dir[2] = v->Z();
  } // if
  Double_t new_dir[3];

  if(rot){
    rot->LocalToMaster(dir, new_dir);
  } else {
    memcpy(new_dir, dir, 3*sizeof(Double_t));
  } // if

  Double_t deltax = nx == 1 ? dx/2 : dx/(nx - 1);
  Double_t deltay = ny == 1 ? dy/2 : dy/(ny - 1);

  for(Int_t i = 0; i < nx; i++){
    for(Int_t j = 0; j < ny; j++){
      Double_t new_pos[3];
      Double_t x[3] = {i*deltax - dx/2, j*deltay - dy/2, 0};

      if(rot){
        rot->LocalToMaster(x, new_pos);
      } else {
        memcpy(new_pos, x, 3*sizeof(Double_t));
      } // if

      if(tr) {
        tr->LocalToMaster(new_pos, x);
      } else {
        memcpy(x, new_pos, 3*sizeof(Double_t));
      } // if

      ARay* ray = new ARay(0, lambda, x[0], x[1], x[2], 0,
                           new_dir[0], new_dir[1], new_dir[2]);
      array->Add(ray);
    } // j
  } // i

  return array;
}

//_____________________________________________________________________________
ARayArray* ARayShooter::Square(Double_t lambda, Double_t d, Int_t n,
                               TGeoRotation* rot, TGeoTranslation* tr,
                               TVector3* v)
{
  // Create initial photons aligned in squares
  return Rectangle(lambda, d, d, n, n, rot, tr, v);
}
