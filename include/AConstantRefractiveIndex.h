// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_CONSTANT_REFRACTIVE_INDEX_H
#define A_CONSTANT_REFRACTIVE_INDEX_H

#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// AConstantRefractiveIndex
//
// Abstract class for refractive index
//
///////////////////////////////////////////////////////////////////////////////

class AConstantRefractiveIndex : public ARefractiveIndex {
 public:
  AConstantRefractiveIndex(Double_t n, Double_t k = 0.);
  virtual ~AConstantRefractiveIndex() {}

  ClassDef(AConstantRefractiveIndex, 1)
};

#endif  // A_CONSTANT_REFRACTIVE_INDEX_H
