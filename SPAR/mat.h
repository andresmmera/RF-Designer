/***************************************************************************
                                mat.h
                                ----------
    copyright            :  QUCS team
    author                :  2019 Andres Martinez-Mera
    email                  :  andresmmera@protonmail.com
 ***************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 ***************************************************************************/

#ifndef MAT_H
#define MAT_H

#include <complex>
#include <cstring>
#include <iostream>
#include <vector>
using namespace std;

typedef vector<complex<double>> vec;

class Mat {
private:
  vec data;

public:
  Mat();
  ~Mat();

  unsigned int rows;
  unsigned int cols;

  complex<double> operator()(unsigned int r, unsigned int c) const {
    return data[r * cols + c];
  }
  complex<double> &operator()(unsigned int r, unsigned int c) {
    return data[r * cols + c];
  }

  void print();
  friend Mat operator*(complex<double>, Mat);
  friend Mat operator*(Mat, Mat);

  vec getRow(unsigned int);
  vec getCol(unsigned int);
  void eye();
  void ones();
  vector<double> getRowReal(unsigned int);
  vector<double> getRowImag(unsigned int);
  vector<double> getColReal(unsigned int);
  vector<double> getColImag(unsigned int);
};

#endif // SPARENGINE_H
