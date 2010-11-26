// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ASchottFormula
//
// SCHOTT's formula for calculation of refractive index
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
ASchottFormula::ASchottFormula(Double_t p0, Double_t p1, Double_t p2,
                               Double_t p3, Double_t p4, Double_t p5)
  : ARefractiveIndex()
{
  fPar[0] = p0;
  fPar[1] = p1;
  fPar[2] = p2;
  fPar[3] = p3;
  fPar[4] = p4;
  fPar[5] = p5;
}

//_____________________________________________________________________________
ASchottFormula::ASchottFormula(const Double_t* p)
{
  for(Int_t i = 0; i < 6; i++){
    fPar[i] = p[i];
  } // i
}

//_____________________________________________________________________________
ASchottFormula::~ASchottFormula()
{
}

//_____________________________________________________________________________
Double_t ASchottFormula::GetIndex(Double_t lambda) const
{
  lambda /= AOpticsManager::um(); // Convert (m) to (um)
  return TMath::Sqrt(fPar[0]                         +
                     fPar[1]*TMath::Power(lambda,  2.) +
                     fPar[2]*TMath::Power(lambda, -2.) +
                     fPar[3]*TMath::Power(lambda, -4.) +
                     fPar[4]*TMath::Power(lambda, -6.) +
                     fPar[5]*TMath::Power(lambda, -8.));
}
