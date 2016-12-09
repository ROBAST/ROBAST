/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AOpticsManager
//
// Manager of optics
//
///////////////////////////////////////////////////////////////////////////////

#include "TRandom.h"
#include "TThread.h"

#include "ABorderSurfaceCondition.h"
#include "AOpticsManager.h"
#include <iostream>
static const Double_t kEpsilon = 1e-6; // Fixed in TGeoNavigator.cxx (equiv to 1e-6 cm)

ClassImp(AOpticsManager)

//_____________________________________________________________________________
AOpticsManager::AOpticsManager() : TGeoManager(), fDisableFresnelReflection(kFALSE)
{
  fLimit = 100;
  fClassList[kLens]   = ALens::Class();
  fClassList[kFocus]  = AFocalSurface::Class();
  fClassList[kMirror] = AMirror::Class();
  fClassList[kObs]    = AObscuration::Class();
  fClassList[kOpt]    = AOpticalComponent::Class();
}

//_____________________________________________________________________________
AOpticsManager::AOpticsManager(const char* name, const char* title)
 : TGeoManager(name, title), fDisableFresnelReflection(kFALSE)
{
  fLimit = 100;
  fClassList[kLens]   = ALens::Class();
  fClassList[kFocus]  = AFocalSurface::Class();
  fClassList[kMirror] = AMirror::Class();
  fClassList[kObs]    = AObscuration::Class();
  fClassList[kOpt]    = AOpticalComponent::Class();
}

//_____________________________________________________________________________
AOpticsManager::~AOpticsManager()
{
}

//_____________________________________________________________________________
void AOpticsManager::DoFresnel(Double_t n1, Double_t n2, ARay& ray, TGeoNavigator* nav, TGeoNode* currentNode, TGeoNode* nextNode)
{
  Double_t step = nav->GetStep();

  // Use the same notation used in Wikipedia
  // http://en.wikipedia.org/wiki/Fresnel_equations
  // theta_i = incident angle
  // theta_t = transmission angle
  TVector3 n = GetFacetNormal(nav, currentNode, nextNode); // normal vect perpendicular to the surface
  Double_t d1[3];
  ray.GetDirection(d1);
  Double_t cosi = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2]; // cos(theta_i)
  Double_t sini = TMath::Sqrt(1 - cosi*cosi);
  Double_t sint = n1*sini/n2; // Snell's law

  if(sint > 1.){ // total internal reflection
    DoReflection(n1, ray, nav, currentNode, nextNode);
    return;
  } // if

  Double_t cost = TMath::Sqrt(1 - sint*sint);

  if(fDisableFresnelReflection == kFALSE){
    Double_t Rs = TMath::Power((n1*cosi - n2*cost)/(n1*cosi + n2*cost), 2); // reflectivity for s-polarized photon
    Double_t Rp = TMath::Power((n1*cost - n2*cosi)/(n1*cost + n2*cosi), 2); // reflectivity for p-polarized photon
    Double_t R = (Rs + Rp)/2.; // We assume that polarization is random

    if(gRandom->Uniform(1) < R){ // reflection at the boundary
      DoReflection(n1, ray, nav, currentNode, nextNode);
      return;
    } // if
  } // if

  Double_t x1[4], d2[3];
  ray.GetLastPoint(x1);
  const Double_t* x2 = nav->GetCurrentPoint();
  for(Int_t i = 0; i < 3; i++){
    if(sini != 0){
      d2[i] = (d1[i] - cosi*n[i])*sint/sini + n[i]*cost;
    } else {
      d2[i] = d1[i];
    } // if
  } // i
  ray.SetDirection(d2);
  nav->SetCurrentDirection(d2);
  // step (m), c (m/s)
  Double_t speed = TMath::C()*m()/n1;
  Double_t t = x1[3] + step/speed;
  ray.AddPoint(x2[0], x2[1], x2[2], t);
  ray.AddNode(nextNode);
}

