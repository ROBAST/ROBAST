// $Id$
// Author: Akira Okumura 2007/09/24

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
  fReflectance = 0;
  SetLineColor(16);
}

//_____________________________________________________________________________
AMirror::AMirror(const char* name, const TGeoShape* shape,
                 const TGeoMedium* med) : AOpticalComponent(name, shape, med)
{
  fReflectance = 0;
  SetLineColor(16);
}

//_____________________________________________________________________________
AMirror::~AMirror()
{
  SafeDelete(fReflectance);
}

//_____________________________________________________________________________
Double_t AMirror::GetReflectance(Double_t lambda, Double_t angle)
{
  // Return mirror reflectance for a photon whose wavelength is lambda, and
  // whose incident angle is "angle" (deg)
  Double_t ret;

  if(fReflectance){
    if(fReflectance->InheritsFrom(TGraph2D::Class())){
      ret = ((TGraph2D*)fReflectance)->Interpolate(lambda, angle);
    } else if(fReflectance->InheritsFrom(TGraph::Class())){
      ret = ((TGraph*)fReflectance)->Eval(lambda);
    } else {
      ret = 1.;
    } // if
    ret = ret > 1 ? 1 : ret;
    ret = ret < 0 ? 0 : ret;
  } else {
    ret = 1;
  } // if

  return ret;
}
