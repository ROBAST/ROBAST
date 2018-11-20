/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// ARefractiveIndexDotInfo
//
// Wrapper class to read text files retreived from https://reflactiveindex.info
//
///////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>

#include "AOpticsManager.h"
#include "ARefractiveIndexDotInfo.h"

ClassImp(ARefractiveIndexDotInfo);

ARefractiveIndexDotInfo::ARefractiveIndexDotInfo() : ARefractiveIndex() {}

//_____________________________________________________________________________
ARefractiveIndexDotInfo::ARefractiveIndexDotInfo(const char* fname) {
  std::ifstream fin;
  fin.open(fname);

  if (!fin.is_open()) {
    Error("ARefractiveIndexDotInfo", "Cannot open %s", fname);
    return;
  }

  char buf[50];
  fin.getline(buf, 50);

  bool CSV;   // comma separated values
  bool CRLF;  // delimiter \r\n
  if (strcmp(buf, "wl,n\r") == 0) {
    CSV = true;
    CRLF = true;
  } else if (strcmp(buf, "wl,n") == 0) {
    CSV = true;
    CRLF = false;  // \n
  } else if (strcmp(buf, "wl\tn\r") == 0) {
    CSV = false;  // TSV, tab separated values
    CRLF = true;
  } else if (strcmp(buf, "wl\tn") == 0) {
    CSV = false;  // TSV, tab separated values
    CRLF = false;
  } else {
    Error("ARefractiveIndexDotInfo", "Invalide data format");
    return;
  }

  fGraph.Set(0);  // reset all the previous data

  while (1) {
    double wl, n;
    if (CSV) {
      char* endptr;
      fin.getline(buf, 50, ',');
      wl = strtod(buf, &endptr);
      if (*endptr != '\0') {  // cannot convert to double
        break;
      }
      fin.getline(buf, 50, CRLF ? '\r' : '\n');
      n = strtod(buf, &endptr);
      if (*endptr != '\0') {  // cannot convert to double
        break;
      }
    } else {
      fin >> wl >> n;
      if (!fin.good()) {
        break;
      }
    }

    fGraph.SetPoint(fGraph.GetN(), wl * AOpticsManager::um(), n);
  }
}

//_____________________________________________________________________________
Double_t ARefractiveIndexDotInfo::GetIndex(Double_t lambda) const {
  return fGraph.Eval(lambda);
}
