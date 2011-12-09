// $Id$
// Author: Akira Okumura 2007/09/24

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
#include "RVersion.h"

#include "AOpticsManager.h"

#ifdef _OPENMP
#include <omp.h>
#if ROOT_VERSION_CODE >= ROOT_VERSION(5,32,0)
#define MULTI_THREAD_NAVIGATION
#endif
#endif

static const Double_t kEpsilon = 1e-6; // Fixed in TGeoNavigator.cxx (equiv to 1e-6 cm)

ClassImp(AOpticsManager)

//_____________________________________________________________________________
AOpticsManager::AOpticsManager() : TGeoManager(), fDisableFresnelReflection(kFALSE)
{
  fLimit = 100;
  fNThreads = 1;
#ifdef MULTI_THREAD_NAVIGATION
  SetMultiThread(kTRUE);
#endif
}

//_____________________________________________________________________________
AOpticsManager::AOpticsManager(const char* name, const char* title)
 : TGeoManager(name, title), fDisableFresnelReflection(kFALSE)
{
  fLimit = 100;
  fNThreads = 1;
#ifdef MULTI_THREAD_NAVIGATION
  SetMultiThread(kTRUE);
#endif
}

//_____________________________________________________________________________
AOpticsManager::~AOpticsManager()
{
}

//_____________________________________________________________________________
void AOpticsManager::DoFresnel(Double_t n1, Double_t n2, ARay& ray)
{
  // Use the same notation used in Wikipedia
  // http://en.wikipedia.org/wiki/Fresnel_equations
  // theta_i = incident angle
  // theta_t = transmission angle
  Double_t* n = FindNormal(); // normal vect perpendicular to the surface
  Double_t cosi = fD1[0]*n[0] + fD1[1]*n[1] + fD1[2]*n[2]; // cos(theta_i)
  Double_t sini = TMath::Sqrt(1 - cosi*cosi);
  Double_t sint = n1*sini/n2; // Snell's law

  if(sint > 1.){ // total internal reflection
    fStep -= kEpsilon*2.; // stop the step before reaching the boundary
    DoReflection(n1, ray);
    return;
  } // if

  Double_t cost = TMath::Sqrt(1 - sint*sint);

  if(fDisableFresnelReflection == kFALSE){
    Double_t Rs = TMath::Power((n1*cosi - n2*cost)/(n1*cosi + n2*cost), 2); // reflectivity for s-polarized photon
    Double_t Rp = TMath::Power((n1*cost - n2*cosi)/(n1*cost + n2*cosi), 2); // reflectivity for p-polarized photon
    Double_t R = (Rs + Rp)/2.; // We assume that polarization is random

    if(gRandom->Uniform(1) < R){ // reflection at the boundary
      fStep -= kEpsilon*2.; // stop the step before reaching the boundary
      DoReflection(n1, ray);
      return;
    } // if
  } // if

  for(Int_t i = 0; i < 3; i++){
    fX2[i] = fX1[i] + fStep*fD1[i];
    fD2[i] = (fD1[i] - cosi*n[i])*sint/sini + n[i]*cost;
  } // i
  ray.SetDirection(fD2);

  // step (m), c (m/s)
  Double_t speed = TMath::C()*m()/n1;
  fX2[3] = fX1[3] + fStep/speed;
  ray.AddPoint(fX2[0], fX2[1], fX2[2], fX2[3]);
}

//_____________________________________________________________________________
void AOpticsManager::DoReflection(Double_t n1, ARay& ray)
{
  Double_t* n = FindNormal(); // normal vect perpendicular to the surface
  Double_t cosi = fD1[0]*n[0] + fD1[1]*n[1] + fD1[2]*n[2];
  for(Int_t i = 0; i < 3; i++){ // d2 = d1 - 2n*(d1*n)
    fX2[i] = fX1[i] + fStep*fD1[i];
    fD2[i] = fD1[i] - 2*n[i]*cosi;
  } // i
  ray.SetDirection(fD2);

  // step (m), c (m/s)
  Double_t speed = TMath::C()*m()/n1;
  fX2[3] = fX1[3] + fStep/speed;
  ray.AddPoint(fX2[0], fX2[1], fX2[2], fX2[3]);
}

