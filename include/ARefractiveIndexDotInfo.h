// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_REFRACTIVE_INDEX_DOT_INFO_H
#define A_REFRACTIVE_INDEX_DOT_INFO_H

#include "TGraph.h"

#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// ARefractiveIndexDotInfo
//
// Wrapper class to read text files retreived from https://reflactiveindex.info
//
///////////////////////////////////////////////////////////////////////////////

class ARefractiveIndexDotInfo : public ARefractiveIndex {
 private:
 public:
  virtual ~ARefractiveIndexDotInfo(){};
  ARefractiveIndexDotInfo(const char* fname);

  ClassDef(ARefractiveIndexDotInfo, 1)
};

#endif  // A_REFRACTIVE_INDEX_DOT_INFO_H
