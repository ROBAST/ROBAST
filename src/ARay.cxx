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
#include "AOpticsManager.h"
#include "TMath.h"

ClassImp(ARay);

ARay::ARay() {
  // Default constructor
  fLambda = 0;
  fDirection = TVector3(1, 0, 0);
  fStatus = kRun;
}

//_____________________________________________________________________________
ARay::ARay(Int_t id, Double_t lambda, Double_t x, Double_t y, Double_t z,
           Double_t t, Double_t nx, Double_t ny, Double_t nz)
    : TGeoTrack(id, 22 /*photon*/, 0, 0) {
  // Constructor
  AddPoint(x, y, z, t);
  fLambda = lambda;
  SetDirection(nx, ny, nz);
  fStatus = kRun;
}

//_____________________________________________________________________________
ARay::~ARay() {}

//_____________________________________________________________________________
TGeoNode* ARay::FindNodeStartWith(const char* name) const {
  for (Int_t i = 0; i < fNodeHisotry.GetEntries(); i++) {
    TGeoNode* node = (TGeoNode*)fNodeHisotry.At(i);
    if (node and strncmp(node->GetName(), name, strlen(name)) == 0) {
      return node;
    }
  }

  return 0;
}

//_____________________________________________________________________________
Int_t ARay::FindNodeNumberStartWith(const char* name) const {
  for (Int_t i = 0; i < fNodeHisotry.GetEntries(); i++) {
    TGeoNode* node = (TGeoNode*)fNodeHisotry.At(i);
    if (node and strncmp(node->GetName(), name, strlen(name)) == 0) {
      return i;
    }
  }

  return -1;
}

//_____________________________________________________________________________
void ARay::GetDirection(Double_t* v) const { fDirection.GetXYZ(v); }

//_____________________________________________________________________________
void ARay::GetLastPoint(Double_t* x) const {
  GetPoint(GetNpoints() - 1, x[0], x[1], x[2], x[3]);
}

//_____________________________________________________________________________
Bool_t ARay::IsAbsorbed() const {
  if (fStatus == kAbsorb) {
    return kTRUE;
  }

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsExited() const {
  if (fStatus == kExit) {
    return kTRUE;
  }

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsFocused() const {
  if (fStatus == kFocus) {
    return kTRUE;
  }

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsRunning() const {
  if (fStatus == kRun) {
    return kTRUE;
  }

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsStopped() const {
  if (fStatus == kStop) {
    return kTRUE;
  }

  return kFALSE;
}

//_____________________________________________________________________________
Bool_t ARay::IsSuspended() const {
  if (fStatus == kSuspend) {
    return kTRUE;
  }

  return kFALSE;
}

//_____________________________________________________________________________
TColor* ARay::MakeColor() const {
  // The origianl code in FORTRAN was written by Dan Bruton
  // See http://www.physics.sfasu.edu/astro/color/spectra.html
  Double_t wl = fLambda / AOpticsManager::nm();
  Double_t R, G, B;

  if (300. <= wl && wl < 380.) {
    R = (wl - 300.) / (380. - 300.);
    G = 0.;
    B = (wl - 300.) / (380. - 300.);
  } else if (380. <= wl && wl < 440.) {
    R = -(wl - 440.) / (440. - 380.);
    G = 0.;
    B = 1.;
  } else if (440. <= wl && wl < 490.) {
    R = 0.;
    G = (wl - 440.) / (490. - 440.);
    B = 1.;
  } else if (490. <= wl && wl < 510.) {
    R = 0.;
    G = 1.;
    B = -(wl - 510.) / (510. - 490.);
  } else if (510. <= wl && wl < 580.) {
    R = (wl - 510.) / (580. - 510.);
    G = 1.;
    B = 0.;
  } else if (580. <= wl && wl < 645) {
    R = 1.;
    G = -(wl - 645.) / (645. - 580.);
    B = 0.;
  } else if (645 <= wl && wl < 780.) {
    R = 1.;
    G = 0.;
    B = 0.;
  } else if (780. <= wl && wl < 880.) {
    R = -(wl - 880.) / (880. - 781.);
    G = 0.;
    B = 0.;
  } else {
    R = 0.;
    G = 0.;
    B = 0.0;
  }

  Double_t sss = 0.;
  if (300. <= wl && wl < 380.) {
    sss = 0.3;
  } else if (380. <= wl && wl < 420.) {
    sss = 0.3 + 0.7 * (wl - 380.) / (420. - 380.);
  } else if (420. <= wl && wl < 700.) {
    sss = 1.0;
  } else if (700. <= wl && wl < 781) {
    sss = 0.3 + 0.7 * (780. - wl) / (780. - 700.);
  } else if (781 <= wl && wl < 880.) {
    sss = 0.3;
  }

  const Double_t gamma = 0.80;
  R = R > 0. ? TMath::Power(R * sss, gamma) : 0.;
  G = G > 0. ? TMath::Power(G * sss, gamma) : 0.;
  B = B > 0. ? TMath::Power(B * sss, gamma) : 0.;

  Int_t ci = TColor::GetFreeColorIndex();
  return new TColor(ci, R, G, B);
}

//_____________________________________________________________________________
TPolyLine3D* ARay::MakePolyLine3D() const {
  TPolyLine3D* pol = new TPolyLine3D;

  for (Int_t i = 0; i < GetNpoints(); i++) {
    Double_t x, y, z, t;
    GetPoint(i, x, y, z, t);
    pol->SetPoint(i, x, y, z);
  }

  return pol;
}

//_____________________________________________________________________________
void ARay::SetDirection(Double_t* d) {
  Double_t mag = TMath::Sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
  if (mag > 0) {
    fDirection.SetXYZ(d[0] / mag, d[1] / mag, d[2] / mag);
  }
}

//_____________________________________________________________________________
void ARay::SetDirection(Double_t dx, Double_t dy, Double_t dz) {
  Double_t mag = TMath::Sqrt(dx * dx + dy * dy + dz * dz);
  if (mag > 0) {
    fDirection.SetXYZ(dx / mag, dy / mag, dz / mag);
  }
}
