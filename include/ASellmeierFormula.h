// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_SELLMEIER_FORMULA_H
#define A_SELLMEIER_FORMULA_H

#include "TF1.h"
#include "TGraph.h"

#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// ASellmeierFormula
//
// Sellmeier formula for calculation of refractive index
// See http://en.wikipedia.org/wiki/Sellmeier_equation
//
///////////////////////////////////////////////////////////////////////////////

class ASellmeierFormula : public ARefractiveIndex {
 private:
  Double_t fPar[6]; // Parameters

 public:
  ASellmeierFormula();
  ASellmeierFormula(Double_t B1, Double_t B2, Double_t B3,
                    Double_t C1, Double_t C2, Double_t C3);
  ASellmeierFormula(const Double_t* p);

  virtual Double_t GetIndex(Double_t lambda) const;
  virtual TF1*     FitData(TGraph* graph, const char* tf1name, Option_t* option = "");
  virtual TF1*     MakeGraph(const char* tf1name, Double_t xmin, Double_t xmax);

  ClassDef(ASellmeierFormula, 1)
};

#endif // A_SELLMEIER_FORMULA_H
