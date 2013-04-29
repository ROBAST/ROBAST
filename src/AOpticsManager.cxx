// $Id: AOpticsManager.cxx 92 2012-11-30 02:59:57Z oxon $
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
#include "TThread.h"

#include "ABorderSurfaceCondition.h"
#include "AOpticsManager.h"

static const Double_t kEpsilon = 1e-6; // Fixed in TGeoNavigator.cxx (equiv to 1e-6 cm)

ClassImp(AOpticsManager)

//_____________________________________________________________________________
AOpticsManager::AOpticsManager() : TGeoManager(), fDisableFresnelReflection(kFALSE)
{
  fLimit = 100;
}

//_____________________________________________________________________________
AOpticsManager::AOpticsManager(const char* name, const char* title)
 : TGeoManager(name, title), fDisableFresnelReflection(kFALSE)
{
  fLimit = 100;
}

//_____________________________________________________________________________
AOpticsManager::~AOpticsManager()
{
}

//_____________________________________________________________________________
void AOpticsManager::DoFresnel(Double_t n1, Double_t n2, ARay& ray)
{
  TGeoNavigator* nav = GetCurrentNavigator();
  Double_t step = nav->GetStep();
  TGeoNode* startNode = nav->GetCurrentNode();
  TGeoNode* endNode = nav->GetNextNode();

  // Use the same notation used in Wikipedia
  // http://en.wikipedia.org/wiki/Fresnel_equations
  // theta_i = incident angle
  // theta_t = transmission angle
  // Double_t* n = FindNormal(); // normal vect perpendicular to the surface
  TVector3 n = GetFacetNormal(); // normal vect perpendicular to the surface
  Double_t d1[3];
  ray.GetDirection(d1);
  Double_t cosi = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2]; // cos(theta_i)
  Double_t sini = TMath::Sqrt(1 - cosi*cosi);
  Double_t sint = n1*sini/n2; // Snell's law

  if(sint > 1.){ // total internal reflection
    step -= kEpsilon*2.; // stop the step before reaching the boundary
    nav->SetStep(step);
    DoReflection(n1, ray);
    return;
  } // if

  Double_t cost = TMath::Sqrt(1 - sint*sint);

  if(fDisableFresnelReflection == kFALSE){
    Double_t Rs = TMath::Power((n1*cosi - n2*cost)/(n1*cosi + n2*cost), 2); // reflectivity for s-polarized photon
    Double_t Rp = TMath::Power((n1*cost - n2*cosi)/(n1*cost + n2*cosi), 2); // reflectivity for p-polarized photon
    Double_t R = (Rs + Rp)/2.; // We assume that polarization is random

    if(gRandom->Uniform(1) < R){ // reflection at the boundary
      step -= kEpsilon*2.; // stop the step before reaching the boundary
      nav->SetStep(step);
      DoReflection(n1, ray);
      return;
    } // if
  } // if

  Double_t x1[4], x2[4], d2[3];
  ray.GetLastPoint(x1);

  for(Int_t i = 0; i < 3; i++){
    x2[i] = x1[i] + step*d1[i];
    d2[i] = (d1[i] - cosi*n[i])*sint/sini + n[i]*cost;
  } // i
  ray.SetDirection(d2);

  // step (m), c (m/s)
  Double_t speed = TMath::C()*m()/n1;
  x2[3] = x1[3] + step/speed;
  ray.AddPoint(x2[0], x2[1], x2[2], x2[3]);
}

//_____________________________________________________________________________
void AOpticsManager::DoReflection(Double_t n1, ARay& ray)
{
  TGeoNavigator* nav = GetCurrentNavigator();
  Double_t step = nav->GetStep();
  TGeoNode* startNode = nav->GetCurrentNode();
  TGeoNode* endNode = nav->GetNextNode();

  //Double_t* n = FindNormal(); // normal vect perpendicular to the surface
  TVector3 n = GetFacetNormal(); // normal vect perpendicular to the surface
  Double_t d1[3];
  ray.GetDirection(d1);
  Double_t cosi = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2];

  Bool_t absorbed = kFALSE;

  if(IsMirror(endNode) == kMirror){
    Double_t angle = TMath::ACos(cosi)*TMath::RadToDeg();
    Double_t lambda = ray.GetLambda();
    Double_t ref = ((AMirror*)endNode->GetVolume())->GetReflectivity(lambda, angle);
    if(ref < gRandom->Uniform(1)){
      absorbed = kTRUE;
      ray.Absorb();
    } // if
  } // if

  Double_t x1[4], x2[4], d2[3];
  ray.GetLastPoint(x1);

  for(Int_t i = 0; i < 3; i++){ // d2 = d1 - 2n*(d1*n)
    x2[i] = x1[i] + step*d1[i];
    d2[i] = d1[i] - 2*n[i]*cosi;
  } // i
  if(not absorbed){
    ray.SetDirection(d2);
  } // if

  Double_t speed = TMath::C()*m()/n1;
  x2[3] = x1[3] + step/speed;
  ray.AddPoint(x2[0], x2[1], x2[2], x2[3]);
}

