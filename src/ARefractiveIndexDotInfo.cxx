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

#include "TSystem.h"

#include "AOpticsManager.h"
#include "ARefractiveIndexDotInfo.h"

ClassImp(ARefractiveIndexDotInfo);

ARefractiveIndexDotInfo::ARefractiveIndexDotInfo(const char* fname)
  : ARefractiveIndex() {
  std::ifstream fin;
  fin.open(gSystem->ExpandPathName(fname));

  if (!fin.is_open()) {
    Error("ARefractiveIndexDotInfo", "Cannot open %s", fname);
    return;
  }

  char buf[50];
  fin.getline(buf, 50);

  bool CSV; // comma separated values
  bool CRLF; // delimiter \r\n
  char S = ','; // separator
  if (strcmp(buf, "wl,n\r") == 0) {
    CSV = true;
    CRLF = true;
  } else if (strcmp(buf, "wl,n") == 0) {
    CSV = true;
    CRLF = false;  // \n
  } else if (strcmp(buf, "wl\tn\r") == 0) {
    CSV = false;  // TSV, tab separated values
    S = '\t';
    CRLF = true;
  } else if (strcmp(buf, "wl\tn") == 0) {
    CSV = false;  // TSV, tab separated values
    S = '\t';
    CRLF = false;
  } else {
    Error("ARefractiveIndexDotInfo", "Invalide data format");
    return;
  }

  fRefractiveIndex = std::make_shared<TGraph>();

  while (1) {
    double wl, n;
    char* endptr;
    fin.getline(buf, 50, S);
    wl = strtod(buf, &endptr);
    if (*endptr != '\0') {  // cannot convert to double
      break;
    }
    fin.getline(buf, 50, CRLF ? '\r' : '\n');
    n = strtod(buf, &endptr);
    if (*endptr != '\0') {  // cannot convert to double
      break;
    }
    
    fRefractiveIndex->SetPoint(fRefractiveIndex->GetN(), wl * AOpticsManager::um(), n);
  }

  // Check if extinction coefficient data exists
  fin.getline(buf, 50, CRLF ? '\r' : '\n');
  // "wl," or "wl\t" should have been alread read in the previous while loop
  if (strcmp(buf, "k") != 0) {
    Error("ARefractiveIndexDotInfo", "Invalide data format");
    return;
  }

  fExtinctionCoefficient = std::make_shared<TGraph>();

  while (1) {
    double wl, k;
    char* endptr;
    fin.getline(buf, 50, S);
    wl = strtod(buf, &endptr);
    if (*endptr != '\0') {  // cannot convert to double
      break;
    }
    fin.getline(buf, 50, CRLF ? '\r' : '\n');
    k = strtod(buf, &endptr);
    if (*endptr != '\0') {  // cannot convert to double
      break;
    }
    
    fExtinctionCoefficient->SetPoint(fExtinctionCoefficient->GetN(), wl * AOpticsManager::um(), k);
  }
}
