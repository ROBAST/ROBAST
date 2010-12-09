// $Id: AOpticsManager.h 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_OPTICS_MANAGER_H
#define A_OPTICS_MANAGER_H

///////////////////////////////////////////////////////////////////////////////
//
// AOpticsManager
//
// Manager of optics
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TGeoManager
#include "TGeoManager.h"
#endif
#ifndef ROOT_TMath
#include "TMath.h"
#endif

#ifndef A_RAY_ARRAY_H
#include "ARayArray.h"
#endif
#ifndef A_FOCAL_SURFACE_H
#include "AFocalSurface.h"
#endif
#ifndef A_LENS_H
#include "ALens.h"
#endif
#ifndef A_MIRROR_H
#include "AMirror.h"
#endif
#ifndef A_OPTICAL_COMPONENT_H
#include "AOpticalComponent.h"
#endif
#ifndef A_OBSCURATION_H
#include "AObscuration.h"
#endif

class AOpticsManager : public TGeoManager {
 private:
  Int_t fLimit; // Maximum number of crossing calculations

 public:
  AOpticsManager();
  AOpticsManager(const char* name, const char* title);
  virtual ~AOpticsManager();

  static Double_t  m() { return 100.;};
  static Double_t cm() { return 1;};
  static Double_t mm() { return 1e-1;};
  static Double_t um() { return 1e-4;};
  static Double_t nm() { return 1e-7;};

  Bool_t IsFocalSurface(TGeoNode* node) const { return node->GetVolume()->IsA() == AFocalSurface::Class();};
  Bool_t IsLens(TGeoNode* node) const { return node->GetVolume()->IsA() == ALens::Class();};
  Bool_t IsMirror(TGeoNode* node) const { return node->GetVolume()->IsA() == AMirror::Class();};
  Bool_t IsObscuration(TGeoNode* node) const { return node->GetVolume()->IsA() == AObscuration::Class();};
  Bool_t IsOpticalComponent(TGeoNode* node) const { return node->GetVolume()->IsA() == AOpticalComponent::Class();};
  void SetLimit(Int_t n);
  void TraceNonSequential(ARay& ray);
  void TraceNonSequential(ARayArray& array);

  ClassDef(AOpticsManager, 1)
};

#endif // A_OPTICS_MANAGER_H
