// $Id$
// Author: Akira Okumura 2007/10/01

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ASellmeierFormula
//
// Sellmeier's formula for calculation of refractive index
// See http://en.wikipedia.org/wiki/Sellmeier_equation
//
///////////////////////////////////////////////////////////////////////////////

#include "AOpticsManager.h"
#include "ASellmeierFormula.h"
#include "TMath.h"

ClassImp(ASellmeierFormula)

ASellmeierFormula::ASellmeierFormula() : ARefractiveIndex()
{
}

//_____________________________________________________________________________
ASellmeierFormula::ASellmeierFormula(Double_t B1, Double_t B2, Double_t B3,
                                     Double_t C1, Double_t C2, Double_t C3)
  : ARefractiveIndex()
{
  // n(lambda) = 1 + B1*lamda^2/(lamda^2 - C1) + B2*lamda^2/(lamda^2 - C2) + B3*lamda^2/(lamda^2 - C3)
  // where lambda is measured in (um)
  fPar[0] = B1;
  fPar[1] = B2;
  fPar[2] = B3;
  fPar[3] = C1;
  fPar[4] = C2;
  fPar[5] = C3;
}

//_____________________________________________________________________________
ASellmeierFormula::ASellmeierFormula(const Double_t* p)
{
  for(Int_t i = 0; i < 6; i++){
    fPar[i] = p[i];
  } // i
}

//_____________________________________________________________________________
Double_t ASellmeierFormula::GetIndex(Double_t lambda) const
{
  // Calculate the refractive index at wavelength = lambda (m)
  lambda /= AOpticsManager::um(); // Convert (m) to (um)
  Double_t lambda2 = lambda*lambda;
  return TMath::Sqrt(1 + fPar[0]*lambda2/(lambda2 - fPar[3])
                       + fPar[1]*lambda2/(lambda2 - fPar[4])
                       + fPar[2]*lambda2/(lambda2 - fPar[5]));
}
