/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AMultilayer
//
///////////////////////////////////////////////////////////////////////////////

#include "A2x2ComplexMatrix.h"
#include "AMultilayer.h"
#include "AOpticsManager.h"

#include <complex>
#include <iostream>

static const Double_t EPSILON = std::numeric_limits<double>::epsilon();
static const Double_t inf = std::numeric_limits<Double_t>::infinity();

ClassImp(AMultilayer);

AMultilayer::AMultilayer(std::shared_ptr<ARefractiveIndex> top,
                         std::shared_ptr<ARefractiveIndex> bottom)
{
  fRefractiveIndexList.push_back(bottom);
  fThicknessList.push_back(inf);
  InsertLayer(top, inf);
}

//______________________________________________________________________________
AMultilayer::~AMultilayer() {
}

//______________________________________________________________________________
Bool_t AMultilayer::IsForwardAngle(std::complex<Double_t> n, std::complex<Double_t> theta) const
{
  // Copied from tmm.is_forward_angle

  // if a wave is traveling at angle theta from normal in a medium with index n,
  // calculate whether or not this is the forward-traveling wave (i.e., the one
  // going from front to back of the stack, like the incoming or outgoing waves,
  // but unlike the reflected wave). For real n & theta, the criterion is simply
  // -pi/2 < theta < pi/2, but for complex n & theta, it's more complicated.
  // See https://arxiv.org/abs/1603.02720 appendix D. If theta is the forward
  // angle, then (pi-theta) is the backward angle and vice-versa.

  if(n.real() * n.imag() < 0){
    Error("IsForwardAngle",
          "For materials with gain, it's ambiguous which "
          "beam is incoming vs outgoing. See "
          "https://arxiv.org/abs/1603.02720 Appendix C.\n"
          "n: %.3e + %.3ei   angle: %.3e + %.3ei", n.real(), n.imag(),
          theta.real(), theta.imag());
  }
  auto ncostheta = n * std::cos(theta);
  Bool_t answer;
  if (std::abs(ncostheta.imag()) > 100 * EPSILON) {
    // Either evanescent decay or lossy medium. Either way, the one that
    // decays is the forward-moving wave      
    answer = ncostheta.imag() > 0;
  } else {
    // Forward is the one with positive Poynting vector
    // Poynting vector is Re[n cos(theta)] for s-polarization or
    // Re[n cos(theta*)] for p-polarization, but it turns out they're consistent
    // so I'll just assume s then check both below
    answer = ncostheta.real() > 0;
  }
  // double-check the answer ... can't be too careful!
  std::string
    error_string(Form("It's not clear which beam is incoming vs outgoing. Weird"
                      " index maybe?\n"
                      "n: %.3e + %.3ei   angle: %.3e + %.3ei",
                      n.real(), n.imag(), theta.real(), theta.imag()));
  if (answer == true) {
    if (ncostheta.imag() <= -100 * EPSILON) Error("IsForwardAngle", "%s", error_string.c_str());
    if (ncostheta.real() <= -100 * EPSILON) Error("IsForwardAngle", "%s", error_string.c_str());
    if ((n * std::cos(std::conj(theta))).real() <= -100 * EPSILON) Error("IsForwardAngle", "%s", error_string.c_str());
  } else {
    if(ncostheta.imag() >= 100 * EPSILON) Error("IsForwardAngle", "%s", error_string.c_str());
    if(ncostheta.real() >= 100 * EPSILON) Error("IsForwardAngle", "%s", error_string.c_str());
    if ((n * std::cos(std::conj(theta))).real() < 100 * EPSILON) Error("IsForwardAngle", "%s", error_string.c_str());
  }
  return answer;
}

