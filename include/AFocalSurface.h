// $Id: AFocalSurface.h 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/10/01

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_FOCAL_SURFACE_H
#define A_FOCAL_SURFACE_H

///////////////////////////////////////////////////////////////////////////////
//
// AFocalSurface
//
// Focal surface
//
///////////////////////////////////////////////////////////////////////////////

#include "TGraph.h"

#ifndef A_OPTICAL_COMPONENT_H
#include "AOpticalComponent.h"
#endif

class AFocalSurface : public AOpticalComponent {
 private:
  TGraph* fQuantumEfficiency; // Quantum efficiency

 public:
  AFocalSurface();
  AFocalSurface(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);

  void     SetQuantumEfficiency(TGraph* qe) {fQuantumEfficiency = qe;}
  Double_t GetQuantumEfficiency(Double_t lambda) const;

  ClassDef(AFocalSurface, 1)
};

#endif // A_FOCAL_SURFACE_H
