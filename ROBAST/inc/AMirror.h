// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_MIRROR_H
#define A_MIRROR_H

///////////////////////////////////////////////////////////////////////////////
//
// AMirror
//
// Mirror class
//
///////////////////////////////////////////////////////////////////////////////

#include "AOpticalComponent.h"

class AMirror : public AOpticalComponent {
 private:
  TObject* fReflectance; // Reflectance data

 public:
  AMirror();
  AMirror(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AMirror();

  virtual Double_t GetReflectance(Double_t lambda, Double_t angle /*(deg)*/);
  virtual void     SetReflectance(TObject* ref) {fReflectance = ref;}

  ClassDef(AMirror, 1)
};

#endif // A_MIRROR_H
