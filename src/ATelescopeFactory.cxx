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

ClassImp(ATelescopeFactory)

//______________________________________________________________________________
ATelescopeFactory::ATelescopeFactory()
{
  fNTelescopes = 0;
}

//______________________________________________________________________________
ATelescope* ATelescopeFactory::MakeTelescope(const char* config)
{
  ATelescope* telescope = new ATelescope(config, fNTelescopes);
  fNTelescopes++;

  return telescope;
}
