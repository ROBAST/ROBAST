// $Id$
// Author: Akira Okumura 2007/10/02

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
}

//_____________________________________________________________________________
ARayArray::~ARayArray()
{
  // Destructor
  fExited.Delete();
  fFocused.Delete();
  fRunning.Delete();
  fStopped.Delete();
  fSuspended.Delete();
}

//_____________________________________________________________________________
void ARayArray::Add(ARay* ray)
{
  if     (ray->IsExited()   )    fExited.Add(ray);
  else if(ray->IsFocused()  )   fFocused.Add(ray);
  else if(ray->IsRunning()  )   fRunning.Add(ray);
  else if(ray->IsStopped()  )   fStopped.Add(ray);
  else if(ray->IsSuspended()) fSuspended.Add(ray);
}
