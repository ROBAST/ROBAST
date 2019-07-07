/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// A2x2ComplexMatrix
//
// 2 x 2 complex matrix with minimum functionality
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "A2x2ComplexMatrix.h"

A2x2ComplexMatrix::A2x2ComplexMatrix(std::complex<Double_t> c0,
                                     std::complex<Double_t> c1,
                                     std::complex<Double_t> c2,
                                     std::complex<Double_t> c3)
{
  fC[0] = c0;
  fC[1] = c1;
  fC[2] = c2;
  fC[3] = c3;
}

//______________________________________________________________________________
void A2x2ComplexMatrix::Print() const
{
  std::cout << fC[0] << ", " << fC[1] << std::endl;
  std::cout << fC[2] << ", " << fC[3] << std::endl;
}