//_____________________________________________________________________________
void AOpticsManager::DoReflection(Double_t n1, ARay& ray, TGeoNavigator* nav, TGeoNode* currentNode, TGeoNode* nextNode)
{
  Double_t step = nav->GetStep();

  TVector3 n = GetFacetNormal(nav, currentNode, nextNode); // normal vect perpendicular to the surface
  Double_t d1[3];
  ray.GetDirection(d1);
  Double_t cosi = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2];

  Bool_t absorbed = kFALSE;

  if(IsMirror(nextNode)){
    Double_t angle = TMath::ACos(cosi);
    Double_t lambda = ray.GetLambda();
    Double_t ref = ((AMirror*)nextNode->GetVolume())->GetReflectance(lambda, angle);
    if(ref < gRandom->Uniform(1)){
      absorbed = kTRUE;
      ray.Absorb();
    } // if
  } // if

  Double_t d2[3];
  for(Int_t i = 0; i < 3; i++){ // d2 = d1 - 2n*(d1*n)
    d2[i] = d1[i] - 2*n[i]*cosi;
  } // i
  if(not absorbed){
    ray.SetDirection(d2);
  } // if

  Double_t x1[4];
  ray.GetLastPoint(x1);
  const Double_t* x2 = nav->GetCurrentPoint();
  Double_t speed = TMath::C()*m()/n1;
  Double_t t = x1[3] + step/speed;
  nav->SetCurrentDirection(-d1[0], -d1[1], -d1[2]);
  nav->SetStep(kEpsilon);
  nav->Step();
  nav->SetCurrentDirection(d2);
  ray.AddPoint(x2[0], x2[1], x2[2], t);
  ray.AddNode(nextNode);
}

//_____________________________________________________________________________
TVector3 AOpticsManager::GetFacetNormal(TGeoNavigator* nav, TGeoNode* currentNode, TGeoNode* nextNode)
{
  AOpticalComponent* component1 = (AOpticalComponent*)currentNode->GetVolume();
  AOpticalComponent* component2 = nextNode ? (AOpticalComponent*)nextNode->GetVolume() : 0;

  TVector3 normal(nav->FindNormal());
  TVector3 momentum(nav->GetCurrentDirection());

  ABorderSurfaceCondition* condition = component1 ? component1->FindSurfaceCondition(component2) : 0;

  if(condition and condition->GetGaussianRoughness() != 0){
    // The following method is based on G4OpBoundaryProcess::GetFacetNormal in
    // Geant4 optics
    TVector3 facetNormal;
    Double_t alpha;
    Double_t sigma_alpha = condition->GetGaussianRoughness();
    Double_t f_max = TMath::Min(1., 4.*sigma_alpha);

    do {
      do {
        alpha = gRandom->Gaus(0, sigma_alpha);
      } while (gRandom->Uniform(f_max) > TMath::Sin(alpha) || alpha >= TMath::PiOver2());

      Double_t phi = gRandom->Uniform(TMath::TwoPi());

      Double_t SinAlpha = TMath::Sin(alpha);
      Double_t CosAlpha = TMath::Cos(alpha);
      Double_t SinPhi = TMath::Sin(phi);
      Double_t CosPhi = TMath::Cos(phi);

      Double_t unit_x = SinAlpha * CosPhi;
      Double_t unit_y = SinAlpha * SinPhi;
      Double_t unit_z = CosAlpha;

      facetNormal.SetXYZ(unit_x, unit_y, unit_z);

      TVector3 tmpNormal = normal;

      facetNormal.RotateUz(tmpNormal);
    } while (momentum * facetNormal <= 0.0);
    normal = facetNormal;
  } // if

  return normal;
}

//_____________________________________________________________________________
void* AOpticsManager::Thread(void* args)
{
  AOpticsManager* manager = (AOpticsManager*)((TObject**)args)[0];
  ARayArray* array = (ARayArray*)((TObject**)args)[1];

  TObjArray* running = array->GetRunning();
  manager->TraceNonSequential(running);

  //TThread::Lock();
  Int_t n = running->GetLast();

  for(Int_t i = 0; i <= n; i++){
    ARay* ray = (ARay*)(running->RemoveAt(i));
    if(!ray) continue;
    array->Add(ray);
  } // i

  running->Expand(0);
  //TThread::UnLock();

  manager->RemoveNavigator(manager->GetCurrentNavigator());

  return 0;
}

//_____________________________________________________________________________
void AOpticsManager::TraceNonSequential(ARay& ray)
{
  TObjArray array;
  array.SetOwner(kFALSE);
  array.Add(&ray);
  TraceNonSequential(&array);
}

