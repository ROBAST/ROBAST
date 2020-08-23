/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ABorderSurfaceCondition
//
// Defines the condition of the boarder surface between two components. Works as
// G4LogicalBorderSurface + G4OpticalSurface in Geant4.
//
///////////////////////////////////////////////////////////////////////////////

#include "ABorderSurfaceCondition.h"
#include "AOpticalComponent.h"

ClassImp(ABorderSurfaceCondition);

ABorderSurfaceCondition::ABorderSurfaceCondition(AOpticalComponent* component1,
                                                 AOpticalComponent* component2)
  : fSigma(0), fMultilayer(0), fLambertian(false) {
  fComponent[0] = component1;
  fComponent[1] = component2;

  if (!component1) {
    return;
  }

  component1->AddBorderSurfaceCondition(this);
}

//______________________________________________________________________________
ABorderSurfaceCondition::~ABorderSurfaceCondition() {}

//______________________________________________________________________________
void ABorderSurfaceCondition::SetGaussianRoughness(Double_t sigma /* (rad) */) {
  // Set Gaussian-like roughness in unit of (rad). Works as sigma_alpha in
  // Geant4 optics.
  fSigma = TMath::Abs(sigma);
}
