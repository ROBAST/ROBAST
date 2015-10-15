/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AObscuration
//
// Obscuration class
//
///////////////////////////////////////////////////////////////////////////////

#include "AObscuration.h"

ClassImp(AObscuration)

AObscuration::AObscuration()
{
  // Default constructor
  SetLineColor(12);
}

//_____________________________________________________________________________
AObscuration::AObscuration(const char* name, const TGeoShape* shape,
                           const TGeoMedium* med)
  : AOpticalComponent(name, shape, med)
{
  SetLineColor(12);
}

//_____________________________________________________________________________
AObscuration::~AObscuration()
{
}
