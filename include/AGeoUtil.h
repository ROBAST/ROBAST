// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GEO_UTIL_H
#define A_GEO_UTIL_H

#include "TGeoMatrix.h"
#include "TGeoBBox.h"
#include "TGeoTube.h"
#include "TVector3.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// AGeoUtil                                                                   //
//                                                                            //
// Utility functions to build complex geometries easily                       //
////////////////////////////////////////////////////////////////////////////////

namespace AGeoUtil {

void MakePointToPointBBox(const char* name, TVector3& v1, TVector3& v2, Double_t dx, Double_t dy, TGeoBBox** box, TGeoCombiTrans** combi);
void MakePointToPointTube(const char* name, TVector3& v1, TVector3& v2, Double_t radius, TGeoTube** tube, TGeoCombiTrans** combi);

} // AGeoUtil

#endif // A_GEO_UTIL_H
