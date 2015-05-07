// -*- mode: C++; c-indent-level: 2; c-basic-offset: 2; tab-width: 8 -*-
///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012  Whit Armstrong                                    //
//                                                                       //
// This program is free software: you can redistribute it and/or modify  //
// it under the terms of the GNU General Public License as published by  //
// the Free Software Foundation, either version 3 of the License, or     //
// (at your option) any later version.                                   //
//                                                                       //
// This program is distributed in the hope that it will be useful,       //
// but WITHOUT ANY WARRANTY; without even the implied warranty of        //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
// GNU General Public License for more details.                          //
//                                                                       //
// You should have received a copy of the GNU General Public License     //
// along with this program.  If not, see <http://www.gnu.org/licenses/>. //
///////////////////////////////////////////////////////////////////////////

#ifndef ARMA_CONTEXT_H
#define ARMA_CONTEXT_H

#include <stdexcept>
#include <Rinternals.h>
#include <Rcpp.h>
#include <RcppArmadillo.h>

enum armaT { doubleT, vecT, matT, intT, ivecT, imatT };

// R-compatible arma based vectors and matrices
using ivec = arma::Col<int>;
using imat = arma::Mat<int>;

class ArmaContextVisitor {
  virtual void visit(double& v) = 0;
  virtual void visit(arma::vec& v) = 0;
  virtual void visit(arma::mat& v) = 0;
  virtual void visit(int& v) = 0;
  virtual void visit(ivec& v) = 0;
  virtual void visit(imat& v) = 0;
};
 
class ArmaContext {
  armaT armatype_;
public:
  ArmaContext(armaT armatype): armatype_(armatype) {}
  const armaT getArmaType() const { return armatype_; }

  void apply(ArmaContextVisitor& visitor) = 0;
};
 
lass ArmaDouble : public ArmaContext {
private:
  double& x_;
public:
  ArmaDouble(SEXP x_sexp): ArmaContext(doubleT), x_(REAL(x_sexp)[0]) {}
  void apply(ArmaContextVisitor& visitor) { visit(x_); }
};

class ArmaVec : public ArmaContext {
private:
  arma::vec x_;
public:
  ArmaVec(SEXP x_sexp): ArmaContext(vecT), x_(arma::vec(REAL(x_sexp), Rf_length(x_sexp), false)) {}
  void apply(ArmaContextVisitor& visitor) { visit(x_); }
};

class ArmaMat : public ArmaContext {
private:
  arma::mat x_;
public:
  ArmaMat(SEXP x_sexp): ArmaContext(matT), x_(arma::mat(REAL(x_sexp), Rf_nrows(x_sexp), Rf_ncols(x_sexp), false)) {}
  void apply(ArmaContextVisitor& visitor) { visit(x_); }
};


class ArmaInt : public ArmaContext {
private:
  int& x_;
public:
  ArmaInt(SEXP x_sexp): ArmaContext(intT), x_(INTEGER(x_sexp)[0]) {}
  void apply(ArmaContextVisitor& visitor) { visit(x_); }
};

class ArmaiVec : public ArmaContext {
private:
  ivec x_;
public:
  ArmaiVec(SEXP x_sexp): ArmaContext(ivecT), x_(ivec(INTEGER(x_sexp), Rf_length(x_sexp), false)) {}
  void apply(ArmaContextVisitor& visitor) { visit(x_); }
};

class ArmaiMat : public ArmaContext {
private:
  imat x_;
public:
  ArmaiMat(SEXP x_sexp): ArmaContext(imatT), x_(imat(INTEGER(x_sexp), arma::uword(Rf_nrows(x_sexp)), arma::uword(Rf_ncols(x_sexp)), false)) {}
  void apply(ArmaContextVisitor& visitor) { visit(x_); }
};


#endif // ARMA_CONTEXT_H
