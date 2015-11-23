// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_FOCAL_SURFACE_H
#define A_FOCAL_SURFACE_H

#include "TGraph.h"

#include "AOpticalComponent.h"

///////////////////////////////////////////////////////////////////////////////
//
// AFocalSurface
//
// Focal surface
//
///////////////////////////////////////////////////////////////////////////////

class AFocalSurface : public AOpticalComponent {
 private:
  TGraph* fQuantumEfficiencyLambda; // Quantum efficiency (QE vs lambda)
  TGraph* fQuantumEfficiencyAngle; // Quantum efficiency (QE vs angle)

 public:
  AFocalSurface();
  AFocalSurface(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);

  Bool_t   HasQEAngle() const {return fQuantumEfficiencyAngle ? kTRUE : kFALSE;}
  void     SetQuantumEfficiency(TGraph* qe) {fQuantumEfficiencyLambda = qe;}
  void     SetQuantumEfficiencyAngle(TGraph* qe) {fQuantumEfficiencyAngle = qe;}
  Double_t GetQuantumEfficiency(Double_t lambda) const;
  Double_t GetQuantumEfficiency(Double_t lambda, Double_t angle) const;

  ClassDef(AFocalSurface, 1)
};

#endif // A_FOCAL_SURFACE_H
