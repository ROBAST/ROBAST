// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_CORSIKA_IACT_FILE_H
#define A_CORSIKA_IACT_FILE_H

#ifdef __CINT__
#define _SYS_TYPES_H_
#define _STDINT_H_
#endif

#include "bernlohr/fileopen.h"
#include "bernlohr/io_basic.h"
#include "bernlohr/mc_tel.h"

#include "TTree.h"

#include "ACorsikaIACTEventHeader.h"
#include "ACorsikaIACTRunHeader.h"
#include "ARayArray.h"

////////////////////////////////////////////////////////////////////////////////
//
// ACorsikaIACTFile
//
// Wrapper class for I/O functions of CORSIKA IACT files.
//
////////////////////////////////////////////////////////////////////////////////

class ACorsikaIACTFile : public TObject {
 private:
  static const Int_t       kMaxArrays;
  static const Int_t       kMaxTelescopes;

  IO_ITEM_HEADER           fBlockHeader;
  TTree*                   fBunches;
  struct linked_string     fCorsikaInputs;
  ACorsikaIACTEventHeader* fEventHeader;
  TString                  fFileName;
  IO_BUFFER*               fIOBuffer;
  Int_t                    fMaxPhotonBunches;
  Int_t                    fNumberOfTelescopes;
  ACorsikaIACTRunHeader*   fRunHeader;
  Double_t*                fTelescopePosition[4]; //
  Double_t                 fMaxWavelength;
  Double_t                 fMinWavelength;

  Int_t    ReadNextBlock();

 public:
  ACorsikaIACTFile(Int_t bufferLenght = 20000000);
  virtual ~ACorsikaIACTFile();

  void     Close();
  TTree*   GetBunches() const { return fBunches;}
  const Char_t* GetFileName() const { return fFileName.Data();}
  Int_t    GetNumberOfTelescopes() const { return fNumberOfTelescopes;}
  ARayArray* GetRayArray(Int_t telNo, Int_t arrayNo, Double_t zoffset, Double_t refractiveIndex);
  Double_t GetTelescopeR(Int_t i) const;
  Double_t GetTelescopeX(Int_t i) const;
  Double_t GetTelescopeY(Int_t i) const;
  Double_t GetTelescopeZ(Int_t i) const;
  Bool_t   IsAllocated();
  Bool_t   IsOpen();
  void     Open(const Char_t* fname);
  void     PrintInputCard() const;
  Int_t    ReadEvent(Int_t num);
  void     SetMaxPhotonBunches(UInt_t max) { fMaxPhotonBunches = max;}

  ACorsikaIACTEventHeader* GetEventHeader() const { return fEventHeader;}
  ACorsikaIACTRunHeader*   GetRunHeader() const { return fRunHeader;}

  ClassDef(ACorsikaIACTFile, 0)
};

#endif // A_CORSIKA_IACT_FILE_H
