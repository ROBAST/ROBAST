// $Id$
// Author: Akira Okumura 2007/10/02

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_RAY_SHOOTER_H
#define A_RAY_SHOOTER_H

///////////////////////////////////////////////////////////////////////////////
//
// ARayShooter
//
// Ray shooter
//
///////////////////////////////////////////////////////////////////////////////

#include "ARayArray.h"
#include "TMath.h"
#include "TGeoMatrix.h"

class ARayShooter : public TObject {
 private:
  
 public:
  ARayShooter();
  virtual ~ARayShooter();

  static ARayArray* Circle(Double_t lambda, Double_t rmax, Int_t nr,
                           Int_t nphi, TGeoRotation* rot = 0,
                           TGeoTranslation* tr = 0);
  static ARayArray* Rectangle(Double_t lambda, Double_t dx, Double_t dy,
                              Int_t nx, Int_t ny, TGeoRotation* rot = 0,
                              TGeoTranslation* tr = 0);
  static ARayArray* Sphere(Double_t lambda, Double_t theta, Int_t nr,
                           Int_t ntheta, TGeoRotation* rot = 0,
                           TGeoTranslation* tr = 0);
  static ARayArray* Square(Double_t lambda, Double_t d, Int_t n,
                           TGeoRotation* rot = 0, TGeoTranslation* tr = 0);

  ClassDef(ARayShooter, 1)
};

#endif // A_RAY_SHOOTER_H
