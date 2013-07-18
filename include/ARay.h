// $Id: ARay.h 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_RAY_H
#define A_RAY_H

///////////////////////////////////////////////////////////////////////////////
//
// ARay
//
// Classical ray class
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TGeoTrack
#include "TGeoTrack.h"
#endif
#ifndef ROOT_TPolyLine3D
#include "TPolyLine3D.h"
#endif
#ifndef ROOT_TVector3
#include "TVector3.h"
#endif

class ARay : public TGeoTrack {
 private:
  enum {kRun, kStop, kExit, kFocus, kSuspend, kAbsorb};
  Double_t     fLambda;     // Wavelength
  TVector3     fDirection;  // Current direction vector
  Int_t        fStatus;     // status of ray
  UInt_t       fWeight;     // weight used for photon bunch

 public:
  ARay();
  ARay(Int_t id, Double_t lambda, Double_t x, Double_t y, Double_t z,
       Double_t t, Double_t dx, Double_t dy, Double_t dz);
  virtual ~ARay();

  void         Absorb() {fStatus = kAbsorb;}
  void         Exit() {fStatus = kExit;}
  void         Focus() {fStatus = kFocus;}
  void         GetDirection(Double_t* d) const;
  Double_t     GetLambda() const {return fLambda;}
  void         GetLastPoint(Double_t* x) const;
  UInt_t       GetWeight() const {return fWeight;}
  Bool_t       IsAbsorbed() const;
  Bool_t       IsExited() const;
  Bool_t       IsFocused() const;
  Bool_t       IsRunning() const;
  Bool_t       IsStopped() const;
  Bool_t       IsSuspended() const;
  TPolyLine3D* MakePolyLine3D() const;
  void         SetDirection(Double_t dx, Double_t dy, Double_t dz);
  void         SetDirection(Double_t* d);
  void         SetLambda(Double_t lambda) {fLambda = lambda;}
  void         SetWeight(UInt_t weight) {fWeight = weight;}
  void         Stop() {fStatus = kStop;}
  void         Suspend() {fStatus = kSuspend;}

  ClassDef(ARay, 1)
};

#endif // A_RAY_H
