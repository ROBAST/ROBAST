// $Id: ARefractiveIndex.h 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/09/24

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_REFRACTIVE_INDEX_H
#define A_REFRACTIVE_INDEX_H

///////////////////////////////////////////////////////////////////////////////
//
// ARefractiveIndex
//
// Abstract class for refractive index
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class ARefractiveIndex : public TObject {
 private:

 public:
  ARefractiveIndex();
  virtual ~ARefractiveIndex();

  virtual Double_t GetIndex(Double_t lambda) const = 0;

  ClassDef(ARefractiveIndex, 1)
};

#endif // A_REFRACTIVE_INDEX_H