//_____________________________________________________________________________
void AOpticsManager::TraceNonSequential(ARay& ray)
{
  Double_t lambda = ray.GetLambda();
  while(ray.IsRunning()){
    //Double_t fX1[4]; // start point
    //Double_t fX2[3]; // end point
    //Double_t fD1[3]; // start direction
    //Double_t fD2[3]; // end direction
    ray.GetLastPoint(fX1);
    ray.GetDirection(fD1);

    TGeoNode* startnode = InitTrack(fX1, fD1); // start node
    if(IsOutside()){ // if the current position is outside of top volume
      startnode = 0;
    } // if

    TGeoNode* endnode = FindNextBoundaryAndStep();
    enum {kLens, kObs, kMirror, kFocus, kOpt, kOther, kNull};
    Int_t type1, type2;

    // Check type of start node
    if     (                  !startnode)  type1 = kNull;
    else if(            IsLens(startnode)) type1 = kLens;
    else if(     IsObscuration(startnode)) type1 = kObs;
    else if(          IsMirror(startnode)) type1 = kMirror;
    else if(IsOpticalComponent(startnode)) type1 = kOpt;
    else if(    IsFocalSurface(startnode)) type1 = kFocus;
    else                                   type1 = kOther;
    
    // Check type of end node
    if     (                  !endnode)  type2 = kNull;
    else if(            IsLens(endnode)) type2 = kLens;
    else if(     IsObscuration(endnode)) type2 = kObs;
    else if(          IsMirror(endnode)) type2 = kMirror;
    else if(IsOpticalComponent(endnode)) type2 = kOpt;
    else if(    IsFocalSurface(endnode)) type2 = kFocus;
    else                                 type2 = kOther;

    fStep = GetStep(); // distance to the next boundary
    if(type2 == kMirror){
      fStep -= kEpsilon; // make sure that the photon do NOT cross the boundary
    } else {
      fStep += kEpsilon; // make sure that the photon crosses the boundary
    } // if

    if(type1 == kLens){
      Double_t abs = ((ALens*)startnode->GetVolume())->GetAbsorptionLength(lambda);
      if(abs > 0){
        Double_t abs_step = gRandom->Exp(abs);
        if(abs_step < fStep){
          Double_t n1 = ((ALens*)startnode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
          Double_t speed = TMath::C()*m()/n1;
          ray.AddPoint(fX1[0] + fD1[0]*abs_step, fX1[1] + fD1[1]*abs_step, fX1[2] + fD1[2]*abs_step, fX1[3] + abs_step/speed);
          ray.Stop();
          continue;
        } // if
      } // if
    } // if

    if((type1 == kNull or type1 == kOpt or type1 == kLens or type1 == kOther)
       and type2 == kMirror){
      Double_t n1 = type1 == kLens ? ((ALens*)startnode->GetVolume())->GetRefractiveIndex(ray.GetLambda()) : 1.;
      DoReflection(n1, ray);
    } else if((type1 == kNull or type1 == kOpt or type1 == kOther)
               and type2 == kLens){
      Double_t n1 = 1; // Assume refractive index equals 1 (= vacuum)
      Double_t n2 = ((ALens*)endnode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
      DoFresnel(n1, n2, ray);
    } else if((type1 == kNull or type1 == kLens or type1 == kOpt or type1 == kOther)
               and (type2 == kObs or type2 == kFocus)){

      for(Int_t i = 0; i < 3; i++){
        fX2[i] = fX1[i] + fStep*fD1[i];
      } // i
      if (type1 == kLens){
        Double_t n1 = ((ALens*)startnode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
        Double_t speed = TMath::C()*m()/n1;
        fX2[3] = fX1[3] + fStep/speed;
      } else {
        Double_t speed = TMath::C()*m();
        fX2[3] = fX1[3] + fStep/speed;
      } // if
      ray.AddPoint(fX2[0], fX2[1], fX2[2], fX2[3]);
    } else if((type1 == kNull or type1 == kOpt or type1 == kOther)
               and (type2 == kOther or type2 == kOpt)){

      for(Int_t i = 0; i < 3; i++){
        fX2[i] = fX1[i] + fStep*fD1[i];
      } // i
      Double_t speed = TMath::C()*m();
      fX2[3] = fX1[3] + fStep/speed;
      ray.AddPoint(fX2[0], fX2[1], fX2[2], fX2[3]);
    } else if(type1 == kLens and type2 == kLens){
      Double_t n1 = ((ALens*)startnode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t n2 = ((ALens*)endnode->GetVolume())->GetRefractiveIndex(lambda);
      DoFresnel(n1, n2, ray);
    } else if(type1 == kLens and
              (type2 == kNull or type2 == kOpt or type2 == kOther)){
      Double_t n1 = ((ALens*)startnode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t n2 = 1; // Assume refractive index equals 1 (= vacuum)
      DoFresnel(n1, n2, ray);
    } // if

    if(type2 == kNull){
      for(Int_t i = 0; i < 3; i++){
        fX2[i] = fX1[i] + fStep*fD1[i];
      } // i
      Double_t speed = TMath::C()*m();
      fX2[3] = fX1[3] + fStep/speed;
      ray.AddPoint(fX2[0], fX2[1], fX2[2], fX2[3]);
      ray.Exit();
    } else if(type1 == kFocus or type1 == kObs or type1 == kMirror or type2 == kObs){
      ray.Stop();
    } else if(type2 == kFocus){
      ray.Focus();
    } // if

    if(ray.IsRunning() and ray.GetNpoints() >= fLimit){
      ray.Suspend();
    } // if

  } // while
}
  
//_____________________________________________________________________________
void AOpticsManager::TraceNonSequential(ARayArray& array)
{
  TObjArray* running = array.GetRunning();

#ifdef MULTI_THREAD_NAVIGATION
  omp_set_num_threads(fNThreads);
#pragma omp parallel
#pragma omp parallel for
#endif
  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(*running)[i];
    if(!ray) continue;

    ray = (ARay*)running->RemoveAt(i);
    TraceNonSequential(*ray);
    array.Add(ray);
  } // i
}

//_____________________________________________________________________________
void AOpticsManager::SetLimit(Int_t n)
{
  if(n > 0){
    fLimit = n;
  } // if
}
