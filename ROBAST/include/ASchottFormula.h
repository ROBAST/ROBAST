// $Id: ASchottFormula.h,v 1.2 2008/03/26 05:50:47 oxon Exp $
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
// SCHOTT's formula for calculation of refractive index
//
///////////////////////////////////////////////////////////////////////////////

#include "ARefractiveIndex.h"

class ASchottFormula : public ARefractiveIndex {
 private:
  Double_t fPar[6]; // Parameters

 public:
  ASchottFormula();
  ASchottFormula(Double_t p0, Double_t p1, Double_t p2, Double_t p3,
                 Double_t p4, Double_t p5);
  ASchottFormula(const Double_t* p);
  virtual ~ASchottFormula();

  virtual Double_t GetIndex(Double_t lambda) const;

  ClassDef(ASchottFormula, 1)
};

#endif // A_SCHOTT_FORMULA_H
