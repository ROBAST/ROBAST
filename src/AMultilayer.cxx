/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AMultilayer
//
///////////////////////////////////////////////////////////////////////////////

#include "TMatrixDSym.h"

#include "AMultilayer.h"
#include "A2x2ComplexMatrix.h"
#include "AOpticsManager.h"

#include <complex>
#include <iostream>

static const Double_t EPSILON = std::numeric_limits<double>::epsilon();
static const Double_t inf = std::numeric_limits<Double_t>::infinity();

ClassImp(AMultilayer);

void interface_rt(AMultilayer::EPolarization polarization,
                  std::complex<Double_t> n_i,
                  std::complex<Double_t> n_f,
                  std::complex<Double_t> th_i,
                  std::complex<Double_t> th_f,
                  std::complex<Double_t>& r,
                  std::complex<Double_t>& t) {
  /*
  reflection amplitude (from Fresnel equations)
  transmission amplitude (frem Fresnel equations)
   
  polarization is either "s" or "p" for polarization
    
  n_i, n_f are (complex) refractive index for incident and final

  th_i, th_f are (complex) propegation angle for incident and final
  (in radians, where 0=normal). "th" stands for "theta".
  */
  auto ii = n_i * std::cos(th_i);
  if (polarization == AMultilayer::kS) {
    auto ff = n_f * std::cos(th_f);
    r = (ii - ff) / (ii + ff);
    t = 2. * ii / (ii + ff);
  } else {

    auto fi = n_f * std::cos(th_i);
    auto _if = n_i * std::cos(th_f);
    r = (fi - _if) / (fi + _if);
    t = 2. * ii / (fi + _if);
  }
}

Double_t R_from_r(std::complex<Double_t> r) {
  /*
  Calculate reflected power R, starting with reflection amplitude r.
  */
  auto absr = std::abs(r);
  return absr * absr;
}
      
Double_t T_from_t(AMultilayer::EPolarization pol,
                  std::complex<Double_t> t,
                  std::complex<Double_t> n_i,
                  std::complex<Double_t> n_f,
                  std::complex<Double_t> th_i,
                  std::complex<Double_t> th_f) {
  /*
  Calculate transmitted power T, starting with transmission amplitude t.

  n_i,n_f are refractive indices of incident and final medium.

  th_i, th_f are (complex) propegation angles through incident & final medium
  (in radians, where 0=normal). "th" stands for "theta".

  In the case that n_i,n_f,th_i,th_f are real, formulas simplify to
  T=|t|^2 * (n_f cos(th_f)) / (n_i cos(th_i)).

  See manual for discussion of formulas
  */
  if (pol == AMultilayer::kS) {
    return std::abs(t*t) * (((n_f * std::cos(th_f)).real()) / (n_i * std::cos(th_i)).real());
  } else {
    return std::abs(t*t) * (((n_f * std::conj(std::cos(th_f))).real()) /
                            (n_i * std::conj(std::cos(th_i))).real());
  }
}

void interface_RT(AMultilayer::EPolarization polarization,
                  std::complex<Double_t> n_i,
                  std::complex<Double_t> n_f,
                  std::complex<Double_t> th_i,
                  std::complex<Double_t> th_f,
                  Double_t& R,
                  Double_t& T) {
  std::complex<Double_t>r, t;
  interface_rt(polarization, n_i, n_f, th_i, th_f, r, t);
  R = R_from_r(r);
  T = T_from_t(polarization, t, n_i, n_f, th_i, th_f);
}

AMultilayer::AMultilayer(std::shared_ptr<ARefractiveIndex> top,
                         std::shared_ptr<ARefractiveIndex> bottom)
    : fNthreads(1) {
  fRefractiveIndexList.push_back(bottom);
  fThicknessList.push_back(inf);
  Bool_t coherent = kFALSE;
  fCoherentList.push_back(coherent); // the inf-thick layers are incoherent
  InsertLayer(top, inf, coherent);
}

//______________________________________________________________________________
AMultilayer::~AMultilayer() {}

