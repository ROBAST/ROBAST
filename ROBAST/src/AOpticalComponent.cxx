// $Id: AOpticalComponent.cxx,v 1.2 2008/03/26 05:50:47 oxon Exp $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AOpticalComponent
//
// Classical ray class
//
///////////////////////////////////////////////////////////////////////////////

#include "AOpticalComponent.h"

ClassImp(AOpticalComponent)

AOpticalComponent::AOpticalComponent() : TGeoVolume()
{
}

//_____________________________________________________________________________
AOpticalComponent::AOpticalComponent(const char* name, const TGeoShape* shape,
                                     const TGeoMedium* med)
  : TGeoVolume(name, shape, med)
{
}

//_____________________________________________________________________________
AOpticalComponent::~AOpticalComponent()
{
}
