// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_CORSIKA_IACT_EVENT_HEADER_H
#define A_CORSIKA_IACT_EVENT_HEADER_H

#include "TDatime.h"

////////////////////////////////////////////////////////////////////////////////
//
// ACorsikaIACTEventHeader
//
// A calss providing getters for CORSIKA IACT evenet headers.
//
////////////////////////////////////////////////////////////////////////////////

class ACorsikaIACTEventHeader {
 private:
  static const Int_t kMaxArrays;

  Float_t  fEventHeader[273];
  Int_t    fNumberOfArrays;
  Double_t fTimeOffset;
  Double_t fXOffset[20];
  Double_t fYOffset[20];

 public:
  ACorsikaIACTEventHeader(Float_t* eventHeader = 0);
  virtual ~ACorsikaIACTEventHeader();

  // Read CORSIKA_GUIDE69xx.pdf (table 8) for the detail
  Int_t   GetEventNumber() const { return (Int_t)fEventHeader[1];}
  Int_t   GetParticleID() const { return (Int_t)fEventHeader[2];}
  Float_t GetTotalEnergy() const { return fEventHeader[3];}
  Float_t GetStartingAltitude() const { return fEventHeader[4];}
  Int_t   GetNumberOfFirstTarget() const { return (Int_t)fEventHeader[5];}
  Float_t GetZCoordinateOfFirstInteraction() const { return fEventHeader[6];}
  Float_t GetPxMomentum() const { return fEventHeader[7];}
  Float_t GetPyMomentum() const { return fEventHeader[8];}
  Float_t GetPzMomentum() const { return fEventHeader[9];}
  Float_t GetZenithAngle() const { return fEventHeader[10];}
  Float_t GetAzimuthAngle() const { return fEventHeader[11];}
  Int_t   GetNumberOfDifferentRandomNumberSequences() const { return (Int_t)fEventHeader[12];}
  Int_t   GetNumberOfRandomCalls(Int_t i) const;
  Int_t   GetRunNumber() const { return (Int_t)fEventHeader[43];}
  TDatime GetDateOfBeginRun() const;
  Float_t GetVersionOfProgram() const { return fEventHeader[45];}
  Int_t   GetNumberOfObservationLevels() const { return (Int_t)fEventHeader[46];}
  Float_t GetHeightOfLevel(Int_t i) const;
  Float_t GetSlopeOfEnergySpectrum() const { return fEventHeader[57];}
  Float_t GetLowerLimitOfEnergyRange() const { return fEventHeader[58];}
  Float_t GetUpperLimitOfEnergyRange() const { return fEventHeader[59];}
  Float_t GetKineticEnergyCutoffForHadrons() const { return fEventHeader[60];}
  Float_t GetKineticEnergyCutoffForMuons() const { return fEventHeader[61];}
  Float_t GetKineticEnergyCutoffForElectrons() const { return fEventHeader[62];}
  Float_t GetEnergyCutoffForPhotons() const { return fEventHeader[63];}
  Int_t   GetNFLAIN() const { return (Int_t)fEventHeader[64];}
  Int_t   GetNFLDIF() const { return (Int_t)fEventHeader[65];}
  Int_t   GetNFLPI0() const { return (Int_t)fEventHeader[66];}
  Int_t   GetNFLPIF() const { return (Int_t)fEventHeader[67];}
  Int_t   GetNFLCHE() const { return (Int_t)fEventHeader[68];}
  Int_t   GetNFRAGM() const { return (Int_t)fEventHeader[69];}
  Float_t GetXComponentOfEarthMagneticField() const { return fEventHeader[70];}
  Float_t GetZComponentOfEarthMagneticField() const { return fEventHeader[71];}
  Int_t   GetFlagForEGS4Treatment() const { return (Int_t)fEventHeader[72];}
  Int_t   GetFlagForNKGTreatment() const { return (Int_t)fEventHeader[73];}
  Int_t   GetLowEnergyHadronModelFlag() const { return (Int_t)fEventHeader[74];}
  Int_t   GetHighEnergyHadronModelFlag() const { return (Int_t)fEventHeader[75];}
  Int_t   GetCherenkovFlag() const { return (Int_t)fEventHeader[76];}
  Int_t   GetNeutrinoFlag() const { return (Int_t)fEventHeader[77];}
  Int_t   GetCurvedFlag() const { return (Int_t)fEventHeader[78];}
  Int_t   GetComputerFlag() const { return (Int_t)fEventHeader[79];}
  Float_t GetLowerEdgeOfThetaInterval() const { return fEventHeader[80];}
  Float_t GetUpperEdgeOfThetaInterval() const { return fEventHeader[81];}
  Float_t GetLowerEdgeOfPhiInterval() const { return fEventHeader[82];}
  Float_t GetUpperEdgeOfPhiInterval() const { return fEventHeader[82];}
  Int_t   GetCherenkovBunchSize() const { return (Int_t)fEventHeader[84];}
  Int_t   GetNumberOfCherenkovDetectorsInX() const { return (Int_t)fEventHeader[85];}
  Int_t   GetNumberOfCherenkovDetectorsInY() const { return (Int_t)fEventHeader[86];}
  Int_t   GetGridSpacingOfCherenkovDetectorsInX() const { return (Int_t)fEventHeader[87];}
  Int_t   GetGridSpacingOfCherenkovDetectorsInY() const { return (Int_t)fEventHeader[88];}
  Int_t   GetLengthOfEachCherenkovDetectorInX() const { return (Int_t)fEventHeader[89];}
  Int_t   GetLengthOfEachCherenkovDetectorInY() const { return (Int_t)fEventHeader[90];}
  // Skip [91]
  Float_t GetAngleBetweenArrayXDirectionAndMagneticNorth() const { return fEventHeader[92];}
  Int_t   GetAdditionalMuonInformationFlag() const { return (Int_t)fEventHeader[93];}
  Float_t GetStepLengthFactorForMultipleScattering() const { return fEventHeader[94];}
  Float_t GetCherenkovBandwidthLowerEnd() const { return fEventHeader[95];}
  Float_t GetCherenkovBandwidthUpperEnd() const { return fEventHeader[96];}
  Int_t   GetNumberOfUsesOfEachCherenkovEvent() const { return (Int_t)fEventHeader[97];}
  Float_t GetXCoordinateOfCoreLocationForScatteredEvent(Int_t i) const;
  Float_t GetYCoordinateOfCoreLocationForScatteredEvent(Int_t i) const;
  Int_t   GetSIBYLLInteractionFlag() const { return (Int_t)fEventHeader[138];}
  Int_t   GetSIBYLLCrossSectionFlag() const { return (Int_t)fEventHeader[139];}
  Int_t   GetQGSJETInteractionFlag() const { return (Int_t)fEventHeader[140];}
  Int_t   GetQGSJETCrossSectionFlag() const { return (Int_t)fEventHeader[141];}
  Int_t   GetDPMJETInteractionFlag() const { return (Int_t)fEventHeader[142];}
  Int_t   GetDPMJETCrossSectionFlag() const { return (Int_t)fEventHeader[143];}
  Int_t   GetVENUS_NEXUSCrossSectionFlag() const { return (Int_t)fEventHeader[144];}
  Int_t   GetMuonMultipleScatteringFlag() const { return (Int_t)fEventHeader[145];}
  Float_t GetNKGRadialDistributionRange() const { return fEventHeader[146];}
  Float_t GetEFRCTHNEnergyFractionOfThinningLevelHadronic() const { return fEventHeader[147];}
  Float_t GetEFRCTHN_THINRATEnergyFractionOfThinningLevelElectromagnetic() const { return fEventHeader[148];}
  Float_t GetActualWeightLimitWMAXForThinningHadronic() const { return fEventHeader[149];}
  Float_t GetActualWeightLimitWMAX_WEITRATForThinningElectromagnetic() const { return fEventHeader[150];}
  Float_t GetMaxRadiusForRadialThinning() const { return fEventHeader[151];}
  Float_t GetInnerAngleOfViewingCone() const { return fEventHeader[152];}
  Float_t GetOuterAngleOfViewingCone() const { return fEventHeader[153];}
  Float_t GetTransitionEnergyHighEnergyLowEnergyModel() const { return fEventHeader[154];}
  // Skip 155-167

  Int_t    GetNumberOfArrays() const { return fNumberOfArrays;}
  Double_t GetTimeOffset() const { return fTimeOffset;}
  Double_t GetXOffset(Int_t i) const;
  Double_t GetYOffset(Int_t i) const;

  void SetMultipleUseHeader(Int_t numberOfArrays, Double_t timeOffset, Double_t* xOffset, Double_t* yOffset);

  ClassDef(ACorsikaIACTEventHeader, 1)
};

#endif // A_CORSIKA_IACT_EVENT_HEADER_H
