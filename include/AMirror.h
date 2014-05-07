// $Id: AMirror.h 3 2010-11-26 17:17:31Z oxon $
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

#ifndef ROOT_TGraph
#include "TGraph.h"
#endif
#ifndef ROOT_TGraph2D
#include "TGraph2D.h"
#endif

#ifndef A_OPTICAL_COMPONENT_H
#include "AOpticalComponent.h"
#endif

class AMirror : public AOpticalComponent {
 private:
  TGraph*   fReflectivity1D; // Reflectivity data (ref v.s. wavelength)
  TGraph2D* fReflectivity2D; // Reflectivity data (ref v.s. angle v.s. wavelength)

 public:
  AMirror();
  AMirror(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AMirror();

  virtual Double_t GetReflectivity(Double_t lambda, Double_t angle /* (rad) */);
  virtual void     SetReflectivity(TGraph* ref) {fReflectivity1D = ref;}
  virtual void     SetReflectivity(TGraph2D* ref) {fReflectivity2D = ref;}

  ClassDef(AMirror, 1)
};

#endif // A_MIRROR_H
