// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_REFRACTIVE_INDEX_H
#define A_REFRACTIVE_INDEX_H

#include "TObject.h"

///////////////////////////////////////////////////////////////////////////////
//
// ARefractiveIndex
//
// Abstract class for refractive index
//
///////////////////////////////////////////////////////////////////////////////

class ARefractiveIndex : public TObject {
 private:

 public:
  ARefractiveIndex();
  virtual ~ARefractiveIndex();

  virtual Double_t GetAbbeNumber() const;
  virtual Double_t GetIndex(Double_t lambda) const = 0;

  ClassDef(ARefractiveIndex, 1)
};

#endif // A_REFRACTIVE_INDEX_H
