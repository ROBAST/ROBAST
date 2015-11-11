#include <stdlib.h>

#include "ACorsikaIACTRunHeader.h"

ClassImp(ACorsikaIACTRunHeader)

ACorsikaIACTRunHeader::ACorsikaIACTRunHeader(Float_t* runHeader)
{
  memcpy(fRunHeader, runHeader, sizeof(Float_t)*273);
}

//_____________________________________________________________________________
ACorsikaIACTRunHeader::~ACorsikaIACTRunHeader()
{
}

//_____________________________________________________________________________
TDatime ACorsikaIACTRunHeader::GetDateOfBeginRun() const
{
  Int_t yymmdd = (Int_t)fRunHeader[2];
  Int_t year  = yymmdd/10000;
  Int_t month = (yymmdd - year*10000)/100;
  Int_t day   = yymmdd - year*10000 - month*100;

  if(year >= 95) {
    year += 1900;
  } else {
    year += 2000;
  } // if

  return TDatime(year, month, day, 0, 0, 0);
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetHeightOfLevel(Int_t i) const
{
  if(1 <= i and i<= 10){
    return fRunHeader[4 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetC(Int_t i) const
{
  if(1 <= i and i <= 50){
    return fRunHeader[23 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetCKA(Int_t i) const
{
  if(1 <= i and i <= 40){
    return fRunHeader[93+i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetCETA(Int_t i) const
{
  if(1 <= i and i <= 5){
    return fRunHeader[133 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetCSTRBA(Int_t i) const
{
  if(1 <= i and i <= 11){
    return fRunHeader[138 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetHLAY(Int_t i) const
{
  if(1 <= i and i <= 5){
    return fRunHeader[248 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetAATM(Int_t i) const
{
  if(1 <= i and i <= 5){
    return fRunHeader[253 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetBATM(Int_t i) const
{
  if(1 <= i and i <= 5){
    return fRunHeader[258 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTRunHeader::GetCATM(Int_t i) const
{
  if(1 <= i and i <= 5){
    return fRunHeader[263 + i];
  } else {
    return atof("NaN");
  } // if
}
