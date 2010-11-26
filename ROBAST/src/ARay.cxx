// $Id: ARay.cxx,v 1.3 2008/03/26 05:50:47 oxon Exp $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ARay
//
// Classical ray class
//
///////////////////////////////////////////////////////////////////////////////

#include "ARay.h"
#include "TMath.h"

ClassImp(ARay)

ARay::ARay()
{
  // Default constructor
  fLambda = 0;
  fDirection = TVector3(1, 0, 0);
  fStatus = kRun;
}

//_____________________________________________________________________________
ARay::ARay(Int_t id, Double_t lambda, Double_t x, Double_t y, Double_t z,
           Double_t t, Double_t nx, Double_t ny, Double_t nz)
  : TGeoTrack(id, 22 /*photon*/, 0, 0)
{
  // Constructor
  AddPoint(x, y, z, t);
  fLambda = lambda;
  SetDirection(nx, ny, nz);
  fStatus = kRun;
}

//_____________________________________________________________________________
ARay::~ARay()
{
}

//_____________________________________________________________________________
void ARay::GetDirection(Double_t* v) const
{
  fDirection.GetXYZ(v);
}

//_____________________________________________________________________________
void ARay::GetLastPoint(Double_t* x) const
{
  GetPoint(GetNpoints() - 1, x[0], x[1], x[2], x[3]);
}

//_____________________________________________________________________________
Bool_t ARay::IsExited() const
{
  if(fStatus == kExit){
    return kTRUE;
  } // if

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsFocused() const
{
  if(fStatus == kFocus){
    return kTRUE;
  } // if

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsRunning() const
{
  if(fStatus == kRun){
    return kTRUE;
  } // if

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsStopped() const
{
  if(fStatus == kStop){
    return kTRUE;
  } // if

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsSuspended() const
{
  if(fStatus == kSuspend){
    return kTRUE;
  } // if

  return kFALSE;
}

//_____________________________________________________________________________
void ARay::SetDirection(Double_t* d)
{
  Double_t mag = TMath::Sqrt(d[0]*d[0] + d[1]*d[1] +d[2]*d[2]);
  if(mag > 0){
    fDirection.SetXYZ(d[0]/mag, d[1]/mag, d[2]/mag);
  } // if
}

//_____________________________________________________________________________
void ARay::SetDirection(Double_t dx, Double_t dy, Double_t dz)
{
  Double_t mag = TMath::Sqrt(dx*dx + dy*dy + dz*dz);
  if(mag > 0){
    fDirection.SetXYZ(dx/mag, dy/mag, dz/mag);
  } // if
}
