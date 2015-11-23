// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GEO_BEZIER_PGON_H
#define A_GEO_BEZIER_PGON_H

#include "TGeoPgon.h"
#include "TVector2.h"

///////////////////////////////////////////////////////////////////////////////
//
// AGeoBezierPgon
//
// Geometry class for Pgon-like volume, but the side surfaces are defined by
// a Bezier curve (https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
//
///////////////////////////////////////////////////////////////////////////////

class AGeoBezierPgon : public TGeoPgon {
protected:
   Double_t fLength; // fDZ will be fLength/2
   Double_t fR1; // Half of the upper aperture
   Double_t fR2; // Half of the lower aperture
   TVector2 fP1; // Relative coordinates of the control point 1
   TVector2 fP2; // Relative coordinates of the control point 2
   Int_t    fNcontrol; // Number of control points (0, 1, or 2)

public:
   AGeoBezierPgon();
   AGeoBezierPgon(Double_t phi, Double_t dphi, Int_t nedges, Int_t nz, Double_t r1, Double_t r2, Double_t dz);
   AGeoBezierPgon(const char* name, Double_t phi, Double_t dphi, Int_t nedges, Int_t nz, Double_t r1, Double_t r2, Double_t dz);
   virtual ~AGeoBezierPgon();

   virtual void Bezier(Double_t t, Double_t& r, Double_t& z);
   virtual void SetControlPoints(Double_t r1, Double_t z1);
   virtual void SetControlPoints(Double_t r1, Double_t z1, Double_t r2, Double_t z2);
   virtual void SetSections();

   ClassDef(AGeoBezierPgon, 1)
};

#endif // A_GEO_BEZIER_PGON_H
