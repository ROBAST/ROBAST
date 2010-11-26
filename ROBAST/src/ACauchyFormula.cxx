// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ACauchyFormula
//
// Cauchy's formula for calculation of refractive index
//
///////////////////////////////////////////////////////////////////////////////

#include "ACauchyFormula.h"
#include "AOpticsManager.h"
#include "TMath.h"

ClassImp(ACauchyFormula)

ACauchyFormula::ACauchyFormula()
{
}

//_____________________________________________________________________________
ACauchyFormula::ACauchyFormula(Double_t p0, Double_t p1, Double_t p2)
{
  fPar[0] = p0;
  fPar[1] = p1;
  fPar[2] = p2;
}

//_____________________________________________________________________________
ACauchyFormula::ACauchyFormula(const Double_t* p)
{
  for(Int_t i = 0; i < 3; i++){
    fPar[i] = p[i];
  } // i
}

//_____________________________________________________________________________
ACauchyFormula::~ACauchyFormula()
{
}

//_____________________________________________________________________________
Double_t ACauchyFormula::GetIndex(Double_t lambda) const
{
  lambda /= AOpticsManager::um(); // Convert (m) to (um)
  return fPar[0] + fPar[1]*TMath::Power(lambda, -2) + fPar[2]*TMath::Power(lambda, -4);
}
