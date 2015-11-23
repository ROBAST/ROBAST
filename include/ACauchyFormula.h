// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_CAUCHY_FORMULA_H
#define A_CAUCHY_FORMULA_H

#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// ACauchyFormula
//
// Cauchy's formula for calculation of refractive index
// See http://en.wikipedia.org/wiki/Cauchy's_equation
//
///////////////////////////////////////////////////////////////////////////////

class ACauchyFormula : public ARefractiveIndex {
 private:
  Double_t fPar[3]; // Parameters

 public:
  ACauchyFormula();
  ACauchyFormula(Double_t A, Double_t B, Double_t C = 0.);
  ACauchyFormula(const Double_t* p);

  virtual Double_t GetIndex(Double_t lambda /* (m) */) const;

  ClassDef(ACauchyFormula, 1)
};

#endif // A_CAUCHY_FORMULA_H