//_____________________________________________________________________________
void AOpticsManager::TraceNonSequential(TObjArray* array)
{
  TGeoNavigator* nav = GetCurrentNavigator();
  if(!nav){
    nav = AddNavigator();
  } // if

  Int_t n = array->GetLast();
  for(Int_t j = 0; j <= n; j++){
    ARay* ray = (ARay*)array->At(j);
    if(not ray or not ray->IsRunning()){
      continue;
    } // if

    Double_t lambda = ray->GetLambda();
    Double_t x1[4], d1[3];
    ray->GetLastPoint(x1);
    ray->GetDirection(d1);
    nav->InitTrack(x1, d1);

    while(ray->IsRunning()){
      ray->GetLastPoint(x1);
      ray->GetDirection(d1);

      TGeoNode* currentNode = nav->GetCurrentNode();
      if(nav->IsOutside()){ // if the current position is outside of top volume
        currentNode = 0;
      } // if

      TGeoNode* nextNode = nav->FindNextBoundaryAndStep();
      Double_t step = nav->GetStep(); // distance to the next boundary

      // Check type of start node
      Int_t typeCurrent = kOther;
      Int_t typeNext    = kOther;

      if     (                  !currentNode)  typeCurrent = kNull;
      else if(            IsLens(currentNode)) typeCurrent = kLens;
      else if(     IsObscuration(currentNode)) typeCurrent = kObs;
      else if(          IsMirror(currentNode)) typeCurrent = kMirror;
      else if(    IsFocalSurface(currentNode)) typeCurrent = kFocus;
      else if(IsOpticalComponent(currentNode)) typeCurrent = kOpt;

      // Check type of next node
      if     (                  !nextNode)  typeNext = kNull;
      else if(            IsLens(nextNode)) typeNext = kLens;
      else if(     IsObscuration(nextNode)) typeNext = kObs;
      else if(          IsMirror(nextNode)) typeNext = kMirror;
      else if(    IsFocalSurface(nextNode)) typeNext = kFocus;
      else if(IsOpticalComponent(nextNode)) typeNext = kOpt;

      if(typeCurrent == kLens){
        Double_t abs = ((ALens*)currentNode->GetVolume())->GetAbsorptionLength(lambda);
        if(abs > 0){
          Double_t abs_step = gRandom->Exp(abs);
          if(abs_step < step){
            Double_t n1 = ((ALens*)currentNode->GetVolume())->GetRefractiveIndex(lambda);
            Double_t speed = TMath::C()*m()/n1;
            Double_t x2[3];
            for(Int_t i = 0; i < 3; i++){
              x2[i] = x1[i] + abs_step*d1[i];
            } // if
            Double_t t = x1[3] + abs_step/speed;
            ray->AddPoint(x2[0], x2[1], x2[2], t);
            ray->AddNode(nextNode);
            ray->Absorb();
            continue;
          } // if
        } // if
      } // if

      if((typeCurrent == kNull or typeCurrent == kOpt or typeCurrent == kLens or typeCurrent == kOther)
          and typeNext == kMirror){
        Double_t n1 = typeCurrent == kLens ? ((ALens*)currentNode->GetVolume())->GetRefractiveIndex(lambda) : 1.;
        DoReflection(n1, *ray, nav, currentNode, nextNode);
      } else if((typeCurrent == kNull or typeCurrent == kOpt or typeCurrent == kOther)
          and typeNext == kLens){
        Double_t n1 = 1; // Assume refractive index equals 1 (= vacuum)
        Double_t n2 = ((ALens*)nextNode->GetVolume())->GetRefractiveIndex(lambda);
        DoFresnel(n1, n2, *ray, nav, currentNode, nextNode);
      } else if((typeCurrent == kNull or typeCurrent == kLens or typeCurrent == kOpt or typeCurrent == kOther)
          and (typeNext == kObs or typeNext == kFocus)){

        const Double_t* x2 = nav->GetCurrentPoint();
        Double_t t;
        if (typeCurrent == kLens){
          Double_t n1 = ((ALens*)currentNode->GetVolume())->GetRefractiveIndex(lambda);
          Double_t speed = TMath::C()*m()/n1;
          t = x1[3] + step/speed;
        } else {
          Double_t speed = TMath::C()*m();
          t = x1[3] + step/speed;
        } // if
        ray->AddPoint(x2[0], x2[1], x2[2], t);
        ray->AddNode(nextNode);
      } else if((typeCurrent == kNull or typeCurrent == kOpt or typeCurrent == kOther)
            and (typeNext == kOther or typeNext == kOpt)){
        const Double_t* x2 = nav->GetCurrentPoint();

        Double_t speed = TMath::C()*m();
        Double_t t = x1[3] + step/speed;
        ray->AddPoint(x2[0], x2[1], x2[2], t);
        ray->AddNode(nextNode);
      } else if(typeCurrent == kLens and typeNext == kLens){
        Double_t n1 = ((ALens*)currentNode->GetVolume())->GetRefractiveIndex(lambda);
        Double_t n2 = ((ALens*)nextNode->GetVolume())->GetRefractiveIndex(lambda);
        DoFresnel(n1, n2, *ray, nav, currentNode, nextNode);
      } else if(typeCurrent == kLens and
               (typeNext == kNull or typeNext == kOpt or typeNext == kOther)){
        Double_t n1 = ((ALens*)currentNode->GetVolume())->GetRefractiveIndex(lambda);
        Double_t n2 = 1; // Assume refractive index equals 1 (= vacuum)
        DoFresnel(n1, n2, *ray, nav, currentNode, nextNode);
      } // if

      if(typeNext == kNull){
        const Double_t* x2 = nav->GetCurrentPoint();
        Double_t speed = TMath::C()*m();
        Double_t t = x1[3] + step/speed;
        ray->AddPoint(x2[0], x2[1], x2[2], t);
        ray->AddNode(nextNode);
        ray->Exit();
      } else if(typeCurrent == kFocus or typeCurrent == kObs or typeCurrent == kMirror or typeNext == kObs){
        ray->Stop();
      } else if(typeNext == kFocus){
        AFocalSurface* focal = (AFocalSurface*)nextNode->GetVolume();
        Double_t angle = 0.;
        if(focal->HasQEAngle()){
          TVector3 n = GetFacetNormal(nav, currentNode, nextNode); // normal vect perpendicular to the surface
          Double_t d1[3];
          ray->GetDirection(d1);
          Double_t cosi = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2];
          angle = TMath::ACos(cosi);
        } // if
        Double_t qe = focal->GetQuantumEfficiency(lambda, angle);
        if(qe == 1 or gRandom->Uniform(0, 1) < qe){
          ray->Focus();
        } else {
          ray->Stop();
        } // if
      } // if

      if(ray->IsRunning() and ray->GetNpoints() >= fLimit){
        ray->Suspend();
      } // if
    } // while
  } // j
}

