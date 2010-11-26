// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ARayShooter
//
// Ray shooter
//
///////////////////////////////////////////////////////////////////////////////

#include "ARayShooter.h"

ClassImp(ARayShooter)

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
                               TGeoTranslation* tr)
{
  ARayArray* array = new ARayArray;

  if(0 > rmax or nr < 1 or nphi < 1){
    return array;
  } // if

  Double_t position[3] = {0, 0, 0};
  Double_t new_pos[3];
  Double_t dir[3] = {0, 0, 1};
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
                                  TGeoTranslation* tr)
{
  ARayArray* array = new ARayArray;

  if(dx < 0 or dy < 0 or nx < 1 or ny < 1){
    return array;
  } // if

  Double_t dir[3] = {0, 0, 1};
  Double_t new_dir[3];

  if(rot){
    rot->LocalToMaster(dir, new_dir);
  } else {
    memcpy(new_dir, dir, 3*sizeof(Double_t));
  } // if

  Double_t deltax = nx == 1 ? dx/2 : dx/(ny - 1);
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
ARayArray* ARayShooter::Sphere(Double_t lambda, Double_t theta, Int_t ntheta,
                               Int_t nphi, TGeoRotation* rot,
                               TGeoTranslation* tr)
{
  ARayArray* array = new ARayArray;
  
  while(theta<0){
    theta += 2*TMath::Pi();
  } // while
  
  while(theta > 2*TMath::Pi()){
    theta -= 2*TMath::Pi();
  } // while

  Double_t position[3] = {0, 0, 0};
  Double_t new_pos[3];
  if(tr) {
    tr->LocalToMaster(position, new_pos);
  } else {
    memcpy(new_pos, position, 3*sizeof(Double_t));
  } // if

  if(1 <= ntheta and 1 <= nphi){
    Double_t dir[3] = {0, 0, 1};
    Double_t new_dir[3];
    if(rot) {
      rot->LocalToMaster(dir, new_dir);
    } else {
      memcpy(new_dir, dir, 3*sizeof(Double_t));
    } // if
    array->Add(new ARay(0, lambda, new_pos[0], new_pos[1], new_pos[2], 0,
                        new_dir[0], new_dir[1], new_dir[2]));

    for(Int_t i = 0; i < ntheta; i++){
      Double_t th = theta*(i + 1)/ntheta;
      for(Int_t j = 0; j < nphi*(i + 1); j++){
        Double_t phi = 2*TMath::Pi()/nphi/(i+1)*j;

        Double_t dir[3] = {TMath::Sin(th)*TMath::Cos(phi),
                           TMath::Sin(th)*TMath::Sin(phi),
                           TMath::Cos(th)};
        Double_t new_dir[3];
        if(rot) {
          rot->LocalToMaster(dir, new_dir);
        } else {
          memcpy(new_dir, dir, 3*sizeof(Double_t));
        } // if

        ARay* ray = new ARay(0, lambda, new_pos[0], new_pos[1], new_pos[2], 0,
                             new_dir[0], new_dir[1], new_dir[2]);
        array->Add(ray);
      } // j
    } // i
  } // if

  return array;
}

//_____________________________________________________________________________
ARayArray* ARayShooter::Square(Double_t lambda, Double_t d, Int_t n,
                               TGeoRotation* rot, TGeoTranslation* tr)
{
  return Rectangle(lambda, d, d, n, n, rot, tr);
}
