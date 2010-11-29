// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

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

  ARayArray* array = ARayShooter::Circle(400*nm, 100*m, 20, 6, 0, 0);
  TGraph* circle = new TGraph;
  circle->SetTitle("Circle(400*nm, 100*m, 20, 6, 0, 0);X (m);Y (m)");
  TObjArray* running = array->GetRunning();

  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(*running)[i];
    if(!ray) continue;
    Double_t p[4];
    ray->GetLastPoint(p);
    circle->SetPoint(i, p[0], p[1]);
  } // i

  array = ARayShooter::Rectangle(400*nm, 100*m, 80*m, 20, 10, 0, 0);
  TGraph* rectangle = new TGraph;
  rectangle->SetTitle("Rectangle(400*nm, 100*m, 80*m, 20, 10, 0, 0);X (m);Y (m)");
  running = array->GetRunning();

  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(*running)[i];
    if(!ray) continue;
    Double_t p[4];
    ray->GetLastPoint(p);
    rectangle->SetPoint(i, p[0], p[1]);
  } // i

  array = ARayShooter::Square(400*nm, 90*m, 30, 0, 0);
  TGraph* square = new TGraph;
  square->SetTitle("Square(400*nm, 90*m, 30, 0, 0);X (m);Y (m)");
  running = array->GetRunning();

  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(*running)[i];
    if(!ray) continue;
    Double_t p[4];
    ray->GetLastPoint(p);
    square->SetPoint(i, p[0], p[1]);
  } // i

  TCanvas* can = new TCanvas("can", "can", 400, 1200);
  can->Divide(1, 3);
  can->cd(1);
  circle->Draw("ap");
  can->cd(2);
  rectangle->Draw("ap");
  can->cd(3);
  square->Draw("ap");

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
