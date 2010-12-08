// $Id: ARayShooter.h 3 2010-11-26 17:17:31Z oxon $
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

#ifndef ROOT_TGeoMatrix
#include "TGeoMatrix.h"
#endif
#ifndef ROOT_TMath
#include "TMath.h"
#endif
#ifndef ROOT_TVector3
#include "TVector3.h"
#endif

#ifndef A_RAY_ARRAY_H
#include "ARayArray.h"
#endif

class ARayShooter : public TObject {
 private:
  
 public:
  ARayShooter();
  virtual ~ARayShooter();

  static ARayArray* Circle(Double_t lambda, Double_t rmax, Int_t nr,
                           Int_t nphi, TGeoRotation* rot = 0,
                           TGeoTranslation* tr = 0, TVector3* v = 0);
  static ARayArray* RandomCone(Double_t lambda, Double_t r, Double_t d, Int_t n,
                               TGeoRotation* rot = 0, TGeoTranslation* tr = 0);
  static ARayArray* Rectangle(Double_t lambda, Double_t dx, Double_t dy,
                              Int_t nx, Int_t ny, TGeoRotation* rot = 0,
                              TGeoTranslation* tr = 0, TVector3* v = 0);
  static ARayArray* Square(Double_t lambda, Double_t d, Int_t n,
                           TGeoRotation* rot = 0, TGeoTranslation* tr = 0, TVector3* v = 0);

  ClassDef(ARayShooter, 1)
};

#endif // A_RAY_SHOOTER_H
