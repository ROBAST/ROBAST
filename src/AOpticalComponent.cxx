/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AOpticalComponent
//
// Classical ray class
//
///////////////////////////////////////////////////////////////////////////////

#include "TGeoManager.h"

#include "AOpticalComponent.h"

ClassImp(AOpticalComponent);

AOpticalComponent::AOpticalComponent() : TGeoVolume() { fSurfaceArray = 0; }

//_____________________________________________________________________________
AOpticalComponent::AOpticalComponent(const char* name, const TGeoShape* shape,
                                     const TGeoMedium* med)
    : TGeoVolume(name, shape, med) {
  fSurfaceArray = 0;
#if ROOT_VERSION(5, 34, 16) <= ROOT_VERSION_CODE
  if (GetMedium() == TGeoVolume::DummyMedium()) {
    SetMedium(GetOpaqueVacuumMedium());
  }
#endif
}

//_____________________________________________________________________________
AOpticalComponent::~AOpticalComponent() { SafeDelete(fSurfaceArray); }

//______________________________________________________________________________
void AOpticalComponent::AddSurfaceCondition(
    ABorderSurfaceCondition* condition) {
  if (!fSurfaceArray) {
    fSurfaceArray = new TObjArray;
    fSurfaceArray->SetOwner(kTRUE);
  }

  fSurfaceArray->Add(condition);
}

//______________________________________________________________________________
ABorderSurfaceCondition* AOpticalComponent::FindSurfaceCondition(
    AOpticalComponent* component2) {
  if (!fSurfaceArray) {
    return 0;
  }

  for (Int_t i = 0; i < fSurfaceArray->GetEntries(); i++) {
    if (((ABorderSurfaceCondition*)(*fSurfaceArray)[i])->GetComponent2() ==
        component2) {
      return (ABorderSurfaceCondition*)(*fSurfaceArray)[i];
    }
  }

  return 0;
}

//______________________________________________________________________________
TGeoMaterial* AOpticalComponent::GetOpaqueVacuumMaterial() const {
  if (!fGeoManager) {
    return 0;
  }

  TGeoMaterial* mat = fGeoManager->GetMaterial("ROBAST_OpaqueVacuumMaterial");
  if (!mat) {
    mat = new TGeoMaterial("ROBAST_OpaqueVacuumMaterial", 0, 0, 0);
  }

  return mat;
}

//______________________________________________________________________________
TGeoMaterial* AOpticalComponent::GetTransparentVacuumMaterial() const {
  if (!fGeoManager) {
    return 0;
  }

  TGeoMaterial* mat =
      fGeoManager->GetMaterial("ROBAST_TransparentVacuumMaterial");
  if (!mat) {
    mat = new TGeoMaterial("ROBAST_TransparentVacuumMaterial", 0, 0, 0);
    mat->SetTransparency(70);
  }

  return mat;
}

//______________________________________________________________________________
TGeoMedium* AOpticalComponent::GetOpaqueVacuumMedium() const {
  if (!fGeoManager) {
    return 0;
  }

  TGeoMedium* med = fGeoManager->GetMedium("ROBAST_OpaqueVacuumMedium");
  if (!med) {
    TGeoMaterial* mat = GetOpaqueVacuumMaterial();
    med = new TGeoMedium("ROBAST_OpaqueVacuumMedium", 1, mat);
  }

  return med;
}

//______________________________________________________________________________
TGeoMedium* AOpticalComponent::GetTransparentVacuumMedium() const {
  if (!fGeoManager) {
    return 0;
  }

  TGeoMedium* med = fGeoManager->GetMedium("ROBAST_TransparentVacuumMedium");
  if (!med) {
    TGeoMaterial* mat = GetTransparentVacuumMaterial();
    med = new TGeoMedium("ROBAST_TransparentVacuumMedium", 1, mat);
  }

  return med;
}