//_____________________________________________________________________________
TVector3 AOpticsManager::GetFacetNormal()
{
  TGeoNavigator* nav = GetCurrentNavigator();
  TGeoNode* startNode = nav->GetCurrentNode();
  TGeoNode* endNode = nav->GetNextNode();

  TGeoVolume* volume1 = startNode->GetVolume();
  TGeoVolume* volume2 = endNode ? endNode->GetVolume() : 0;

  TVector3 normal(FindNormal());
  TVector3 momentum(nav->GetCurrentDirection());

  ABorderSurfaceCondition* condition = ABorderSurfaceCondition::GetSurfaceCondition(volume1, volume2);

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
  AOpticsManager* manager = (AOpticsManager*)((TObjArray**)args)[0];
  ARayArray* array = (ARayArray*)((TObjArray**)args)[1];

  TObjArray* running = array->GetRunning();
  
  for(Int_t i = 0; i <= running->GetLast(); i++){
    ARay* ray = (ARay*)(running->RemoveAt(i));
    if(!ray) continue;
    manager->TraceNonSequential(*ray);
    TThread::Lock();
    array->Add(ray);
    TThread::UnLock();
  } // i

  return 0;
}

//_____________________________________________________________________________
void AOpticsManager::TraceNonSequential(ARay& ray)
{
  if(not ray.IsRunning()){
    return;
  } // if

  Double_t lambda = ray.GetLambda();

  TGeoNavigator* nav = GetCurrentNavigator();
  if(!nav){
    nav = AddNavigator();
  } // if

  while(ray.IsRunning()){
    Double_t x1[4], d1[3];
    ray.GetLastPoint(x1);
    ray.GetDirection(d1);

    TGeoNode* startNode = nav->InitTrack(x1, d1); // start node
    if(nav->IsOutside()){ // if the current position is outside of top volume
      startNode = 0;
    } // if

    TGeoNode* endNode = nav->FindNextBoundaryAndStep();

    // Check type of start node
    Int_t typeStart = kOther;
    Int_t typeEnd   = kOther;
    if     (                  !startNode)  typeStart = kNull;
    else if(            IsLens(startNode)) typeStart = kLens;
    else if(     IsObscuration(startNode)) typeStart = kObs;
    else if(          IsMirror(startNode)) typeStart = kMirror;
    else if(IsOpticalComponent(startNode)) typeStart = kOpt;
    else if(    IsFocalSurface(startNode)) typeStart = kFocus;

    // Check type of end node
    if     (                  !endNode)  typeEnd = kNull;
    else if(            IsLens(endNode)) typeEnd = kLens;
    else if(     IsObscuration(endNode)) typeEnd = kObs;
    else if(          IsMirror(endNode)) typeEnd = kMirror;
    else if(IsOpticalComponent(endNode)) typeEnd = kOpt;
    else if(    IsFocalSurface(endNode)) typeEnd = kFocus;

    Double_t step = nav->GetStep(); // distance to the next boundary
    if(typeEnd == kMirror){
      step -= kEpsilon; // make sure that the photon do NOT cross the boundary
    } else {
      step += kEpsilon; // make sure that the photon crosses the boundary
    } // if
    nav->SetStep(step);

    if(typeStart == kLens){
      Double_t abs = ((ALens*)startNode->GetVolume())->GetAbsorptionLength(lambda);
      if(abs > 0){
        Double_t abs_step = gRandom->Exp(abs);
        if(abs_step < step){
          Double_t n1 = ((ALens*)startNode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
          Double_t speed = TMath::C()*m()/n1;
          ray.AddPoint(x1[0] + x1[0]*abs_step, x1[1] + x1[1]*abs_step, x1[2] + d1[2]*abs_step, x1[3] + abs_step/speed);
          ray.Absorb();
          nav->SetStep(step);
          continue;
        } // if
      } // if
    } // if

    if((typeStart == kNull or typeStart == kOpt or typeStart == kLens or typeStart == kOther)
       and typeEnd == kMirror){
      Double_t n1 = typeStart == kLens ? ((ALens*)startNode->GetVolume())->GetRefractiveIndex(ray.GetLambda()) : 1.;
      DoReflection(n1, ray);
    } else if((typeStart == kNull or typeStart == kOpt or typeStart == kOther)
               and typeEnd == kLens){
      Double_t n1 = 1; // Assume refractive index equals 1 (= vacuum)
      Double_t n2 = ((ALens*)endNode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
      DoFresnel(n1, n2, ray);
    } else if((typeStart == kNull or typeStart == kLens or typeStart == kOpt or typeStart == kOther)
               and (typeEnd == kObs or typeEnd == kFocus)){

      Double_t x2[4];
      for(Int_t i = 0; i < 3; i++){
        x2[i] = x1[i] + step*d1[i];
      } // i
      if (typeStart == kLens){
        Double_t n1 = ((ALens*)startNode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
        Double_t speed = TMath::C()*m()/n1;
        x2[3] = x1[3] + step/speed;
      } else {
        Double_t speed = TMath::C()*m();
        x2[3] = x1[3] + step/speed;
      } // if
      ray.AddPoint(x2[0], x2[1], x2[2], x2[3]);
    } else if((typeStart == kNull or typeStart == kOpt or typeStart == kOther)
               and (typeEnd == kOther or typeEnd == kOpt)){

      Double_t x2[4];
      for(Int_t i = 0; i < 3; i++){
        x2[i] = x1[i] + step*d1[i];
      } // i
      Double_t speed = TMath::C()*m();
      x2[3] = x1[3] + step/speed;
      ray.AddPoint(x2[0], x2[1], x2[2], x2[3]);
    } else if(typeStart == kLens and typeEnd == kLens){
      Double_t n1 = ((ALens*)startNode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t n2 = ((ALens*)endNode->GetVolume())->GetRefractiveIndex(lambda);
      DoFresnel(n1, n2, ray);
    } else if(typeStart == kLens and
              (typeEnd == kNull or typeEnd == kOpt or typeEnd == kOther)){
      Double_t n1 = ((ALens*)startNode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t n2 = 1; // Assume refractive index equals 1 (= vacuum)
      DoFresnel(n1, n2, ray);
    } // if

    if(typeEnd == kNull){
      Double_t x2[4];
      for(Int_t i = 0; i < 3; i++){
        x2[i] = x1[i] + step*d1[i];
      } // i
      Double_t speed = TMath::C()*m();
      x2[3] = x1[3] + step/speed;
      ray.AddPoint(x2[0], x2[1], x2[2], x2[3]);
      ray.Exit();
    } else if(typeStart == kFocus or typeStart == kObs or typeStart == kMirror or typeEnd == kObs){
      ray.Stop();
    } else if(typeEnd == kFocus){
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

  Int_t n = running->GetLast();
  Int_t nthreads = GetMaxThreads();

  if(IsMultiThread() and nthreads >= 1){
    TThread** threads = new TThread*[nthreads];
    ARayArray** dividedArray = new ARayArray*[nthreads];

    for(Int_t i = 0; i < nthreads; i++){
      dividedArray[i] = new ARayArray();
      for(Int_t j = ((n + 1)/nthreads)*i; j < ((n + 1)/nthreads)*(i + 1) and j <= n; j++){
        ARay* ray = (ARay*)running->RemoveAt(j);
        if(!ray) continue;
        dividedArray[i]->Add(ray);
      } // j

      TObject* args[2] = {this, dividedArray[i]};
      threads[i] = new TThread(Form("thread%d", i), AOpticsManager::Thread, (void*)args);
      threads[i]->Run();
    } // i

    for(Int_t i = 0; i < nthreads; i++){
      threads[i]->Join();
    } // i

    ClearThreadsMap();

    for(Int_t i = 0; i < nthreads; i++){
      array.Merge(dividedArray[i]);
      SafeDelete(dividedArray[i]);
      SafeDelete(threads[i]);
    } // i

    delete [] threads;
    delete [] dividedArray;
  } else { // single thread
    for(Int_t i = 0; i <= n; i++){
      ARay* ray = (ARay*)(*running)[i];
      if(!ray) continue;

      ray = (ARay*)running->RemoveAt(i);
      TraceNonSequential(*ray);
      array.Add(ray);
    } // i
  } // if
}

//_____________________________________________________________________________
void AOpticsManager::SetLimit(Int_t n)
{
  if(n > 0){
    fLimit = n;
  } // if
}
