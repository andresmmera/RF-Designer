/***************************************************************************
                                general.h
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
#ifndef GENERAL_H
#define GENERAL_H

#include <QString>
#include <cmath>
#include <complex>
#include <deque>
enum Units {
  Capacitance,
  Inductance,
  Length,
  Resistance,
  Degrees,
  Frequency,
  NoUnits
};

struct S2P_DATA {
  std::deque<double> Freq;
  std::deque<std::complex<double>> S11;
  std::deque<std::complex<double>> S12;
  std::deque<std::complex<double>> S21;
  std::deque<std::complex<double>> S22;
};

QString RoundVariablePrecision(double);
QString RoundVariablePrecision(double, int);
QString num2str(double, Units);
QString num2str(double, int);
QString num2str(double, int, Units);
QString num2str(std::complex<double>, Units);
QString num2str(double);
std::complex<double> Str2Complex(QString);
QString ConvertLengthFromM(QString, double);
struct S2P_DATA Sort(struct S2P_DATA);

#endif // GENERAL_H
