/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ACauchyFormula
//
// Cauchy's formula for calculation of refractive index
// See http://en.wikipedia.org/wiki/Cauchy's_equation
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
ACauchyFormula::ACauchyFormula(Double_t A, Double_t B, Double_t C)
{
  // n(lambda) = A + B/lambda^2 + C/lambda^4
  // where lambda is measured in (um)
  fPar[0] = A;
  fPar[1] = B;
  fPar[2] = C;
}

//_____________________________________________________________________________
ACauchyFormula::ACauchyFormula(const Double_t* p)
{
  for(Int_t i = 0; i < 3; i++){
    fPar[i] = p[i];
  } // i
}

//_____________________________________________________________________________
Double_t ACauchyFormula::GetIndex(Double_t lambda) const
{
  // Calculate the refractive index at wavelength = lambda (m)
  // Use AOpticsManager::m() to get the unit length in (m)
  lambda /= AOpticsManager::um(); // Convert (m) to (um)
  return fPar[0] + fPar[1]*TMath::Power(lambda, -2) + fPar[2]*TMath::Power(lambda, -4);
}
