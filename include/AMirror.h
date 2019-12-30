// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_MIRROR_H
#define A_MIRROR_H

#include "TGraph.h"
#include "TGraph2D.h"
#include "TH2.h"

#include "AOpticalComponent.h"

///////////////////////////////////////////////////////////////////////////////
//
// AMirror
//
// Mirror class
//
///////////////////////////////////////////////////////////////////////////////

class AMirror : public AOpticalComponent {
 private:
  Double_t fReflectance;
  std::shared_ptr<const TGraph>
      fReflectance1D;  // Reflectance data (ref v.s. wavelength)
  std::shared_ptr<TGraph2D>
      fReflectance2D;  // Reflectance data (ref v.s. angle v.s. wavelength)
  std::shared_ptr<TH2>
      fReflectanceTH2;  // Reflectance data (ref v.s. angle v.s. wavelength)

 public:
  AMirror();
  AMirror(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AMirror();

  Double_t GetReflectance(Double_t lambda, Double_t angle /* (rad) */);
  void SetReflectance(Double_t ref) { fReflectance = ref; }
  void SetReflectance(std::shared_ptr<const TGraph> ref) { fReflectance1D = ref; }
  void SetReflectance(std::shared_ptr<TH2> ref) { fReflectanceTH2 = ref; }
  void SetReflectance(std::shared_ptr<TGraph2D> ref) { fReflectance2D = ref; }

  ClassDef(AMirror, 1)
};

#endif  // A_MIRROR_H
