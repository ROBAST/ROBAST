/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// AMixedRefractiveIndex
//
// Refractive index for two mixed materials
//
///////////////////////////////////////////////////////////////////////////////

#include "AMixedRefractiveIndex.h"

ClassImp(AMixedRefractiveIndex);

AMixedRefractiveIndex::AMixedRefractiveIndex(std::shared_ptr<ARefractiveIndex> materialA,
                                             std::shared_ptr<ARefractiveIndex> materialB,
                                             Double_t fractionA,
                                             Double_t fractionB)
  : ARefractiveIndex(), fMaterialA(materialA), fMaterialB(materialB)
{
  fFractionA = fractionA / (fractionA + fractionB);
  fFractionB = fractionB / (fractionA + fractionB);
}
