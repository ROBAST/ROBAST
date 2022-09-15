// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_MULTILAYER_H
#define A_MULTILAYER_H

#include <memory>
#include <thread>

#include <TH2.h>

#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// AMultilayer
//
///////////////////////////////////////////////////////////////////////////////

class AMultilayer : public TObject {
 public:
  enum EPolarization { kS, kP };

 private:
  std::vector<std::shared_ptr<ARefractiveIndex>> fRefractiveIndexList;
  std::vector<Double_t> fThicknessList;
  std::vector<Double_t> fCoherentList;
  std::size_t fNthreads;
  std::shared_ptr<TH2D> fPreCalculatedReflectanceMixed;
  std::shared_ptr<TH2D> fPreCalculatedTransmittanceMixed;

  Bool_t IsForwardAngle(std::complex<Double_t> n,
                        std::complex<Double_t> theta) const;
  void ListSnell(std::complex<Double_t> th_0,
                 const std::vector<std::complex<Double_t>>& n_list,
                 std::vector<std::complex<Double_t>>& th_list) const;
  void CoherentTMMMixedMultiAngle(
      std::vector<std::complex<Double_t>>::const_iterator th_0_cbegin,
      std::vector<std::complex<Double_t>>::const_iterator th_0_cend,
      Double_t lam_vac, std::vector<Double_t>::iterator reflectance_it,
      std::vector<Double_t>::iterator transmittance_it) {
    for (auto cit = th_0_cbegin; cit != th_0_cend; ++cit) {
      Double_t r, t;
      CoherentTMMMixed(*cit, lam_vac, r, t);
      *reflectance_it = r;
      *transmittance_it = t;
      ++reflectance_it;
      ++transmittance_it;
    }
  }
  void CoherentTMMMixedMultiWavelength(
      std::complex<Double_t> th_0,
      std::vector<Double_t>::const_iterator lam_vac_cbegin,
      std::vector<Double_t>::const_iterator lam_vac_cend,
      std::vector<Double_t>::iterator reflectance_it,
      std::vector<Double_t>::iterator transmittance_it) {
    for (auto cit = lam_vac_cbegin; cit != lam_vac_cend; ++cit) {
      Double_t r, t;
      CoherentTMMMixed(th_0, *cit, r, t);
      *reflectance_it = r;
      *transmittance_it = t;
      ++reflectance_it;
      ++transmittance_it;
    }
  }
  void IncGroupLayers(std::vector<std::vector<Double_t>>& stack_d_list,
                      std::vector<std::vector<std::shared_ptr<ARefractiveIndex>>>& stack_n_list,
                      std::vector<std::size_t>& all_from_inc,
                      std::vector<std::size_t>& inc_from_all,
                      std::vector<std::vector<std::size_t>>& all_from_stack,
                      std::vector<std::vector<std::size_t>>& stack_from_all,
                      std::vector<std::size_t>& inc_from_stack,
                      std::vector<std::size_t>& stack_from_inc
                      ) const;

 public:
  AMultilayer(std::shared_ptr<ARefractiveIndex> top,
              std::shared_ptr<ARefractiveIndex> bottom);
  virtual ~AMultilayer();

  void AddLayer(std::shared_ptr<ARefractiveIndex> idx, Double_t thickness, Bool_t coherent = kTRUE);
  void InsertLayer(std::shared_ptr<ARefractiveIndex> idx, Double_t thickness, Bool_t coherent = kTRUE);
  void ChangeThickness(std::size_t i, Double_t thickness) {
    if (i < 1 || i > fThicknessList.size() - 2) {
      Error("ChangeThickness", "Cannot change the thickness of the %luth layer",
            i);
    } else {
      fThicknessList[i] = thickness;
    }
  }

  std::complex<Double_t> Snell(std::complex<Double_t> n1,
                               std::complex<Double_t> n2,
                               std::complex<Double_t> th) const {
    auto th_2_guess = std::asin(n1 * std::sin(th) / n2);
    if (IsForwardAngle(n2, th_2_guess)) {
      return th_2_guess;
    } else {
      return TMath::Pi() - th_2_guess;
    }
  }
                                      
  void CoherentTMM(EPolarization polarization, std::complex<Double_t> th_0,
                   Double_t lam_vac, Double_t& reflectance,
                   Double_t& transmittance, Bool_t reverse = kFALSE) const;
  void IncoherentTMM(EPolarization polarization,
                     std::complex<Double_t> th_0, Double_t lam_vac,
                     Double_t& reflectance,
                     Double_t& transmittance) const;