//______________________________________________________________________________
Bool_t AMultilayer::IsForwardAngle(std::complex<Double_t> n,
                                   std::complex<Double_t> theta) const {
  // Copied from tmm.is_forward_angle

  // if a wave is traveling at angle theta from normal in a medium with index n,
  // calculate whether or not this is the forward-traveling wave (i.e., the one
  // going from front to back of the stack, like the incoming or outgoing waves,
  // but unlike the reflected wave). For real n & theta, the criterion is simply
  // -pi/2 < theta < pi/2, but for complex n & theta, it's more complicated.
  // See https://arxiv.org/abs/1603.02720 appendix D. If theta is the forward
  // angle, then (pi-theta) is the backward angle and vice-versa.

  if (n.real() * n.imag() < 0) {
    Error("IsForwardAngle",
          "For materials with gain, it's ambiguous which "
          "beam is incoming vs outgoing. See "
          "https://arxiv.org/abs/1603.02720 Appendix C.\n"
          "n: %.3e + %.3ei   angle: %.3e + %.3ei",
          n.real(), n.imag(), theta.real(), theta.imag());
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
  std::string error_string(
      Form("It's not clear which beam is incoming vs outgoing. Weird"
           " index maybe?\n"
           "n: %.3e + %.3ei   angle: %.3e + %.3ei",
           n.real(), n.imag(), theta.real(), theta.imag()));
  if (answer == true) {
    if (ncostheta.imag() <= -100 * EPSILON)
      Error("IsForwardAngle", "%s", error_string.c_str());
    if (ncostheta.real() <= -100 * EPSILON)
      Error("IsForwardAngle", "%s", error_string.c_str());
    if ((n * std::cos(std::conj(theta))).real() <= -100 * EPSILON)
      Error("IsForwardAngle", "%s", error_string.c_str());
  } else {
    if (ncostheta.imag() >= 100 * EPSILON)
      Error("IsForwardAngle", "%s", error_string.c_str());
    if (ncostheta.real() >= 100 * EPSILON)
      Error("IsForwardAngle", "%s", error_string.c_str());
    if ((n * std::cos(std::conj(theta))).real() < 100 * EPSILON)
      Error("IsForwardAngle", "%s", error_string.c_str());
  }
  return answer;
}

//______________________________________________________________________________
void AMultilayer::ListSnell(
    std::complex<Double_t> th_0,
    const std::vector<std::complex<Double_t>>& n_list,
    std::vector<std::complex<Double_t>>& th_list) const {
  // Copied from tmm.list_snell

  // return list of angle theta in each layer based on angle th_0 in layer 0,
  // using Snell's law. n_list is index of refraction of each layer. Note that
  // "angles" may be complex!!

  auto num_layers = fRefractiveIndexList.size();
  th_list.resize(num_layers);
  {
    auto n_i = n_list.cbegin();
    auto th_i = th_list.begin();
    auto n0_sinth0 = (*n_i) * std::sin(th_0);
    for (std::size_t i = 0; i < num_layers; ++i) {
      *th_i = std::asin(n0_sinth0 / *n_i);
      ++n_i;
      ++th_i;
    }
  }

  // The first and last entry need to be the forward angle (the intermediate
  // layers don't matter, see https://arxiv.org/abs/1603.02720 Section 5)
  if (!IsForwardAngle(n_list[0], th_list[0])) {
    th_list[0] = TMath::Pi() - th_list[0];
  }
  if (!IsForwardAngle(n_list.back(), th_list.back())) {
    th_list.back() = TMath::Pi() - th_list.back();
  }
}

//______________________________________________________________________________
void AMultilayer::AddLayer(std::shared_ptr<ARefractiveIndex> idx,
                              Double_t thickness, Bool_t coherent) {
  // ----------------- Top inf material
  // ----------------- <------------- Add a new layer here
  // ----------------- 1st layer
  // ----------------- 2nd layer
  // ...
  // ----------------- Bottom inf material
  fRefractiveIndexList.insert(fRefractiveIndexList.begin() + 1, idx);
  fThicknessList.insert(fThicknessList.begin() + 1, thickness);
  fCoherentList.insert(fCoherentList.begin() + 1, coherent);
}

//______________________________________________________________________________
void AMultilayer::InsertLayer(std::shared_ptr<ARefractiveIndex> idx,
                              Double_t thickness, Bool_t coherent) {
  // ----------------- Top inf material
  // ----------------- 1st layer
  // ----------------- 2nd layer
  // ...
  // ----------------- <------------- Insert a new layer here
  // ----------------- Bottom inf material
  fRefractiveIndexList.insert(fRefractiveIndexList.end() - 1, idx);
  fThicknessList.insert(fThicknessList.end() - 1, thickness);
  fCoherentList.insert(fCoherentList.end() - 1, coherent);
}

//______________________________________________________________________________
void AMultilayer::CoherentTMM(AMultilayer::EPolarization polarization,
                              std::complex<Double_t> th_0, Double_t lam_vac,
                              Double_t& reflectance,
                              Double_t& transmittance,
                              Bool_t reverse) const {
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
  // one-to-one with elements of n_list. First and last elements should be
  // "inf".
  //
  // lam_vac is vacuum wavelength of the light.

  auto reversedRefractiveIndexList = fRefractiveIndexList;
  std::reverse(reversedRefractiveIndexList.begin(),
               reversedRefractiveIndexList.end());
  auto _RefractiveIndexList = reverse ? reversedRefractiveIndexList : fRefractiveIndexList;

  auto reversedThicknessList = fThicknessList;
  std::reverse(reversedThicknessList.begin(),
               reversedThicknessList.end());
  auto _ThicknessList = reverse ? reversedThicknessList : fThicknessList;

  auto num_layers = _RefractiveIndexList.size();
  std::vector<std::complex<Double_t>> n_list(num_layers);

  {
    auto n_i = n_list.begin();
    auto ref_i = _RefractiveIndexList.cbegin();
    for (std::size_t i = 0; i < num_layers; ++i) {
      *n_i = (*ref_i)->GetComplexRefractiveIndex(lam_vac);
      ++n_i;
      ++ref_i;
    }
  }

  // Input tests
  if (std::abs((n_list[0] * std::sin(th_0)).imag()) >= 100 * EPSILON ||
      !IsForwardAngle(n_list[0], th_0)) {
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
    for (std::size_t i = 0; i < num_layers; ++i) {
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
    auto thickness_i = _ThicknessList.cbegin();
    for (std::size_t i = 0; i < num_layers; ++i) {
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
  {
    auto delta_i = delta.begin();
    ++delta_i;  // start from i = 1
    for (std::size_t i = 1; i < num_layers - 1; ++i) {
      if ((*delta_i).imag() > 35) {
        *delta_i = (*delta_i).real() + std::complex<Double_t>(0, 35);
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
      ++delta_i;
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
    auto th_f = th_list.cbegin();
    ++th_f;  // increment to access th_f (th[i + 1])
    auto n_i = n_list.cbegin();
    auto n_f = n_list.cbegin();
    ++n_f;                                 // increment to access n_f (n[i + 1])
    auto cos_th_i = cos_th_list.cbegin();  // cos(th_i)
    auto cos_th_f = cos_th_list.cbegin();
    ++cos_th_f;  // increment to access cos(th_f)

    for (std::size_t i = 0; i < num_layers - 1; ++i) {
      std::complex<Double_t> r, t;
      interface_rt(polarization, *n_i, *n_f, *th_i, *th_f, r, t);
      *r_i = r;
      *t_i = t;

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
    const std::complex<Double_t> j(0, 1);
    auto M_i = M_list.begin();
    ++M_i;  // start i from 1
    auto t_i = t_list.cbegin();
    ++t_i;
    auto r_i = r_list.cbegin();
    ++r_i;
    auto delta_i = delta.cbegin();
    ++delta_i;
    for (std::size_t i = 1; i < num_layers - 1; ++i) {
      auto j_delta_i = j * (*delta_i);
      *M_i =
          1. / (*t_i) *
          A2x2ComplexMatrix(std::exp(-j_delta_i), 0, 0, std::exp(j_delta_i)) *
          A2x2ComplexMatrix(1, *r_i, *r_i, 1);
      ++M_i;
      ++t_i;
      ++r_i;
      ++delta_i;
    }
  }

  A2x2ComplexMatrix Mtilde(1, 0, 0, 1);
  {
    auto M_i = M_list.cbegin();
    ++M_i;  // start i from 1
    for (std::size_t i = 1; i < num_layers - 1; ++i) {
      Mtilde = Mtilde * (*M_i);
      ++M_i;
    }
  }

  Mtilde = A2x2ComplexMatrix(1, r_list[0], r_list[0], 1) / t_list[0] * Mtilde;

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
  auto n_i = n_list[0];
  auto n_f = n_list.back();
  auto th_i = th_0;
  auto th_f = th_list.back();
  if (polarization == kS) {
    transmittance = std::abs(t * t) * (((n_f * std::cos(th_f)).real()) /
                                       (n_i * std::cos(th_i)).real());
  } else {
    transmittance =
        std::abs(t * t) * (((n_f * std::conj(std::cos(th_f))).real()) /
                           (n_i * std::conj(std::cos(th_i))).real());
  }
}

//______________________________________________________________________________
void AMultilayer::IncGroupLayers(std::vector<std::vector<Double_t>>& stack_d_list,
                                 std::vector<std::vector<std::shared_ptr<ARefractiveIndex>>>& stack_n_list,
                                 std::vector<std::size_t>& all_from_inc,
                                 std::vector<std::size_t>& inc_from_all,
                                 std::vector<std::vector<std::size_t>>& all_from_stack,
                                 std::vector<std::vector<std::size_t>>& stack_from_all,
                                 std::vector<std::size_t>& inc_from_stack,
                                 std::vector<std::size_t>& stack_from_inc
                                 ) const {
  // C++ version of tmm.inc_group_layers

  std::size_t inc_index = 0;
  std::size_t stack_index = 0;
  std::vector<Double_t> ongoing_stack_d_list;
  std::vector<std::shared_ptr<ARefractiveIndex>> ongoing_stack_n_list;
  std::size_t within_stack_index;
  Bool_t stack_in_progress = kFALSE;

  stack_d_list.resize(0);
  stack_n_list.resize(0);
  all_from_inc.resize(0);
  inc_from_all.resize(0);
  all_from_stack.resize(0);
  stack_from_all.resize(0);
  inc_from_stack.resize(0);
  stack_from_inc.resize(0);

  for (std::size_t alllayer_index = 0; alllayer_index < fRefractiveIndexList.size(); ++alllayer_index) {
    if (fCoherentList[alllayer_index] == kTRUE) {
      inc_from_all.push_back(std::nan(""));
      if (!stack_in_progress) {
        stack_in_progress = kTRUE;
        ongoing_stack_d_list
          = std::vector<Double_t>{inf, fThicknessList[alllayer_index]};
        ongoing_stack_n_list
          = std::vector<std::shared_ptr<ARefractiveIndex>>{
          fRefractiveIndexList[alllayer_index - 1],
          fRefractiveIndexList[alllayer_index]};
        stack_from_all.push_back(std::vector<std::size_t>{stack_index, 1});
        all_from_stack.push_back(std::vector<std::size_t>{alllayer_index - 1, alllayer_index});
        inc_from_stack.push_back(inc_index - 1);
        within_stack_index = 1;
      } else {
        ongoing_stack_d_list.push_back(fThicknessList[alllayer_index]);
        ongoing_stack_n_list.push_back(fRefractiveIndexList[alllayer_index]);
        within_stack_index += 1;
        stack_from_all.push_back(std::vector<std::size_t>{stack_index, within_stack_index});
        all_from_stack.back().push_back(alllayer_index);
      }
    } else {
      stack_from_all.push_back(std::vector<std::size_t>{std::size_t(std::nan(""))});
      inc_from_all.push_back(inc_index);
      all_from_inc.push_back(alllayer_index);
      if (!stack_in_progress) {  // previous layer was also incoherent
        stack_from_inc.push_back(std::size_t(std::nan("")));
      } else { // previous layer was coherent
        stack_in_progress = kFALSE;
        stack_from_inc.push_back(stack_index);
        ongoing_stack_d_list.push_back(inf);
        stack_d_list.push_back(ongoing_stack_d_list);
        ongoing_stack_n_list.push_back(fRefractiveIndexList[alllayer_index]);
        stack_n_list.push_back(ongoing_stack_n_list);
        all_from_stack.back().push_back(alllayer_index);
        stack_index += 1;
      }
      inc_index += 1;
    }
  }
}

//______________________________________________________________________________
void AMultilayer::IncoherentTMM(AMultilayer::EPolarization polarization,
                                std::complex<Double_t> th_0, Double_t lam_vac,
                                Double_t& reflectance,
                                Double_t& transmittance) const {
  // Copied from tmm.inc_tmm

  // Incoherent, or partly-incoherent-partly-coherent, transfer matrix method.
  // See coh_tmm for definitions of pol, n_list, d_list, th_0, lam_vac.
  // c_list is "coherency list". Each entry should be 'i' for incoherent or 'c'
  // for 'coherent'.
  // If an incoherent layer has real refractive index (no absorption), then its
  // thickness doesn't affect the calculation results.
  // See https://arxiv.org/abs/1603.02720 for physics background and some
  // of the definitions.

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

  // Input test
  if (std::abs((n_list[0] * std::sin(th_0)).imag()) >= 100 * EPSILON) {
    Error("IncoherentTMM", "Error in n0 or th0!");
  }

  std::vector<std::vector<Double_t>> stack_d_list;
  std::vector<std::vector<std::shared_ptr<ARefractiveIndex>>> stack_n_list;
  std::vector<std::size_t> all_from_inc;
  std::vector<std::size_t> inc_from_all;
  std::vector<std::vector<std::size_t>> all_from_stack;
  std::vector<std::vector<std::size_t>> stack_from_all;
  std::vector<std::size_t> inc_from_stack;
  std::vector<std::size_t> stack_from_inc;

  IncGroupLayers(stack_d_list, stack_n_list, all_from_inc,
                 inc_from_all, all_from_stack, stack_from_all, inc_from_stack,
                 stack_from_inc);

  // th_list is a list with, for each layer, the angle that the light travels
  // through the layer. Computed with Snell's law. Note that the "angles" may be
  // complex!
  std::vector<std::complex<Double_t>> th_list;
  ListSnell(th_0, n_list, th_list);

  // coh_tmm_data_list[i] is the output of coh_tmm for the i'th stack
  std::vector<std::pair<Double_t, Double_t>> coh_tmm_data_list;
  // coh_tmm_bdata_list[i] is the same stack as coh_tmm_data_list[i] but
  // with order of layers reversed
  std::vector<std::pair<Double_t, Double_t>> coh_tmm_bdata_list;

  for (std::size_t i = 0; i < all_from_stack.size(); ++i) {
    Double_t R, T;
    AMultilayer multi(stack_n_list[i][0], stack_n_list[i].back());

    for (std::size_t j = 1; j < stack_n_list[i].size() - 1; ++j) {
      multi.InsertLayer(stack_n_list[i][j], stack_d_list[i][j]);
    }

    multi.CoherentTMM(polarization, th_list[all_from_stack[i][0]], lam_vac, R, T);
    coh_tmm_data_list.push_back(std::pair<Double_t, Double_t>{R, T});

    Bool_t reverse = kTRUE;
    multi.CoherentTMM(polarization, th_list[all_from_stack[i].back()], lam_vac, R, T, reverse);
    coh_tmm_bdata_list.push_back(std::pair<Double_t, Double_t>{R, T});
  }

  // P_list[i] is fraction not absorbed in a single pass through i'th incoherent
  // layer.
  auto num_inc_layers = all_from_inc.size();
  std::vector<Double_t> P_list(num_inc_layers);
  P_list[0] = 0;

  for (std::size_t inc_index = 1; inc_index < num_inc_layers - 1; ++inc_index) { // skip 0'th and last (infinite)
    auto i = all_from_inc[inc_index];
    auto n = fRefractiveIndexList[i]->GetComplexRefractiveIndex(lam_vac);
    auto d = fThicknessList[i];
    auto th = th_list[i];

    P_list[inc_index] = TMath::Exp(-4 * TMath::Pi() * d
                                   * (n * std::cos(th)).imag() / lam_vac);
    // For a very opaque layer, reset P to avoid divide-by-0 and similar
    // errors.
    if (P_list[inc_index] < 1e-30) {
      P_list[inc_index] = 1e-30;
    }
  }

  // T_list[i,j] and R_list[i,j] are transmission and reflection powers,
  // respectively, coming from the i'th incoherent layer, going to the j'th
  // incoherent layer. Only need to calculate this when j=i+1 or j=i-1.
  // (2D array is overkill but helps avoid confusion.)
  // initialize these arrays
  Double_t T_list[num_inc_layers][num_inc_layers];
  Double_t R_list[num_inc_layers][num_inc_layers];
  std::memset(T_list, 0, sizeof(T_list));
  std::memset(R_list, 0, sizeof(R_list));

  for (std::size_t inc_index = 0; inc_index < num_inc_layers - 1; ++inc_index){
    // looking at interface i -> i+1
    auto alllayer_index = all_from_inc[inc_index];
    auto nextstack_index = stack_from_inc[inc_index + 1];
    if (nextstack_index == std::size_t(std::nan(""))) { // next layer is incoherent
      Double_t R, T;
      interface_RT(polarization,
                   n_list[alllayer_index],
                   n_list[alllayer_index + 1],
                   th_list[alllayer_index],
                   th_list[alllayer_index + 1],
                   R, T);

      R_list[inc_index][inc_index + 1] = R;
      T_list[inc_index][inc_index + 1] = T;

      interface_RT(polarization,
                   n_list[alllayer_index + 1],
                   n_list[alllayer_index],
                   th_list[alllayer_index + 1],
                   th_list[alllayer_index],
                   R, T);

      R_list[inc_index + 1][inc_index] = R;
      T_list[inc_index+1][inc_index] = T;
    } else { // next layer is coherent
      R_list[inc_index][inc_index + 1]
        = coh_tmm_data_list[nextstack_index].first;
      T_list[inc_index][inc_index + 1]
        = coh_tmm_data_list[nextstack_index].second;
      R_list[inc_index + 1][inc_index]
        = coh_tmm_bdata_list[nextstack_index].first;
      T_list[inc_index + 1][inc_index]
        = coh_tmm_bdata_list[nextstack_index].second;
    }
  }

  // L is the transfer matrix from the i'th to (i+1)st incoherent layer, see
  // manual
  std::vector<TMatrixT<Double_t>> L_list{TMatrixDSym(2)};
  // L_0 is not defined because 0'th layer has no beginning.
  L_list[0][0][0] = std::nan("");

  TMatrixT<Double_t> Ltilde(2, 2);
  Ltilde[0][0] = 1;
  Ltilde[0][1] = -R_list[1][0];
  Ltilde[1][0] = R_list[0][1];
  Ltilde[1][1] = T_list[1][0] * T_list[0][1] - R_list[1][0] * R_list[0][1];
  Ltilde *= 1 / T_list[0][1];

  for (std::size_t i = 1; i < num_inc_layers - 1; ++i) {
    TMatrixT<Double_t> L1(2, 2), L2(2, 2);

    L1[0][0] = 1/P_list[i];
    L1[0][1] = 0;
    L1[1][0] = 0;
    L1[1][1] = P_list[i];
    L2[0][0] = 1;
    L2[0][1] = -R_list[i + 1][i];
    L2[1][0] = R_list[i][i + 1];
    L2[1][1] = T_list[i + 1][i] * T_list[i][i + 1]
      - R_list[i + 1][i] * R_list[i][i + 1];

    auto L = L1 * L2 * (1 / T_list[i][i + 1]);
    L_list.push_back(L);
    auto tmp = Ltilde * L;
    Ltilde = tmp;
  }

  transmittance = 1 / Ltilde[0][0];
  reflectance = Ltilde[1][0] / Ltilde[0][0];
}

//______________________________________________________________________________
void AMultilayer::PrintLayers(Double_t lambda) const {
  auto n = fRefractiveIndexList.size();
  for (std::size_t i = 0; i < n; ++i) {
    std::cout << "----------------------------------------\n";
    std::cout << i << "\tn_i = "
              << fRefractiveIndexList[i]->GetComplexRefractiveIndex(lambda)
              << "\td_i = " << fThicknessList[i] / AOpticsManager::nm()
              << " (nm)\n";
  }
  std::cout << "----------------------------------------" << std::endl;
}

//______________________________________________________________________________
void AMultilayer::SetNthreads(std::size_t n) {
  // Note that having n larger than 1 frequently decreases the total
  // performance. Use this method only when you feed a very long vector.
  if (n == 0) {
    fNthreads =
        std::thread::hardware_concurrency();  // can return 0 if n is unknown
    if (fNthreads == 0) fNthreads = 1;
  } else if (n > 0) {
    fNthreads = n;
  }
}
