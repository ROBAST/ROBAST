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

AFocalSurface::AFocalSurface()
{
  // Default constructor
  SetLineColor(2);
}

//_____________________________________________________________________________
AFocalSurface::AFocalSurface(const char* name, const TGeoShape* shape,
                             const TGeoMedium* med)
  : AOpticalComponent(name, shape, med)
{
  // Constructor
  SetLineColor(2);
}
