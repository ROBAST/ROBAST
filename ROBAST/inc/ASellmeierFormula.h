// $Id$
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
//
///////////////////////////////////////////////////////////////////////////////

#include "ARefractiveIndex.h"

class ASellmeierFormula : public ARefractiveIndex {
 private:
  Double_t         fPar[6]; // Parameters

 public:
  ASellmeierFormula();
  ASellmeierFormula(Double_t b1, Double_t b2, Double_t b3,
		    Double_t c1, Double_t c2, Double_t c3);
  ASellmeierFormula(const Double_t* p);
  virtual ~ASellmeierFormula();

  virtual Double_t GetIndex(Double_t lambda) const;

  ClassDef(ASellmeierFormula, 1)
};

#endif // A_SELLMEIER_FORMULA_H
