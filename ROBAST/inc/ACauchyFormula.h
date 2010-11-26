// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_CAUCHY_FORMULA_H
#define A_CAUCHY_FORMULA_H

///////////////////////////////////////////////////////////////////////////////
//
// ACauchyFormula
//
// Cauchy's formula for calculation of refractive index
//
///////////////////////////////////////////////////////////////////////////////

#include "ARefractiveIndex.h"

class ACauchyFormula : public ARefractiveIndex {
 private:
  Double_t         fPar[3]; // Parameters

 public:
  ACauchyFormula();
  ACauchyFormula(Double_t p0, Double_t p1, Double_t p2);
  ACauchyFormula(const Double_t* p);
  virtual ~ACauchyFormula();

  virtual Double_t GetIndex(Double_t lambda /*[cm]*/) const;

  ClassDef(ACauchyFormula, 1)
};

#endif // A_CAUCHY_FORMULA_H
