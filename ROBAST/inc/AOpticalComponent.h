// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_OPTICAL_COMPONENT_H
#define A_OPTICAL_COMPONENT_H

///////////////////////////////////////////////////////////////////////////////
//
// AOpticalComponent
//
// Abstract class for optical components
//
///////////////////////////////////////////////////////////////////////////////

#include "TGeoVolume.h"

class AOpticalComponent : public TGeoVolume {
 private:

 public:
  AOpticalComponent();
  AOpticalComponent(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AOpticalComponent();

  ClassDef(AOpticalComponent, 1)
};

#endif // A_OPTICAL_COMPONENT_H
