// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_LENS_H
#define A_LENS_H

#include "TGraph.h"

#include "AOpticalComponent.h"
#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// ALens
//
// Lens class
//
///////////////////////////////////////////////////////////////////////////////

class ALens : public AOpticalComponent {
 private:
  TGraph*           fAbsorptionLength; // Absorption length of the material
  ARefractiveIndex* fIndex; // Refractive index
  Double_t          fConstantIndex; // Constant refractive index
  Double_t          fConstantAbsorptionLength; // Absorption length of the material

 public:
  ALens();
  ALens(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~ALens();

  virtual Double_t GetAbsorptionLength(Double_t lambda) const;
  virtual Double_t GetRefractiveIndex(Double_t lambda) const;
  virtual void     SetAbsorptionLength(TGraph* graph) {fAbsorptionLength = graph;}
  virtual void     SetConstantAbsorptionLength(Double_t length) {fConstantAbsorptionLength = length;}
  virtual void     SetConstantRefractiveIndex(Double_t index) {fConstantIndex = index;}
  virtual void     SetRefractiveIndex(ARefractiveIndex* index) {fIndex = index;}


  ClassDef(ALens, 1)
};

#endif // A_LENS_H
