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

#include "AOpticsManager.h"

ClassImp(AOpticsManager)

//_____________________________________________________________________________
AOpticsManager::AOpticsManager() : TGeoManager()
{
  fLimit = 100;
}

//_____________________________________________________________________________
AOpticsManager::AOpticsManager(const char* name, const char* title)
 : TGeoManager(name, title)
{
  fLimit = 100;
}

//_____________________________________________________________________________
AOpticsManager::~AOpticsManager()
{
}

//_____________________________________________________________________________
void AOpticsManager::TraceNonSequential(ARay& ray)
{
  Double_t lambda = ray.GetLambda();
  while(ray.IsRunning()){
    Double_t x[4]; // start point
    Double_t nx[3]; // end point
    Double_t d1[3]; // start direction
    Double_t d2[3]; // end direction
    ray.GetLastPoint(x);
    ray.GetDirection(d1);
    
    TGeoNode* startnode = InitTrack(x, d1); // start node
    if(IsOutside()){ // if the current position is outside of top volume
      startnode = 0;
    } // if

    TGeoNode* endnode = FindNextBoundaryAndStep();
    Double_t step = GetStep(); // distance to the next boundary
 
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
    if(type2 == kMirror){
      Double_t epsilon = 1e-6; // Fixed in TGeoNavigator.cxx (equiv to 1e-6 cm)
      step -= epsilon*2; // stop the step before reaching the mirror
    } // if

    if(type1 == kLens){
      Double_t abs = ((ALens*)startnode->GetVolume())->GetAbsorptionLength(lambda);
      if(abs > 0){
        Double_t abs_step = gRandom->Exp(abs);
        if(abs_step < step){
          Double_t speed = (TMath::C()*m())/((ALens*)startnode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
          ray.AddPoint(x[0] + d1[0]*abs_step, x[1] + d1[1]*abs_step, x[2] + d1[2]*abs_step, x[3] + step/speed);
          ray.Stop();
          continue;
        } // if
      } // if
    } // if

    if((type1 == kNull or type1 == kOpt or type1 == kLens or type1 == kOther)
       and type2 == kMirror){

      Double_t* n = FindNormal(); // normal vect perpendicular to the surface
      Double_t dn = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2];
      for(Int_t i = 0; i < 3; i++){ // d2 = d1 - 2n*(d1*n)
        nx[i] = x[i] + step*d1[i];
        d2[i] = d1[i] - 2*n[i]*dn;
      } // i

      // step (m), c (m/s)
      if (type1 == kLens){
        Double_t speed = (TMath::C()*m())/((ALens*)startnode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
        ray.AddPoint(nx[0], nx[1], nx[2], x[3] + step/speed);
      } else {
        ray.AddPoint(nx[0], nx[1], nx[2], x[3] + step/(TMath::C()*m()));
      } // if
      ray.SetDirection(d2); // reflected

    } else if((type1 == kNull or type1 == kOpt or type1 == kOther)
               and type2 == kLens){

      Double_t* n = FindNormal(); // normal vect perpendicular to the surface
      Double_t n1 = 1; // Assume refractive index equals 1 (= vacuum)
      Double_t n2 = ((ALens*)endnode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
      Double_t dn = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2]; // d1*n
      Double_t p = TMath::Sqrt(1 - (n1*n1/n2/n2)*(1 - dn*dn));
      for(Int_t i = 0; i < 3; i++){
        nx[i] = x[i] + step*d1[i];
        d2[i] = (d1[i] - dn*n[i])*n1/n2 + n[i]*p;
      } // i
      ray.SetDirection(d2); // refracted
      ray.AddPoint(nx[0], nx[1], nx[2], x[3] + step/TMath::C());

    } else if((type1 == kNull or type1 == kLens or type1 == kOpt or type1 == kOther)
               and (type2 == kObs or type2 == kFocus)){

      for(Int_t i = 0; i < 3; i++){
        nx[i] = x[i] + step*d1[i];
      } // i
      if (type1 == kLens){
        Double_t speed = TMath::C()/((ALens*)startnode->GetVolume())->GetRefractiveIndex(ray.GetLambda());
        ray.AddPoint(nx[0], nx[1], nx[2], x[3] + step/speed);
      } else {
        ray.AddPoint(nx[0], nx[1], nx[2], x[3] + step/TMath::C());
      } // if
    } else if((type1 == kNull or type1 == kOpt or type1 == kOther)
               and (type2 == kOther or type2 == kOpt)){

      for(Int_t i = 0; i < 3; i++){
        nx[i] = x[i] + step*d1[i];
      } // i
      ray.AddPoint(nx[0], nx[1], nx[2], x[3] + step/TMath::C());

    } else if(type1 == kLens and type2 == kLens){

      Double_t* n = FindNormal(); // normal vect perpendicular to the surface
      Double_t n1 = ((ALens*)startnode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t n2 = ((ALens*)endnode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t dn = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2]; // d1*n
      Double_t p = TMath::Sqrt(1 - (n1*n1/n2/n2)*(1 - dn*dn));
      for(Int_t i = 0; i < 3; i++){
        nx[i] = x[i] + step*d1[i];
        d2[i] = (d1[i] - dn*n[i])*n1/n2 + n[i]*p;
      } // i
      ray.SetDirection(d2); // refracted
      ray.AddPoint(nx[0], nx[1], nx[2], x[3] + step/(TMath::C()/n1));

    } else if(type1 == kLens and
              (type2 == kNull or type2 == kOpt or type2 == kOther)){

      Double_t* n = FindNormal(); // normal vect perpendicular to the surface
      Double_t n1 = ((ALens*)startnode->GetVolume())->GetRefractiveIndex(lambda);
      Double_t n2 = 1; // Assume refractive index equals 1 (= vacuum)
      Double_t dn = d1[0]*n[0] + d1[1]*n[1] + d1[2]*n[2];
      Double_t p = sqrt(1 - (n1*n1/n2/n2)*(1 - dn*dn));
      for(Int_t i = 0; i < 3; i++){
        nx[i] = x[i] + step*d1[i];
        d2[i] = (d1[i] - dn*n[i])*n1/n2 + n[i]*p;
      } // i
      ray.SetDirection(d2); // refracted
      ray.AddPoint(nx[0], nx[1], nx[2], x[3] + step/(TMath::C()/n1));

    } // if

    if(type2 == kNull){
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
