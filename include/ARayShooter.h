// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_RAY_SHOOTER_H
#define A_RAY_SHOOTER_H

#include "TGeoMatrix.h"
#include "TMath.h"
#include "TVector3.h"

#include "ARayArray.h"

///////////////////////////////////////////////////////////////////////////////
//
// ARayShooter
//
// Ray shooter
//
///////////////////////////////////////////////////////////////////////////////

class ARayShooter : public TObject {
 private:
  
 public:
  ARayShooter();
  virtual ~ARayShooter();

  static ARayArray* Circle(Double_t lambda, Double_t rmax, Int_t nr,
                           Int_t nphi, TGeoRotation* rot = 0,
                           TGeoTranslation* tr = 0, TVector3* v = 0);
  static ARayArray* RandomCircle(Double_t lambda, Double_t rmax, Int_t n,
                                 TGeoRotation* rot = 0,
                                 TGeoTranslation* tr = 0, TVector3* v = 0);
  static ARayArray* RandomCone(Double_t lambda, Double_t r, Double_t d, Int_t n,
                               TGeoRotation* rot = 0, TGeoTranslation* tr = 0);
  static ARayArray* RandomRectangle(Double_t lambda, Double_t dx, Double_t dy,
                                    Int_t n, TGeoRotation* rot = 0,
                                    TGeoTranslation* tr = 0, TVector3* v = 0);
  static ARayArray* RandomSphere(Double_t lambda, Int_t n, TGeoTranslation* tr = 0);
  static ARayArray* RandomSphericalCone(Double_t lambda, Int_t n, Double_t theta, TGeoRotation* rot = 0, TGeoTranslation* tr = 0);
  static ARayArray* RandomSquare(Double_t lambda, Double_t d,
                                 Int_t n, TGeoRotation* rot = 0,
                                 TGeoTranslation* tr = 0, TVector3* v = 0);
  static ARayArray* Rectangle(Double_t lambda, Double_t dx, Double_t dy,
                              Int_t nx, Int_t ny, TGeoRotation* rot = 0,
                              TGeoTranslation* tr = 0, TVector3* v = 0);
  static ARayArray* Square(Double_t lambda, Double_t d, Int_t n,
                           TGeoRotation* rot = 0, TGeoTranslation* tr = 0, TVector3* v = 0);

  ClassDef(ARayShooter, 1)
};

#endif // A_RAY_SHOOTER_H
