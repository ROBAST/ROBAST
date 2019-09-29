// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_GLASS_CALTALOG_H
#define A_GLASS_CALTALOG_H

#include <map>
#include <memory>

#include "ACauchyFormula.h"
#include "ASchottFormula.h"
#include "ASellmeierFormula.h"
#include "TObject.h"

///////////////////////////////////////////////////////////////////////////////
//
// AGlassCatalog
//
// Glass catalog
//
///////////////////////////////////////////////////////////////////////////////

class AGlassCatalog : public TObject {
 private:
  std::map<std::string, std::shared_ptr<ARefractiveIndex>> fIndexMap;

 public:
  AGlassCatalog();
  AGlassCatalog(const std::string& catalog_file);
  virtual ~AGlassCatalog();

  std::shared_ptr<ARefractiveIndex> GetRefractiveIndex(const std::string& name);

  ClassDef(AGlassCatalog, 0)
};

#endif  // A_GLASS_CALTALOG_H
