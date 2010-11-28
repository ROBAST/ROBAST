// $Id: AObscuration.h 3 2010-11-26 17:17:31Z oxon $
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

#ifndef A_OPTICAL_COMPONENT_H
#include "AOpticalComponent.h"
#endif

class AObscuration : public AOpticalComponent {
 private:

 public:
  AObscuration();
  AObscuration(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AObscuration();

  ClassDef(AObscuration, 1)
};

#endif // A_OBSCURATION_H
