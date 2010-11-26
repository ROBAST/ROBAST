// $Id$
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

#include "TGeoManager.h"
#include "TMath.h"

#include "ARayArray.h"
#include "AFocalSurface.h"
#include "ALens.h"
#include "AMirror.h"
#include "AOpticalComponent.h"
#include "AObscuration.h"

class AOpticsManager : public TGeoManager {
 private:
  Int_t fLimit; // Maximum number of crossing calculations

 public:
  AOpticsManager();
  AOpticsManager(const char* name, const char* title);
  virtual ~AOpticsManager();

  static Double_t  m() { return 1.;};
  static Double_t cm() { return 1e-2;};
  static Double_t mm() { return 1e-3;};
  static Double_t um() { return 1e-6;};
  static Double_t nm() { return 1e-9;};

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
