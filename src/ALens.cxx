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

ALens::ALens() : fAbsorptionLength(NULL), fIndex(NULL), fIndexGraph(NULL),
  fConstantIndex(1), fConstantAbsorptionLength(-1)
{
  // Default constructor
  SetLineColor(7);
}

//_____________________________________________________________________________
ALens::ALens(const char* name, const TGeoShape* shape,
             const TGeoMedium* med) : AOpticalComponent(name, shape, med),
                                      fAbsorptionLength(NULL), fIndex(NULL),
                                      fIndexGraph(NULL), fConstantIndex(1),
                                      fConstantAbsorptionLength(-1)
{
  SetLineColor(7);
#if ROOT_VERSION(5, 34, 16) <= ROOT_VERSION_CODE
  if(GetMedium() == TGeoVolume::DummyMedium() ||
     GetMedium() == GetOpaqueVacuumMedium() ){
    SetMedium(GetTransparentVacuumMedium());
  }
#endif
}

//_____________________________________________________________________________
ALens::~ALens()
{
  SafeDelete(fAbsorptionLength);
  SafeDelete(fIndex);
  SafeDelete(fIndexGraph);
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
  }

  if(fIndexGraph){
    ret = fIndexGraph->Eval(lambda);
  }

  return ret;
}
