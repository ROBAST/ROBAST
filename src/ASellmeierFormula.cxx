/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ASellmeierFormula
//
// Sellmeier's formula for calculation of refractive index
// See http://en.wikipedia.org/wiki/Sellmeier_equation
//
///////////////////////////////////////////////////////////////////////////////

#include "AOpticsManager.h"
#include "ASellmeierFormula.h"
#include "TMath.h"
#include "TROOT.h"

ClassImp(ASellmeierFormula)

ASellmeierFormula::ASellmeierFormula() : ARefractiveIndex()
{
}

//______________________________________________________________________________
ASellmeierFormula::ASellmeierFormula(Double_t B1, Double_t B2, Double_t B3,
                                     Double_t C1, Double_t C2, Double_t C3)
  : ARefractiveIndex()
{
  // n^2(lambda) = 1 + B1*lamda^2/(lamda^2 - C1) + B2*lamda^2/(lamda^2 - C2) + B3*lamda^2/(lamda^2 - C3)
  // where lambda is measured in (um)
  fPar[0] = B1;
  fPar[1] = B2;
  fPar[2] = B3;
  fPar[3] = C1;
  fPar[4] = C2;
  fPar[5] = C3;
}

//______________________________________________________________________________
ASellmeierFormula::ASellmeierFormula(const Double_t* p)
{
  for(Int_t i = 0; i < 6; i++){
    fPar[i] = p[i];
  } // i
}

//______________________________________________________________________________
Double_t ASellmeierFormula::GetIndex(Double_t lambda) const
{
  // Calculate the refractive index at wavelength = lambda (m)
  // Use AOpticsManager::m() to get the unit length in (m)
  lambda /= AOpticsManager::um(); // Convert (nm) to (um)
  Double_t lambda2 = lambda*lambda;
  return TMath::Sqrt(1 + fPar[0]*lambda2/(lambda2 - fPar[3])
                       + fPar[1]*lambda2/(lambda2 - fPar[4])
                       + fPar[2]*lambda2/(lambda2 - fPar[5]));
}

//______________________________________________________________________________
TF1* ASellmeierFormula::FitData(TGraph* graph, const char* tf1name, Option_t* option)
{
  // Fit the given TGraph with the Sellmeier formula. If function "tf1name"
  // already exists, the existing function is used, otherwise, new TF1 is
  // created. The unit of wavelength must be (m) using AOpticsManager::m().
  if(graph == 0 or tf1name == 0){
    return 0;
  } // if

  TF1* f = (TF1*)gROOT->GetFunction(tf1name);
  if(!f){
    Double_t xmin = TMath::MinElement(graph->GetN(), graph->GetX());
    Double_t xmax = TMath::MaxElement(graph->GetN(), graph->GetX());

    f = MakeGraph(tf1name, xmin, xmax);
  } // if

  graph->Fit(f, option);
  for(Int_t i = 0; i < (Int_t)(sizeof(fPar)/sizeof(Double_t)); i++){
    fPar[i] = f->GetParameter(i);
  } // i

  return f;
}

//______________________________________________________________________________
TF1* ASellmeierFormula::MakeGraph(const char* tf1name, Double_t xmin, Double_t xmax)
{
  Double_t um = AOpticsManager::um();
  TF1* f = new TF1(tf1name, Form("sqrt(1 + [0]*(x/%f)**2/((x/%f)**2 - [3]) + "
                                          "[1]*(x/%f)**2/((x/%f)**2 - [4]) + "
                                          "[2]*(x/%f)**2/((x/%f)**2 - [5]))",
                                          um, um, um, um, um, um),
                   xmin, xmax);

  for(Int_t i = 0; i < (Int_t)(sizeof(fPar)/sizeof(Double_t)); i++){
    f->SetParameter(i, fPar[i]);
  } // i
  f->SetParNames("B1", "B2", "B3", "C1", "C2", "C3");

  return f;
}

