// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GEO_UTIL_H
#define A_GEO_UTIL_H

#include <vector>

#include "TGeoArb8.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TGeoTube.h"
#include "TGeoXtru.h"
#include "TH2.h"
#include "TVector3.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// AGeoUtil                                                                   //
//                                                                            //
// Utility functions to build complex geometries easily                       //
////////////////////////////////////////////////////////////////////////////////

namespace AGeoUtil {

void MakeArb8FromPoints(const char* name, const TVector3& v1,
                        const TVector3& v2, const TVector3& v3,
                        const TVector3& v4, const TVector3& v5,
                        TGeoArb8** arb8, TGeoCombiTrans** combi);
void MakeXtruFromPoints(const char* name, const std::vector<TVector3>& vecs,
                        TGeoXtru** xtru, TGeoCombiTrans** combi);
void MakePointToPointBBox(const char* name, const TVector3& v1,
                          const TVector3& v2, Double_t dx, Double_t dy,
                          TGeoBBox** box, TGeoCombiTrans** combi);
void MakePointToPointTube(const char* name, const TVector3& v1,
                          const TVector3& v2, Double_t radius, TGeoTube** tube,
                          TGeoCombiTrans** combi);
void MakePointToPointTube(const char* name, const TVector3& v1,
                          const TVector3& v2, Double_t rmin, Double_t rmax,
                          TGeoTube** tube, TGeoCombiTrans** combi);
void ContainmentRadius(TH2* h2, Double_t fraction, Double_t& r, Double_t& x,
                       Double_t& y);

}  // namespace AGeoUtil

#endif  // A_GEO_UTIL_H
