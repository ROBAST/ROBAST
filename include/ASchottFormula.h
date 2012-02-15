// $Id: ASchottFormula.h 7 2010-11-28 01:42:20Z oxon $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_SCHOTT_FORMULA_H
#define A_SCHOTT_FORMULA_H

///////////////////////////////////////////////////////////////////////////////
//
// ASchottFormula
//
// SCHOTT's formula for calculation of refractive index. SCHOTT does not use
// this formula in its catalog any more. Sellmeier's formula is used instead.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef A_REFRACTIVE_INDEX_H
#include "ARefractiveIndex.h"
#endif

class ASchottFormula : public ARefractiveIndex {
 private:
  Double_t fPar[6]; // Parameters

 public:
  ASchottFormula();
  ASchottFormula(Double_t A0, Double_t A1, Double_t A2, Double_t A3,
                 Double_t A4, Double_t A5);
  ASchottFormula(const Double_t* p);

  virtual Double_t GetIndex(Double_t lambda /* (m) */) const;

  ClassDef(ASchottFormula, 1)
};

#endif // A_SCHOTT_FORMULA_H
