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
#ifndef ROOT_TThread
#include "TThread.h"
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
  Int_t  fLimit; // Maximum number of crossing calculations
  Bool_t fDisableFresnelReflection; // disable Fresnel reflection
  TGeoNode* fStoredStartNode; // start node
  TGeoNode* fStoredEndNode; // end node
  std::map<long, TGeoNode*> fStoredStartNodes; // for multi threading
  std::map<long, TGeoNode*> fStoredEndNodes; // for multi threading
  TClass* fClassList[5];

  static void* Thread(void* args);

  void     DoFresnel(Double_t n1, Double_t n2, ARay& ray, TGeoNavigator* nav, TGeoNode* startNode, TGeoNode* endNode);
  void     DoReflection(Double_t n1, ARay& ray, TGeoNavigator* nav, TGeoNode* startNode, TGeoNode* endNode);
  TVector3 GetFacetNormal(TGeoNavigator* nav, TGeoNode* startNode, TGeoNode* endNode);

 public:
  enum {kLens = 0, kObs = 1, kMirror = 2, kFocus = 3, kOpt = 4, kOther = 5, kNull = 6};

  AOpticsManager();
  AOpticsManager(const char* name, const char* title);
  virtual ~AOpticsManager();

  static Double_t km() { return 1e3*m();};
  static Double_t  m() { return 1e2*cm();};
  static Double_t cm() { return 1;};
  static Double_t mm() { return 1e-3*m();};
  static Double_t um() { return 1e-6*m();};
  static Double_t nm() { return 1e-9*m();};
  static Double_t inch() { return 2.54*cm();};
  static Double_t  s() { return 1.;};
  static Double_t ms() { return 1e-3*s();};
  static Double_t us() { return 1e-6*s();};
  static Double_t ns() { return 1e-9*s();};

  void   DisableFresnelReflection(Bool_t disable) {fDisableFresnelReflection = disable;}
  TGeoNode* GetStoredStartNode() const;
  TGeoNode* GetStoredEndNode() const;
  void   SetStoredStartNode(TGeoNode* node);
  void   SetStoredEndNode(TGeoNode* node);
  Bool_t IsFocalSurface(TGeoNode* node) const { return node ? node->GetVolume()->IsA() == fClassList[kFocus] : kFALSE;};
  Bool_t IsLens(TGeoNode* node) const { return node ? node->GetVolume()->IsA() == fClassList[kLens] : kFALSE;};
  Bool_t IsMirror(TGeoNode* node) const { return node ? node->GetVolume()->IsA() == fClassList[kMirror] : kFALSE;};
  Bool_t IsObscuration(TGeoNode* node) const { return node ? node->GetVolume()->IsA() == fClassList[kObs] : kFALSE;};
  Bool_t IsOpticalComponent(TGeoNode* node) const { return node ? node->GetVolume()->IsA() == fClassList[kOpt] : kFALSE;};
  void   SetLimit(Int_t n);
  void   TraceNonSequential(ARay& ray);
  void   TraceNonSequential(ARay* ray) {if(ray) TraceNonSequential(*ray);}
  void   TraceNonSequential(ARayArray& array);
  void   TraceNonSequential(ARayArray* array) {if(array) TraceNonSequential(*array);}
  void   TraceNonSequential(TObjArray* array);

  ClassDef(AOpticsManager, 1)
};

//_____________________________________________________________________________
inline void AOpticsManager::SetStoredStartNode(TGeoNode* node)
{
  if(!IsMultiThread()){
    fStoredStartNode = node;
  } else {
    Long_t threadId = TThread::SelfId();
    fStoredStartNodes[threadId] = node;
  } // if
}

//_____________________________________________________________________________
inline void AOpticsManager::SetStoredEndNode(TGeoNode* node)
{
  if(!IsMultiThread()){
    fStoredEndNode = node;
  } else {
    Long_t threadId = TThread::SelfId();
    fStoredEndNodes[threadId] = node;
  } // if
}

#endif // A_OPTICS_MANAGER_H
