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

#include "AGlassCatalog.h"

ClassImp(AGlassCatalog)

const Char_t AGlassCatalog::kNameCauchy[kNCauchy][10] = {};
const Char_t AGlassCatalog::kNameSchott[kNSchott][10] = {};
const Char_t AGlassCatalog::kNameSellmeier[kNSellmeier][10] = {"N-BK7",
                                                               "SF6"};

const Double_t AGlassCatalog::kParCauchy[kNCauchy][3] = {};
const Double_t AGlassCatalog::kParSchott[kNSchott][6] = {};
const Double_t AGlassCatalog::kParSellmeier[kNSellmeier][6] =
// B1 B2 B3 C1 C2 C3
// data from "Optical Glass Data Sheets" by SCHOTT
// http://www.us.schott.com/advanced_optics/english/download/schott_optical_glass_collection_datasheets_dec_2011_us.pdf
  {{1.03961212e0,  2.31792344e-1, 1.01046945e0,
    6.00069867e-3, 2.00179144e-2, 1.03560653e2}, // N-BK7
   {1.72448482e0,  3.90104889e-1, 1.04572858e0,
    1.34871947e-2, 5.69318095e-2, 1.18557185e2} // SF6
  };

//_____________________________________________________________________________
AGlassCatalog::AGlassCatalog()
{
}

//_____________________________________________________________________________
AGlassCatalog::~AGlassCatalog()
{
}

//_____________________________________________________________________________
ARefractiveIndex* AGlassCatalog::GetRefractiveIndex(const char* name)
{
  for(Int_t i = 0; i < kNCauchy; i++){
    if(strcmp(name, kNameCauchy[i]) == 0){
      return new ACauchyFormula(&kParCauchy[i][0]);
      break;
    } //if
  } // i

  for(Int_t i = 0; i < kNSchott; i++){
    if(strcmp(name, kNameSchott[i]) == 0){
      return new ASchottFormula(&kParSchott[i][0]);
      break;
    } //if
  } // i

  for(Int_t i = 0; i < kNSellmeier; i++){
    if(strcmp(name, kNameSellmeier[i]) == 0){
      return new ASellmeierFormula(&kParSellmeier[i][0]);
      break;
    } //if
  } // i

  return 0;
}
