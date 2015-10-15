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

#include "AOpticsManager.h"
#include "ARefractiveIndex.h"

ClassImp(ARefractiveIndex)

ARefractiveIndex::ARefractiveIndex()
{
}

//______________________________________________________________________________
ARefractiveIndex::~ARefractiveIndex()
{
}

//______________________________________________________________________________
Double_t ARefractiveIndex::GetAbbeNumber() const
{
  static Double_t nm = AOpticsManager::nm();

  Double_t nC = GetIndex(656.2725*nm);
  Double_t nD = GetIndex(589.2938*nm);
  Double_t nF = GetIndex(486.1327*nm);

  return (nD - 1.)/(nF - nC);
}
