/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ASchottFormula
//
// SCHOTT's formula for calculation of refractive index. SCHOTT does not use
// this formula in its catalog any more. Sellmeier's formula is used instead.
//
///////////////////////////////////////////////////////////////////////////////

#include "AOpticsManager.h"
#include "ASchottFormula.h"
#include "TMath.h"

ClassImp(ASchottFormula)

ASchottFormula::ASchottFormula() : ARefractiveIndex()
{
}

//_____________________________________________________________________________
ASchottFormula::ASchottFormula(Double_t A0, Double_t A1, Double_t A2,
                               Double_t A3, Double_t A4, Double_t A5)
  : ARefractiveIndex()
{
  fPar[0] = A0;
  fPar[1] = A1;
  fPar[2] = A2;
  fPar[3] = A3;
  fPar[4] = A4;
  fPar[5] = A5;
}

//_____________________________________________________________________________
ASchottFormula::ASchottFormula(const Double_t* p)
{
  for(Int_t i = 0; i < 6; i++){
    fPar[i] = p[i];
  } // i
}

//_____________________________________________________________________________
Double_t ASchottFormula::GetIndex(Double_t lambda) const
{
  // Calculate the refractive index at wavelength = lambda (m)
  // Use AOpticsManager::m() to get the unit length in (m)
  //
  // n(lambda)^2 = A0 + A1*lamda^2 + A2*lamda^-2 + A3*lamda^-4 + A4*lamda^-6 + A5*lamda^-8
  // where lambda is measured in (um)
  lambda /= AOpticsManager::um(); // Convert (nm) to (um)
  return TMath::Sqrt(fPar[0]                         +
                     fPar[1]*TMath::Power(lambda,  2.) +
                     fPar[2]*TMath::Power(lambda, -2.) +
                     fPar[3]*TMath::Power(lambda, -4.) +
                     fPar[4]*TMath::Power(lambda, -6.) +
                     fPar[5]*TMath::Power(lambda, -8.));
}
