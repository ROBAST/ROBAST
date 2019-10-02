// Author: Akira Okumura <mailto:oxon@mac.com>
/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_2x2_COMPLEX_MATRIX_H
#define A_2x2_COMPLEX_MATRIX_H

#include <complex>

#include "TObject.h"

///////////////////////////////////////////////////////////////////////////////
//
// A2x2ComplexMatrix
//
// 2 x 2 complex matrix with minimum functionality
//
///////////////////////////////////////////////////////////////////////////////

class A2x2ComplexMatrix {
 private:
  std::complex<Double_t> fC[4];
  // ( fC[0], fC[1] )
  // ( fC[2], fC[3] )

 public:
  A2x2ComplexMatrix() : A2x2ComplexMatrix(0, 0, 0, 0){};
  A2x2ComplexMatrix(std::complex<Double_t> c0, std::complex<Double_t> c1,
                    std::complex<Double_t> c2, std::complex<Double_t> c3);
  virtual ~A2x2ComplexMatrix() {}

  std::complex<Double_t> Get00() const { return fC[0]; }
  std::complex<Double_t> Get01() const { return fC[1]; }
  std::complex<Double_t> Get10() const { return fC[2]; }
  std::complex<Double_t> Get11() const { return fC[3]; }
  A2x2ComplexMatrix Transpose() const {
    return A2x2ComplexMatrix(fC[0], fC[2], fC[1], fC[3]);
  }
  void Print() const;

  inline A2x2ComplexMatrix& operator=(const A2x2ComplexMatrix& other);
  inline A2x2ComplexMatrix operator*(const A2x2ComplexMatrix& other);
  friend A2x2ComplexMatrix operator*(const std::complex<Double_t>& lhs,
                                     const A2x2ComplexMatrix& rhs) {
    return A2x2ComplexMatrix(lhs * rhs.fC[0], lhs * rhs.fC[1], lhs * rhs.fC[2],
                             lhs * rhs.fC[3]);
  }
  friend A2x2ComplexMatrix operator*(const A2x2ComplexMatrix& lhs,
                                     const std::complex<Double_t>& rhs) {
    return A2x2ComplexMatrix(lhs.fC[0] * rhs, lhs.fC[1] * rhs, lhs.fC[2] * rhs,
                             lhs.fC[3] * rhs);
  }
  friend A2x2ComplexMatrix operator/(const A2x2ComplexMatrix& lhs,
                                     const std::complex<Double_t>& rhs) {
    return A2x2ComplexMatrix(lhs.fC[0] / rhs, lhs.fC[1] / rhs, lhs.fC[2] / rhs,
                             lhs.fC[3] / rhs);
  }
};

//______________________________________________________________________________
A2x2ComplexMatrix& A2x2ComplexMatrix::operator=(
    const A2x2ComplexMatrix& other) {
  if (this != &other) {
    int n = sizeof(fC) / sizeof(std::complex<Double_t>);
    for (int i = 0; i < n; ++i) {
      fC[i] = other.fC[i];
    }
  }
  return *this;
}

//______________________________________________________________________________
A2x2ComplexMatrix A2x2ComplexMatrix::operator*(const A2x2ComplexMatrix& other) {
  // (A0, A1)   (B0, B1)
  // (A2, A3) x (B2, B3)
  std::complex<Double_t>* A = fC;
  const std::complex<Double_t>* B = other.fC;

  return A2x2ComplexMatrix(A[0] * B[0] + A[1] * B[2], A[0] * B[1] + A[1] * B[3],
                           A[2] * B[0] + A[3] * B[2],
                           A[2] * B[1] + A[3] * B[3]);
}

#endif  // A_2x2_COMPLEX_MATRIX_H
