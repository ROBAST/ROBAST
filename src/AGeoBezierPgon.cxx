/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AGeoBezierPgon
//
// Geometry class for Pgon-like volume, but the side surfaces are defined by
// a Bezier curve (http://en.wikipedia.org/wiki/BŽzier_curve)
//
///////////////////////////////////////////////////////////////////////////////

#include "AGeoBezierPgon.h"

ClassImp(AGeoBezierPgon)

//_____________________________________________________________________________
AGeoBezierPgon::AGeoBezierPgon() : TGeoPgon()
{
  // Default constructor
}

//_____________________________________________________________________________
AGeoBezierPgon::AGeoBezierPgon(Double_t phi, Double_t dphi, Int_t nedges,
                               Int_t nz, Double_t r1, Double_t r2, Double_t dz)
  : TGeoPgon(phi, dphi, nedges, nz), fLength(dz*2), fR1(r1), fR2(r2)
{
  fNcontrol = 0;
}

//_____________________________________________________________________________
AGeoBezierPgon::AGeoBezierPgon(const char* name, Double_t phi, Double_t dphi,
                               Int_t nedges, Int_t nz, Double_t r1, Double_t r2,
                               Double_t dz)
  : TGeoPgon(name, phi, dphi, nedges, nz), fLength(dz*2), fR1(r1), fR2(r2)
{
  fNcontrol = 0;
}

//_____________________________________________________________________________
AGeoBezierPgon::~AGeoBezierPgon()
{
  // destructor
}
//_____________________________________________________________________________
void AGeoBezierPgon::Bezier(Double_t t, Double_t& r, Double_t& z)
{
  TVector2 P0(0, 0);
  TVector2 B;
  if(fNcontrol == 0){
    TVector2 P1(1, 1);
    B = (1 - t)*P0 + t*P1;
  } else if(fNcontrol == 1){
    TVector2 P2(1, 1);
    B = (1 - t)*(1 - t)*P0 + 2*(1 - t)*t*fP1 + t*t*P2;
  } else if(fNcontrol == 2){
    TVector2 P3(1, 1);
    B = (1 - t)*(1 - t)*(1 - t)*P0 + 3*(1 - t)*(1 - t)*t*fP1 + 3*(1 - t)*t*t*fP2 + t*t*t*P3;
  } // if

  r = fR2 + B.X()*(fR1 - fR2);
  z = -fLength/2. + B.Y()*fLength;
}

//_____________________________________________________________________________
void AGeoBezierPgon::SetControlPoints(Double_t r1, Double_t z1)
{
  // Set the relative coordinates of control point 1
  // (r1, z1) must be given in relative coordinates against P0 and P2
  // For example, when (r1, z1) = (0.6, 0.7), the coordinates of P1 is
  // (R, Z) = (R2 + 0.6*(R1 - R2), -DZ + 0.7*(+DZ - (-DZ)))
  // The Bezier curve becomes quadratic
  //
  // Z
  // ^
  // |<--R1-->P2 (R1, +DZ)
  // |        /
  // |       /  P1
  // |      /
  // +-----P0---------> R
  // |<-R2-> (R2, -DZ)
  fNcontrol = 1;
  fP1.Set(r1, z1);
  fP2.Set(0., 0.);
  SetSections();
}

//_____________________________________________________________________________
void AGeoBezierPgon::SetControlPoints(Double_t r1, Double_t z1, Double_t r2, Double_t z2)
{
  // Set the relative coordinates of control points 1 and 2
  // The Bezier curve becomes cubic
  fNcontrol = 2;
  fP1.Set(r1, z1);
  fP2.Set(r2, z2);
  SetSections();
}

//_____________________________________________________________________________
void AGeoBezierPgon::SetSections()
{
  for(Int_t i = 0; i < fNz; i++){
    Double_t t = Double_t(i)/(fNz - 1);
    Double_t r, z;
    Bezier(t, r, z);
    DefineSection(i, z, 0, r);
  } // i
}
