// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GEO_UTIL_H
#define A_GEO_UTIL_H

#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TGeoTube.h"
#include "TH2.h"
#include "TVector3.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// AGeoUtil                                                                   //
//                                                                            //
// Utility functions to build complex geometries easily                       //
////////////////////////////////////////////////////////////////////////////////

namespace AGeoUtil {

void MakePointToPointBBox(const char* name, const TVector3& v1,
                          const TVector3& v2, Double_t dx, Double_t dy,
                          TGeoBBox** box, TGeoCombiTrans** combi);
void MakePointToPointTube(const char* name, const TVector3& v1,
                          const TVector3& v2, Double_t radius, TGeoTube** tube,
                          TGeoCombiTrans** combi);
void ContainmentRadius(TH2* h2, Double_t fraction, Double_t& r, Double_t& x,
                       Double_t& y);

}  // namespace AGeoUtil

#endif  // A_GEO_UTIL_H
