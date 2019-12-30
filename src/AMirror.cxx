/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AMirror
//
// Mirror class
//
///////////////////////////////////////////////////////////////////////////////

#include "AMirror.h"
#include "TGraph.h"
#include "TGraph2D.h"

ClassImp(AMirror);

//_____________________________________________________________________________
AMirror::AMirror() {
  // Default constructor
  fReflectance = 1.0;
  SetLineColor(16);
}

//_____________________________________________________________________________
AMirror::AMirror(const char* name, const TGeoShape* shape,
                 const TGeoMedium* med)
    : AOpticalComponent(name, shape, med) {
  fReflectance = 1.0;
  SetLineColor(16);
}

//_____________________________________________________________________________
AMirror::~AMirror() {}

//_____________________________________________________________________________
Double_t AMirror::GetReflectance(Double_t lambda, Double_t angle) {
  // Return mirror reflectance for a photon whose wavelength is lambda, and
  // whose incident angle is "angle" (rad)
  // This method should be const but TGraph2D::Interpolate and TH2::Interpolate
  // are non-const methods.
  Double_t ret;

  if (fReflectance2D) {
    ret = fReflectance2D->Interpolate(lambda, angle); // non-const as of 6.18
  } else if (fReflectanceTH2) {
    ret = fReflectanceTH2->Interpolate(lambda, angle); // const since ROOT 6.19
  } else if (fReflectance1D) {
    ret = fReflectance1D->Eval(lambda); // const
  } else {
    ret = fReflectance;
  }

  ret = ret > 1 ? 1 : ret;
  ret = ret < 0 ? 0 : ret;

  return ret;
}
