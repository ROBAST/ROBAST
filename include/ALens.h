// $Id: ALens.h 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_LENS_H
#define A_LENS_H

///////////////////////////////////////////////////////////////////////////////
//
// ALens
//
// Lens class
//
///////////////////////////////////////////////////////////////////////////////

#ifndef A_OPTICAL_COMPONENT_H
#include "AOpticalComponent.h"
#endif
#ifndef A_REFRACTIVE_INDEX_H
#include "ARefractiveIndex.h"
#endif

class ALens : public AOpticalComponent {
 private:
  ARefractiveIndex* fIndex; // Refractive index
  Double_t          fConstantIndex; // Constant refractive index
  Double_t          fConstantAbsorptionLength; // Absorption length of the material

 public:
  ALens();
  ALens(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~ALens();

  virtual Double_t GetRefractiveIndex(Double_t lambda) const;
  virtual Double_t GetAbsorptionLength(Double_t lambda) const {return fConstantAbsorptionLength;}
  virtual void     SetConstantAbsorptionLength(Double_t length) {fConstantAbsorptionLength = length;}
  virtual void     SetConstantRefractiveIndex(Double_t index) {fConstantIndex = index;}
  virtual void     SetRefractiveIndex(ARefractiveIndex* index) {fIndex = index;}


  ClassDef(ALens, 1)
};

#endif // A_LENS_H
