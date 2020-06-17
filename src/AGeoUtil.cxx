////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// AGeoUtil                                                                   //
//                                                                            //
// Utility functions to build complex geometries easily                       //
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
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
void MakeArb8FromPoints(const char* name, const TVector3& v1,
                        const TVector3& v2, const TVector3& v3,
                        const TVector3& v4, const TVector3& v5,
                        TGeoArb8** arb8, TGeoCombiTrans** combi) {
  // Create an Arb8 object from 5 different points. v1-v4 are for the top
  // surface and they must be clock-wise when seen from the top. v5 for the
  // bottom and v6-8 will be automatically calculated. It is assumed that
  // v1-v4 and v5-v8 are paralell and that they have the same coordinates when
  // seen from the top. Very small coordinates residual due to computation or
  // the input CAD file is ignored.

  TVector3 normal = v1 - v5;
  Double_t dZ = normal.Mag() / 2.;
  Double_t theta = normal.Theta();
  Double_t phi = normal.Phi();

  TVector3 v[4] = {TVector3(0, 0, 0), v2 - v1, v3 - v1, v4 - v1};
  Double_t vertices[16];

  for(Int_t i = 1; i <= 3; ++i) {
    v[i].RotateZ(-phi - TMath::Pi() / 2.); // in radian
    v[i].RotateX(-theta); // in radian
    vertices[2 * i    ] = v[i].X();
    vertices[2 * i + 1] = v[i].Y();
    vertices[2 * i + 8] = v[i].X();
    vertices[2 * i + 9] = v[i].Y();
  }

  *arb8 = new TGeoArb8(name, dZ, vertices);
  TGeoTranslation tr(v5.X() + normal.X() / 2., v5.Y() + normal.Y() / 2.,
                     v5.Z() + normal.Z() / 2.);
  TGeoRotation rot = TGeoRotation("", 0, 0, phi * TMath::RadToDeg() + 90)
    * TGeoRotation("", 0, theta * TMath::RadToDeg(), 0); // deg
  *combi = new TGeoCombiTrans(tr, rot);
  (*combi)->SetName(Form("%scombi", name));
  (*combi)->RegisterYourself();
}

//______________________________________________________________________________
void MakeXtruFromPoints(const char* name, const std::vector<TVector3>& vecs,
                        TGeoXtru** xtru, TGeoCombiTrans** combi) {
  // Create an Xtru object from nvert + 1 different points. v_1 to v_nvert are
  // for the top surface and they must be clock-wise when seen from the top.
  // n_(nvert + 1) for the bottom and it must corrspond to v1. It is assumed
  // that the plane made of v1 to v_nvert is perpendiculr to v1 - v_(nvert + 1).
  // Very small coordinates residual due to computation or the input CAD file
  // is ignored.
  std::size_t nvert = vecs.size() - 1;
  TVector3 normal = vecs[0] - vecs[nvert];
  Double_t dZ = normal.Mag() / 2.;
  Double_t theta = normal.Theta();
  Double_t phi = normal.Phi();

  std::vector<TVector3> v;
  for(std::size_t i = 0; i < nvert; ++i) {
    v.push_back(vecs[i] - vecs[0]);
  }
  Double_t x[nvert], y[nvert];

  for(std::size_t i = 0; i < nvert; ++i) {
    v[i].RotateZ(-phi - TMath::Pi() / 2.); // in radian
    v[i].RotateX(-theta); // in radian
    x[i] = v[i].X();
    y[i] = v[i].Y();
  }

  *xtru = new TGeoXtru(2); // nz = 2
  (*xtru)->SetName(name);
  (*xtru)->DefinePolygon(nvert, x, y);
  (*xtru)->DefineSection(0, -dZ);
  (*xtru)->DefineSection(1, +dZ);
  TVector3 shift = vecs[0] - normal * .5;
  TGeoTranslation tr(shift.X(), shift.Y(), shift.Z());
  TGeoRotation rot = TGeoRotation("", 0, 0, phi * TMath::RadToDeg() + 90)
    * TGeoRotation("", 0, theta * TMath::RadToDeg(), 0); // deg
  *combi = new TGeoCombiTrans(tr, rot);
  (*combi)->SetName(Form("%scombi", name));
  (*combi)->RegisterYourself();
}

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
  MakePointToPointTube(name, v1, v2, 0, radius, tube, combi);
}

//______________________________________________________________________________
void MakePointToPointTube(const char* name, const TVector3& v1,
                          const TVector3& v2, Double_t rmin, Double_t rmax,
                          TGeoTube** tube, TGeoCombiTrans** combi) {
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

  *tube = new TGeoTube(Form("%stube", name), rmin, rmax, v4.Mag());
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
