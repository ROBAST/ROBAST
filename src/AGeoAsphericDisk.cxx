/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AGeoAsphericDisk
//
// Geometry class for tubes which have two aspheric surface
//
///////////////////////////////////////////////////////////////////////////////

#include "AGeoAsphericDisk.h"

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

ClassImp(AGeoAsphericDisk)

//_____________________________________________________________________________
AGeoAsphericDisk::AGeoAsphericDisk()
 : fConic1(0), fConic2(0), fKappa1(1), fKappa2(1),
   fNPol1(0), fNPol2(0), fK1(0), fK2(0), fSteps(100), fRepeat(4)
{
  // Default constructor
  SetShapeBit(TGeoShape::kGeoBox);
  SetAsphDimensions(0, 0, 0, 0, 0, 0);
  ComputeBBox();
}

//_____________________________________________________________________________
AGeoAsphericDisk::AGeoAsphericDisk(Double_t z1, Double_t curve1,
                                   Double_t z2, Double_t curve2,
                                   Double_t rmax, Double_t rmin)
  : TGeoBBox(0, 0, 0), fConic1(0), fConic2(0), fKappa1(1), fKappa2(1),
    fNPol1(0), fNPol2(0), fK1(0), fK2(0), fSteps(100), fRepeat(4)
{
  SetShapeBit(TGeoShape::kGeoBox);
  SetAsphDimensions(z1, curve1, z2, curve2, rmax, rmin);
  ComputeBBox();
}

//_____________________________________________________________________________
AGeoAsphericDisk::AGeoAsphericDisk(const char *name,
                                   Double_t z1, Double_t curve1,
                                   Double_t z2, Double_t curve2,
                                   Double_t rmax, Double_t rmin)
  : TGeoBBox(name, 0, 0, 0), fConic1(0), fConic2(0), fKappa1(1), fKappa2(1),
    fNPol1(0), fNPol2(0), fK1(0), fK2(0), fSteps(100), fRepeat(4)
{
  SetShapeBit(TGeoShape::kGeoBox);
  SetAsphDimensions(z1, curve1, z2, curve2, rmax, rmin);
  ComputeBBox();
}

