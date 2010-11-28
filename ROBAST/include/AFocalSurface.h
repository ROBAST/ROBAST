// $Id: AFocalSurface.h,v 1.2 2008/03/26 05:50:47 oxon Exp $
// Author: Akira Okumura 2007/10/01

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_FOCAL_SURFACE_H
#define A_FOCAL_SURFACE_H

///////////////////////////////////////////////////////////////////////////////
//
// AFocalSurface
//
// Focal surface class
//
///////////////////////////////////////////////////////////////////////////////

#include "AOpticalComponent.h"

class AFocalSurface : public AOpticalComponent {
 private:

 public:
  AFocalSurface();
  AFocalSurface(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AFocalSurface();

  ClassDef(AFocalSurface, 1)
};

#endif // A_FOCAL_SURFACE_H
