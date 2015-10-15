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
TGeoNode* ARay::FindNodeStartWith(const char* name) const
{
  for(Int_t i = 0; i < fNodeHisotry.GetEntries(); i++){
    TGeoNode* node = (TGeoNode*)fNodeHisotry.At(i);
    if(node and strncmp(node->GetName(), name, strlen(name)) == 0){
      return node;
    } // if
  } // i

  return 0;
}

//_____________________________________________________________________________
Int_t ARay::FindNodeNumberStartWith(const char* name) const
{
  for(Int_t i = 0; i < fNodeHisotry.GetEntries(); i++){
    TGeoNode* node = (TGeoNode*)fNodeHisotry.At(i);
    if(node and strncmp(node->GetName(), name, strlen(name)) == 0){
      return i;
    } // if
  } // i

  return -1;
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
Bool_t ARay::IsAbsorbed() const
{
  if(fStatus == kAbsorb){
    return kTRUE;
  } // if

  return kFALSE;
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
TPolyLine3D* ARay::MakePolyLine3D() const
{
  TPolyLine3D* pol = new TPolyLine3D;

  for(Int_t i = 0; i < GetNpoints(); i++){
    Double_t x, y, z, t;
    GetPoint(i, x, y, z, t);
    pol->SetPoint(i, x, y, z);
  } // i

  return pol;
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
