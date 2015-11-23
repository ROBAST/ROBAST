// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_RAY_H
#define A_RAY_H

#include "TGeoNode.h"
#include "TGeoTrack.h"
#include "TPolyLine3D.h"
#include "TVector3.h"

///////////////////////////////////////////////////////////////////////////////
//
// ARay
//
// Classical ray class
//
///////////////////////////////////////////////////////////////////////////////

class ARay : public TGeoTrack {
 private:
  enum {kRun, kStop, kExit, kFocus, kSuspend, kAbsorb};
  Double_t     fLambda;     // Wavelength
  TVector3     fDirection;  // Current direction vector
  Int_t        fStatus;     // status of ray
  TObjArray    fNodeHisotry; // History of nodes on which the photon has hi

 public:
  ARay();
  ARay(Int_t id, Double_t lambda, Double_t x, Double_t y, Double_t z,
       Double_t t, Double_t dx, Double_t dy, Double_t dz);
  virtual ~ARay();

  void         Absorb() {fStatus = kAbsorb;}
  void         Exit() {fStatus = kExit;}
  void         Focus() {fStatus = kFocus;}
  void         GetDirection(Double_t* d) const;
  const TObjArray* GetNodeHistory() const {return &fNodeHisotry;}
  Double_t     GetLambda() const {return fLambda;}
  void         GetLastPoint(Double_t* x) const;
  void         AddNode(TGeoNode* node) {fNodeHisotry.Add(node);}
  TGeoNode*    FindNode(const char* name) const {return (TGeoNode*)fNodeHisotry.FindObject(name);}
  TGeoNode*    FindNodeStartWith(const char* name) const;
  Int_t        FindNodeNumberStartWith(const char* name) const;
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
  void         Stop() {fStatus = kStop;}
  void         Suspend() {fStatus = kSuspend;}

  ClassDef(ARay, 1)
};

#endif // A_RAY_H
