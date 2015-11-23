// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GLASS_CALTALOG_H
#define A_GLASS_CALTALOG_H

#include "TObject.h"
#include "ACauchyFormula.h"
#include "ASchottFormula.h"
#include "ASellmeierFormula.h"

///////////////////////////////////////////////////////////////////////////////
//
// AGlassCatalog
//
// Glass catalog
//
///////////////////////////////////////////////////////////////////////////////

class AGlassCatalog : public TObject {
 private:
  enum {kNSellmeier = 2, kNSchott = 0, kNCauchy = 0};
  static const Char_t   kNameCauchy[kNCauchy][10]; // Cauchy name
  static const Char_t   kNameSchott[kNSchott][10]; // SCHOTT name (e.g. BK7)
  static const Char_t   kNameSellmeier[kNSellmeier][10]; // Sellmeier name (e.g. BK7)
  static const Double_t kParCauchy[kNCauchy][3];  // Cauchy formula parameters
  static const Double_t kParSchott[kNSchott][6];  // SCHOTT formula parameters
  static const Double_t kParSellmeier[kNSellmeier][6];  // Sellmeier formula parameters
 public:
  AGlassCatalog();
  virtual ~AGlassCatalog();

  static ARefractiveIndex* GetRefractiveIndex(const char* name);

  ClassDef(AGlassCatalog, 0)
};

#endif // A_GLASS_CALTALOG_H
