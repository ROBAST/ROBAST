/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AFocalSurface
//
// Focal surface
//
///////////////////////////////////////////////////////////////////////////////

#include "AFocalSurface.h"

ClassImp(AFocalSurface)

AFocalSurface::AFocalSurface() : fQuantumEfficiencyLambda(0), fQuantumEfficiencyAngle(0)
{
  // Default constructor
  SetLineColor(2);
}

//_____________________________________________________________________________
AFocalSurface::AFocalSurface(const char* name, const TGeoShape* shape,
                             const TGeoMedium* med)
  : AOpticalComponent(name, shape, med), fQuantumEfficiencyLambda(0), fQuantumEfficiencyAngle(0)
{
  // Constructor
  SetLineColor(2);
}

//_____________________________________________________________________________
Double_t AFocalSurface::GetQuantumEfficiency(Double_t lambda) const
{
  if(fQuantumEfficiencyLambda){
    return fQuantumEfficiencyLambda->Eval(lambda);
  } else {
    return 1.;
  } // if
}

//_____________________________________________________________________________
Double_t AFocalSurface::GetQuantumEfficiency(Double_t lambda, Double_t angle) const
{
  Double_t qe = GetQuantumEfficiency(lambda);
  if(HasQEAngle()){
    qe *= fQuantumEfficiencyAngle->Eval(angle);
  } // if

  return qe;
}
