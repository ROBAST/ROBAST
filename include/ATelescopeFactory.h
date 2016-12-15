// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_TELESCOPE_FACTORY_H
#define A_TELESCOPE_FACTORY_H

#include "ATelescope.h"

///////////////////////////////////////////////////////////////////////////////
//
// ATelescopeFactory
//
// A class to create telescope geometries
//
///////////////////////////////////////////////////////////////////////////////

class ATelescopeFactory : public TObject {
private:
  static UInt_t fNTelescopes; // number of telescopes

public:
  ATelescopeFactory() {};
  virtual ~ATelescopeFactory() {};

  static ATelescope* MakeTelescope(const char* config);

  ClassDef(ATelescopeFactory, 1)
};

#endif // A_TELESCOPE_FACTORY_H
