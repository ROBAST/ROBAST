// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_REFRACTIVE_INDEX_H
#define A_REFRACTIVE_INDEX_H

#include "TGraph.h"
#include "TMath.h"

#include <complex>
#include <limits>
#include <memory>

///////////////////////////////////////////////////////////////////////////////
//
// ARefractiveIndex
//
// Abstract class for refractive index
//
///////////////////////////////////////////////////////////////////////////////

class ARefractiveIndex : public TObject {
 protected:
  std::shared_ptr<TGraph> fRefractiveIndex;
  std::shared_ptr<TGraph> fExtinctionCoefficient;

 public:
  ARefractiveIndex() {};
  ARefractiveIndex(Double_t n, Double_t k = 0.);
  virtual ~ARefractiveIndex() {};

  virtual Double_t GetAbbeNumber() const;
  virtual Double_t GetIndex(Double_t lambda) const {
    return GetRefractiveIndex(lambda);
  }
  virtual Double_t GetRefractiveIndex(Double_t lambda) const {
    return fRefractiveIndex ? fRefractiveIndex->Eval(lambda) : 1.;
  }
  virtual Double_t GetExtinctionCoefficient(Double_t lambda) const {
    return fExtinctionCoefficient ? fExtinctionCoefficient->Eval(lambda) : 0.;
  }
  virtual Double_t GetAbsorptionLength(Double_t lambda) const {
    static const Double_t inf = std::numeric_limits<Double_t>::infinity();
    Double_t k = GetExtinctionCoefficient(lambda);
    return k <= 0. ? inf : ExtinctionCoefficientToAbsorptionLength(k, lambda);
  }
  virtual std::complex<Double_t> GetComplexRefractiveIndex(Double_t lambda) const {
    return std::complex<Double_t>(GetRefractiveIndex(lambda), GetExtinctionCoefficient(lambda));
  }
  virtual void SetExtinctionCoefficient(std::shared_ptr<TGraph> graph) {
    fExtinctionCoefficient = graph;
  }
  virtual void SetRefractiveIndex(std::shared_ptr<TGraph> graph) {
    fRefractiveIndex = graph;
  }
  static Double_t AbsorptionLengthToExtinctionCoefficient(Double_t a, Double_t lambda) {
    return lambda / (4 * TMath::Pi() * a);
  }
  static Double_t ExtinctionCoefficientToAbsorptionLength(Double_t k, Double_t lambda) {
    return lambda / (4 * TMath::Pi() * k);
  }

  ClassDef(ARefractiveIndex, 1)
};

#endif  // A_REFRACTIVE_INDEX_H
