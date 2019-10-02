////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// AGeoUtil                                                                   //
//                                                                            //
// Utility functions to build complex geometries easily                       //
////////////////////////////////////////////////////////////////////////////////

#include "TGeoTube.h"
#include "TMath.h"

#include "AGeoUtil.h"
#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && \
    !defined(R__FBSD)
NamespaceImp(AGeoUtil)
#endif

    static Double_t SumInRadius(TH2* h2, double x, double y, double r) {
  Double_t r2 = r * r;
  TAxis* xax = h2->GetXaxis();
  TAxis* yax = h2->GetYaxis();
  Int_t nbinsx = xax->GetNbins();
  Int_t nbinsy = yax->GetNbins();

  Double_t total = 0.;

  for (Int_t ix = 1; ix <= nbinsx; ++ix) {
    Double_t cx = xax->GetBinCenter(ix);
    for (Int_t iy = 1; iy <= nbinsy; ++iy) {
      Double_t cy = yax->GetBinCenter(iy);
      Double_t c = h2->GetBinContent(ix, iy);
      if (c <= 0) {
        continue;
      }
      Double_t d2 = (cx - x) * (cx - x) + (cy - y) * (cy - y);
      if (d2 <= r2) {
        total += c;
      }
    }
  }
  return total;
}

