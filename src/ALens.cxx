/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ALens
//
// Lens class
//
///////////////////////////////////////////////////////////////////////////////

#include "ALens.h"

ClassImp(ALens);

ALens::ALens()
  : fIndex(NULL) {
  // Default constructor
  SetLineColor(7);
}

//_____________________________________________________________________________
ALens::ALens(const char* name, const TGeoShape* shape, const TGeoMedium* med)
    : AOpticalComponent(name, shape, med),
      fIndex(NULL) {
  SetLineColor(7);
#if ROOT_VERSION(5, 34, 16) <= ROOT_VERSION_CODE
  if (GetMedium() == TGeoVolume::DummyMedium() ||
      GetMedium() == GetOpaqueVacuumMedium()) {
    SetMedium(GetTransparentVacuumMedium());
  }
#endif
}

//_____________________________________________________________________________
Double_t ALens::GetAbsorptionLength(Double_t lambda) const {
  if (!fIndex) {
    return std::numeric_limits<Double_t>::infinity();
  }

  Double_t abs = fIndex->GetAbsorptionLength(lambda);

  return abs;
}

//_____________________________________________________________________________
Double_t ALens::GetExtinctionCoefficient(Double_t lambda) const {
  if (!fIndex) {
    return 0;
  }

  Double_t ex = fIndex->GetExtinctionCoefficient(lambda);

  return ex;
}

//_____________________________________________________________________________
Double_t ALens::GetRefractiveIndex(Double_t lambda) const {
  return fIndex ? fIndex->GetRefractiveIndex(lambda) : 1.;
}
