#include <stdlib.h>

#include "ACorsikaIACTEventHeader.h"

ClassImp(ACorsikaIACTEventHeader)

const Int_t ACorsikaIACTEventHeader::kMaxArrays = 20;

ACorsikaIACTEventHeader::ACorsikaIACTEventHeader(Float_t* eventHeader)
{
  if(eventHeader){
    for(Int_t i = 0; i < 273; i++){
      fEventHeader[i] = eventHeader[i];
    } // i
  } // if
}

//_____________________________________________________________________________
ACorsikaIACTEventHeader::~ACorsikaIACTEventHeader()
{
}

//_____________________________________________________________________________
TDatime ACorsikaIACTEventHeader::GetDateOfBeginRun() const
{
  Int_t yymmdd = (Int_t)fEventHeader[44];
  Int_t year  = yymmdd/10000; // Y2K problem may be happened.
  Int_t month = (yymmdd-year*10000)/100;
  Int_t day   = yymmdd-year*10000-month*100;

  return TDatime(year, month, day, 0, 0, 0);
}

//_____________________________________________________________________________
Int_t ACorsikaIACTEventHeader::GetNumberOfRandomCalls(Int_t i) const
{
  if(1 <= i and i <= 10){
    return (Int_t)fEventHeader[11 + 3*i] + (Int_t)fEventHeader[12 + 3*i]*1000000;
  } else {
    return -1;
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTEventHeader::GetHeightOfLevel(Int_t i) const
{
  if(1 <= i and i <= 10){
    return fEventHeader[46 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTEventHeader::GetXCoordinateOfCoreLocationForScatteredEvent(Int_t i) const
{
  if(1 <= i and i <= kMaxArrays){
    return fEventHeader[97 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Float_t ACorsikaIACTEventHeader::GetYCoordinateOfCoreLocationForScatteredEvent(Int_t i) const
{
  if(1 <= i and i <= kMaxArrays){
    return fEventHeader[117 + i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Double_t ACorsikaIACTEventHeader::GetXOffset(Int_t i) const
{
  if(1 <= i and i <= kMaxArrays){
    return fXOffset[i - 1];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Double_t ACorsikaIACTEventHeader::GetYOffset(Int_t i) const
{
  if(1 <= i and i <= kMaxArrays){
    return fYOffset[i - 1];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
void ACorsikaIACTEventHeader::SetMultipleUseHeader(Int_t numberOfArrays, Double_t timeOffset, Double_t* xOffset, Double_t* yOffset)
{
  fNumberOfArrays = numberOfArrays;
  fTimeOffset = timeOffset;

  for(Int_t i = 0; i < fNumberOfArrays; i++){
    fXOffset[i] = xOffset[i];
    fYOffset[i] = yOffset[i];
  } // i
}
