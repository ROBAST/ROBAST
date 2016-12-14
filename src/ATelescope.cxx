/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ATelescope
//
// Telescope class
//
///////////////////////////////////////////////////////////////////////////////

#include "ATelescope.h"

ClassImp(ATelescope)

//_____________________________________________________________________________
ATelescope::ATelescope()
{
  // Default constructor
  fID = 0;
  fManager = 0;
  fPointingDirection.SetMagThetaPhi(1., 0., 0.);
}

//_____________________________________________________________________________
ATelescope::ATelescope(const char* config, UInt_t id)
{
  fID = id;
  BuildGeometry(config);
}

//_____________________________________________________________________________
ATelescope::~ATelescope()
{
  if (fManager) {
    gGeoManager = fManager;
    SafeDelete(fManager);
  }
}

//_____________________________________________________________________________
void ATelescope::BuildGeometry(const char* config)
{
  gGeoManager = 0;
  fManager = new AOpticsManager("manager", "Opics Manager");

  // Read the config file and build the geometry
}

//_____________________________________________________________________________
void ATelescope::SetPointingDirection(Double_t zenith , Double_t azimuth)
{
  fPointingDirection.SetMagThetaPhi(1., zenith*TMath::DegToRad(),
                                    (90. - azimuth)*TMath::DegToRad());
}

//_____________________________________________________________________________
void ATelescope::Trace(ARayArray* array)
{
  // First the "world" or the rays must be rotated to make the telescope points to
  // (fZenith, fAzimuth)
  // Write your code here...

  
}
