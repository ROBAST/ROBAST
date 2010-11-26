// $Id$
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

#include "AOpticalComponent.h"
#include "ARefractiveIndex.h"

#include "TGraph.h"

class ALens : public AOpticalComponent {
 private:
  ARefractiveIndex* fIndex; // Refractive index
  Double_t          fConstantIndex; // Constant refractive index

 public:
  ALens();
  ALens(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~ALens();

  virtual Double_t GetRefractiveIndex(Double_t lambda);
  virtual void     SetConstantRefractiveIndex(Double_t index) {fConstantIndex = index;}
  virtual void     SetRefractiveIndex(ARefractiveIndex* index) {fIndex = index;}


  ClassDef(ALens, 1)
};

#endif // A_LENS_H
