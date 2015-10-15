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
  fReflectivity1D = 0;
  fReflectivity2D = 0;
  SetLineColor(16);
}

//_____________________________________________________________________________
AMirror::AMirror(const char* name, const TGeoShape* shape,
                 const TGeoMedium* med) : AOpticalComponent(name, shape, med)
{
  fReflectivity1D = 0;
  fReflectivity2D = 0;
  SetLineColor(16);
}

//_____________________________________________________________________________
AMirror::~AMirror()
{
  SafeDelete(fReflectivity1D);
  SafeDelete(fReflectivity2D);
}

//_____________________________________________________________________________
Double_t AMirror::GetReflectivity(Double_t lambda, Double_t angle)
{
  // Return mirror reflectivity for a photon whose wavelength is lambda, and
  // whose incident angle is "angle" (rad)
  Double_t ret;

  if(fReflectivity2D){
    ret = fReflectivity2D->Interpolate(lambda, angle);
  } else if(fReflectivity1D){
    ret = fReflectivity1D->Eval(lambda);
  } else {
    ret = 1.;
  } // if

  ret = ret > 1 ? 1 : ret;
  ret = ret < 0 ? 0 : ret;

  return ret;
}
