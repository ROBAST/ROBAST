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
  std::complex<Double_t> InterfaceR(EPolarization polarization,
                                    std::complex<Double_t> n_i,
                                    std::complex<Double_t> n_f,
                                    std::complex<Double_t> th_i,
                                    std::complex<Double_t> th_f) const;
  std::complex<Double_t> InterfaceT(EPolarization polarization,
                                    std::complex<Double_t> n_i,
                                    std::complex<Double_t> n_f,
                                    std::complex<Double_t> th_i,
                                    std::complex<Double_t> th_f) const;
 public:
  AMultilayer(std::shared_ptr<ARefractiveIndex> top,
              std::shared_ptr<ARefractiveIndex> bottom);
  virtual ~AMultilayer();

  void InsertLayer(std::shared_ptr<ARefractiveIndex> idx, Double_t thickness);

  void CoherentTMM(EPolarization polarization, std::complex<Double_t> th_0,
                   Double_t lam_vac, Double_t& reflectance,
                   Double_t& transmittance) const;
  void PrintLayers(Double_t lambda) const;

  ClassDef(AMultilayer, 1)
};

#endif  // A_MULTILAYER_H