//_____________________________________________________________________________
void AOpticsManager::TraceNonSequential(ARayArray& array)
{
  TObjArray* running = array.GetRunning();

  Int_t n = running->GetLast();
#ifdef SINGLE_THREAD
  Int_t nthreads = 1;
#else
  Int_t nthreads = GetMaxThreads();
#endif

  if(IsMultiThread() and nthreads >= 1){
    TThread** threads = new TThread*[nthreads];
    ARayArray** dividedArray = new ARayArray*[nthreads];

    for(Int_t i = 0; i < nthreads; i++){
      dividedArray[i] = new ARayArray();
      for(Int_t j = ((n + 1)/nthreads)*i; (i == (nthreads - 1)) ? j <= n : j < ((n + 1)/nthreads)*(i + 1); j++){
        ARay* ray = (ARay*)running->RemoveAt(j);
        dividedArray[i]->Add(ray);
      } // j
    } // i

    TObject*** args = new TObject**[nthreads];
    for(Int_t i = 0; i < nthreads; i++){
      args[i] = new TObject*[2];
      args[i][0] = this;
      args[i][1] = dividedArray[i];
      threads[i] = new TThread(Form("thread%d", i), AOpticsManager::Thread, (void*)args[i]);
      threads[i]->Run();
    } // i

    for(Int_t i = 0; i < nthreads; i++){
      threads[i]->Join();
    } // i

    ClearThreadsMap();

    for(Int_t i = 0; i < nthreads; i++){
      delete [] args[i];
      array.Merge(dividedArray[i]);
      SafeDelete(dividedArray[i]);
      SafeDelete(threads[i]);
    } // i

    delete [] args;
    delete [] threads;
    delete [] dividedArray;
  } else { // single thread
    TObjArray* objarray = new TObjArray;
    for(Int_t i = 0; i <= n; i++){
      ARay* ray = (ARay*)running->RemoveAt(i);
      if(!ray) continue;
      objarray->Add(ray);
    } // i
    TraceNonSequential(objarray);
    n = objarray->GetLast();
    for(Int_t i = 0; i <= n; i++){
      ARay* ray = (ARay*)objarray->RemoveAt(i);
      if(!ray) continue;
      array.Add(ray);
    } // i
    delete objarray;
  } // if

  running->Expand(0); // shrink the array
}

//_____________________________________________________________________________
void AOpticsManager::SetLimit(Int_t n)
{
  if(n > 0){
    fLimit = n;
  } // if
}
