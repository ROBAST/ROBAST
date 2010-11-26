// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_OBSCURATION_H
#define A_OBSCURATION_H

///////////////////////////////////////////////////////////////////////////////
//
// AObscuration
//
// Obscuration class
//
///////////////////////////////////////////////////////////////////////////////

#include "AOpticalComponent.h"

#include "TGraph.h"

class AObscuration : public AOpticalComponent {
 private:

 public:
  AObscuration();
  AObscuration(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AObscuration();

  ClassDef(AObscuration, 1)
};

#endif // A_OBSCURATION_H
