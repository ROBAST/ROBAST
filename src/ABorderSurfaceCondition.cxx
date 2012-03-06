// $Id: ARefractiveIndex.cxx 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ABorderSurfaceCondition
//
// Defines the condition of the boarder surface between two volumes. Works as
// G4LogicalBorderSurface + G4OpticalSurface in Geant4.
//
///////////////////////////////////////////////////////////////////////////////

#include "ABorderSurfaceCondition.h"
#include "AOpticalComponent.h"

ClassImp(ABorderSurfaceCondition)

TObjArray ABorderSurfaceCondition::fSurfaceArray;

ABorderSurfaceCondition::ABorderSurfaceCondition(TGeoVolume* volume1, TGeoVolume* volume2)
  : fSigma(0)
{
  fVolume[0] = volume1;
  fVolume[1] = volume2;

  if(not fSurfaceArray.IsOwner()){
    fSurfaceArray.SetOwner(kTRUE);
  } // if

  ABorderSurfaceCondition* condition = GetSurfaceCondition(volume1, volume2);
  if(condition){
    fSurfaceArray.Remove(condition);
    SafeDelete(condition);
  } // if

  fSurfaceArray.Add(this);
}

//______________________________________________________________________________
ABorderSurfaceCondition::~ABorderSurfaceCondition()
{
}

//______________________________________________________________________________
ABorderSurfaceCondition* ABorderSurfaceCondition::GetSurfaceCondition(TGeoVolume* volume1, TGeoVolume* volume2)
{
  for(Int_t i = 0; i < fSurfaceArray.GetEntries(); i++){
    if(((ABorderSurfaceCondition*)fSurfaceArray[i])->GetVolume1() == volume1 &&
       ((ABorderSurfaceCondition*)fSurfaceArray[i])->GetVolume2() == volume2){
      return (ABorderSurfaceCondition*)fSurfaceArray[i];
    } // if
  } // i

  return 0;
}

//______________________________________________________________________________
void ABorderSurfaceCondition::SetGaussianRoughness(Double_t sigma /* (rad) */)
{
  // Set Gaussian-like roughness in unit of (rad). Works as sigma_alpha in
  // Geant4 optics.
  fSigma = TMath::Abs(sigma);
}
