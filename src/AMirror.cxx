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

ClassImp(AMirror)

//_____________________________________________________________________________
AMirror::AMirror()
{
  // Default constructor
  fReflectance = 1.0;
  fReflectance1D = 0;
  fReflectance2D = 0;
  fReflectanceTH2 = 0;
  SetLineColor(16);
}

//_____________________________________________________________________________
AMirror::AMirror(const char* name, const TGeoShape* shape,
                 const TGeoMedium* med) : AOpticalComponent(name, shape, med)
{
  fReflectance = 1.0;
  fReflectance1D = 0;
  fReflectance2D = 0;
  fReflectanceTH2 = 0;
  SetLineColor(16);}

//_____________________________________________________________________________
AMirror::~AMirror()
{
  SafeDelete(fReflectance1D);
  SafeDelete(fReflectance2D);
  SafeDelete(fReflectanceTH2);
}

//_____________________________________________________________________________
Double_t AMirror::GetReflectance(Double_t lambda, Double_t angle)
{
  // Return mirror reflectivity for a photon whose wavelength is lambda, and
  // whose incident angle is "angle" (rad)
  Double_t ret;

  if(fReflectance2D){
    ret = fReflectance2D->Interpolate(lambda, angle);
  } else if(fReflectanceTH2){
    ret = fReflectanceTH2->Interpolate(lambda, angle);
  } else if(fReflectance1D){
    ret = fReflectance1D->Eval(lambda);
  } else {
    ret = fReflectance;
  } // if

  ret = ret > 1 ? 1 : ret;
  ret = ret < 0 ? 0 : ret;

  return ret;
}
