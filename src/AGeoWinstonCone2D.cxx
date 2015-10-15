/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AGeoWinstonCone2D
//
// Geometry class for tubes which have two aspheric surface
//
///////////////////////////////////////////////////////////////////////////////

#include "AGeoWinstonCone2D.h"

#include "Riostream.h"
#include "TBuffer3D.h"
#include "TBuffer3DTypes.h"
#include "TGeoCone.h"
#include "TGeoManager.h"
#include "TGeoTube.h"
#include "TGeoVolume.h"
#include "TMath.h"
#include "TVector3.h"
#include "TVirtualGeoPainter.h"
#include "TVirtualPad.h"

ClassImp(AGeoWinstonCone2D)

//_____________________________________________________________________________
AGeoWinstonCone2D::AGeoWinstonCone2D() : TGeoBBox(0, 0, 0)
{
  // Default constructor
  SetShapeBit(TGeoShape::kGeoBox);
}

//_____________________________________________________________________________
AGeoWinstonCone2D::AGeoWinstonCone2D(Double_t r1, Double_t r2, Double_t y)
  : TGeoBBox(0, 0, 0)
{
  SetShapeBit(TGeoShape::kGeoBox);
  SetWinstonDimensions(r1, r2, y);
  ComputeBBox();
}

//_____________________________________________________________________________
AGeoWinstonCone2D::AGeoWinstonCone2D(const char *name,
                                     Double_t r1, Double_t r2, Double_t y)
  : TGeoBBox(name, 0, 0, 0)
{
  SetShapeBit(TGeoShape::kGeoBox);
  SetWinstonDimensions(r1, r2, y);
  ComputeBBox();
}

//_____________________________________________________________________________
AGeoWinstonCone2D::~AGeoWinstonCone2D()
{
  // Destructor
}

//_____________________________________________________________________________
Double_t AGeoWinstonCone2D::CalcdRdZ(Double_t z) const throw(std::exception)
{
  if(TMath::Abs(z) > fDZ + 1e-10){
    throw std::exception();
  } // if

  Double_t sint = TMath::Sin(fTheta);
  Double_t cost = TMath::Cos(fTheta);

  Double_t t = z + fDZ;
  Double_t a0 = t*t*sint*sint - 4.*fF*(t*cost + fF);
  Double_t a1 = 2.*t*sint*cost + 4.*fF*sint;
  Double_t a2 = cost*cost;
  Double_t da0dt = 2*t*sint*sint - 4*fF*cost;
  Double_t da1dt = 2*sint*cost;

  Double_t drdz = (-da1dt + (a1*da1dt - 2*da0dt*a2)/TMath::Sqrt(a1*a1 - 4*a0*a2))/(2*a2);

  return drdz;
}

//_____________________________________________________________________________
Double_t AGeoWinstonCone2D::CalcR(Double_t z) const throw(std::exception)
{
  if(TMath::Abs(z) > fDZ + 1e-10){
    throw std::exception();
  } // i

  Double_t sint = TMath::Sin(fTheta);
  Double_t cost = TMath::Cos(fTheta);

  Double_t t = z + fDZ;
  Double_t a0 = t*t*sint*sint - 4.*fF*(t*cost + fF);
  Double_t a1 = 2.*t*sint*cost + 4.*fF*sint;
  Double_t a2 = cost*cost;

  Double_t r = (- a1 + TMath::Sqrt(a1*a1 - 4.*a0*a2))/(2*a2) - fR2;

  return r;
}