//______________________________________________________________________________
void AMultilayer::ListSnell(std::complex<Double_t> th_0,
                            const std::vector<std::complex<Double_t>>& n_list,
                            std::vector<std::complex<Double_t>>& th_list) const
{
  // Copied from tmm.list_snell
  
  // return list of angle theta in each layer based on angle th_0 in layer 0,
  // using Snell's law. n_list is index of refraction of each layer. Note that
  // "angles" may be complex!!

  auto num_layers = fRefractiveIndexList.size();
  for(std::size_t i = 0; i < num_layers; ++i) {
    auto angle_i = std::asin(n_list[0] * std::sin(th_0) / n_list[i]);
    th_list.push_back(angle_i);
  }

  // The first and last entry need to be the forward angle (the intermediate
  // layers don't matter, see https://arxiv.org/abs/1603.02720 Section 5)
  if (!IsForwardAngle(n_list[0], th_list[0])) {
    th_list.front() = TMath::Pi() - th_list.front();
  }
  if (!IsForwardAngle(n_list.back(), th_list.back())) {
    th_list.back() = TMath::Pi() - th_list.back();
  }
}

//______________________________________________________________________________
void AMultilayer::InsertLayer(std::shared_ptr<ARefractiveIndex> idx, Double_t thickness)
{
  // ----------------- Top layer
  // ----------------- 1st layer
  // ----------------- 2nd layer
  // ...
  // ----------------- <------------- Insert a new layer here
  // ----------------- Bottom layer 
  fRefractiveIndexList.insert(fRefractiveIndexList.end() - 1, idx);
  fThicknessList.insert(fThicknessList.end() - 1, thickness);
}

