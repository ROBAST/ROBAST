// $Id: AGeoWinstonConePoly.h 10 2010-11-28 06:35:46Z oxon $
// Author: Akira Okumura 2011/03/05

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AGeoWinstonConePoly
//
// Geometry class for polygonal Winston cone
// Roland Winston (1970) J. Opt. Soc.Amer. 60, 245-247
//
///////////////////////////////////////////////////////////////////////////////

#ifndef A_GEO_WINSTON_CONE_POLY_H
#define A_GEO_WINSTON_CONE_POLY_H

#ifndef A_GEO_WINSTON_CONE_2D_H
#include "AGeoWinstonCone2D.h"
#endif

class AGeoWinstonConePoly : public AGeoWinstonCone2D {
protected:
  Int_t fPolyN; //

public:
   AGeoWinstonConePoly();
   AGeoWinstonConePoly(Double_t r1, Double_t r2, Int_t n);
   AGeoWinstonConePoly(const char *name, Double_t r1, Double_t r2, Int_t n);
   virtual ~AGeoWinstonConePoly();

   virtual void        ComputeBBox();
   virtual void        ComputeNormal(Double_t* point, Double_t* dir, Double_t* norm);
   virtual Bool_t      Contains(Double_t* point) const;
   virtual Int_t       DistancetoPrimitive(Int_t px, Int_t py);
   virtual Double_t    DistFromInside(Double_t* point, Double_t* dir,
                                      Int_t iact = 1,
                                      Double_t step = TGeoShape::Big(),
                                      Double_t *safe = 0) const;
   virtual Double_t    DistFromOutside(Double_t* point, Double_t* dir,
                                       Int_t iact = 1,
                                       Double_t step = TGeoShape::Big(),
                                       Double_t* safe = 0) const;
   virtual void        GetBoundingCylinder(Double_t* param) const;
   virtual const TBuffer3D& GetBuffer3D(Int_t reqSections, Bool_t localFrame) const;
   virtual void        GetMeshNumbers(Int_t& nvert, Int_t& nsegs, Int_t& npols) const;
   virtual Int_t       GetNmeshVertices() const;
   virtual Bool_t      InsidePolygon(Double_t x, Double_t y, Double_t r) const;
   virtual void        InspectShape() const;
   virtual TBuffer3D*  MakeBuffer3D() const;
   virtual void        SavePrimitive(ostream& out, Option_t* option = "");
   virtual void        SetWinstonDimensions(Double_t r1, Double_t r2, Int_t n);
   virtual void        SetDimensions(Double_t* param);
   virtual void        SetPoints(Double_t* points) const;
   virtual void        SetPoints(Float_t* points) const;
   virtual void        SetSegsAndPols(TBuffer3D& buff) const;
   virtual void        Sizeof3D() const;

   ClassDef(AGeoWinstonConePoly, 1)
};

#endif // A_GEO_WINSTON_CONE_POLY_H
