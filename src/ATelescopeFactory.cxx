/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ATelescopeFactory
//
// Factory class for telescopes
//
///////////////////////////////////////////////////////////////////////////////

#include "ATelescopeFactory.h"

UInt_t ATelescopeFactory::fNTelescopes = 0;

ClassImp(ATelescopeFactory)

//______________________________________________________________________________
ATelescope* ATelescopeFactory::MakeTelescope(const char* config)
{
  ATelescope* telescope = new ATelescope(config, fNTelescopes);
  fNTelescopes++;

  return telescope;
}
