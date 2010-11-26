// $Id: ARay.h,v 1.3 2008/03/26 05:50:47 oxon Exp $
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

#include "TGeoTrack.h"
#include "TPolyLine3D.h"
#include "TVector3.h"

class ARay : public TGeoTrack {
 private:
  enum {kRun, kStop, kExit, kFocus, kSuspend};
  Double_t fLambda;    // Wavelength
  TVector3 fDirection; // Current direction vector
  Int_t    fStatus;    // status of ray
  
 public:
  ARay();
  ARay(Int_t id, Double_t lambda, Double_t x, Double_t y, Double_t z,
       Double_t t, Double_t dx, Double_t dy, Double_t dz);
  virtual ~ARay();

  Bool_t   IsExited() const;
  Bool_t   IsFocused() const;
  Bool_t   IsRunning() const;
  Bool_t   IsStopped() const;
  Bool_t   IsSuspended() const;
  void     Exit() { fStatus = kExit;}
  void     Focus() { fStatus = kFocus;}
  Double_t GetLambda() const { return fLambda;}
  void     GetLastPoint(Double_t* x) const;
  void     GetDirection(Double_t* d) const;
  void     SetDirection(Double_t dx, Double_t dy, Double_t dz);
  void     SetDirection(Double_t* d);
  void     SetLambda(Double_t lambda) { fLambda = lambda;}
  void     Stop() { fStatus = kStop;}
  void     Suspend() { fStatus = kSuspend;}

  ClassDef(ARay, 1)
};

#endif // A_RAY_H
