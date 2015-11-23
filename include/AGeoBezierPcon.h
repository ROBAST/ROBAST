// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GEO_BEZIER_PCON_H
#define A_GEO_BEZIER_PCON_H

#include "TGeoPcon.h"
#include "TVector2.h"

///////////////////////////////////////////////////////////////////////////////
//
// AGeoBezierPcon
//
// Geometry class for Pcon-like volume, but the side surfaces are defined by
// a Bezier curve (https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
//
///////////////////////////////////////////////////////////////////////////////

class AGeoBezierPcon : public TGeoPcon {
protected:
   Double_t fLength; // fDZ will be fLength/2
   Double_t fR1; // Half of the upper aperture
   Double_t fR2; // Half of the lower aperture
   TVector2 fP1; // Relative coordinates of the control point 1
   TVector2 fP2; // Relative coordinates of the control point 2
   Int_t    fNcontrol; // Number of control points (0, 1, or 2)

public:
   AGeoBezierPcon();
   AGeoBezierPcon(Double_t phi, Double_t dphi, Int_t nz, Double_t r1, Double_t r2, Double_t dz);
   AGeoBezierPcon(const char* name, Double_t phi, Double_t dphi, Int_t nz, Double_t r1, Double_t r2, Double_t dz);
   virtual ~AGeoBezierPcon();

   virtual void Bezier(Double_t t, Double_t& r, Double_t& z);
   virtual void SetControlPoints(Double_t r1, Double_t z1);
   virtual void SetControlPoints(Double_t r1, Double_t z1, Double_t r2, Double_t z2);
   virtual void SetSections();

   ClassDef(AGeoBezierPcon, 1)
};

#endif // A_GEO_BEZIER_PCON_H
