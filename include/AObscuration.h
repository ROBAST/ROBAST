// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_OBSCURATION_H
#define A_OBSCURATION_H

#include "AOpticalComponent.h"

///////////////////////////////////////////////////////////////////////////////
//
// AObscuration
//
// Obscuration class
//
///////////////////////////////////////////////////////////////////////////////

class AObscuration : public AOpticalComponent {
 private:

 public:
  AObscuration();
  AObscuration(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AObscuration();

  ClassDef(AObscuration, 1)
};

#endif // A_OBSCURATION_H