//_____________________________________________________________________________
AGeoAsphericDisk::~AGeoAsphericDisk()
{
  // Destructor
  DeleteArrays();
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::CalcdF1dr(Double_t r) const throw(std::exception)
{
  // Calculate dF1/dr
  Double_t p = r*r*fCurve1*fCurve1*fKappa1;
  if(1 - p <= 0) throw std::exception();

  Double_t ret = r*fCurve1/sqrt(1 - p);

  for(Int_t i = 0; i < fNPol1; i++){
    ret += 2*(i+1)*fK1[i]*TMath::Power(r, 2*(i+1)-1);
  } // i

  return ret;
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::CalcdF2dr(Double_t r) const throw(std::exception)
{
  // Calculate dF2/dr
  Double_t p = r*r*fCurve2*fCurve2*fKappa2;
  if(1 - p <= 0) throw std::exception();

  Double_t ret = r*fCurve2/sqrt(1 - p);

  for(Int_t i = 0; i < fNPol2; i++){
    ret += 2*(i+1)*fK2[i]*pow(r, 2*(i+1)-1);
  } // i

  return ret;
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::CalcF1(Double_t r) const throw(std::exception)
{
  // Calculate z value of surface 1 at given r
  Double_t p = r*r*fCurve1*fCurve1*fKappa1;
  if(1 - p < 0) throw std::exception();

  Double_t ret = fZ1 + r*r*fCurve1/(1 + sqrt(1 - p));

  for(Int_t i = 0; i < fNPol1; i++){
    ret += fK1[i]*pow(r, 2*(i+1));
  } // i

  return ret;
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::CalcF2(Double_t r) const throw(std::exception)
{
  // Calculate z value of surface 1 at given r
  Double_t p = r*r*fCurve2*fCurve2*fKappa2;
  if(1 - p < 0) throw std::exception();

  Double_t ret = fZ2 + r*r*fCurve2/(1 + sqrt(1 - p));

  for(Int_t i = 0; i < fNPol2; i++){
    ret += fK2[i]*pow(r, 2*(i+1));
  } // i

  return ret;
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::Capacity() const
{
  // Compute capacity of the shape in [length^3]
  // Not implemeted yet

  return 0;
}

//_____________________________________________________________________________
void AGeoAsphericDisk::ComputeBBox()
{
  // Compute bounding box of the shape
  Double_t zmax = -TGeoShape::Big();
  if(fNPol2 == 0){
    Double_t f1, f2;
    try{
      f1 = CalcF2(fRmin);
      f2 = CalcF2(fRmax);
    } catch (...) {
      f1 = TGeoShape::Big();
      f2 = TGeoShape::Big();
    } // try
    zmax = f1 > f2 ? f1 : f2;
  } else {
    Double_t r1 = fRmin;
    Double_t r2 = fRmax;
    for(Int_t i = 0; i < fRepeat; i++){
      Double_t step = (r2 - r1)/fSteps;
      Double_t r_ = r1;
      for(Int_t j = 0; j <= fSteps + 1; j++){
        Double_t r = r1 + j*step;
        Double_t f;
        try {
          f = CalcF2(r);
        } catch (...) {
          f = -TGeoShape::Big();
        } // try
        if(f > zmax){
          zmax = f;
          r_ = r;
        } // if
      } // j
      r1 = r_==fRmin ? fRmin : r_ - step;
      r2 = r_==fRmax ? fRmax : r_ + step;
    } // i
  } // if

  Double_t zmin = TGeoShape::Big();
  if(fNPol1 == 0){
    Double_t f1, f2;
    try {
      f1 = CalcF1(fRmin);
      f2 = CalcF1(fRmax);
    } catch (...) {
      f1 = -TGeoShape::Big();
      f2 = -TGeoShape::Big();
    } // try
    zmin = f1 < f2 ? f1 : f2;
  } else {
    Double_t r1 = fRmin;
    Double_t r2 = fRmax;
    for(Int_t i = 0; i < fRepeat; i++){
      Double_t step = (r2 - r1)/fSteps;
      Double_t r_ = r1;
      for(Int_t j = 0; j <= fSteps + 1; j++){
        Double_t r = r1 + j*step;
        Double_t f;
        try {
          f = CalcF1(r);
        } catch (...) {
          f = TGeoShape::Big();
        } // try
        if(f < zmin){
          zmin = f;
          r_ = r;
        } // if
      } // r
      r1 = r_ == fRmin ? fRmin : r_ - step;
      r2 = r_ == fRmax ? fRmax : r_ + step;
    } // i
  } // if

  fOrigin[0] = 0;
  fOrigin[1] = 0;
  fOrigin[2] = (zmax + zmin)/2;;
  fDX = fRmax;
  fDY = fRmax;
  fDZ = (zmax - zmin)/2;
}

//_____________________________________________________________________________
void AGeoAsphericDisk::ComputeNormal(CONST53410 Double_t* point, CONST53410 Double_t* dir,
                                     Double_t* norm)
{
  // Compute normal to closest surface from POINT.

  // Following calculation assumes that the point is very close to surfaces.

  Double_t r = sqrt(point[0]*point[0] + point[1]*point[1]);
  Double_t phi = atan2(point[1], point[0]);

  Double_t saf[4];
  saf[0] = TestShapeBit(kGeoRSeg) ? TMath::Abs(r - fRmin) : TGeoShape::Big();
  saf[1] = TMath::Abs(r - fRmax);

  Double_t f1, f2, df1, df2;
  try {
    f1 = CalcF1(r);
  } catch (...) {
    f1 = -TGeoShape::Big();
  } // try
  if(f1 == -TGeoShape::Big()){
    saf[2] = TGeoShape::Big();
  } else {
    try {
      df1 = CalcdF1dr(r);
      saf[2] = TMath::Abs(f1 - point[2])/sqrt(1 + df1*df1);
    } catch (...) {
      saf[2] = TGeoShape::Big();
    } // try
  } // if

  try {
    f2 = CalcF2(r);
  } catch (...) {
    f2 = TGeoShape::Big();
  } // try
  if(f2 == TGeoShape::Big()){
    saf[3] = TGeoShape::Big();
  } else {
    try {
      df2 = CalcdF2dr(r);
      saf[3] = TMath::Abs(f2 - point[2])/sqrt(1 + df2*df2);
    } catch (...) {
      saf[3] = TGeoShape::Big();
    } // try
  } // if

  Int_t i = TMath::LocMin(4, saf); // find minimum

  memset(norm, 0, 3*sizeof(Double_t));
  if(i == 0 or i == 1){
    norm[0] = 1;
  } else if(i == 2){
    if(df1 == 0){
      norm[2] = 1;
    } else {
      norm[0] = df1/sqrt(1 + df1*df1);
      norm[2] = -1/sqrt(1 + df1*df1);
    } // if
  } else {
    if(df2 == 0){
      norm[2] = 1;
    } else {
      norm[0] = df2/sqrt(1 + df2*df2);
      norm[2] = -1/sqrt(1 + df2*df2);
    } // if
  } // if

  TVector3 vec(norm);
  vec.RotateZ(phi);

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
Bool_t AGeoAsphericDisk::Contains(CONST53410 Double_t* point) const
{
  // Test if point is in this shape

  Double_t r = sqrt(point[0]*point[0] + point[1]*point[1]);
  if(r > fRmax or r < fRmin) return kFALSE;

  Double_t f1, f2;
  try {
    f1 = CalcF1(r);
    f2 = CalcF2(r);
  } catch (...) {
    return kFALSE;
  } // try

  if(point[2] < f1 or f2 < point[2]) return kFALSE;

  return kTRUE;
}

//_____________________________________________________________________________
Int_t AGeoAsphericDisk::DistancetoPrimitive(Int_t px, Int_t py)
{
  // compute closest distance from point px,py to each corner
  Int_t n = gGeoManager->GetNsegments();

  Int_t numPoints = 2*n*(n + 1);
  if(!TestShapeBit(kGeoRSeg)){
    numPoints = 2*(n*n + 1);
  } // if

  return ShapeDistancetoPrimitive(numPoints, px, py);
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::DistFromInside(CONST53410 Double_t* point, CONST53410 Double_t* dir,
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
  Double_t d[4];
  d[0] = DistToAsphere(1, point, dir);
  d[1] = DistToAsphere(2, point, dir);
  d[2] = DistToInner(point, dir);
  d[3] = DistToOuter(point, dir);

  return d[TMath::LocMin(4, d)];
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::DistFromOutside(CONST53410 Double_t* point, CONST53410 Double_t* dir,
                                           Int_t iact, Double_t step,
                                           Double_t* safe) const
{
  // compute distance from outside point to surface of the sphere

  // Check if the bounding box is crossed within the requested distance
  Double_t point_[3] = {point[0], point[1], point[2] - fOrigin[2]};
  Double_t sdist = TGeoTube::DistFromOutsideS(point_, dir, fRmin, fRmax, fDZ);
  if(sdist >= step) return TGeoShape::Big();

  // compute safe distance
  if(iact < 3 and safe){
    *safe = Safety(point, kFALSE);
    if (iact == 0) return TGeoShape::Big();
    if (iact == 1 && step < *safe) return TGeoShape::Big();
  } // if

  // calculate distance
  Double_t d[4];
  d[0] = DistToAsphere(1, point, dir);
  d[1] = DistToAsphere(2, point, dir);
  d[2] = DistToInner(point, dir);
  d[3] = DistToOuter(point, dir);

  return d[TMath::LocMin(4, d)];
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::DistToAsphere(Int_t n, CONST53410 Double_t* point, CONST53410 Double_t* dir) const
{
  if(n!=1 and n!=2) return TGeoShape::Big();

  // following calculations are based on Y.Matsui's text book pp 42-45
  // Matsui (x, y, z) -> in this function (z, x, y)
  // !WARNING! this method cannot handle with large incident angles nor
  // complicated surfaces which have many extremums. I strongly recommend you
  // to check 3D view of rays. In most cases using Ashra optics, this will not
  // limit your application.
  Double_t H2 = point[0]*point[0] + point[1]*point[1];
  Double_t d = n==1 ? fZ1 : fZ2;
  Double_t p = -((point[2] - d)*dir[2] + point[0]*dir[0] + point[1]*dir[1]);
  Double_t M = p*dir[2] + point[2] - d;
  Double_t M2 = (point[2] - d)*(point[2] - d) + H2 - p*p;

  Double_t check = n == 1
    ? 1 - (M2*fCurve1 - 2*M)*fCurve1/dir[2]/dir[2]
    : 1 - (M2*fCurve2 - 2*M)*fCurve2/dir[2]/dir[2];

  if(check < 0){
    // Actually, the ray still has a chance to cross the surface. But this is
    // the limitation of this function.
    return TGeoShape::Big();
  } // if

  Double_t q = n == 1
    ? p + (M2*fCurve1 - 2*M)/(dir[2]*(1 + TMath::Sqrt(check)))
    : p + (M2*fCurve2 - 2*M)/(dir[2]*(1 + TMath::Sqrt(check)));

  Double_t npoint[3] = {point[0] + q*dir[0],
                        point[1] + q*dir[1],
                        point[2] + q*dir[2] - d};

  for(Int_t i = 0;; i++){
    if(i > 100){
      return TGeoShape::Big();
    } // if
    H2 = npoint[0]*npoint[0] + npoint[1]*npoint[1];
    check = n == 1 ? (1 - fKappa1*H2*fCurve1*fCurve1) : (1 - fKappa2*H2*fCurve2*fCurve2);
    if(check < 0){
      // This is a limitation too
      return TGeoShape::Big();
    } // if

    Double_t l = sqrt(check);

    Double_t x = 0;
    if(n == 1){
      if(fCurve1 != 0) x += (1 - l)/fCurve1/fKappa1;
      for(Int_t j = 0; j < fNPol1; j++){
        x += fK1[j]*TMath::Power(H2, j + 1);
      } // j
    } else {
      if(fCurve2 != 0) x += (1 - l)/fCurve2/fKappa2;
      for(int j = 0; j < fNPol2; j++){
        x += fK2[j]*TMath::Power(H2, j + 1);
      } // j
    } // if

    Double_t v = 0;
    if(n == 1){
      for(int j = 0; j < fNPol1; j++){
        v += 2*(j + 1)*fK1[j]*TMath::Power(H2, j);
      } // i
      v = fCurve1*fKappa1 + l*v;
    } else {
      for(Int_t j = 0; j < fNPol2; j++){
        v += 2*(j + 1)*fK2[j]*TMath::Power(H2, j);
      } // i
      v = fCurve2*fKappa2 + l*v;
    } // if

    Double_t m = -npoint[0]*v;
    Double_t n = -npoint[1]*v;
    Double_t norm = TMath::Sqrt(l*l + m*m + n*n);
    l /= norm;
    m /= norm;
    n /= norm;

    check = dir[2]*l + dir[0]*m + dir[1]*n;
    if(check == 0){
      return TGeoShape::Big();
    } // if

    Double_t e = l*(x - npoint[2])/check;

    for(Int_t j = 0; j < 3; j++){
      npoint[j] += e*dir[j];
    } // j

    if(TMath::Abs(e) < 1e-10) break;
  } // i

  npoint[2] += d;
  check = dir[0]*(npoint[0] - point[0]) + dir[1]*(npoint[1] - point[1])
    + dir[2]*(npoint[2] - point[2]);
  if(check < 0){
    return TGeoShape::Big();
  } // if

  Double_t dist_to_zaxis = TMath::Power(npoint[0]*npoint[0] + npoint[1]*npoint[1], 0.5);
  if(dist_to_zaxis < fRmin or dist_to_zaxis > fRmax){
    return TGeoShape::Big();
  }

  return TMath::Sqrt(TMath::Power(npoint[0] - point[0], 2) +
                     TMath::Power(npoint[1] - point[1], 2) +
                     TMath::Power(npoint[2] - point[2], 2));
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::DistToInner(CONST53410 Double_t* point, CONST53410 Double_t* dir) const
{
  if(!TestShapeBit(kGeoRSeg)){
    return TGeoShape::Big();
  } // if

  Double_t rsq = point[0]*point[0] + point[1]*point[1];
  Double_t nsq = dir[0]*dir[0] + dir[1]*dir[1];
  if(TMath::Sqrt(nsq) < TGeoShape::Tolerance()){
    return TGeoShape::Big();
  } // if
  Double_t rdotn = point[0]*dir[0] + point[1]*dir[1];
  Double_t b, delta;
  TGeoTube::DistToTube(rsq, nsq, rdotn, fRmin, b, delta);

  if(delta < 0){
    return TGeoShape::Big();
  } // if

  // new points are x[] = (-b +/- delta)*dir[] + point[]
  Double_t t1 = - b + delta;
  Double_t t2 = - b - delta;

  if(t1 < 0 and t2 < 0){
    return TGeoShape::Big();
  } // if

  Double_t zmin = CalcF1(fRmin);
  Double_t zmax = CalcF2(fRmin);

  if(t2 > 0){
    if(t1 > 0){
      Double_t z1 = t1*dir[2] + point[2];
      Double_t z2 = t2*dir[2] + point[2];
      if(z1 < zmin or zmax < z1){
        t1 = TGeoShape::Big();
      } // if
      if(z2 < zmin or zmax < z2){
        t2 = TGeoShape::Big();
      } // if
      
      return t1 < t2 ? t1 : t2;
    } else {
      // never happens, because t1 > t2 > 0
    } // if
  } else if(t2 == 0){
    if(t1 > 0){
      if(zmin <= point[2] and point[2] <= zmax){
        return 0;
      } else {
        Double_t z1 = t1*dir[2] + point[2];
        if(zmin <= z1 and z1 <= zmax){
          return t1;
        } // if
      } // if
    } else if(t1 == 0){
      if(zmin <= point[2] and point[2] <= zmax){
        return 0;
      } // if
    } else {
      // never happens, because t1 > t2 = 0
    } // if
  } else { // t2 < 0
    if(t1 > 0){
      Double_t z1 = t1*dir[2] + point[2];
      if(zmin <= z1 and z1 <= zmax){
        return t1;
      } // if
    } else if(t1 == 0){
      if(zmin <= point[2] and point[2] <= zmax){
        return 0;
      } // if
    } else { // t1 < 0
      // do nothing here, because this condition has already returned Big
    } // if
  } // if

  return TGeoShape::Big();
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::DistToOuter(CONST53410 Double_t* point, CONST53410 Double_t* dir) const
{
  Double_t rsq = point[0]*point[0] + point[1]*point[1];
  Double_t nsq = dir[0]*dir[0] + dir[1]*dir[1];
  if(TMath::Sqrt(nsq) < TGeoShape::Tolerance()){
    return TGeoShape::Big();
  } // if
  Double_t rdotn = point[0]*dir[0] + point[1]*dir[1];
  Double_t b, delta;
  TGeoTube::DistToTube(rsq, nsq, rdotn, fRmax, b, delta);

  if(delta < 0){
    return TGeoShape::Big();
  } // if

  // new points are x[] = (-b +/- delta)*dir[] + point[]
  Double_t t1 = - b + delta;
  Double_t t2 = - b - delta;

  if(t1 < 0 and t2 < 0){
    return TGeoShape::Big();
  } // if

  Double_t zmin = CalcF1(fRmax);
  Double_t zmax = CalcF2(fRmax);

  if(t2 > 0){
    if(t1 > 0){
      Double_t z1 = t1*dir[2] + point[2];
      Double_t z2 = t2*dir[2] + point[2];
      if(z1 < zmin or zmax < z1){
        t1 = TGeoShape::Big();
      } // if
      if(z2 < zmin or zmax < z2){
        t2 = TGeoShape::Big();
      } // if
      
      return t1 < t2 ? t1 : t2;
    } else {
      // never happens, because t1 > t2 > 0
    } // if
  } else if(t2 == 0){
    if(t1 > 0){
      if(zmin <= point[2] and point[2] <= zmax){
        return 0;
      } else {
        Double_t z1 = t1*dir[2] + point[2];
        if(zmin <= z1 and z1 <= zmax){
          return t1;
        } // if
      } // if
    } else if(t1 == 0){
      if(zmin <= point[2] and point[2] <= zmax){
        return 0;
      } // if
    } else {
      // never happens, because t1 > t2 = 0
    } // if
  } else { // t2 < 0
    if(t1 > 0){
      Double_t z1 = t1*dir[2] + point[2];
      if(zmin <= z1 and z1 <= zmax){
        return t1;
      } // if
    } else if(t1 == 0){
      if(zmin <= point[2] and point[2] <= zmax){
        return 0;
      } // if
    } else { // t1 < 0
      // do nothing here, because this condition has already returned Big
    } // if
  } // if

  return TGeoShape::Big();
}

//_____________________________________________________________________________
TGeoVolume* AGeoAsphericDisk::Divide(TGeoVolume*, const char*, Int_t, Int_t,
                                     Double_t, Double_t)
{
  Error("Divide", "Division of a aspheric disk not implemented");
  return 0;
}

//_____________________________________________________________________________
void AGeoAsphericDisk::GetBoundingCylinder(Double_t* param) const
{
  //--- Fill vector param[4] with the bounding cylinder parameters. The order
  // is the following : Rmin, Rmax, Phi1, Phi2
  param[0] = fRmin;
  param[1] = fRmax;
  param[2] = 0;
  param[3] = 360;
}

//_____________________________________________________________________________
const TBuffer3D& AGeoAsphericDisk::GetBuffer3D(Int_t reqSections,
                                               Bool_t localFrame) const
{
  // Fills a static 3D buffer and returns a reference
  static TBuffer3D buffer(TBuffer3DTypes::kGeneric);

  TGeoBBox::FillBuffer3D(buffer, reqSections, localFrame);

  if(reqSections & TBuffer3D::kRawSizes){
    Int_t n = gGeoManager->GetNsegments();
    Int_t nbPnts = 2*n*(n + 1); // Number of points
    Int_t nbSegs = 4*n*(n + 1); // Number of segments
    Int_t nbPols = 2*n*(n + 1); // Number of polygons

    if(!TestShapeBit(kGeoRSeg)){
      nbPnts = 2*(n*n + 1);
      nbSegs = n*(4*n + 1);
      nbPols = n*(2*n + 1);
    } // if

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
void AGeoAsphericDisk::GetMeshNumbers(Int_t& nvert, Int_t& nsegs, Int_t& npols) const
{
  Int_t n = gGeoManager->GetNsegments();

  if(TestShapeBit(kGeoRSeg)){
    nvert = 2*n*(n + 1);
    nsegs = 4*n*(n + 1);
    npols = 2*n*(n + 1);
  } else {
    nvert = 2*(n*n + 1);
    nsegs = n*(4*n + 1);
    npols = n*(2*n + 1);
  } // if
}

//_____________________________________________________________________________
Int_t AGeoAsphericDisk::GetNmeshVertices() const
{
  // Return number of vertices of the mesh representation
  Int_t n = gGeoManager->GetNsegments();
  Int_t nbPnts = 2*n*(n + 1);
  if(!TestShapeBit(kGeoRSeg)){
    nbPnts = 2*(n*n + 1);
  } // if

  return nbPnts;
}

//_____________________________________________________________________________
void AGeoAsphericDisk::InspectShape() const
{
  // print shape parameters
  printf("*** Shape %s: AGeoAsphericDisk ***\n", GetName());
  printf("    Z1     = %11.5f\n", fZ1);
  printf("    Z2     = %11.5f\n", fZ2);
  printf("    Curve1 = %11.5f\n", fCurve1);
  printf("    Curve2 = %11.5f\n", fCurve2);
  printf("    Rmin   = %11.5f\n", fRmin);
  printf("    Rmax   = %11.5f\n", fRmax);
  printf("    NPol1  = %d\n", fNPol1);
  printf("    NPol2  = %d\n", fNPol2);
  printf("    K1:");
  for(Int_t i=0; i<fNPol1; i++){
    printf("    %d: %11.5f\n", (i+1)*2, fK1[i]);
  } // i
  printf("    K2:");
  for(Int_t i=0; i<fNPol2; i++){
    printf("    %d: %11.5f\n", (i+1)*2, fK2[i]);
  } // i
  printf(" Bounding box:\n");
  TGeoBBox::InspectShape();
}

//_____________________________________________________________________________
TBuffer3D* AGeoAsphericDisk::MakeBuffer3D() const
{
  Int_t n = gGeoManager->GetNsegments();
  Int_t nbPnts = 2*n*(n + 1); // Number of points
  Int_t nbSegs = 4*n*(n + 1); // Number of segments
  Int_t nbPols = 2*n*(n + 1); // Number of polygons

  if(!TestShapeBit(kGeoRSeg)){
    nbPnts = 2*(n*n + 1);
    nbSegs = n*(4*n + 1);
    nbPols = n*(2*n + 1);
  } // if

  TBuffer3D* buff = new TBuffer3D(TBuffer3DTypes::kGeneric, nbPnts, 3*nbPnts,
                                  nbSegs, 3*nbSegs, nbPols, 6*nbPols);

  if(buff){
    SetPoints(buff->fPnts);
    SetSegsAndPols(*buff);
  } // if

  return buff;
}

//_____________________________________________________________________________
Double_t AGeoAsphericDisk::Safety(CONST53410 Double_t* point, Bool_t in) const
{
  Double_t safe;

  Double_t rad2 = point[0]*point[0] + point[1]*point[1];
  Double_t rad  = sqrt(rad2);

  Double_t dist[4];

  if(!in){
    Double_t f1rmax, f1rmin, f2rmax, f2rmin;
    try { f1rmax = CalcF1(fRmax);} catch (...) { f1rmax = -TGeoShape::Big();}
    try { f1rmin = CalcF1(fRmin);} catch (...) { f1rmin = -TGeoShape::Big();}
    try { f2rmax = CalcF2(fRmax);} catch (...) { f2rmax =  TGeoShape::Big();}
    try { f2rmin = CalcF2(fRmin);} catch (...) { f2rmin =  TGeoShape::Big();}

    if(rad < fRmin and (f1rmin < point[2] or point[2] < f2rmin)){
      return fRmin - rad;
    } else if(rad > fRmax and (f1rmax < point[2] or point[2] < f2rmax)){
      return rad - fRmax;
    } // if
  } // if

  // calc distance between point and lower surface
  Double_t r1 = fRmin;
  Double_t r2 = fRmax;
  dist[0] = TGeoShape::Big();
  for(Int_t i = 0; i < fRepeat; i++){
    Double_t step = (r2 - r1)/fSteps;
    Double_t r_ = r1;
    for(Int_t j = 0; j <= fSteps+1; j++){
      Double_t r = r1 + j*step;
      Double_t f;
      try {
        f = CalcF1(r);
      } catch (...) {
        f = -TGeoShape::Big();
      } // try
      Double_t d2 = (f - point[2])*(f - point[2]) + (r - rad)*(r - rad);
      if(d2 < dist[0]){
        dist[0] = d2;
        r_ = r;
      } // if
    } // r
    r1 = r_==fRmin ? fRmin : r_ - step;
    r2 = r_==fRmax ? fRmax : r_ + step;
  } // i
  dist[0] = TMath::Sqrt(dist[0]);

  // calc distance between point and upper surface
  r1 = fRmin;
  r2 = fRmax;
  dist[1] = TGeoShape::Big();
  for(Int_t i = 0; i < fRepeat; i++){
    Double_t step = (r2 - r1)/fSteps;
    Double_t r_ = r1;
    for(Int_t j = 0; j <= fSteps+1; j++){
      Double_t r = r1 + j*step;
      Double_t f;
      try {
    f = CalcF2(r);
      } catch (...) {
    f = TGeoShape::Big();
      } // try
      Double_t d2 = (f - point[2])*(f - point[2]) + (r - rad)*(r - rad);
      if(d2 < dist[1]){
    dist[1] = d2;
    r_ = r;
      } // if
    } // r
    r1 = r_==fRmin ? fRmin : r_ - step;
    r2 = r_==fRmax ? fRmax : r_ + step;
  } // i
  dist[1] = sqrt(dist[1]);

  if(in){
    dist[2] = rad - fRmin; // distance to rmin
    dist[3] = fRmax - rad; // distance to rmax

    safe = dist[0];
    for(Int_t i = 1; i < 4; i++){
      safe = dist[i] < safe ? dist[i] : safe;
    } // i
  } else {
    safe = dist[0] < dist[1] ? dist[0] : dist[1];
  } // if

  return safe;
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SavePrimitive(std::ostream& out, Option_t* )
{
  // Save a primitive as a C++ statement(s) on output stream "out".
  if (TObject::TestBit(kGeoSavePrimitive)) return;

  out << "   // Shape: " << GetName() << " type: " << ClassName() << std::endl;
  out << "   rmin   = " << fRmin << ";" << std::endl;
  out << "   rmax   = " << fRmax << ";" << std::endl;
  out << "   curve1 = " << fCurve1 << ";" << std::endl;
  out << "   curve2 = " << fCurve2 << ";" << std::endl;
  out << "   conic1 = " << fConic1 << ";" << std::endl;
  out << "   conic2 = " << fConic2 << ";" << std::endl;
  out << "   z1     = " << fZ1 << ";" << std::endl;
  out << "   z2     = " << fZ2 << ";" << std::endl;
  out << "   AGeoAsphericDisk* asph = new AGeoAsphericDisk(\"" << GetName() << "\",z1, curve1, z2, curve2, rmax, rmin);" << std::endl;

  if(fNPol1 > 0){
    out << "double k1[" << fNPol1 << "] = {";
    for(Int_t i = 0; i < fNPol1; i++){
      out << fK1[i];
      out << (i != fNPol1 - 1 ? "," : "};") << std::endl;
    } // i
  } // if
  if(fNPol2 > 0){
    out << "double k2[" << fNPol2 << "] = {";
    for(Int_t i = 0; i < fNPol2; i++){
      out << fK2[i];
      out << (i != fNPol2 - 1 ? "," : "};") << std::endl;
    } // i
  } // if
  if(fNPol1 > 0 and fNPol2 > 0){
    out << "asph->SetPolynomials(" << fNPol1 << ", k1, " << fNPol2 << ", k2);" << std::endl;
  } else if(fNPol1 == 0 and fNPol2 > 0){
    out << "asph->SetPolynomials(" << fNPol1 << ", 0, " << fNPol2 << ", k2);" << std::endl;
  } else if(fNPol1 > 0 and fNPol2 == 0){
    out << "asph->SetPolynomials(" << fNPol1 << ", k1, " << fNPol2 << ", 0);" << std::endl;
  } // if
  out << "asph->SetConicConstants(conic1, conic2);" << std::endl;
  out << "   TGeoShape* " << GetPointerName() << " = asph;" << std::endl;
  TObject::SetBit(TGeoShape::kGeoSavePrimitive);
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SetAsphDimensions(Double_t z1, Double_t curve1,
                                         Double_t z2, Double_t curve2,
                                         Double_t rmax, Double_t rmin)
{
  if(z1 < z2){
    fZ1 = z1;
    fZ2 = z2;
    fCurve1 = curve1;
    fCurve2 = curve2;
  } else {
    fZ1 = z2;
    fZ2 = z1;
    fCurve1 = curve2;
    fCurve2 = curve1;
  } // if

  if(rmax < 0) rmax *= -1;
  if(rmin < 0) rmin *= -1;

  if(rmax > rmin){
    fRmax = rmax;
    fRmin = rmin;
  } else {
    fRmax = rmin;
    fRmin = rmax;
  } // if

  if(fRmin > 0) {
    SetShapeBit(kGeoRSeg);
  } // if
  fNPol1 = 0;
  fNPol2 = 0;
  fK1 = 0;
  fK2 = 0;
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SetConicConstants(Double_t conic1, Double_t conic2)
{
  fConic1 = conic1;
  fConic2 = conic2;
  fKappa1 = fConic1 + 1;
  fKappa2 = fConic2 + 1;
  ComputeBBox();
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SetDimensions(Double_t* param)
{
  SetAsphDimensions(param[0], param[1], param[2], param[3], param[4], param[5]);
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SetFineness(Int_t steps, Int_t repeat)
{
  if(steps > 0) fSteps = steps;
  if(repeat > 0) fRepeat = repeat;
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SetPoints(Double_t* points) const
{
  // create mesh points
  Int_t n = gGeoManager->GetNsegments();

  if(points){
    if(TestShapeBit(kGeoRSeg)){
      // 2*n*(n + 1) points
      // lower (0, n*(n+1)-1)
      // upper (n*(n+1), 2*n*(n+1)-1)
      // (r0, phi0), (r0, phi1) .... (r1, phi0), (r1, phi1) ...
      for(int i = 0; i < n+1; i++){
        Double_t r = fRmin + i*(fRmax - fRmin)/n;
        for(int j = 0; j < n; j++){
          Double_t phi = j*TMath::Pi()*2/n;
          Int_t index = 3*(i*n + j); // lower
          points[index  ] = r*cos(phi);
          points[index+1] = r*sin(phi);
          try {
            points[index+2] = CalcF1(r);
          } catch (...) {
            points[index+2] = -TGeoShape::Big();
          } // try
          Int_t index2 = index + 3*n*(n + 1); // upper
          points[index2  ] = points[index];
          points[index2+1] = points[index+1];
          try {
            points[index2+2] = CalcF2(r);
          } catch (...) {
            points[index2+2] = TGeoShape::Big();
          } // try
        } // j
      } // i
    } else {
      // 2*(n*n + 1) points
      // lower (0, n*n-1)
      // upper (n*n, 2*n*n-1)
      for(int i = 0; i < n; i++){
        Double_t r = (i+1)*fRmax/n;
        for(int j = 0; j < n; j++){
          Double_t phi = j*TMath::Pi()*2/n;
          Int_t index = 3*(i*n + j); // lower
          points[index  ] = r*cos(phi);
          points[index+1] = r*sin(phi);
          try {
            points[index+2] = CalcF1(r);
          } catch (...) {
            points[index+2] = -TGeoShape::Big();
          } // try
          Int_t index2 = index + 3*n*n; // upper
          points[index2  ] = points[index];
          points[index2+1] = points[index+1];
          try {
            points[index2+2] = CalcF2(r);
          } catch (...) {
            points[index2+2] = TGeoShape::Big();
          } // try
        } // j
      } // i
      // lower center 2*n*n
      // upper center 2*n*n+1
      Int_t index = 3*2*n*n;
      points[index  ] = 0;
      points[index+1] = 0;
      try {
        points[index+2] = CalcF1(0);
      } catch (...) {
        points[index+2] = -TGeoShape::Big();
      } // try
      points[index+3] = 0;
      points[index+4] = 0;
      try {
        points[index+5] = CalcF2(0);
      } catch (...) {
        points[index+5] = -TGeoShape::Big();
      } // try
    } // if
  } // if
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SetPoints(Float_t* points) const
{
  // create mesh points
  Int_t n = gGeoManager->GetNsegments();

  if(points){
    if(TestShapeBit(kGeoRSeg)){
      // 2*n*(n + 1) points
      // lower (0, n*(n+1)-1)
      // upper (n*(n+1), 2*n*(n+1)-1)
      // (r0, phi0), (r0, phi1) .... (r1, phi0), (r1, phi1) ...
      for(int i = 0; i < n + 1; i++){
        Double_t r = fRmin + i*(fRmax - fRmin)/n;
        for(int j = 0; j < n; j++){
          Double_t phi = j*TMath::Pi()*2/n;
          Int_t index = 3*(i*n + j); // lower
          points[index  ] = r*cos(phi);
          points[index+1] = r*sin(phi);
          try {
            points[index+2] = CalcF1(r);
          } catch (...) {
            points[index+2] = -TGeoShape::Big();
          } // try
          Int_t index2 = index + 3*n*(n + 1); // upper
          points[index2  ] = points[index];
          points[index2+1] = points[index+1];
          try {
            points[index2+2] = CalcF2(r);
          } catch (...) {
            points[index2+2] = TGeoShape::Big();
          } // try
        } // j
      } // i
    } else {
      // 2*(n*n + 1) points
      // lower (0, n*n-1)
      // upper (n*n, 2*n*n-1)
      for(int i = 0; i < n; i++){
        Double_t r = (i + 1)*fRmax/n;
        for(int j = 0; j < n; j++){
          Double_t phi = j*TMath::Pi()*2/n;
          Int_t index = 3*(i*n + j); // lower
          points[index  ] = r*cos(phi);
          points[index+1] = r*sin(phi);
          try {
            points[index+2] = CalcF1(r);
          } catch (...) {
            points[index+2] = -TGeoShape::Big();
          } // try
          Int_t index2 = index + 3*n*n; // upper
          points[index2    ] = points[index];
          points[index2 + 1] = points[index+1];
          try {
            points[index2 + 2] = CalcF2(r);
          } catch (...) {
            points[index2 + 2] = TGeoShape::Big();
          } // try
        } // j
      } // i
      // lower center 2*n*n
      // upper center 2*n*n+1
      Int_t index = 3*2*n*n;
      points[index    ] = 0;
      points[index + 1] = 0;
      try {
        points[index + 2] = CalcF1(0);
      } catch (...) {
        points[index + 2] = -TGeoShape::Big();
      } // try
      points[index + 3] = 0;
      points[index + 4] = 0;
      try {
        points[index + 5] = CalcF2(0);
      } catch (...) {
        points[index + 5] = -TGeoShape::Big();
      } // try
    } // if
  } // if
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SetPolynomials(Int_t n1, const Double_t* k1,
                                      Int_t n2, const Double_t* k2)
{
  DeleteArrays();
  fNPol1 = n1;
  fNPol2 = n2;

  if(fNPol1 > 0){
    fK1 = new Double_t[fNPol1];
    for(Int_t i = 0; i < fNPol1; i++) fK1[i] = k1[i];
  } // if

  if(fNPol2 > 0){
    fK2 = new Double_t[fNPol2];
    for(Int_t i = 0; i < fNPol2; i++) fK2[i] = k2[i];
  } // if

  ComputeBBox();
}

//_____________________________________________________________________________
void AGeoAsphericDisk::SetSegsAndPols(TBuffer3D& buff) const
{
  // Fill TBuffer3D structure for segments and polygons.
  Int_t n = gGeoManager->GetNsegments();
  Int_t c = GetBasicColor();

  if(TestShapeBit(kGeoRSeg)){
    // segments
    for(Int_t i = 0; i < n; i++){
      for(Int_t j = 0; j < n; j++){
        // lower radial (0, n*n-1)
        Int_t index = 3*(i*n + j);
        buff.fSegs[index  ] = c;
        buff.fSegs[index + 1] = i*n + j;
        buff.fSegs[index + 2] = (i + 1)*n + j;
        // upper radial (n*n, 2*n*n-1)
        index += 3*(n*n);
        buff.fSegs[index    ] = c;
        buff.fSegs[index + 1] = n*(n + 1) +       i*n + j;
        buff.fSegs[index + 2] = n*(n + 1) + (i + 1)*n + j;
      } // j
    } // i
    for(Int_t i = 0; i < n + 1; i++){
      for(Int_t j = 0; j < n; j++){
        // lower circle (2*n*n, 3*n*n+n-1)
        Int_t index = 3*2*n*n + 3*(i*n + j);
        buff.fSegs[index    ] = c;
        buff.fSegs[index + 1] = i*n + j;
        buff.fSegs[index + 2] = j == n - 1 ? i*n : i*n + j + 1;
        // upper circle (3*n*n+n, 4*n*n+2*n-1)
        index += 3*(n*n + n);
        buff.fSegs[index    ] = c;
        buff.fSegs[index + 1] = n*(n + 1) + i*n + j;
        buff.fSegs[index + 2] = n*(n + 1) + (j == n-1 ? i*n : i*n + j + 1);
      } // j
    } // i
    for(Int_t j = 0; j < n; j++){
      // inner (4*n*n+2*n, 4*n*n+3*n-1)
      Int_t index = 3*(4*n*n + 2*n + j);
      buff.fSegs[index    ] = c + 1;
      buff.fSegs[index + 1] = j;
      buff.fSegs[index + 2] = j + n*(n + 1);
      // outer (4*n*n+3*n, 4*n*n+4*n-1)
      index += 3*n;
      buff.fSegs[index    ] = c + 1;
      buff.fSegs[index + 1] = n*n + j;
      buff.fSegs[index + 2] = n*n + j + n*(n + 1);
    } // i

    // polygons
    for(Int_t i = 0; i < n; i++){
      for(Int_t j = 0; j < n; j++){
        // lower (0, n*n-1)
        Int_t index = 6*(i*n + j);
        buff.fPols[index    ] = c;
        buff.fPols[index + 1] = 4;
        buff.fPols[index + 2] =             i*n + j;
        buff.fPols[index + 3] = 2*n*n + (i + 1)*n + j;
        buff.fPols[index + 4] = j != n - 1 ? i*n + (j + 1) : i*n;
        buff.fPols[index + 5] = 2*n*n + i*n + j;
        // upper (n*n, 2*n*n-1)
        index += 6*n*n;
        buff.fPols[index    ] = c;
        buff.fPols[index + 1] = 4;
        buff.fPols[index + 2] = n*n +     i*n + j;
        buff.fPols[index + 3] = 3*n*n + (i + 1)*n + j;
        buff.fPols[index + 4] = j != n - 1 ? n*n + i*n + (j + 1) : n*n + i*n;
        buff.fPols[index + 5] = 3*n*n + (i + 2)*n + j;
      } // j
    } // i
    for(Int_t j = 0; j < n; j++){
      // inner (2*n*n, 2*n*n+n-1)
      Int_t index = 6*(2*n*n + j);
      buff.fPols[index    ] = c;
      buff.fPols[index + 1] = 4;
      buff.fPols[index + 2] = 2*n*n + j;
      buff.fPols[index + 3] = j != n - 1 ? 4*n*n + 2*n + j + 1 : 4*n*n + 2*n;
      buff.fPols[index + 4] = 3*n*n + n + j;
      buff.fPols[index + 5] = 4*n*n + 2*n + j;
      // outer (2*n*n+n, 2*n*n+2*n-1)
      index += 6*n;
      buff.fPols[index    ] = c + 1;
      buff.fPols[index + 1] = 4;
      buff.fPols[index + 2] = 3*n*n + j;
      buff.fPols[index + 3] = 4*n*n + 3*n + j;
      buff.fPols[index + 4] = 4*n*n + n + j;
      buff.fPols[index + 5] = j != n - 1 ? 4*n*n + 3*n + j + 1 : 4*n*n + 3*n;
    } // j
  } else {
    // segments
    for(Int_t i = 0; i < n; i++){
      for(Int_t j = 0; j < n; j++){
        // lower radial (0, n*n-1)
        Int_t index = 3*(i*n + j);
        buff.fSegs[index    ] = c;
        buff.fSegs[index + 1] = i == 0 ? 2*n*n : (i - 1)*n + j;
        buff.fSegs[index + 2] = i*n + j;

        // upper radial (n*n, 2*n*n-1)
        index += 3*(n*n);
        buff.fSegs[index    ] = c;
        buff.fSegs[index + 1] = i == 0 ? 2*n*n + 1: n*n + (i-1)*n + j;
        buff.fSegs[index + 2] = n*n + i*n + j;
      } // j
    } // i
    for(Int_t i = 0; i < n; i++){
      for(Int_t j = 0; j < n; j++){
        // lower circle (2*n*n, 3*n*n-1)
        Int_t index = 3*(2*n*n + i*n + j);
        buff.fSegs[index    ] = c;
        buff.fSegs[index + 1] = i*n + j;
        buff.fSegs[index + 2] = j != n - 1 ? i*n + (j + 1) : i*n;
        // upper circle (3*n*n, 4*n*n-1)
        index += 3*(n*n);
        buff.fSegs[index    ] = c;
        buff.fSegs[index + 1] = n*n + i*n + j;
        buff.fSegs[index + 2] = j != n - 1 ? n*n + i*n + (j + 1) : n*n + i*n;
      } // j
    } // i
    for(Int_t j = 0; j < n; j++){
      // outer (4*n*n, 4*n*n+n-1)
      Int_t index = 3*(4*n*n + j);
      buff.fSegs[index    ] = c + 1;
      buff.fSegs[index + 1] = n*(n - 1) + j;
      buff.fSegs[index + 2] = n*(n - 1) + n*n + j;
    } // j

    // polygons
    for(Int_t j = 0; j < n; j++){
      // lower center (0, n-1)
      Int_t index = 5*j;
      buff.fPols[index    ] = c;
      buff.fPols[index + 1] = 3;
      buff.fPols[index + 2] = j;
      buff.fPols[index + 3] = 2*n*n + j;
      buff.fPols[index + 4] = j != n-1 ? j + 1 : 0;

      // upper center (n*n, 2*n*n-1)
      index += 6*n*n - n;
      buff.fPols[index    ] = c;
      buff.fPols[index + 1] = 3;
      buff.fPols[index + 2] = n*n + j;
      buff.fPols[index + 3] = j!=n-1 ? n*n + j + 1 : n*n;
      buff.fPols[index + 4] = 3*n*n + j;
    } // j

    for(Int_t i = 1; i < n; i++){
      for(Int_t j = 0; j < n; j++){
        // lower (n, n*n-1)
        Int_t index = 6*(i*n + j) - n;
        buff.fPols[index    ] = c;
        buff.fPols[index + 1] = 4;
        buff.fPols[index + 2] = i*n + j;
        buff.fPols[index + 3] = 2*n*n + i*n + j;
        buff.fPols[index + 4] = j != n-1 ? i*n + (j+1) : i*n;
        buff.fPols[index + 5] = 2*n*n + (i-1)*n + j;

        // upper (n*n+n, 2*n*n-1)
        index += 6*n*n - n;
        buff.fPols[index    ] = c;
        buff.fPols[index + 1] = 4;
        buff.fPols[index + 2] = n*n + i*n + j;
        buff.fPols[index + 3] = 3*n*n + (i - 1)*n + j;
        buff.fPols[index + 4] = j != n - 1 ? n*n + i*n + (j + 1) : n*n + i*n;
        buff.fPols[index + 5] = 3*n*n + i*n + j;

      } // j
    } // i

    for(Int_t j = 0; j < n; j++){
      // outer (2*n*n, 2*n*n+n-1)
      Int_t index = 6*(2*n*(n - 1) + j) + 5*2*n;
      buff.fPols[index    ] = c + 1;
      buff.fPols[index + 1] = 4;
      buff.fPols[index + 2] = 2*n*n + n*(n - 1) + j;
      buff.fPols[index + 3] = 4*n*n + j;
      buff.fPols[index + 4] = 3*n*n + n*(n - 1) + j;
      buff.fPols[index + 5] = j != n - 1 ?  4*n*n + j + 1: 4*n*n;

    } // j
  } // if
}

//_____________________________________________________________________________
void AGeoAsphericDisk::Sizeof3D() const
{
  ///// obsolete - to be removed
}
