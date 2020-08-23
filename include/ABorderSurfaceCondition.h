// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_BORDER_SURFACE_CONDITION_H
#define A_BORDER_SURFACE_CONDITION_H

#include "AMultilayer.h"
#include "TObject.h"

class AOpticalComponent;

///////////////////////////////////////////////////////////////////////////////
//
// ABorderSurfaceCondition
//
// Defines the condition of the boarder surface between two volumes. Works as
// G4LogicalBorderSurface + G4OpticalSurface in Geant4.
//
///////////////////////////////////////////////////////////////////////////////

class ABorderSurfaceCondition : public TObject {
 private:
  AOpticalComponent* fComponent[2];
  Double_t fSigma;
  std::shared_ptr<AMultilayer> fMultilayer;
  Bool_t fLambertian;

 public:
  ABorderSurfaceCondition(AOpticalComponent* component1,
                          AOpticalComponent* component2);
  virtual ~ABorderSurfaceCondition();

  const AOpticalComponent* GetComponent1() const { return fComponent[0]; }
  const AOpticalComponent* GetComponent2() const { return fComponent[1]; }
  Double_t GetGaussianRoughness() const { return fSigma; }
  void SetGaussianRoughness(Double_t sigma /* (rad) */);
  void SetMultilayer(std::shared_ptr<AMultilayer> layer) {
    fMultilayer = layer;
  }
  std::shared_ptr<AMultilayer> GetMultilayer() const { return fMultilayer; }
  Bool_t IsLambertian() const {return fLambertian;}
  void EnableLambertian(Bool_t enable) {fLambertian = enable;}

  ClassDef(ABorderSurfaceCondition, 1)
};

#endif  // A_BORDER_SURFACE_CONDITION_H
