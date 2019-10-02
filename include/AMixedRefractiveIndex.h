// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_MIXED_REFRACTIVE_INDEX_H
#define A_MIXED_REFRACTIVE_INDEX_H

#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// AMixedRefractiveIndex
//
// Refractive index for two mixed materials
//
///////////////////////////////////////////////////////////////////////////////

class AMixedRefractiveIndex : public ARefractiveIndex {
 private:
  std::shared_ptr<ARefractiveIndex> fMaterialA;
  std::shared_ptr<ARefractiveIndex> fMaterialB;
  Double_t fFractionA;
  Double_t fFractionB;

  void SetExtinctionCoefficient(std::shared_ptr<TGraph>){};
  void SetRefractiveIndex(std::shared_ptr<TGraph>){};

 public:
  AMixedRefractiveIndex(std::shared_ptr<ARefractiveIndex> materialA,
                        std::shared_ptr<ARefractiveIndex> materialB,
                        Double_t fractionA, Double_t fractionB);
  virtual ~AMixedRefractiveIndex() {}

  virtual Double_t GetRefractiveIndex(Double_t lambda) const {
    Double_t nA = fMaterialA->GetRefractiveIndex(lambda);
    Double_t nB = fMaterialB->GetRefractiveIndex(lambda);
    return nA * fFractionA + nB * fFractionB;
  }
  virtual Double_t GetExtinctionCoefficient(Double_t lambda) const {
    Double_t kA = fMaterialA->GetExtinctionCoefficient(lambda);
    Double_t kB = fMaterialB->GetExtinctionCoefficient(lambda);
    return kA * fFractionA + kB * fFractionB;
  }
  void SetFraction(Double_t fractionA, Double_t fractionB) {
    fFractionA = fractionA / (fractionA + fractionB);
    fFractionB = fractionB / (fractionA + fractionB);
  }

  ClassDef(AMixedRefractiveIndex, 1)
};

#endif  // A_MIXED_REFRACTIVE_INDEX_H
