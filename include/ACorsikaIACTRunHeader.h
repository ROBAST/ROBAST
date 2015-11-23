// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_CORSIKA_IACT_RUN_HEADER_H
#define A_CORSIKA_IACT_RUN_HEADER_H

#include "TDatime.h"

////////////////////////////////////////////////////////////////////////////////
//
// ACorsikaIACTRunHeader
//
// A class providing getters for the run header of CORSIKA IACT files.
//
////////////////////////////////////////////////////////////////////////////////

class ACorsikaIACTRunHeader {
 private:
  Float_t fRunHeader[273]; //Run header in CORSIKA format

 public:
  ACorsikaIACTRunHeader(Float_t* runHeader = 0);
  virtual ~ACorsikaIACTRunHeader();

  // Read CORSIKA_GUIDE69xx.pdf (table 7) for the detail
  ULong64_t GetRunNumber() const {return (ULong64_t)fRunHeader[1];}
  TDatime   GetDateOfBeginRun() const;
  Float_t   GetVersionOfProgram() const {return fRunHeader[3];}
  Int_t     GetNumberOfObservationLevels() const {return (Int_t)fRunHeader[4];}
  Float_t   GetHeightOfLevel(Int_t i) const;
  Float_t   GetSlopeOfEnergySpectrum() const {return fRunHeader[15];}
  Float_t   GetLowerLimitOfEnergyRange() const {return fRunHeader[16];}
  Float_t   GetUpperLimitOfEnergyRange() const {return fRunHeader[17];}
  Int_t     GetFlagForEGS4Treatment() const {return (Int_t)fRunHeader[18];}
  Int_t     GetFlagForNKGTreatment() const {return (Int_t)fRunHeader[19];}
  Float_t   GetKineticEnergyCutoffForHadrons() const {return fRunHeader[20];}
  Float_t   GetKineticEnergyCutoffForMuons() const {return fRunHeader[21];}
  Float_t   GetKineticEnergyCutoffForElectrons() const {return fRunHeader[22];}
  Float_t   GetEnergyCutoffForPhotons() const {return fRunHeader[23];}
  Float_t   GetC(Int_t i) const;
  Float_t   GetXPINCL() const {return fRunHeader[74];}
  Float_t   GetYPINCL() const {return fRunHeader[75];}
  Float_t   GetZPINCL() const {return fRunHeader[76];}
  Float_t   GetTHINCL() const {return fRunHeader[77];}
  Float_t   GetPHINCL() const {return fRunHeader[78];}
  Float_t   GetCKA(Int_t i) const;
  Float_t   GetCETA(Int_t i) const;
  Float_t   GetCSTRBA(Int_t i) const;
  Float_t   GetXSCATT() const {return fRunHeader[247];}
  Float_t   GetYSCATT() const {return fRunHeader[248];}
  Float_t   GetHLAY(Int_t i) const;
  Float_t   GetAATM(Int_t i) const;
  Float_t   GetBATM(Int_t i) const;
  Float_t   GetCATM(Int_t i) const;
  Int_t     GetNFLAIN() const { return (Int_t)fRunHeader[269];}
  Int_t     GetNFLDIF() const { return (Int_t)fRunHeader[270];}
  Int_t     GetNFLPI0() const { return ((Int_t)fRunHeader[271])%100;}
  Int_t     GetNFLPIF() const { return ((Int_t)fRunHeader[271])/100;}
  Int_t     GetNFLCHE() const { return ((Int_t)fRunHeader[272])%100;}
  Int_t     GetNFRAGM() const { return ((Int_t)fRunHeader[272])/100;}

  ClassDef(ACorsikaIACTRunHeader, 1)
};

#endif // A_CORSIKA_IACT_RUN_HEADER_H
