/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AFilmetrixDotCom
//
// Wrapper class to read text files retreived from https://reflactiveindex.info
//
///////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>

#include "TSystem.h"

#include "AFilmetrixDotCom.h"
#include "AOpticsManager.h"

ClassImp(AFilmetrixDotCom);

AFilmetrixDotCom::AFilmetrixDotCom(const char* fname) : ARefractiveIndex() {
  std::ifstream fin;
  fin.open(gSystem->ExpandPathName(fname));

  if (!fin.is_open()) {
    Error("AFilmetrixDotCom", "Cannot open %s", fname);
    return;
  }

  char buf[100];
  fin.getline(buf, 100);

  // Check header with possible UTF-8 BOM and CR
  // clang-format off
  if (strcmp(buf, "\xef\xbb\xbfWavelength(nm)\tn\tk\r") != 0 &&
      strcmp(buf, "\xef\xbb\xbfWavelength(nm)\tn\tk"  ) != 0 &&
      strcmp(buf,             "Wavelength(nm)\tn\tk\r") != 0 &&
      strcmp(buf,             "Wavelength(nm)\tn\tk")   != 0 ) {
    // clang-format on
    Error("AFilmetrixDotCom", "Invalid data format");
    return;
  }

  fRefractiveIndex = std::make_shared<TGraph>();
  fExtinctionCoefficient = std::make_shared<TGraph>();

  while (1) {
    double wl, n, k;
    fin >> wl >> n >> k;
    if (fin.good()) {
      wl *= AOpticsManager::nm();
      fRefractiveIndex->SetPoint(fRefractiveIndex->GetN(), wl, n);
      fExtinctionCoefficient->SetPoint(fExtinctionCoefficient->GetN(), wl, k);
    } else {
      fin.close();
      break;
    }
  }
}
