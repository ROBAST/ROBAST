/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AGeoWinstonConePoly
//
// Geometry class for tubes which have two aspheric surface
//
///////////////////////////////////////////////////////////////////////////////

#include "AGeoWinstonConePoly.h"

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

ClassImp(AGeoWinstonConePoly)

//_____________________________________________________________________________
AGeoWinstonConePoly::AGeoWinstonConePoly() : AGeoWinstonCone2D()
{
  // Default constructor
}

//_____________________________________________________________________________
AGeoWinstonConePoly::AGeoWinstonConePoly(Double_t r1, Double_t r2, Int_t n)
  : AGeoWinstonCone2D(r1, r2, 0)
{
  SetWinstonDimensions(r1, r2, n);
  ComputeBBox();
}

//_____________________________________________________________________________
AGeoWinstonConePoly::AGeoWinstonConePoly(const char *name,
                                         Double_t r1, Double_t r2, Int_t n)
  : AGeoWinstonCone2D(name, r1, r2, 0)
{
  SetWinstonDimensions(r1, r2, n);
  ComputeBBox();
}

//_____________________________________________________________________________
AGeoWinstonConePoly::~AGeoWinstonConePoly()
{
  // Destructor
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::ComputeBBox()
{
  // Compute bounding box of the shape
  fDX = fDX;
  fDY = fDY;
  fDZ = fDZ;
  fOrigin[0] = 0;
  fOrigin[1] = 0;
  fOrigin[2] = 0;
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::ComputeNormal(CONST53410 Double_t* point, CONST53410 Double_t* dir,
                                        Double_t* norm)
{
  // Compute normal to closest surface from POINT.

  // Following calculation assumes that the point is very close to surfaces.
  Double_t x = point[0];
  Double_t y = point[1];
  Double_t z = point[2];

  Double_t saf[2];
  saf[0] = TMath::Abs(TMath::Abs(fDZ) - TMath::Abs(z));
  Double_t phi = TMath::ATan2(y, x);
  while(phi > TMath::Pi()/fPolyN){
    phi -= TMath::TwoPi()/fPolyN;
  } // while
  while(phi < -TMath::Pi()/fPolyN){
    phi += TMath::TwoPi()/fPolyN;
  } // while
  try {
    saf[1] = TMath::Abs(CalcR(z) - TMath::Sqrt(x*x + y*y)*TMath::Cos(phi));
  } catch (...) {
    saf[1] = TGeoShape::Big();
  } // try

  Int_t i = TMath::LocMin(2, saf); // find minimum

  if(i == 0){ // on the XY surface
    norm[0] = 0;
    norm[1] = 0;
    norm[2] = 1;
  } else {
    phi = TMath::ATan2(y, x);
    if(phi < -TMath::Pi()/fPolyN){
      phi += TMath::TwoPi();
    } // if
    Int_t n = TMath::Floor((phi + TMath::Pi()/fPolyN)/(TMath::TwoPi()/fPolyN));
    norm[0] = TMath::Cos(n*TMath::TwoPi()/fPolyN);
    norm[1] = TMath::Sin(n*TMath::TwoPi()/fPolyN);
    norm[2] = -CalcdRdZ(z);
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
Bool_t AGeoWinstonConePoly::Contains(CONST53410 Double_t* point) const
{
  // Test if point is in this shape
  Double_t x = point[0];
  Double_t y = point[1];
  Double_t z = point[2];
  if(TMath::Abs(z) > fDZ){
    return kFALSE;
  } // if

  Double_t r = CalcR(z);

  return InsidePolygon(x, y, r);
}

//_____________________________________________________________________________
Int_t AGeoWinstonConePoly::DistancetoPrimitive(Int_t px, Int_t py)
{
  // compute closest distance from point px,py to each corner
  Int_t n = gGeoManager->GetNsegments();

  Int_t numPoints = (n + 1)*fPolyN + 2;

  return ShapeDistancetoPrimitive(numPoints, px, py);
}

//_____________________________________________________________________________
Double_t AGeoWinstonConePoly::DistFromInside(CONST53410 Double_t* point, CONST53410 Double_t* dir,
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
  Double_t d[1 + fPolyN];
  d[0] = TGeoShape::Big();
  if(dir[2] < 0){
    d[0] = (- point[2] - fDZ)/dir[2];
  } else if (dir[2] > 0) {
    d[0] = (fDZ - point[2])/dir[2];
  } // if

  for(Int_t i = 0; i < fPolyN; i++){
    d[1 + i] = DistToParabola(point, dir, i*TMath::TwoPi()/fPolyN, TMath::Pi());
  } // i

  return d[TMath::LocMin(1 + fPolyN, d)];
}

//_____________________________________________________________________________
Double_t AGeoWinstonConePoly::DistFromOutside(CONST53410 Double_t* point, CONST53410 Double_t* dir,
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
    if(InsidePolygon(xnew, ynew, fR2)){
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
    if(InsidePolygon(xnew, ynew, fR1)){
      return snxt;
    } // if
  } // if

  Double_t d[fPolyN];
  for(Int_t i = 0; i < fPolyN; i++){
    d[i] = DistToParabola(point, dir, i*TMath::TwoPi()/fPolyN, TMath::TwoPi()/fPolyN);
  } // i

  return d[TMath::LocMin(fPolyN, d)];
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::GetBoundingCylinder(Double_t* param) const
{
  //--- Fill vector param[4] with the bounding cylinder parameters. The order
  // is the following : Rmin, Rmax, Phi1, Phi2
  param[0] = 0;
  param[1] = TMath::Power(fR1/TMath::Cos(TMath::Pi()/fPolyN), 2);
  param[2] = 0;
  param[3] = 360;
}

//_____________________________________________________________________________
const TBuffer3D& AGeoWinstonConePoly::GetBuffer3D(Int_t reqSections,
                                                  Bool_t localFrame) const
{
  // Fills a static 3D buffer and returns a reference
  static TBuffer3D buffer(TBuffer3DTypes::kGeneric);

  TGeoBBox::FillBuffer3D(buffer, reqSections, localFrame);
  
  if(reqSections & TBuffer3D::kRawSizes){
    Int_t n = gGeoManager->GetNsegments();
    Int_t nbPnts = fPolyN*(n + 1) + 2; // Number of points
    Int_t nbSegs = fPolyN*(2*n + 3); // Number of segments
    Int_t nbPols3 = fPolyN*2; // Number of triangle polygons
    Int_t nbPols4 = fPolyN*n; // Number of rectangle polygons

    if(buffer.SetRawSizes(nbPnts, 3*nbPnts, nbSegs, 3*nbSegs, nbPols3 + nbPols4, 5*nbPols3 + 6*nbPols4)){
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
void AGeoWinstonConePoly::GetMeshNumbers(Int_t& nvert, Int_t& nsegs, Int_t& npols) const
{
  Int_t n = gGeoManager->GetNsegments();

  nvert = fPolyN*(n + 1) + 2;
  nsegs = fPolyN*(2*n + 3);
  npols = fPolyN*(n + 2);
}

//_____________________________________________________________________________
Int_t AGeoWinstonConePoly::GetNmeshVertices() const
{
  // Return number of vertices of the mesh representation

  Int_t n = gGeoManager->GetNsegments();
  Int_t nbPnts = fPolyN*(n + 1) + 2;

  return nbPnts;
}

//_____________________________________________________________________________
Bool_t AGeoWinstonConePoly::InsidePolygon(Double_t x, Double_t y, Double_t r) const
{
  Double_t theta = TMath::ATan2(y, x);

  while(theta > TMath::Pi()/fPolyN){
    theta -= TMath::TwoPi()/fPolyN;
  } // while
  while(theta < -TMath::Pi()/fPolyN){
    theta += TMath::TwoPi()/fPolyN;
  } // while

  if(TMath::Sqrt(x*x + y*y)*TMath::Cos(theta) > r){
    return kFALSE;
  } // if

  return kTRUE;
}
//_____________________________________________________________________________
void AGeoWinstonConePoly::InspectShape() const
{
  // print shape parameters
  printf("*** Shape %s: AGeoWinstonConePoly ***\n", GetName());
  printf("    N      = %d\n", fPolyN);
  printf(" 2D Base:\n");
  AGeoWinstonCone2D::InspectShape();
}

//_____________________________________________________________________________
TBuffer3D* AGeoWinstonConePoly::MakeBuffer3D() const
{
  Int_t n = gGeoManager->GetNsegments();
  Int_t nbPnts = fPolyN*(n + 1) + 2;
  Int_t nbSegs = fPolyN*(2*n + 3);
  Int_t nbPols3 = fPolyN*2;
  Int_t nbPols4 = fPolyN*n;

  TBuffer3D* buff = new TBuffer3D(TBuffer3DTypes::kGeneric, nbPnts, 3*nbPnts,
                                  nbSegs, 3*nbSegs, nbPols3 + nbPols4, 5*nbPols3 + 6*nbPols4);

  if(buff){
    SetPoints(buff->fPnts);
    SetSegsAndPols(*buff);
  } // if

  return buff;
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::SavePrimitive(std::ostream& out, Option_t* )
{
  // Save a primitive as a C++ statement(s) on output stream "out".
  if (TObject::TestBit(kGeoSavePrimitive)) return;

  out << "   // Shape: " << GetName() << " type: " << ClassName() << std::endl;
  out << "   r1 = " << fR1 << ";" << std::endl;
  out << "   r2 = " << fR2 << ";" << std::endl;
  out << "   n  = " << fPolyN << ";" << std::endl;
  out << "   AGeoWinstonConePoly* cone = new AGeoWinstonConePoly(\"" << GetName() << "\", r1, r2, n);" << std::endl;

  out << "   TGeoShape* " << GetPointerName() << " = cone;" << std::endl;
  TObject::SetBit(TGeoShape::kGeoSavePrimitive);
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::SetWinstonDimensions(Double_t r1, Double_t r2, Int_t n)
{
  if(TMath::Abs(r1) > TMath::Abs(r2)){
    fR1 = TMath::Abs(r1);
    fR2 = TMath::Abs(r2);
  } else {
    fR1 = TMath::Abs(r2);
    fR2 = TMath::Abs(r1);
  } // if

  fPolyN = n >= 3 ? n : 3;

  Double_t r = r1/TMath::Cos(TMath::Pi()/n);

  fDX = 0;
  for(Int_t i = 0; i < fPolyN; i++){
    fDX = TMath::Max(TMath::Abs(r*TMath::Cos(TMath::Pi()/n*(2*i + 1))), fDX);
  } // i

  fDY = 0;
  for(Int_t i = 0; i < fPolyN; i++){
    fDY = TMath::Max(TMath::Abs(r*TMath::Sin(TMath::Pi()/n*(2*i + 1))), fDY);
  } // i

  fTheta = TMath::ASin(fR2/fR1);
  fDZ = (fR1 + fR2)/TMath::Tan(fTheta)/2.;

  fF = fR2*(1 + TMath::Sin(fTheta));
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::SetDimensions(Double_t* param)
{
  SetWinstonDimensions(param[0], param[1], param[2]);
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::SetPoints(Double_t* points) const
{
  // create mesh points
  if(!points){
    return;
  } // if

  Int_t n = gGeoManager->GetNsegments();

  Int_t index = 0;
  for(int i = 0; i <= n; i++){
    // see http://cherenkov.physics.iastate.edu/research/LightconeStudies-collector_optimization.pdf
    Double_t t = 2.*fDZ*i/n;
    Double_t z = -fDZ + t;
    Double_t r = CalcR(z)/TMath::Cos(TMath::Pi()/fPolyN);

    for(Int_t j = 0; j < fPolyN; j++){
      Double_t theta = (j + 0.5)*TMath::TwoPi()/fPolyN;
      Double_t x = r*TMath::Cos(theta);
      Double_t y = r*TMath::Sin(theta);
      points[index++] = x;
      points[index++] = y;
      points[index++] = z;
    } // j
  } // i

  points[index++] = 0;
  points[index++] = 0;
  points[index++] = -fDZ;

  points[index++] = 0;
  points[index++] = 0;
  points[index++] = fDZ;
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::SetPoints(Float_t* points) const
{
  // create mesh points
  if(!points){
    return;
  } // if

  Int_t n = gGeoManager->GetNsegments();

  Int_t index = 0;
  for(int i = 0; i <= n; i++){
    // see http://cherenkov.physics.iastate.edu/research/LightconeStudies-collector_optimization.pdf
    Double_t t = 2.*fDZ*i/n;
    Double_t z = -fDZ + t;
    Double_t r = CalcR(z)/TMath::Cos(TMath::TwoPi()/fPolyN);;

    for(Int_t j = 0; j < fPolyN; j++){
      Double_t theta = (j + 0.5)*TMath::TwoPi()/fPolyN;
      Double_t x = r*TMath::Cos(theta);
      Double_t y = r*TMath::Sin(theta);
      points[index++] = x;
      points[index++] = y;
      points[index++] = z;
    } // j
  } // i

  points[index++] = 0;
  points[index++] = 0;
  points[index++] = -fDZ;

  points[index++] = 0;
  points[index++] = 0;
  points[index++] = fDZ;
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::SetSegsAndPols(TBuffer3D& buff) const
{
  // Fill TBuffer3D structure for segments and polygons.

  Int_t n = gGeoManager->GetNsegments();
  Int_t c = GetBasicColor();

  // segments
  Int_t index = 0;
  for(Int_t i = 0; i < n; i++){
    // segments on parabola�
    for(Int_t j = 0; j < fPolyN; j++){
      buff.fSegs[index++] = c;
      buff.fSegs[index++] = fPolyN*i + j;
      buff.fSegs[index++] = fPolyN*(i + 1) + j;
    } // j
  } // i

  for(Int_t i = 0; i <= n; i++){
    // segments parallel to X-Y plane
    for(Int_t j = 0; j < fPolyN; j++){
      buff.fSegs[index++] = c;
      buff.fSegs[index++] = fPolyN*i + j;
      buff.fSegs[index++] = j == (fPolyN - 1) ? fPolyN*i : fPolyN*i + j + 1;
    } // j
  } // i

  for(Int_t i = 0; i < fPolyN; i++){
    buff.fSegs[index++] = c;
    buff.fSegs[index++] = i;
    buff.fSegs[index++] = fPolyN*(n + 1);
  } // i

  for(Int_t i = 0; i < fPolyN; i++){
    buff.fSegs[index++] = c;
    buff.fSegs[index++] = fPolyN*(n + 1) - fPolyN + i;
    buff.fSegs[index++] = fPolyN*(n + 1) + 1;
  } // i

  // polygons
  index = 0;
  for(Int_t i = 0; i < n; i++){
    for(Int_t j = 0; j < fPolyN; j++){
      // polygon on parabola
      buff.fPols[index++] = c;
      buff.fPols[index++] = 4;
      buff.fPols[index++] = fPolyN*i + j; // this is a segment number
      buff.fPols[index++] = fPolyN*n + fPolyN*i + j + fPolyN;
      buff.fPols[index++] = j == (fPolyN - 1) ? fPolyN*i : fPolyN*i + j + 1;
      buff.fPols[index++] = fPolyN*n + fPolyN*i + j;
    } // j
  } // i

  // polygon at -fDZ
  for(Int_t i = 0; i < fPolyN; i++){
    buff.fPols[index++] = c;
    buff.fPols[index++] = 3;
    buff.fPols[index++] = fPolyN*n + i;
    buff.fPols[index++] = i == (fPolyN - 1) ? fPolyN*(2*n + 1) + i + 1 - fPolyN : fPolyN*(2*n + 1) + i + 1;
    buff.fPols[index++] = fPolyN*(2*n + 1) + i;
  } // i

  // polygon at +fDZ
  for(Int_t i = 0; i < fPolyN; i++){
    buff.fPols[index++] = c;
    buff.fPols[index++] = 3;
    buff.fPols[index++] = fPolyN*2*n + i;
    buff.fPols[index++] = fPolyN*(2*n + 2) + i;
    buff.fPols[index++] = i == (fPolyN - 1) ? fPolyN*(2*n + 2) + i + 1 - fPolyN : fPolyN*(2*n + 2) + i + 1;
  } // i
}

//_____________________________________________________________________________
void AGeoWinstonConePoly::Sizeof3D() const
{
  ///// obsolete - to be removed
}
