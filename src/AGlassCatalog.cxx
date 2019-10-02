/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AGlassCatalog
//
// Glass catalog
//
///////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include "TSystem.h"

#include "AGlassCatalog.h"
#include "AOpticsManager.h"

ClassImp(AGlassCatalog);

//_____________________________________________________________________________
AGlassCatalog::AGlassCatalog() {}

//_____________________________________________________________________________
AGlassCatalog::AGlassCatalog(const std::string& catalog_file) {
  std::ifstream fin;
  fin.open(gSystem->ExpandPathName(catalog_file.c_str()));

  if (!fin.is_open()) {
    Error("AGlassCatalog", "Cannot open %s", catalog_file.c_str());
    return;
  }

  // ASCII glass format catalog file (AGF)
  if (catalog_file.size() - catalog_file.find(".agf") != 4 &&
      catalog_file.size() - catalog_file.find(".AGF") != 4) {
    Error("AGlassCatalog", "Cannot read a non-ZEMAX file");
    return;
  }

  const std::size_t bufsize = 200;
  char buf[bufsize], glass_name[bufsize];
  std::string glass_name_s;
  Double_t nd;
  Int_t formula;
  std::shared_ptr<TGraph> graph;
  const std::size_t ncd = 8;
  Double_t cd[ncd];

  while (fin.good()) {
    fin.getline(buf, bufsize);
    if (strncmp(buf, "NM ", 3) == 0) {
      // found a glass name
      // e.g., "NM N-BK7 2 517642.251 1.5168 64.17 0 1"
      // where
      // glass name : N-BK7
      // dispersion formula : 2 (2 = Sellmeier)
      // product number? : 517642.251
      // Nd : 1.5168
      // Vd : 64.17
      // ignore thremal expantion: 0
      // exclude substitution: 1
      // meta material? : ?
      char product_number[bufsize];
      Double_t vd;
      if (sscanf(buf, "NM %s %d %s %lf %lf", glass_name, &formula,
                 product_number, &nd, &vd) != 5) {
        Warning("AGlassCatalog", "Bad format line found: %s", buf);
      }

      glass_name_s = std::string(glass_name);
      graph = std::make_shared<TGraph>();

      fIndexMap.insert(
          std::make_pair(glass_name_s, std::shared_ptr<ARefractiveIndex>(0)));
    } else if (strncmp(buf, "CD ", 3) == 0) {
      int ret = sscanf(buf, "CD %lf %lf %lf %lf %lf %lf %lf %lf", &cd[0],
                       &cd[1], &cd[2], &cd[3], &cd[4], &cd[5], &cd[6], &cd[7]);
      if (formula == 2 && ret >= 6) {
        auto it = fIndexMap.find(glass_name_s);
        if (it != fIndexMap.end()) {
          it->second = std::make_shared<ASellmeierFormula>(cd[0], cd[2], cd[4],
                                                           cd[1], cd[3], cd[5]);
          it->second->SetExtinctionCoefficient(graph);
        }
      }
    } else if (strncmp(buf, "IT ", 3) == 0) {
      Double_t wl, T, d;  // lambda (um), transmittance, thickness (mm)
      int ret = sscanf(buf, "IT %lf %lf %lf", &wl, &T, &d);
      if (ret == 3) {
        wl *= AOpticsManager::um();
        d *= AOpticsManager::mm();
        Double_t absl = -d / TMath::Log(T);
        Double_t k =
            ARefractiveIndex::AbsorptionLengthToExtinctionCoefficient(absl, wl);
        graph->SetPoint(graph->GetN(), wl, k);
      } else if (ret == 2) {
        // Some glass materials such as N-LASF9 has incomplete lines
        // Just ignore
      } else {
        Warning("AGlassCatalog", "Bad format line found: %s", buf);
      }
    }
  }
}

//_____________________________________________________________________________
AGlassCatalog::~AGlassCatalog() {}

//_____________________________________________________________________________
std::shared_ptr<ARefractiveIndex> AGlassCatalog::GetRefractiveIndex(
    const std::string& name) {
  auto it = fIndexMap.find(name);
  if (it == fIndexMap.end()) {
    return 0;
  } else {
    return it->second;
  }
}
