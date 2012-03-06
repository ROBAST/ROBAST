// $Id: ARefractiveIndex.h 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_BORDER_SURFACE_CONDITION_H
#define A_BORDER_SURFACE_CONDITION_H

///////////////////////////////////////////////////////////////////////////////
//
// ABorderSurfaceCondition
//
// Defines the condition of the boarder surface between two volumes. Works as
// G4LogicalBorderSurface + G4OpticalSurface in Geant4.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif
#ifndef ROOT_TGeoVolume
#include "TGeoVolume.h"
#endif

class ABorderSurfaceCondition : public TObject {
private:
  static TObjArray fSurfaceArray;
  TGeoVolume*      fVolume[2];
  Double_t         fSigma;

public:
  ABorderSurfaceCondition(TGeoVolume* volume1, TGeoVolume* volume2);
  virtual ~ABorderSurfaceCondition();

  const TGeoVolume* GetVolume1() const {return fVolume[0];}
  const TGeoVolume* GetVolume2() const {return fVolume[1];}
  Double_t          GetGaussianRoughness() const {return fSigma;}
  static ABorderSurfaceCondition* GetSurfaceCondition(TGeoVolume* volume1, TGeoVolume* volume2);

  void              SetGaussianRoughness(Double_t sigma /* (rad) */);

  ClassDef(ABorderSurfaceCondition, 1)
};

#endif // A_BORDER_SURFACE_CONDITION_H
