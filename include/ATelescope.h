// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_TELESCOPE_H
#define A_TELESCOPE_H

#include "AOpticsManager.h"

///////////////////////////////////////////////////////////////////////////////
//
// ATelescope
//
// A class for common interfaces and utilities for telescopes
//
///////////////////////////////////////////////////////////////////////////////

class ATelescope : public TObject {
private:
  Int_t fID;
  AOpticsManager* fManager;
  TVector3 fPointingDirection;

  void BuildGeometry(const char* config);

public:
  ATelescope();
  ATelescope(const char* config, UInt_t id);
  virtual ~ATelescope();

  AOpticsManager* GetManager() {return fManager;}
  void SetPointingDirection(Double_t zenith /* deg */, Double_t azimuth /* deg */);
  void Trace(ARayArray* array);

  // Some 

  ClassDef(ATelescope, 1)
};

#endif // A_TELESCOPE_H
