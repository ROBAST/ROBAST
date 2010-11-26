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
ASellmeierFormula::ASellmeierFormula(Double_t b1, Double_t b2, Double_t b3,
                                     Double_t c1, Double_t c2, Double_t c3)
  : ARefractiveIndex()
{
  fPar[0] = b1;
  fPar[1] = b2;
  fPar[2] = b3;
  fPar[3] = c1;
  fPar[4] = c2;
  fPar[5] = c3;
}

//_____________________________________________________________________________
ASellmeierFormula::ASellmeierFormula(const Double_t* p)
{
  for(Int_t i = 0; i < 6; i++){
    fPar[i] = p[i];
  } // i
}

//_____________________________________________________________________________
ASellmeierFormula::~ASellmeierFormula()
{
}

//_____________________________________________________________________________
Double_t ASellmeierFormula::GetIndex(Double_t lambda) const
{
  lambda /= AOpticsManager::um(); // Convert (m) to (um)
  Double_t lambda2 = lambda*lambda;
  return TMath::Sqrt(1 + fPar[0]*lambda2/(lambda2 - fPar[3])
                       + fPar[1]*lambda2/(lambda2 - fPar[4])
                       + fPar[2]*lambda2/(lambda2 - fPar[5]));
}
