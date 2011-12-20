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

  Bool_t absorbed = kFALSE;

  if(fTypeEnd == kMirror){
    Double_t angle = TMath::ACos(cosi)*TMath::RadToDeg();
    Double_t lambda = ray.GetLambda();
    Double_t ref = ((AMirror*)fEndNode->GetVolume())->GetReflectivity(lambda, angle);
    if(ref < gRandom->Uniform(1)){
      absorbed = kTRUE;
      ray.Absorb();
    } // if
  } // if

  for(Int_t i = 0; i < 3; i++){ // d2 = d1 - 2n*(d1*n)
    fX2[i] = fX1[i] + fStep*fD1[i];
    fD2[i] = fD1[i] - 2*n[i]*cosi;
  } // i
  if(not absorbed){
    ray.SetDirection(fD2);
  } // if

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

    fStartNode = InitTrack(fX1, fD1); // start node
    if(IsOutside()){ // if the current position is outside of top volume
      fStartNode = 0;
    } // if

    fEndNode = FindNextBoundaryAndStep();

    // Check type of start node
    if     (                  !fStartNode)  fTypeStart = kNull;
    else if(            IsLens(fStartNode)) fTypeStart = kLens;
    else if(     IsObscuration(fStartNode)) fTypeStart = kObs;
    else if(          IsMirror(fStartNode)) fTypeStart = kMirror;
    else if(IsOpticalComponent(fStartNode)) fTypeStart = kOpt;
    else if(    IsFocalSurface(fStartNode)) fTypeStart = kFocus;
    else                                    fTypeStart = kOther;
    
    // Check type of end node
    if     (                  !fEndNode)  fTypeEnd = kNull;
    else if(            IsLens(fEndNode)) fTypeEnd = kLens;
    else if(     IsObscuration(fEndNode)) fTypeEnd = kObs;
    else if(          IsMirror(fEndNode)) fTypeEnd = kMirror;
    else if(IsOpticalComponent(fEndNode)) fTypeEnd = kOpt;
    else if(    IsFocalSurface(fEndNode)) fTypeEnd = kFocus;
    else                                  fTypeEnd = kOther;

    fStep = GetStep(); // distance to the next boundary
    if(fTypeEnd == kMirror){
      fStep -= kEpsilon; // make sure that the photon do NOT cross the boundary
    } else {
      fStep += kEpsilon; // make sure that the photon crosses the boundary
    } // if

    if(fTypeStart == kLens){
      Double_t abs = ((ALens*)fStartNode->GetVolume())->GetAbsorptionLength(lambda);
      if(abs > 0){
        Double_t abs_step = gRandom->Exp(abs);
        if(abs_step < fStep){
          Double_t n1 = ((ALens*)fStartNode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
          Double_t speed = TMath::C()*m()/n1;
          ray.AddPoint(fX1[0] + fD1[0]*abs_step, fX1[1] + fD1[1]*abs_step, fX1[2] + fD1[2]*abs_step, fX1[3] + abs_step/speed);
          ray.Absorb();
          continue;
        } // if
      } // if
    } // if

    if((fTypeStart == kNull or fTypeStart == kOpt or fTypeStart == kLens or fTypeStart == kOther)
       and fTypeEnd == kMirror){
      Double_t n1 = fTypeStart == kLens ? ((ALens*)fStartNode->GetVolume())->GetRefractiveIndex(ray.GetLambda()) : 1.;
      DoReflection(n1, ray);
    } else if((fTypeStart == kNull or fTypeStart == kOpt or fTypeStart == kOther)
               and fTypeEnd == kLens){
      Double_t n1 = 1; // Assume refractive index equals 1 (= vacuum)
      Double_t n2 = ((ALens*)fEndNode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
      DoFresnel(n1, n2, ray);
    } else if((fTypeStart == kNull or fTypeStart == kLens or fTypeStart == kOpt or fTypeStart == kOther)
               and (fTypeEnd == kObs or fTypeEnd == kFocus)){

      for(Int_t i = 0; i < 3; i++){
        fX2[i] = fX1[i] + fStep*fD1[i];
      } // i
      if (fTypeStart == kLens){
        Double_t n1 = ((ALens*)fStartNode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
        Double_t speed = TMath::C()*m()/n1;
        fX2[3] = fX1[3] + fStep/speed;
      } else {
        Double_t speed = TMath::C()*m();
        fX2[3] = fX1[3] + fStep/speed;
      } // if
      ray.AddPoint(fX2[0], fX2[1], fX2[2], fX2[3]);
    } else if((fTypeStart == kNull or fTypeStart == kOpt or fTypeStart == kOther)
               and (fTypeEnd == kOther or fTypeEnd == kOpt)){

      for(Int_t i = 0; i < 3; i++){
        fX2[i] = fX1[i] + fStep*fD1[i];
      } // i
      Double_t speed = TMath::C()*m();
      fX2[3] = fX1[3] + fStep/speed;
      ray.AddPoint(fX2[0], fX2[1], fX2[2], fX2[3]);
    } else if(fTypeStart == kLens and fTypeEnd == kLens){
      Double_t n1 = ((ALens*)fStartNode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t n2 = ((ALens*)fEndNode->GetVolume())->GetRefractiveIndex(lambda);
      DoFresnel(n1, n2, ray);
    } else if(fTypeStart == kLens and
              (fTypeEnd == kNull or fTypeEnd == kOpt or fTypeEnd == kOther)){
      Double_t n1 = ((ALens*)fStartNode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t n2 = 1; // Assume refractive index equals 1 (= vacuum)
      DoFresnel(n1, n2, ray);
    } // if

    if(fTypeEnd == kNull){
      for(Int_t i = 0; i < 3; i++){
        fX2[i] = fX1[i] + fStep*fD1[i];
      } // i
      Double_t speed = TMath::C()*m();
      fX2[3] = fX1[3] + fStep/speed;
      ray.AddPoint(fX2[0], fX2[1], fX2[2], fX2[3]);
      ray.Exit();
    } else if(fTypeStart == kFocus or fTypeStart == kObs or fTypeStart == kMirror or fTypeEnd == kObs){
      ray.Stop();
    } else if(fTypeEnd == kFocus){
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
