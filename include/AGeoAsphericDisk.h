// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GEO_ASPHERIC_DISK_H
#define A_GEO_ASPHERIC_DISK_H

#include "TGeoBBox.h"

#if ROOT_VERSION_CODE >= ROOT_VERSION(5,34,10)
#define CONST53410 const
#else
#define CONST53410
#endif

///////////////////////////////////////////////////////////////////////////////
//
// AGeoAsphericDisk
//
// Geometry class for tubes which have two aspheric surface
//
///////////////////////////////////////////////////////////////////////////////

class AGeoAsphericDisk : public TGeoBBox {
protected:
  Double_t  fZ1;     // Z of the center of surface 1 (lower)
  Double_t  fZ2;     // Z of the center of surface 2 (upper)
  Double_t  fCurve1; // Curvature of surface 1 (=1/R1)
  Double_t  fCurve2; // Curvature of surface 2 (=1/R2)
  Double_t  fConic1; // Conic constant of surface 1
  Double_t  fConic2; // Conic constant of surface 2
  Double_t  fKappa1; // conic1 + 1
  Double_t  fKappa2; // conic2 + 1
  Double_t  fRmin;   // inner radius
  Double_t  fRmax;   // outer radius
  Int_t     fNPol1;  // Order of polynomial of surface 1 (=Nmax/2)
  Int_t     fNPol2;  // Order of polynomial of surface 2
  Double_t* fK1;     //[fNPol1] Coefficients of polynomial of surface 1
  Double_t* fK2;     //[fNPol2] Coefficients of polynomial of surface 2

  Int_t     fSteps;  // steps of approximate calculation
  Int_t     fRepeat; // repeat times of approximate calculation

  void DeleteArrays();

public:
  AGeoAsphericDisk();
  AGeoAsphericDisk(Double_t z1, Double_t curve1, Double_t z2, Double_t curve2,
                   Double_t rmax, Double_t rmin = 0);
  AGeoAsphericDisk(const char *name, Double_t z1, Double_t curve1,
                   Double_t z2, Double_t curve2,
                   Double_t rmax, Double_t rmin = 0);
  virtual ~AGeoAsphericDisk();

  virtual Double_t    Capacity() const;
  virtual Double_t    CalcdF1dr(Double_t r) const throw(std::exception);
  virtual Double_t    CalcdF2dr(Double_t r) const throw(std::exception);
  virtual Double_t    CalcF1(Double_t r) const throw(std::exception);
  virtual Double_t    CalcF2(Double_t r) const throw(std::exception);
  virtual void        ComputeBBox();
  virtual void        ComputeNormal(CONST53410 Double_t* point, CONST53410 Double_t* dir, Double_t* norm);
  virtual Bool_t      Contains(CONST53410 Double_t* point) const;
  virtual Int_t       DistancetoPrimitive(Int_t px, Int_t py);
  virtual Double_t    DistFromInside(CONST53410 Double_t* point, CONST53410 Double_t* dir,
                                     Int_t iact = 1,
                                     Double_t step = TGeoShape::Big(),
                                     Double_t *safe = 0) const;
  virtual Double_t    DistFromOutside(CONST53410 Double_t* point, CONST53410 Double_t* dir,
                                      Int_t iact = 1,
                                      Double_t step = TGeoShape::Big(),
                                      Double_t* safe = 0) const;
  virtual Double_t    DistToAsphere(Int_t i, CONST53410 Double_t* point, CONST53410 Double_t* dir) const;
  virtual Double_t    DistToInner(CONST53410 Double_t* point, CONST53410 Double_t* dir) const;
  virtual Double_t    DistToOuter(CONST53410 Double_t* point, CONST53410 Double_t* dir) const;
  virtual TGeoVolume* Divide(TGeoVolume* voldiv, const char* divname,
                             Int_t iaxis, Int_t ndiv, Double_t start,
                             Double_t step);
  virtual void        GetBoundingCylinder(Double_t* param) const;
  virtual const TBuffer3D& GetBuffer3D(Int_t reqSections, Bool_t localFrame) const;
  virtual Int_t       GetByteCount() const {return 68 + 4*(fNPol1 + fNPol2);} // to be checked
  Double_t            GetCurve1() const { return fCurve1;}
  Double_t            GetCurve2() const { return fCurve2;}
  Double_t*           GetK1() const { return fK1;}
  Double_t*           GetK2() const { return fK2;}
  virtual TGeoShape*  GetMakeRuntimeShape(TGeoShape*, TGeoMatrix*) const {return 0;}
  virtual void        GetMeshNumbers(Int_t& nvert, Int_t& nsegs, Int_t& npols) const;
  virtual Int_t       GetNmeshVertices() const;
  Double_t            GetNPol1() const { return fNPol1;}
  Double_t            GetNPol2() const { return fNPol2;}
  Double_t            GetRmax() const { return fRmax;}
  Double_t            GetRmin() const { return fRmin;}
  Double_t            GetZ1() const { return fZ1;}
  Double_t            GetZ2() const { return fZ2;}
  virtual void        InspectShape() const;
  virtual Bool_t      IsCylType() const {return kTRUE;}
  virtual TBuffer3D*  MakeBuffer3D() const;
  virtual void        SavePrimitive(std::ostream& out, Option_t* option = "");
  virtual Double_t    Safety(CONST53410 Double_t* point, Bool_t in=kTRUE) const;
  virtual void        SetAsphDimensions(Double_t x1, Double_t curve1,
                                        Double_t x2, Double_t curve2,
                                        Double_t rmax, Double_t rmin);
  virtual void        SetDimensions(Double_t* param);
  virtual void        SetConicConstants(Double_t conic1, Double_t conic2);
  virtual void        SetPoints(Double_t* points) const;
  virtual void        SetPoints(Float_t* points) const;
  virtual void        SetPolynomials(Int_t n1, const Double_t* k1, Int_t n2, const Double_t* k2);
  virtual void        SetSegsAndPols(TBuffer3D& buff) const;
  virtual void        SetFineness(Int_t steps, Int_t repeat);
  virtual void        Sizeof3D() const;

  ClassDef(AGeoAsphericDisk, 1)
};

//______________________________________________________________________________
inline void AGeoAsphericDisk::DeleteArrays()
{
  // Delete coefficients arrays
  if (fK1) {
    delete[] fK1;
    fK1 = 0;
  } // if

  if (fK2) {
    delete[] fK2;
    fK2 = 0;
  } // if
}

#endif // A_GEO_ASPHERIC_DISK_H