//______________________________________________________________________________
void AMultilayer::CoherentTMM(EPolarization polarization, std::complex<Double_t> th_0,
                              Double_t lam_vac,
                              Double_t& reflectance,
                              Double_t& transmittance) const
{
  // Copied from tmm.ch_tmm
  
  // Main "coherent transfer matrix method" calc. Given parameters of a stack,
  // calculates everything you could ever want to know about how light
  // propagates in it. (If performance is an issue, you can delete some of the
  // calculations without affecting the rest.)
  //
  // pol is light polarization, "s" or "p".
  //
  // n_list is the list of refractive indices, in the order that the light would
  // pass through them. The 0'th element of the list should be the semi-infinite
  // medium from which the light enters, the last element should be the semi-
  // infinite medium to which the light exits (if any exits).
  //
  // th_0 is the angle of incidence: 0 for normal, pi/2 for glancing.
  // Remember, for a dissipative incoming medium (n_list[0] is not real), th_0
  // should be complex so that n0 sin(th0) is real (intensity is constant as
  // a function of lateral position).
  //
  // d_list is the list of layer thicknesses (front to back). Should correspond
  // one-to-one with elements of n_list. First and last elements should be "inf".
  //
  // lam_vac is vacuum wavelength of the light.

  auto num_layers = fRefractiveIndexList.size();
  std::vector<std::complex<Double_t>> n_list(num_layers);

  {
    auto n_i = n_list.begin();
    auto ref_i = fRefractiveIndexList.cbegin();
    for (std::size_t i = 0; i < num_layers; ++i) {
      *n_i = (*ref_i)->GetComplexRefractiveIndex(lam_vac);
      ++n_i;
      ++ref_i;
    }
  }
                 
  // Input tests
  if(std::abs((n_list[0] * std::sin(th_0)).imag()) >= 100 * EPSILON ||
     ! IsForwardAngle(n_list[0], th_0)) {
    Error("CoherentTMM", "Error in n0 or th0!");
  }

  // th_list is a list with, for each layer, the angle that the light travels
  // through the layer. Computed with Snell's law. Note that the "angles" may be
  // complex!
  std::vector<std::complex<Double_t>> th_list;
  ListSnell(th_0, n_list, th_list);

  // kz is the z-component of (complex) angular wavevector for forward-moving
  // wave. Positive imaginary part means decaying.
  std::vector<std::complex<Double_t>> kz_list(num_layers);
  std::vector<std::complex<Double_t>> cos_th_list(num_layers);
  {
    auto kz_i = kz_list.begin();
    auto n_i = n_list.cbegin();
    auto th_i = th_list.cbegin();
    auto cos_th_i = cos_th_list.begin();
    for(std::size_t i = 0; i < num_layers; ++i) {
      *cos_th_i = std::cos(*th_i);
      *kz_i = TMath::TwoPi() * (*n_i) * (*cos_th_i) / lam_vac;
      ++kz_i;
      ++n_i;
      ++th_i;
      ++cos_th_i;
    }
  }

  // delta is the total phase accrued by traveling through a given layer.
  std::vector<std::complex<Double_t>> delta(num_layers);
  {
    auto delta_i = delta.begin();
    auto kz_i = kz_list.cbegin();
    auto thickness_i = fThicknessList.cbegin();
    for(std::size_t i = 0; i < num_layers; ++i) {
      *delta_i = (*kz_i) * (*thickness_i);
      ++delta_i;
      ++kz_i;
      ++thickness_i;
    }
  }

  // For a very opaque layer, reset delta to avoid divide-by-0 and similar
  // errors. The criterion imag(delta) > 35 corresponds to single-pass
  // transmission < 1e-30 --- small enough that the exact value doesn't
  // matter.
  static Bool_t opacity_warning = kFALSE;
  for (std::size_t i = 1; i < num_layers - 1; ++i) {
    if (delta[i].imag() > 35) {
      delta[i] = delta[i].real() + std::complex<Double_t>(0, 35);
      if (opacity_warning == kFALSE) {
        opacity_warning = kTRUE;
        Error("CoherentTMM",
              "Warning: Layers that are almost perfectly opaque "
              "are modified to be slightly transmissive, "
              "allowing 1 photon in 10^30 to pass through. It's "
              "for numerical stability. This warning will not "
              "be shown again.");
      }
    }
  }
  
  // t_list[i,j] and r_list[i,j] are transmission and reflection amplitudes,
  // respectively, coming from i, going to j. Only need to calculate this when
  // j=i+1. (2D array is overkill but helps avoid confusion.)
  std::vector<std::complex<Double_t>> t_list(num_layers);
  std::vector<std::complex<Double_t>> r_list(num_layers);

  {
    auto t_i = t_list.begin();
    auto r_i = r_list.begin();
    auto th_i = th_list.cbegin();
    auto th_f = th_list.cbegin(); ++th_f; // increment to access th_f (th[i + 1])
    auto n_i = n_list.cbegin();
    auto n_f = n_list.cbegin(); ++n_f; // increment to access n_f (n[i + 1])
    auto cos_th_i = cos_th_list.cbegin(); // cos(th_i)
    auto cos_th_f = cos_th_list.cbegin(); ++cos_th_f; // increment to access cos(th_f)
    for(std::size_t i = 0; i < num_layers - 1; ++i) {
      auto ii = *n_i * (*cos_th_i);
      if (polarization == kS) {
        auto ff = *n_f * (*cos_th_f);
        *t_i = 2. * ii / (ii + ff);
        *r_i = (ii - ff) / (ii + ff);
      } else {
        auto fi = *n_f * (*cos_th_i);
        auto if_ = *n_i * (*cos_th_f);
        *t_i = 2. * ii / (fi + if_);
        *r_i = (fi - if_) / (fi + if_);
      }
      ++t_i;
      ++r_i;
      ++th_i;
      ++th_f;
      ++n_i;
      ++n_f;
      ++cos_th_i;
      ++cos_th_f;
    }
  }

  // At the interface between the (n-1)st and nth material, let v_n be the
  // amplitude of the wave on the nth side heading forwards (away from the
  // boundary), and let w_n be the amplitude on the nth side heading backwards
  // (towards the boundary). Then (v_n,w_n) = M_n (v_{n+1},w_{n+1}). M_n is
  // M_list[n]. M_0 and M_{num_layers-1} are not defined.
  // My M is a bit different than Sernelius's, but Mtilde is the same.

  // M_list[0] and M_list[-1] are filled with (0, 0, 0, 0) by default
  std::vector<A2x2ComplexMatrix> M_list(num_layers);
  {
    auto M_i = M_list.begin(); ++M_i; // start i from 1
    auto t_i = t_list.cbegin(); ++t_i;
    auto r_i = r_list.cbegin(); ++r_i;
    auto delta_i = delta.cbegin(); ++delta_i;
    for (std::size_t i = 1; i < num_layers - 1; ++i) {
      static const std::complex<Double_t> j(0, 1);
      auto j_delta_i = j * (*delta_i);
      *M_i = 1. / (*t_i) * A2x2ComplexMatrix(std::exp(- j_delta_i), 0, 0,
                                             std::exp(j_delta_i)) *
        A2x2ComplexMatrix(1, *r_i, *r_i, 1);
      //std::cerr << i << "\t" << M_list[i].Get00() << "\t" << M_list[i].Get00() << std::endl;
      //std::cerr << "\t" << M_list[i].Get10() << "\t" << M_list[i].Get11() << std::endl;
      ++M_i;
      ++t_i;
      ++r_i;
      ++delta_i;
    }
  }

  A2x2ComplexMatrix Mtilde(1, 0, 0, 1);
  {
    auto M_i = M_list.cbegin(); ++M_i; // start i from 1
    for (std::size_t i = 1; i < num_layers - 1; ++i) {
      Mtilde = Mtilde * (*M_i);
      ++M_i;
    }
  }

  Mtilde = A2x2ComplexMatrix(1, r_list.front(), r_list.front(), 1) / t_list.front() * Mtilde;

  // Net complex transmission and reflection amplitudes
  auto r = Mtilde.Get10() / Mtilde.Get00();
  auto t = 1. / Mtilde.Get00();

  // vw_list[n] = [v_n, w_n]. v_0 and w_0 are undefined because the 0th medium
  // has no left interface.
  /* Will not be used at the moment (A.O.)
  std::vector<std::array<std::complex<Double_t>, 2>> vw_list(num_layers);
  
  std::complex<Double_t> vw[2] = {t, 0.};
  vw_list.back()[0] = vw[0];
  vw_list.back()[1] = vw[1];

  for(std::size_t i = num_layers - 2; i > 0; --i) {
    auto v = M_list[i].Get00() * vw[0] + M_list[i].Get01() * vw[1];
    auto w = M_list[i].Get10() * vw[0] + M_list[i].Get11() * vw[1];
    vw[0] = v;
    vw[1] = w;
    vw_list[i][0] = vw[0];
    vw_list[i][1] = vw[1];
  }
  */

  // Net transmitted and reflected power, as a proportion of the incoming light
  // power.
  reflectance = std::abs(r) * std::abs(r);
  auto n_i = n_list.front();
  auto n_f = n_list.back();
  auto th_i = th_0;
  auto th_f = th_list.back();
  if (polarization == kS) {
    transmittance = std::abs(t * t) * (((n_f * std::cos(th_f)).real())
                                       / (n_i * std::cos(th_i)).real());
  } else {
    transmittance = std::abs(t * t) * (((n_f * std::conj(std::cos(th_f))).real())
                                       / (n_i * std::conj(std::cos(th_i))).real());
  }
}

//__________________________________________________________________________________
void AMultilayer::PrintLayers(Double_t lambda) const
{
  auto n = fRefractiveIndexList.size();
  for(std::size_t i = 0; i < n; ++i){
    std::cout << "----------------------------------------\n";
    std::cout << i << "\tn_i = " << fRefractiveIndexList[i]->GetComplexRefractiveIndex(lambda) << "\td_i = " << fThicknessList[i] / AOpticsManager::nm() << " (nm)\n";
  }
  std::cout << "----------------------------------------" << std::endl;
}

