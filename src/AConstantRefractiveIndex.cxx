/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AConstantRefractiveIndex
//
// Abstract class for refractive index
//
///////////////////////////////////////////////////////////////////////////////

#include "AConstantRefractiveIndex.h"
#include "AOpticsManager.h"

ClassImp(AConstantRefractiveIndex);

AConstantRefractiveIndex::AConstantRefractiveIndex(Double_t n, Double_t k)
  : ARefractiveIndex() {
  fRefractiveIndex = new TGraph;
  fRefractiveIndex->SetPoint(0, 0, n);
  fRefractiveIndex->SetPoint(1, 1, n); // other region will be extrapolated

  if (k != 0) {
    fExtinctionCoefficient = new TGraph;
    fExtinctionCoefficient->SetPoint(0, 0, k);
    fExtinctionCoefficient->SetPoint(1, 1, k); // other region will be extrapolated
  }
}
