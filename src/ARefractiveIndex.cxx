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

ARefractiveIndex::ARefractiveIndex() : fRefractiveIndex(0),
                                       fExtinctionCoefficient(0) {}

//______________________________________________________________________________
ARefractiveIndex::~ARefractiveIndex() {
  SafeDelete(fRefractiveIndex);
  SafeDelete(fExtinctionCoefficient);
}

//______________________________________________________________________________
Double_t ARefractiveIndex::GetAbbeNumber() const {
  static Double_t nm = AOpticsManager::nm();

  Double_t nC = GetIndex(656.2725 * nm);
  Double_t nD = GetIndex(589.2938 * nm);
  Double_t nF = GetIndex(486.1327 * nm);

  return (nD - 1.) / (nF - nC);
}

//______________________________________________________________________________
void ARefractiveIndex::SetRefractiveIndex(const TGraph& graph) {
  SafeDelete(fRefractiveIndex);
  fRefractiveIndex = new TGraph;
  for(int i = 0; i < graph.GetN(); ++i){
    double x = graph.GetX()[i];
    double y = graph.GetY()[i];
    fRefractiveIndex->SetPoint(i, x, y);
  }
}

//______________________________________________________________________________
void ARefractiveIndex::SetExtinctionCoefficient(const TGraph& graph) {
  SafeDelete(fExtinctionCoefficient);
  fExtinctionCoefficient = new TGraph;
  for(int i = 0; i < graph.GetN(); ++i){
    double x = graph.GetX()[i];
    double y = graph.GetY()[i];
    fExtinctionCoefficient->SetPoint(i, x, y);
  }
}
