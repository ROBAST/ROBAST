// $Id$
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ALens
//
// Lens class
//
///////////////////////////////////////////////////////////////////////////////

#include "ALens.h"

ClassImp(ALens)

ALens::ALens()
{
  // Default constructor
  fAbsorptionLength = 0;
  fIndex = 0;
  fConstantIndex = 1;
  SetLineColor(7);
}

//_____________________________________________________________________________
ALens::ALens(const char* name, const TGeoShape* shape,
             const TGeoMedium* med) : AOpticalComponent(name, shape, med)
{
  fIndex = 0;
  fConstantIndex = 1;
  fConstantAbsorptionLength = -1;
  SetLineColor(7);
}

//_____________________________________________________________________________
ALens::~ALens()
{
}

//_____________________________________________________________________________
Double_t ALens::GetAbsorptionLength(Double_t lambda) const
{
  if(!fAbsorptionLength){
    return fConstantAbsorptionLength;
  } // if

  Double_t abs = fAbsorptionLength->Eval(lambda);

  return abs >= 0 ? abs : 0;
}

//_____________________________________________________________________________
Double_t ALens::GetRefractiveIndex(Double_t lambda) const
{
  Double_t ret = fConstantIndex;

  if(fIndex){
    ret = fIndex->GetIndex(lambda);
  } // if

  return ret;
}
