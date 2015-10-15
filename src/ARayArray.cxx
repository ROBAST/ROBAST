/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ARayArray
//
// Array of ARay
//
///////////////////////////////////////////////////////////////////////////////

#include "ARayArray.h"

ClassImp(ARayArray)

//_____________________________________________________________________________
ARayArray::ARayArray() : TObject()
{
  // Constructor
  fAbsorbed.SetOwner(kTRUE);
  fExited.SetOwner(kTRUE);
  fFocused.SetOwner(kTRUE);
  fRunning.SetOwner(kTRUE);
  fStopped.SetOwner(kTRUE);
  fSuspended.SetOwner(kTRUE);
}

//_____________________________________________________________________________
ARayArray::~ARayArray()
{
  // Destructor
  fAbsorbed.Clear();
  fExited.Clear();
  fFocused.Clear();
  fRunning.Clear();
  fStopped.Clear();
  fSuspended.Clear();
}

//_____________________________________________________________________________
void ARayArray::Add(ARay* ray)
{
  if(!ray) return;

  if     (ray->IsAbsorbed() )  fAbsorbed.Add(ray);
  else if(ray->IsExited()   )    fExited.Add(ray);
  else if(ray->IsFocused()  )   fFocused.Add(ray);
  else if(ray->IsRunning()  )   fRunning.Add(ray);
  else if(ray->IsStopped()  )   fStopped.Add(ray);
  else if(ray->IsSuspended()) fSuspended.Add(ray);
}

//_____________________________________________________________________________
void ARayArray::Merge(ARayArray* array)
{
  if(!array) return;

  TObjArray* objs[6] = {array->GetAbsorbed(), array->GetExited(),
                        array->GetFocused(),  array->GetRunning(),
                        array->GetStopped(),  array->GetSuspended()};

  for(Int_t j = 0; j < 6; j++){
    for(Int_t i = 0; i <= objs[j]->GetLast(); i++){
      ARay* ray = (ARay*)objs[j]->RemoveAt(i);
      if(!ray) continue;
      Add(ray);
    } // i
  } // j
}
