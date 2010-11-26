// $Id: ALens.cxx,v 1.2 2008/03/26 05:50:47 oxon Exp $
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
  SetLineColor(7);
}

//_____________________________________________________________________________
ALens::~ALens()
{
}

//_____________________________________________________________________________
Double_t ALens::GetRefractiveIndex(Double_t lambda)
{
  Double_t ret;

  if(fIndex){
    ret = fIndex->GetIndex(lambda);
  } else {
    ret = fConstantIndex;
  } // if

  return ret;
}
