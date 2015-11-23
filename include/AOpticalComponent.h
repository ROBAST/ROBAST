// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_OPTICAL_COMPONENT_H
#define A_OPTICAL_COMPONENT_H

#include "TGeoVolume.h"
#include "TObjArray.h"
#include "ABorderSurfaceCondition.h"

///////////////////////////////////////////////////////////////////////////////
//
// AOpticalComponent
//
// Abstract class for optical components
//
///////////////////////////////////////////////////////////////////////////////

class AOpticalComponent : public TGeoVolume {
 private:
  TObjArray* fSurfaceArray;

 public:
  AOpticalComponent();
  AOpticalComponent(const char* name, const TGeoShape* shape, const TGeoMedium* med = 0);
  virtual ~AOpticalComponent();

  void AddSurfaceCondition(ABorderSurfaceCondition* condition);
  ABorderSurfaceCondition* FindSurfaceCondition(AOpticalComponent* component2);
  TGeoMaterial* GetOpaqueVacuumMaterial() const;
  TGeoMaterial* GetTransparentVacuumMaterial() const;
  TGeoMedium* GetOpaqueVacuumMedium() const;
  TGeoMedium* GetTransparentVacuumMedium() const;


  ClassDef(AOpticalComponent, 1)
};

#endif // A_OPTICAL_COMPONENT_H
