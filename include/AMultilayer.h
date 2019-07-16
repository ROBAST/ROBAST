// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_MULTILAYER_H
#define A_MULTILAYER_H

#include <memory>

#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// AMultilayer
//
///////////////////////////////////////////////////////////////////////////////

class AMultilayer : public TObject {
public:
  enum EPolarization {kS, kP};
  
private:
  std::vector<std::shared_ptr<ARefractiveIndex>> fRefractiveIndexList;
  std::vector<Double_t> fThicknessList;

  Bool_t IsForwardAngle(std::complex<Double_t> n, std::complex<Double_t> theta) const;
  void ListSnell(std::complex<Double_t> th_0,
                 const std::vector<std::complex<Double_t>>& n_list,
                 std::vector<std::complex<Double_t>>& th_list) const;

 public:
  AMultilayer(std::shared_ptr<ARefractiveIndex> top,
              std::shared_ptr<ARefractiveIndex> bottom);
  virtual ~AMultilayer();

  void InsertLayer(std::shared_ptr<ARefractiveIndex> idx, Double_t thickness);
  void ChangeThickness(std::size_t i, Double_t thickness) {
    if (i < 1 || i > fThicknessList.size() - 2) {
      Error("ChangeThickness", "Cannot change the thickness of the %luth layer", i);
    } else {
      fThicknessList[i] = thickness;
    }
  }

  void CoherentTMM(EPolarization polarization, std::complex<Double_t> th_0,
                   Double_t lam_vac, Double_t& reflectance,
                   Double_t& transmittance) const;
  void PrintLayers(Double_t lambda) const;

  ClassDef(AMultilayer, 1)
};

#endif  // A_MULTILAYER_H
