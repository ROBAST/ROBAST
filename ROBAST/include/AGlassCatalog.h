// $Id: AGlassCatalog.h 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/10/01

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GLASS_CALTALOG_H
#define A_GLASS_CALTALOG_H

///////////////////////////////////////////////////////////////////////////////
//
// AGlassCatalog
//
// Glass catalog
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef A_CAUCHY_FORMULA_H
#include "ACauchyFormula.h"
#endif
#ifndef A_SCHOTT_FORMULA_H
#include "ASchottFormula.h"
#endif
#ifndef A_SELLMEIER_FORMULA_H
#include "ASellmeierFormula.h"
#endif

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