//_____________________________________________________________________________
Double_t AGeoWinstonCone2D::Capacity() const
{
  // Compute capacity of the shape in [length^3]
  // Not implemeted yet

  return 0;
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::ComputeBBox()
{
  // Compute bounding box of the shape
  fDX = fR1;
  fDY = fDY;
  fDZ = fDZ;
  fOrigin[0] = 0;
  fOrigin[1] = 0;
  fOrigin[2] = 0;
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::ComputeNormal(CONST53410 Double_t* point, CONST53410 Double_t* dir,
                                      Double_t* norm)
{
  // Compute normal to closest surface from POINT.

  // Following calculation assumes that the point is very close to surfaces.
  Double_t x = point[0];
  Double_t y = point[1];
  Double_t z = point[2];

  Double_t saf[3];
  saf[0] = TMath::Abs(TMath::Abs(fDY) - TMath::Abs(y));
  saf[1] = TMath::Abs(TMath::Abs(fDZ) - TMath::Abs(z));
  try {
    saf[2] = TMath::Abs(CalcR(z) - TMath::Abs(x));
  } catch (...) {
    saf[2] = TGeoShape::Big();
  } // try

  Int_t i = TMath::LocMin(3, saf); // find minimum

  if(i == 0){ // on the XZ surface
    norm[0] = 0;
    norm[1] = 1;
    norm[2] = 0;
  } else if(i == 1) { // on the XY surface
    norm[0] = 0;
    norm[1] = 0;
    norm[2] = 1;
  } else {
    if(point[0] > 0){
      norm[0] = 1;
      norm[1] = 0;
      norm[2] = -CalcdRdZ(z);
    } else {
      norm[0] = 1;
      norm[1] = 0;
      norm[2] = CalcdRdZ(z);
    } // if
  } // if

  TVector3 vec(norm);
  vec.SetMag(1.);
  norm[0] = vec.X();
  norm[1] = vec.Y();
  norm[2] = vec.Z();

  if (norm[0]*dir[0] + norm[1]*dir[1] + norm[2]*dir[2] < 0) {
    norm[0] = -norm[0];
    norm[1] = -norm[1];
    norm[2] = -norm[2];
  } // if
}

//_____________________________________________________________________________
Bool_t AGeoWinstonCone2D::Contains(CONST53410 Double_t* point) const
{
  // Test if point is in this shape
  Double_t x = point[0];
  Double_t y = point[1];
  Double_t z = point[2];
  if(TMath::Abs(y) > fDY or TMath::Abs(z) > fDZ){
    return kFALSE;
  } // if

  Double_t r = CalcR(z);
  if(TMath::Abs(x) > r){
    return kFALSE;
  } // if

  return kTRUE;
}

//_____________________________________________________________________________
Int_t AGeoWinstonCone2D::DistancetoPrimitive(Int_t px, Int_t py)
{
  // compute closest distance from point px,py to each corner
  Int_t n = gGeoManager->GetNsegments();

  Int_t numPoints = 4*(n + 1);

  return ShapeDistancetoPrimitive(numPoints, px, py);
}

//_____________________________________________________________________________
Double_t AGeoWinstonCone2D::DistFromInside(CONST53410 Double_t* point, CONST53410 Double_t* dir,
                                           Int_t iact, Double_t step,
                                           Double_t* safe) const
{
  // compute distance from inside point to surface of the sphere

  // compute safe distance
  if(iact < 3 and safe){
    *safe = Safety(point, kFALSE);
    if (iact==0) return TGeoShape::Big();
    if (iact==1 && step < *safe) return TGeoShape::Big();
  } // if

  // calculate distance
  Double_t dz = TGeoShape::Big();
  if(dir[2] < 0){
    dz = -(point[2] + fDZ)/dir[2];
  } else if (dir[2] > 0) {
    dz = (fDZ - point[2])/dir[2];
  } // if

  Double_t dy = TGeoShape::Big();
  if(dir[1] < 0){
    dy = -(point[1] + fDY)/dir[1];
  } else if (dir[1] > 0) {
    dy = (fDY - point[1])/dir[1];
  } // if

  Double_t d[4];
  d[0] = dz;
  d[1] = dy;
  d[2] = DistToParabola(point, dir, 0., TMath::Pi());
  d[3] = DistToParabola(point, dir, TMath::Pi(), TMath::Pi());

  return d[TMath::LocMin(4, d)];
}

//_____________________________________________________________________________
Double_t AGeoWinstonCone2D::DistFromOutside(CONST53410 Double_t* point, CONST53410 Double_t* dir,
                                            Int_t iact, Double_t step,
                                            Double_t* safe) const
{
  // compute distance from outside point to surface of the sphere

  // compute safe distance
  if(iact < 3 and safe){
    *safe = Safety(point, kFALSE);
    if (iact == 0) return TGeoShape::Big();
    if (iact == 1 && step < *safe) return TGeoShape::Big();
  } // if

  // calculate distance

  if(point[2] <= -fDZ) {
    if(dir[2] <= 0){
      return TGeoShape::Big();
    } // if
    Double_t snxt = -(fDZ + point[2])/dir[2];
    // find extrapolated X and Y
    Double_t xnew = point[0] + snxt*dir[0];
    Double_t ynew = point[1] + snxt*dir[1];
    if(TMath::Abs(xnew) <= fR2 and TMath::Abs(ynew) <= fDY){
      return snxt;
    } // if
  } else if(point[2] >= fDZ){
    if(dir[2] >= 0){
      return TGeoShape::Big();
    } // if
    Double_t snxt = (fDZ - point[2])/dir[2];
    // find extrapolated X and Y
    Double_t xnew = point[0] + snxt*dir[0];
    Double_t ynew = point[1] + snxt*dir[1];
    if(TMath::Abs(xnew) <= fR1 and TMath::Abs(ynew) <= fDY){
      return snxt;
    } // if
  } // if

  if(point[1] <= -fDY) {
    if(dir[1] <= 0){
      return TGeoShape::Big();
    } // if
    Double_t snxt = -(fDY + point[1])/dir[1];
    // find extrapolated X and Y
    Double_t xnew = point[0] + snxt*dir[0];
    Double_t znew = point[2] + snxt*dir[2];
    if(TMath::Abs(znew) <= fDZ and TMath::Abs(xnew) <= CalcR(znew)){
      return snxt;
    } // if
  } else if(point[1] >= fDY){
    if(dir[1] >= 0){
      return TGeoShape::Big();
    } // if
    Double_t snxt = (fDY - point[1])/dir[1];
    // find extrapolated X and Y
    Double_t xnew = point[0] + snxt*dir[0];
    Double_t znew = point[2] + snxt*dir[2];
    if(TMath::Abs(znew) <= fDZ and TMath::Abs(xnew) <= CalcR(znew)){
      return snxt;
    } // if
  } // if

  Double_t d[2];
  Double_t snxt = DistToParabola(point, dir, 0., TMath::Pi());
  Double_t ynew = point[1] + snxt*dir[1];
  if(TMath::Abs(ynew) <= fDY){
    d[0] = snxt;
  } else {
    d[0] = TGeoShape::Big();
  } // if

  snxt = DistToParabola(point, dir, TMath::Pi(), TMath::Pi());
  ynew = point[1] + snxt*dir[1];
  if(TMath::Abs(ynew) <= fDY){
    d[1] = snxt;
  } else {
    d[1] = TGeoShape::Big();
  } // if

  return TMath::Min(d[0], d[1]);
}

//_____________________________________________________________________________
Double_t AGeoWinstonCone2D::DistToParabola(CONST53410 Double_t* point, CONST53410 Double_t* dir, Double_t phi, Double_t open) const
{
  Double_t x = TMath::Cos(phi)*point[0] + TMath::Sin(phi)*point[1];
  Double_t y =-TMath::Sin(phi)*point[0] + TMath::Cos(phi)*point[1];
  Double_t z = point[2];
  Double_t px = TMath::Cos(phi)*dir[0] + TMath::Sin(phi)*dir[1];
  Double_t py =-TMath::Sin(phi)*dir[0] + TMath::Cos(phi)*dir[1];
  Double_t pz = dir[2];

  if(px == 0 and pz == 0){
    return TGeoShape::Big();
  } // if

  Double_t cost = TMath::Cos(fTheta);
  Double_t sint = TMath::Sin(fTheta);
  // coordinates in the parabola frame inside the 1st quadrant
  // The focal point is at (X, Z) = (0, f)
  Double_t X = cost*(x + fR2) + (z + fDZ)*sint;
  Double_t Z = -sint*(x + fR2) + (z + fDZ)*cost + fF;
  Double_t alpha = TMath::ATan2(pz, px); // inclination in the x-z plane
  Double_t ALPHA = alpha - fTheta; // inclination in the X-Z plane
  Double_t tanA = TMath::Tan(ALPHA);

  Double_t dist[2];

  Double_t tmp = tanA*tanA - (X*tanA - Z)/fF;
  if(tmp < 0){
    dist[0] = TGeoShape::Big();
    dist[1] = TGeoShape::Big();
  } else {
    Double_t X_cross_p, X_cross_m;
    if(fDZ*2/TMath::Abs(tanA) < TGeoShape::Tolerance()){ // direction is almost parallel to Z axis
      X_cross_p = X;
      X_cross_m = X;
    } else {
      X_cross_p = 2*fF*(tanA + TMath::Sqrt(tmp));
      X_cross_m = 2*fF*(tanA - TMath::Sqrt(tmp));
    } // if
    Double_t Z_cross_p = X_cross_p*X_cross_p/4./fF;
    Double_t Z_cross_m = X_cross_m*X_cross_m/4./fF;

    Double_t x_cross_p = cost*X_cross_p - sint*(Z_cross_p - fF) - fR2;
    Double_t x_cross_m = cost*X_cross_m - sint*(Z_cross_m - fF) - fR2;
    Double_t z_cross_p = sint*X_cross_p + cost*(Z_cross_p - fF) - fDZ;
    Double_t z_cross_m = sint*X_cross_m + cost*(Z_cross_m - fF) - fDZ;
    Double_t y_cross_p;
    Double_t y_cross_m;

    // Avoid using meaningless values
    // such as py/pz when |py| << 1 and |pz| << 1
    // and py/px when |py| << 1 and |px| << 1
    if(TMath::Abs(px) <= TMath::Abs(pz) and TMath::Abs(py) <= TMath::Abs(pz)){
      y_cross_p = y + (z_cross_p - z)*py/pz;
      y_cross_m = y + (z_cross_m - z)*py/pz;
    } else if(TMath::Abs(py) <= TMath::Abs(px) and TMath::Abs(pz) <= TMath::Abs(px)){
      y_cross_p = y + (x_cross_p - x)*py/px;
      y_cross_m = y + (x_cross_m - x)*py/px;
    } else {
      y_cross_p = y + (px == 0 ? (z_cross_p - z)*py/pz : (x_cross_p - x)*py/px);
      y_cross_m = y + (px == 0 ? (z_cross_m - z)*py/pz : (x_cross_m - x)*py/px);
    } // if

    Double_t dx = x_cross_p - x;
    Double_t dy = y_cross_p - y;
    Double_t dz = z_cross_p - z;

    if(x_cross_p < fR2 or fR1 < x_cross_p or
        z_cross_p < -fDZ or fDZ < z_cross_p or
        dx*px + dz*pz < 0){
      dist[0] = TGeoShape::Big();
    } else {
      if(TMath::Abs(TMath::ATan2(y_cross_p, x_cross_p)) <= open/2.){
        dist[0] = TMath::Sqrt(dx*dx + dy*dy + dz*dz);
      } else {
        dist[0] = TGeoShape::Big();
      } // if
    } // if

    dx = x_cross_m - x;
    dy = y_cross_m - y;
    dz = z_cross_m - z;

    if(x_cross_m < fR2 or fR1 < x_cross_m or
        z_cross_m < -fDZ or fDZ < z_cross_m or
        dx*px + dz*pz < 0){
      dist[1] = TGeoShape::Big();
    } else {
      if(TMath::Abs(TMath::ATan2(y_cross_m, x_cross_m)) <= open/2.){
        dist[1] = TMath::Sqrt(dx*dx + dy*dy + dz*dz);
      } else {
        dist[1] = TGeoShape::Big();
      } // if
    } // if
  } // if

  return TMath::Min(dist[0], dist[1]);
}

//_____________________________________________________________________________
TGeoVolume* AGeoWinstonCone2D::Divide(TGeoVolume*, const char*, Int_t, Int_t,
                                      Double_t, Double_t)
{
  Error("Divide", "Division of a 2D Winston cone is not implemented");
  return 0;
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::GetBoundingCylinder(Double_t* param) const
{
  //--- Fill vector param[4] with the bounding cylinder parameters. The order
  // is the following : Rmin, Rmax, Phi1, Phi2
  param[0] = 0;
  param[1] = fDX*fDX + fDY*fDY;
  param[2] = 0;
  param[3] = 360;
}

//_____________________________________________________________________________
const TBuffer3D& AGeoWinstonCone2D::GetBuffer3D(Int_t reqSections,
                                                Bool_t localFrame) const
{
  // Fills a static 3D buffer and returns a reference
  static TBuffer3D buffer(TBuffer3DTypes::kGeneric);

  TGeoBBox::FillBuffer3D(buffer, reqSections, localFrame);

  if(reqSections & TBuffer3D::kRawSizes){
    Int_t n = gGeoManager->GetNsegments();
    Int_t nbPnts = 4*(n + 1); // Number of points
    Int_t nbSegs = 4*(2*n + 1); // Number of segments
    Int_t nbPols = 4*n + 2; // Number of polygons

    if(buffer.SetRawSizes(nbPnts, 3*nbPnts, nbSegs, 3*nbSegs, nbPols, 6*nbPols)){
      buffer.SetSectionsValid(TBuffer3D::kRawSizes);
    } // if
  } // if

  if((reqSections & TBuffer3D::kRaw) && buffer.SectionsValid(TBuffer3D::kRawSizes)){
    SetPoints(buffer.fPnts);
    if(!buffer.fLocalFrame){
      TransformPoints(buffer.fPnts, buffer.NbPnts());
    } // if
    SetSegsAndPols(buffer);
    buffer.SetSectionsValid(TBuffer3D::kRaw);
  } // if

  return buffer;
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::GetMeshNumbers(Int_t& nvert, Int_t& nsegs, Int_t& npols) const
{
  Int_t n = gGeoManager->GetNsegments();

  nvert = 4*(n + 1);
  nsegs = 4*(2*n + 1);
  npols = 4*n + 2;
}

//_____________________________________________________________________________
Int_t AGeoWinstonCone2D::GetNmeshVertices() const
{
  // Return number of vertices of the mesh representation

  Int_t n = gGeoManager->GetNsegments();
  Int_t nbPnts = 4*(n + 1);

  return nbPnts;
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::InspectShape() const
{
  // print shape parameters
  printf("*** Shape %s: AGeoWinstonCone2D ***\n", GetName());
  printf("    R1     = %11.5f\n", fR1);
  printf("    R2     = %11.5f\n", fR2);
  printf(" Bounding box:\n");
  TGeoBBox::InspectShape();
}

//_____________________________________________________________________________
TBuffer3D* AGeoWinstonCone2D::MakeBuffer3D() const
{
  Int_t n = gGeoManager->GetNsegments();
  Int_t nbPnts = 4*(n + 1);
  Int_t nbSegs = 4*(2*n + 1);
  Int_t nbPols = 4*n + 2;

  TBuffer3D* buff = new TBuffer3D(TBuffer3DTypes::kGeneric, nbPnts, 3*nbPnts,
                                  nbSegs, 3*nbSegs, nbPols, 6*nbPols);

  if(buff){
    SetPoints(buff->fPnts);
    SetSegsAndPols(*buff);
  } // if

  return buff;
}

//_____________________________________________________________________________
Double_t AGeoWinstonCone2D::Safety(CONST53410 Double_t*, Bool_t) const
{
  // Not implemented yet. But keep this as is.
  return TGeoShape::Big();
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::SavePrimitive(std::ostream& out, Option_t* )
{
  // Save a primitive as a C++ statement(s) on output stream "out".
  if (TObject::TestBit(kGeoSavePrimitive)) return;

  out << "   // Shape: " << GetName() << " type: " << ClassName() << std::endl;
  out << "   r1 = " << fR1 << ";" << std::endl;
  out << "   r2 = " << fR2 << ";" << std::endl;
  out << "   dy = " << fDY << ";" << std::endl;
  out << "   AGeoWinstonCone2D* cone = new AGeoWinstonCone2D(\"" << GetName() << "\", r1, r2, dy);" << std::endl;

  out << "   TGeoShape* " << GetPointerName() << " = cone;" << std::endl;
  TObject::SetBit(TGeoShape::kGeoSavePrimitive);
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::SetWinstonDimensions(Double_t r1, Double_t r2, Double_t y)
{
  if(TMath::Abs(r1) > TMath::Abs(r2)){
    fR1 = TMath::Abs(r1);
    fR2 = TMath::Abs(r2);
  } else {
    fR1 = TMath::Abs(r2);
    fR2 = TMath::Abs(r1);
  } // if

  fDY = TMath::Abs(y);

  fTheta = TMath::ASin(fR2/fR1);
  fDZ = (fR1 + fR2)/TMath::Tan(fTheta)/2.;

  fF = fR2*(1 + TMath::Sin(fTheta));
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::SetDimensions(Double_t* param)
{
  SetWinstonDimensions(param[0], param[1], param[2]);
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::SetPoints(Double_t* points) const
{
  // create mesh points
  Int_t n = gGeoManager->GetNsegments();

  if(points){
    for(int i = 0; i <= n; i++){
      // see http://cherenkov.physics.iastate.edu/research/LightconeStudies-collector_optimization.pdf
      Double_t t = 2.*fDZ*i/n;
      Double_t z = -fDZ + t;
      Double_t r = CalcR(z);

      points[(i*4 + 0)*3 + 0] = r;
      points[(i*4 + 0)*3 + 1] = fDY;
      points[(i*4 + 0)*3 + 2] = z;

      points[(i*4 + 1)*3 + 0] = -r;
      points[(i*4 + 1)*3 + 1] = fDY;
      points[(i*4 + 1)*3 + 2] = z;

      points[(i*4 + 2)*3 + 0] = -r;
      points[(i*4 + 2)*3 + 1] = -fDY;
      points[(i*4 + 2)*3 + 2] = z;

      points[(i*4 + 3)*3 + 0] = r;
      points[(i*4 + 3)*3 + 1] = -fDY;
      points[(i*4 + 3)*3 + 2] = z;
    } // i
  } // if
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::SetPoints(Float_t* points) const
{
  // create mesh points
  Int_t n = gGeoManager->GetNsegments();

  if(points){
    for(Int_t i = 0; i <= n; i++){
      // see http://cherenkov.physics.iastate.edu/research/LightconeStudies-collector_optimization.pdf
      Double_t t = 2*fDZ*i/n;
      Double_t z = -fDZ + t;
      Double_t r = CalcR(z);

      points[(i*4 + 0)*3 + 0] = r;
      points[(i*4 + 0)*3 + 1] = fDY;
      points[(i*4 + 0)*3 + 2] = z;

      points[(i*4 + 1)*3 + 0] = -r;
      points[(i*4 + 1)*3 + 1] = fDY;
      points[(i*4 + 1)*3 + 2] = z;

      points[(i*4 + 2)*3 + 0] = -r;
      points[(i*4 + 2)*3 + 1] = -fDY;
      points[(i*4 + 2)*3 + 2] = z;

      points[(i*4 + 3)*3 + 0] = r;
      points[(i*4 + 3)*3 + 1] = -fDY;
      points[(i*4 + 3)*3 + 2] = z;
    } // i
  } // if
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::SetSegsAndPols(TBuffer3D& buff) const
{
  // Fill TBuffer3D structure for segments and polygons.

  Int_t n = gGeoManager->GetNsegments();
  Int_t c = GetBasicColor();

  // segments
  Int_t index = 0;
  for(Int_t i = 0; i < n; i++){
    // segments on parabola
    buff.fSegs[index++] = c;
    buff.fSegs[index++] = 4*i;
    buff.fSegs[index++] = 4*i + 4;

    buff.fSegs[index++] = c;
    buff.fSegs[index++] = 4*i + 1;
    buff.fSegs[index++] = 4*i + 5;

    buff.fSegs[index++] = c;
    buff.fSegs[index++] = 4*i + 2;
    buff.fSegs[index++] = 4*i + 6;

    buff.fSegs[index++] = c;
    buff.fSegs[index++] = 4*i + 3;
    buff.fSegs[index++] = 4*i + 7;
  } // i

  for(Int_t i = 0; i <= n; i++){
    // segments parallel to X or Y axis
    buff.fSegs[index++] = c;
    buff.fSegs[index++] = 4*i;
    buff.fSegs[index++] = 4*i + 1;

    buff.fSegs[index++] = c;
    buff.fSegs[index++] = 4*i + 1;
    buff.fSegs[index++] = 4*i + 2;

    buff.fSegs[index++] = c;
    buff.fSegs[index++] = 4*i + 2;
    buff.fSegs[index++] = 4*i + 3;

    buff.fSegs[index++] = c;
    buff.fSegs[index++] = 4*i + 3;
    buff.fSegs[index++] = 4*i;
  } // i

  // polygons
  index = 0;
  for(Int_t i = 0; i < n; i++){
    // polygon parallel to XZ (+Y)
    buff.fPols[index++] = c;
    buff.fPols[index++] = 4;
    buff.fPols[index++] = 4*i;
    buff.fPols[index++] = 4*n + 4*i + 4;
    buff.fPols[index++] = 4*i + 1;
    buff.fPols[index++] = 4*n + 4*i;

    // polygon on parabola (-X)
    buff.fPols[index++] = c;
    buff.fPols[index++] = 4;
    buff.fPols[index++] = 4*i + 1;
    buff.fPols[index++] = 4*n + 4*i + 5;
    buff.fPols[index++] = 4*i + 2;
    buff.fPols[index++] = 4*n + 4*i + 1;

    // polygon parallel to XZ (-Y)
    buff.fPols[index++] = c;
    buff.fPols[index++] = 4;
    buff.fPols[index++] = 4*i + 2;
    buff.fPols[index++] = 4*n + 4*i + 6;
    buff.fPols[index++] = 4*i + 3;
    buff.fPols[index++] = 4*n + 4*i + 2;

    // polygon on parabola (+X)
    buff.fPols[index++] = c;
    buff.fPols[index++] = 4;
    buff.fPols[index++] = 4*i + 3;
    buff.fPols[index++] = 4*n + 4*i + 7;
    buff.fPols[index++] = 4*i;
    buff.fPols[index++] = 4*n + 4*i + 3;
  } // i

  // polygon parallel to XY (-Z)
  buff.fPols[index++] = c;
  buff.fPols[index++] = 4;
  buff.fPols[index++] = 4*n;
  buff.fPols[index++] = 4*n + 1;
  buff.fPols[index++] = 4*n + 2;
  buff.fPols[index++] = 4*n + 3;

  // polygon parallel to XY (+Z)
  buff.fPols[index++] = c;
  buff.fPols[index++] = 4;
  buff.fPols[index++] = 8*n + 3;
  buff.fPols[index++] = 8*n + 2;
  buff.fPols[index++] = 8*n + 1;
  buff.fPols[index++] = 8*n + 0;
}

//_____________________________________________________________________________
void AGeoWinstonCone2D::Sizeof3D() const
{
  ///// obsolete - to be removed
}