  void CoherentTMMMixed(std::complex<Double_t> th_0, Double_t lam_vac,
                        Double_t& reflectance, Double_t& transmittance) const {
    if(fPreCalculatedReflectanceMixed and fPreCalculatedTransmittanceMixed) {
      reflectance = fPreCalculatedReflectanceMixed->Interpolate(lam_vac, th_0.real());
      transmittance = fPreCalculatedTransmittanceMixed->Interpolate(lam_vac, th_0.real());
      return;
    }
    Double_t r = 0;
    Double_t t = 0;
    CoherentTMMP(th_0, lam_vac, reflectance, transmittance);
    r += reflectance;
    t += transmittance;
    CoherentTMMS(th_0, lam_vac, reflectance, transmittance);
    r += reflectance;
    t += transmittance;

    reflectance = r / 2.;
    transmittance = t / 2.;
  }
  void IncoherentTMMMixed(std::complex<Double_t> th_0, Double_t lam_vac,
                        Double_t& reflectance, Double_t& transmittance) const {
    if(fPreCalculatedReflectanceMixed and fPreCalculatedTransmittanceMixed) {
      reflectance = fPreCalculatedReflectanceMixed->Interpolate(lam_vac, th_0.real());
      transmittance = fPreCalculatedTransmittanceMixed->Interpolate(lam_vac, th_0.real());
      return;
    }
    Double_t r = 0;
    Double_t t = 0;
    IncoherentTMMP(th_0, lam_vac, reflectance, transmittance);
    r += reflectance;
    t += transmittance;
    IncoherentTMMS(th_0, lam_vac, reflectance, transmittance);
    r += reflectance;
    t += transmittance;

    reflectance = r / 2.;
    transmittance = t / 2.;
  }
  void CoherentTMMMixed(std::vector<std::complex<Double_t>>& th_0,
                        Double_t lam_vac, std::vector<Double_t>& reflectance,
                        std::vector<Double_t>& transmittance) const {
    auto n = th_0.size();
    reflectance.resize(n);
    transmittance.resize(n);
    if(fPreCalculatedReflectanceMixed and fPreCalculatedTransmittanceMixed) {
      for(std::size_t i = 0; i < n; ++i){
        auto th = th_0[i].real();
        reflectance[i] =  fPreCalculatedReflectanceMixed->Interpolate(lam_vac, th);
        transmittance[i] = fPreCalculatedTransmittanceMixed->Interpolate(lam_vac, th);
      }
      return;
    }

    std::vector<std::thread> threads(fNthreads);

    auto th_0_cbegin = th_0.cbegin();
    auto th_0_cend = th_0.end();
    auto reflectance_begin = reflectance.begin();
    auto transmittance_begin = transmittance.begin();

    auto step = n / fNthreads;

    for (std::size_t i = 0; i < fNthreads; ++i) {
      if (i == fNthreads - 1) {
        threads[i] =
            std::thread(&AMultilayer::CoherentTMMMixedMultiAngle, *this,
                        th_0_cbegin, th_0_cbegin + step, lam_vac,
                        reflectance_begin, transmittance_begin);
        th_0_cbegin += step;
      } else {
        threads[i] = std::thread(&AMultilayer::CoherentTMMMixedMultiAngle,
                                 *this, th_0_cbegin, th_0_cend, lam_vac,
                                 reflectance_begin, transmittance_begin);
      }
    }
    for (std::size_t i = 0; i < fNthreads; ++i) {
      threads[i].join();
    }
  }
  void CoherentTMMMixed(std::complex<Double_t> th_0,
                        std::vector<Double_t>& lam_vac,
                        std::vector<Double_t>& reflectance,
                        std::vector<Double_t>& transmittance) const {
    auto n = lam_vac.size();
    reflectance.resize(n);
    transmittance.resize(n);

    std::vector<std::thread> threads(fNthreads);

    auto lam_vac_cbegin = lam_vac.cbegin();
    auto lam_vac_cend = lam_vac.end();
    auto reflectance_begin = reflectance.begin();
    auto transmittance_begin = transmittance.begin();

    auto step = n / fNthreads;

    for (std::size_t i = 0; i < fNthreads; ++i) {
      if (i == fNthreads - 1) {
        threads[i] =
            std::thread(&AMultilayer::CoherentTMMMixedMultiWavelength, *this,
                        th_0, lam_vac_cbegin, lam_vac_cbegin + step,
                        reflectance_begin, transmittance_begin);
        lam_vac_cbegin += step;
      } else {
        threads[i] = std::thread(&AMultilayer::CoherentTMMMixedMultiWavelength,
                                 *this, th_0, lam_vac_cbegin, lam_vac_cend,
                                 reflectance_begin, transmittance_begin);
      }
    }
    for (std::size_t i = 0; i < fNthreads; ++i) {
      threads[i].join();
    }
  }
  void CoherentTMMP(std::complex<Double_t> th_0, Double_t lam_vac,
                    Double_t& reflectance, Double_t& transmittance) const {
    CoherentTMM(kP, th_0, lam_vac, reflectance, transmittance);
  }
  void CoherentTMMS(std::complex<Double_t> th_0, Double_t lam_vac,
                    Double_t& reflectance, Double_t& transmittance) const {
    CoherentTMM(kS, th_0, lam_vac, reflectance, transmittance);
  }
  void IncoherentTMMP(std::complex<Double_t> th_0, Double_t lam_vac,
                    Double_t& reflectance, Double_t& transmittance) const {
    IncoherentTMM(kP, th_0, lam_vac, reflectance, transmittance);
  }
  void IncoherentTMMS(std::complex<Double_t> th_0, Double_t lam_vac,
                    Double_t& reflectance, Double_t& transmittance) const {
    IncoherentTMM(kS, th_0, lam_vac, reflectance, transmittance);
  }
  void PreCalculateCoherentTMM(Int_t lam_nbins, Double_t lam_min, Double_t lam_max,
                               Int_t th_nbins, Double_t th_min, Double_t th_max) {
    // make temporary objects because CoherentTMMMixed checks if
    // fPreCalculstedXXX are null or not
    auto preReflectanceMixed = std::make_shared<TH2D>("", "", lam_nbins, lam_min, lam_max, th_nbins, th_min, th_max);
    auto preTransmittanceMixed = std::make_shared<TH2D>("", "", lam_nbins, lam_min, lam_max, th_nbins, th_min, th_max);
    for (Int_t j = 1; j <= th_nbins; ++j) {
      Double_t th = preReflectanceMixed->GetYaxis()->GetBinCenter(j);
      for (Int_t i = 1; i <= lam_nbins; ++i) {
        Double_t lam = preReflectanceMixed->GetXaxis()->GetBinCenter(i);
        Double_t reflectance, transmittance;
        CoherentTMMMixed(th, lam, reflectance, transmittance);
        preReflectanceMixed->SetBinContent(i, j, reflectance);
        preTransmittanceMixed->SetBinContent(i, j, transmittance);
      }
    }

    fPreCalculatedReflectanceMixed = preReflectanceMixed;
    fPreCalculatedTransmittanceMixed = preTransmittanceMixed;
  }
  void PreCalculateIncoherentTMM(Int_t lam_nbins, Double_t lam_min, Double_t lam_max,
                                 Int_t th_nbins, Double_t th_min, Double_t th_max) {
    // make temporary objects because CoherentTMMMixed checks if
    // fPreCalculstedXXX are null or not
    auto preReflectanceMixed = std::make_shared<TH2D>("", "", lam_nbins, lam_min, lam_max, th_nbins, th_min, th_max);
    auto preTransmittanceMixed = std::make_shared<TH2D>("", "", lam_nbins, lam_min, lam_max, th_nbins, th_min, th_max);
    for (Int_t j = 1; j <= th_nbins; ++j) {
      Double_t th = preReflectanceMixed->GetYaxis()->GetBinCenter(j);
      for (Int_t i = 1; i <= lam_nbins; ++i) {
        Double_t lam = preReflectanceMixed->GetXaxis()->GetBinCenter(i);
        Double_t reflectance, transmittance;
        IncoherentTMMMixed(th, lam, reflectance, transmittance);
        preReflectanceMixed->SetBinContent(i, j, reflectance);
        preTransmittanceMixed->SetBinContent(i, j, transmittance);
      }
    }

    fPreCalculatedReflectanceMixed = preReflectanceMixed;
    fPreCalculatedTransmittanceMixed = preTransmittanceMixed;
  }
  const std::shared_ptr<const TH2D> GetPrecalculatedReflectanceMixed() const {
    return fPreCalculatedReflectanceMixed;
  }
  const std::shared_ptr<const TH2D> GetPrecalculatedTransmittanceMixed() const {
    return fPreCalculatedTransmittanceMixed;
  }
  Double_t GetThickness(std::size_t i) const {
    if (i >= fThicknessList.size()) {
      Error("GetThickness", "Index out of range");
      return 0;
    }
    return fThicknessList[i];
  }
  void PrintLayers(Double_t lambda) const;
  TGraph* MakeIndexGraph(Double_t lambda, std::size_t stack_index = 0) const;
  void SetNthreads(std::size_t n);

  ClassDef(AMultilayer, 1)
};

#endif  // A_MULTILAYER_H
