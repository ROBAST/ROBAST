// $Id$
// Author: Akira Okumura 2007/10/01

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AFocalSurface
//
// Focal surface
//
///////////////////////////////////////////////////////////////////////////////

#include "AFocalSurface.h"

ClassImp(AFocalSurface)

AFocalSurface::AFocalSurface() : fQuantumEfficiency(0)
{
  // Default constructor
  SetLineColor(2);
}

//_____________________________________________________________________________
AFocalSurface::AFocalSurface(const char* name, const TGeoShape* shape,
                             const TGeoMedium* med)
  : AOpticalComponent(name, shape, med), fQuantumEfficiency(0)
{
  // Constructor
  SetLineColor(2);
}

//_____________________________________________________________________________
Double_t AFocalSurface::GetQuantumEfficiency(Double_t lambda) const
{
  if(fQuantumEfficiency){
    return fQuantumEfficiency->Eval(lambda);
  } else {
    return 1.;
  } // if
}