namespace AGeoUtil {

//______________________________________________________________________________
void MakePointToPointBBox(const char* name, const TVector3& v1,
                          const TVector3& v2, Double_t dx, Double_t dy,
                          TGeoBBox** box, TGeoCombiTrans** combi) {
  TVector3 v3 = v1 + v2;
  v3 *= 0.5;  // center of the gravity
  TVector3 v4 = v3 - v1;

  Double_t theta = v4.Theta() * TMath::RadToDeg();
  Double_t phi = v4.Phi() * TMath::RadToDeg();

  *box = new TGeoBBox(Form("%sbox", name), dx, dy, v4.Mag());
  *combi = new TGeoCombiTrans(TGeoTranslation(v3.X(), v3.Y(), v3.Z()),
                              TGeoRotation("", phi + 90, theta, 0));
  (*combi)->SetName(Form("%scombi", name));
  (*combi)->RegisterYourself();
}

//______________________________________________________________________________
void MakePointToPointTube(const char* name, const TVector3& v1,
                          const TVector3& v2, Double_t radius, TGeoTube** tube,
                          TGeoCombiTrans** combi) {
  // Create a TGeoTube whose both ends are rotated to locate at v1 or v2 by
  // using combi
  /*
  Begin_Macro(gui, source)
  {
    Double_t m = AOpticsManager::m();
    AOpticsManager manager("manager", "manager");
    TGeoBBox* worldbox = new TGeoBBox("worldbox", 10*m, 10*m, 10*m);
    AOpticalComponent* world = new AOpticalComponent("world", worldbox);
    manager.SetTopVolume(world);

    TGeoTube* tube;
    TGeoCombiTrans* combi;
    TVector3 v1(-1*m, -2*m, 3*m);
    TVector3 v2(4*m, 5*m, -1*m);
    AGeoUtil::MakePointToPointTube("tube", v1, v2, 0.5*m, &tube, &combi);
    ALens* lens = new ALens("lens", tube);
    world->AddNode(lens, 1, combi);
    manager.CloseGeometry();

    world->Draw("ogl");
    TGLViewer* gl = (TGLViewer*)gPad->GetViewer3D();

    return gl;
  }
  End_Macro
  */
  TVector3 v3 = v1 + v2;
  v3 *= 0.5;  // center of the gravity
  TVector3 v4 = v3 - v1;

  Double_t theta = v4.Theta() * TMath::RadToDeg();
  Double_t phi = v4.Phi() * TMath::RadToDeg();

  *tube = new TGeoTube(Form("%stube", name), 0., radius, v4.Mag());
  *combi = new TGeoCombiTrans(TGeoTranslation(v3.X(), v3.Y(), v3.Z()),
                              TGeoRotation("", phi + 90, theta, 0));
  (*combi)->SetName(Form("%scombi", name));
  (*combi)->RegisterYourself();
}

//______________________________________________________________________________
void ContainmentRadius(TH2* h2, Double_t fraction, Double_t& r, Double_t& x,
                       Double_t& y) {
  // compute the radius of containment from a 2D histogram.
  Int_t no_shift = 0;
  Int_t no_stable = 0;

  x = h2->GetMean(1);  // Initial x
  y = h2->GetMean(2);  // Initial y
  r = TMath::Sqrt(h2->GetStdDev(1) * h2->GetStdDev(1) +
                  h2->GetStdDev(2) * h2->GetStdDev(2)) *
      1.5;
  Double_t dr = 0.1 * r;
  Double_t sum_goal = h2->Integral() * fraction;

  for (Int_t i = 0; i < 100 && no_shift < 30; i++) {
    Bool_t stable_r = false, stable_x = true, stable_y = true;
    Double_t sum0 = SumInRadius(h2, x, y, r);
    Double_t next_r = r;
    if (sum0 < sum_goal) {
      Double_t sum1 = SumInRadius(h2, x, y, r + dr);
      if (sum1 == sum0) {
        dr *= 2.;
        continue;
      }
      next_r = r + dr * (sum_goal - sum0) / (sum1 - sum0);
    } else if (sum0 != sum_goal) {
      Double_t sum1 = SumInRadius(h2, x, y, r - dr);
      if (sum1 == sum0) {
        dr *= 2.;
        continue;
      }
      next_r = r - dr * (sum0 - sum_goal) / (sum0 - sum1);
    }
    if (next_r < 0.) next_r = 0.5 * r;
    if (next_r < 0.5 * r) next_r = 0.5 * r;
    if (next_r > 2. * r) next_r = 2. * r;

    stable_r = fabs(next_r - r) < 0.0001 * r;

    r = next_r;

    {
      Double_t sum1 = SumInRadius(h2, x, y, r);

      dr *=
          sum0 != sum_goal ? fabs((sum1 - sum_goal) / (sum0 - sum_goal)) : 0.5;

      if (dr > 0.5 * r) {
        dr = 0.5 * r;
      }
      if (dr < 0.0005 * r) {
        dr = 0.0005 * r;
      }

      no_shift++;

      for (Double_t dx = 0.25 * r; dx > 0.1 * dr; dx *= 0.25) {
        Double_t sum_x1 = SumInRadius(h2, x + dx, y, r);
        Double_t sum_x2 = SumInRadius(h2, x - dx, y, r);
        while (sum_x1 > sum1) {
          no_shift = 0;
          x += dx;
          sum_x2 = sum1;
          sum1 = sum_x1;
          sum_x1 = SumInRadius(h2, x + dx, y, r);
          stable_x = false;
        }
        while (sum_x2 > sum1) {
          no_shift = 0;
          x -= dx;
          sum_x1 = sum1;
          sum1 = sum_x2;
          sum_x2 = SumInRadius(h2, x - dx, y, r);
          stable_x = false;
        }
      }
    }

    for (Double_t dy = 0.1 * r; dy > 0.1 * dr; dy *= 0.25) {
      Double_t sum1 = SumInRadius(h2, x, y, r);
      Double_t sum_y1 = SumInRadius(h2, x, y + dy, r);
      Double_t sum_y2 = SumInRadius(h2, x, y - dy, r);
      while (sum_y1 > sum1) {
        no_shift = 0;
        y += dy;
        sum_y2 = sum1;
        sum1 = sum_y1;
        sum_y1 = SumInRadius(h2, x, y + dy, r);
        stable_y = false;
      }
      while (sum_y2 > sum1) {
        no_shift = 0;
        y -= dy;
        sum_y1 = sum1;
        sum1 = sum_y2;
        sum_y2 = SumInRadius(h2, x, y - dy, r);
        stable_y = false;
      }
    }

    if (stable_r && stable_x && stable_y) {
      no_stable++;
    } else {
      no_stable = 0;
    }
    // Enough rounds without any change in radius and position?
    if (no_stable >= 4) {
      break;
    }
  }
}

}  // namespace AGeoUtil
