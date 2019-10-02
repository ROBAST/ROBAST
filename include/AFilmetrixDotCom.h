// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_FILMETRIX_DOT_COM_H
#define A_FILMETRIX_DOT_COM_H

#include "TGraph.h"

#include "ARefractiveIndex.h"

///////////////////////////////////////////////////////////////////////////////
//
// AFilmetrixDotCom
//
// Wrapper class to read text files retreived from
// https://www.filmetrics.com/refractive-index-database/
//
///////////////////////////////////////////////////////////////////////////////

class AFilmetrixDotCom : public ARefractiveIndex {
 private:
 public:
  AFilmetrixDotCom(const char* fname);
  virtual ~AFilmetrixDotCom(){};

  ClassDef(AFilmetrixDotCom, 1)
};

#endif  // A_FILMETRIX_DOT_COM_H
