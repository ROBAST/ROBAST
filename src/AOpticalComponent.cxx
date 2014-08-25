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

#include "AOpticalComponent.h"

ClassImp(AOpticalComponent)

AOpticalComponent::AOpticalComponent() : TGeoVolume()
{
  fSurfaceArray = 0;
}

//_____________________________________________________________________________
AOpticalComponent::AOpticalComponent(const char* name, const TGeoShape* shape,
                                     const TGeoMedium* med)
  : TGeoVolume(name, shape, med)
{
  fSurfaceArray = 0;
}

//_____________________________________________________________________________
AOpticalComponent::~AOpticalComponent()
{
  SafeDelete(fSurfaceArray);
}

//______________________________________________________________________________
void AOpticalComponent::AddSurfaceCondition(ABorderSurfaceCondition* condition)
{
  if(!fSurfaceArray){
    fSurfaceArray = new TObjArray;
    fSurfaceArray->SetOwner(kTRUE);
  } // if

  fSurfaceArray->Add(condition);
}

//______________________________________________________________________________
ABorderSurfaceCondition* AOpticalComponent::FindSurfaceCondition(AOpticalComponent* component2)
{
  if(!fSurfaceArray){
    return 0;
  } // if

  for(Int_t i = 0; i < fSurfaceArray->GetEntries(); i++){
    if(((ABorderSurfaceCondition*)(*fSurfaceArray)[i])->GetComponent2() == component2){
      return (ABorderSurfaceCondition*)(*fSurfaceArray)[i];
    } // if
  } // i

  return 0;
}
