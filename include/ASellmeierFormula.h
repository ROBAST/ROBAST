// $Id: ASellmeierFormula.h 6 2010-11-26 17:33:15Z oxon $
// Author: Akira Okumura 2007/10/01

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_SELLMEIER_FORMULA_H
#define A_SELLMEIER_FORMULA_H

///////////////////////////////////////////////////////////////////////////////
//
// ASellmeierFormula
//
// Sellmeier formula for calculation of refractive index
// See http://en.wikipedia.org/wiki/Sellmeier_equation
//
///////////////////////////////////////////////////////////////////////////////

#ifndef A_REFRACTIVE_INDEX_H
#include "ARefractiveIndex.h"
#endif

class ASellmeierFormula : public ARefractiveIndex {
 private:
  Double_t fPar[6]; // Parameters

 public:
  ASellmeierFormula();
  ASellmeierFormula(Double_t B1, Double_t B2, Double_t B3,
                    Double_t C1, Double_t C2, Double_t C3);
  ASellmeierFormula(const Double_t* p);

  virtual Double_t GetIndex(Double_t lambda /* (m) */) const;

  ClassDef(ASellmeierFormula, 1)
};

#endif // A_SELLMEIER_FORMULA_H
