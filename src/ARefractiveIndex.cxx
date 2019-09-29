/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ARefractiveIndex
//
// Abstract class for refractive index
//
///////////////////////////////////////////////////////////////////////////////

#include "ARefractiveIndex.h"
#include "AOpticsManager.h"

ClassImp(ARefractiveIndex);

ARefractiveIndex::ARefractiveIndex(Double_t n, Double_t k) {
  fRefractiveIndex = std::make_shared<TGraph>();
  fRefractiveIndex->SetPoint(0, 0, n);

  if (k > 0) {
    fExtinctionCoefficient = std::make_shared<TGraph>();
    fExtinctionCoefficient->SetPoint(0, 0, k);
  }
}

//______________________________________________________________________________
Double_t ARefractiveIndex::GetAbbeNumber() const {
  static Double_t nm = AOpticsManager::nm();

  Double_t nC = GetRefractiveIndex(656.2725 * nm);
  Double_t nD = GetRefractiveIndex(589.2938 * nm);
  Double_t nF = GetRefractiveIndex(486.1327 * nm);

  return (nD - 1.) / (nF - nC);
}
