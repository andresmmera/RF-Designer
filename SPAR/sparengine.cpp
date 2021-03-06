/***************************************************************************
                                sparengine.cpp
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
#include "sparengine.h"

SparEngine::SparEngine() {}

// Returns the S matrix at a given frequency for the specified input and load
// impedances
Mat SparEngine::getSparams(QList<ComponentInfo> x, complex<double> zs,
                           complex<double> zl, double f) {
  Mat ABCD = getABCDmatrix(x, f);
  Mat S;
  // Convert ABCD to S parameters
  S(0, 0) =
      (ABCD(0, 0) * zl + ABCD(0, 1) - ABCD(1, 0) * conj(zs) * zl -
       ABCD(1, 1) * conj(zs)) /
      (ABCD(0, 0) * zl + ABCD(0, 1) + ABCD(1, 0) * zs * zl + ABCD(1, 1) * zs);
  S(0, 1) =
      (2. * (ABCD(0, 0) * ABCD(1, 1) - ABCD(0, 1) * ABCD(1, 0)) *
       sqrt(real(zs) * real(zl))) /
      (ABCD(0, 0) * zl + ABCD(0, 1) + ABCD(1, 0) * zs * zl + ABCD(1, 1) * zs);
  S(1, 0) =
      (2. * sqrt(real(zs) * real(zl))) /
      (ABCD(0, 0) * zl + ABCD(0, 1) + ABCD(1, 0) * zs * zl + ABCD(1, 1) * zs);
  S(1, 1) =
      (-ABCD(0, 0) * conj(zl) + ABCD(0, 1) - ABCD(1, 0) * conj(zl) * zs +
       ABCD(1, 1) * zs) /
      (ABCD(0, 0) * zl + ABCD(0, 1) + ABCD(1, 0) * zs * zl + ABCD(1, 1) * zs);

  return S;
}

QMap<QString, vector<complex<double>>> SparEngine::getData() {
  QMap<QString, vector<complex<double>>> data;
  data["S[2,1]"] = S21;
  data["S[1,1]"] = S11;
  data["S[2,2]"] = S22;
  // data["S12"] = S12;
  return data;
}

// Returns the ABCD matrix at a given frequency
Mat SparEngine::getABCDmatrix(QList<ComponentInfo> x, double f) {
  double w = 2 * pi * f;
  double beta = w / c0;
  complex<double> gamma = complex<double>(0, beta);
  complex<double> Z, Y, num, den;

  Mat ABCD, ABCD_t;
  ABCD.eye();

  for (int i = 0; i < x.size(); i++) {
    switch (x[i].Type) {
    case GND:
      continue;
    case Capacitor:
      if (x[i].Rotation == 90) { // Series
        ABCD_t(0, 0) = 1.;
        ABCD_t(0, 1) = complex<double>(0, -1. / (w * x[i].getVal("C")));
        ABCD_t(1, 0) = 0;
        ABCD_t(1, 1) = 1.;
      } else { // Shunt
        ABCD_t(0, 0) = 1.;
        ABCD_t(0, 1) = 0;
        ABCD_t(1, 0) = complex<double>(0, w * x[i].getVal("C"));
        ABCD_t(1, 1) = 1.;
      }
      break;
    case Inductor:
      if (x[i].Rotation == -90) { // Series
        ABCD_t(0, 0) = 1.;
        ABCD_t(0, 1) = complex<double>(0, w * x[i].getVal("L"));
        ABCD_t(1, 0) = 0;
        ABCD_t(1, 1) = 1.;
      } else { // Shunt
        ABCD_t(0, 0) = 1.;
        ABCD_t(0, 1) = 0;
        ABCD_t(1, 0) = complex<double>(0, -1. / (w * x[i].getVal("L")));
        ABCD_t(1, 1) = 1.;
      }
      break;
    case OpenStub:
      ABCD_t(0, 0) = 1.;
      ABCD_t(0, 1) = 0;
      ABCD_t(1, 0) = tanh(gamma * x[i].getVal("Length")) / (x[i].getVal("Z0"));
      ABCD_t(1, 1) = 1;
      break;
    case ShortStub:
      ABCD_t(0, 0) = 1.;
      ABCD_t(0, 1) = 0;
      ABCD_t(1, 0) =
          1. / (x[i].getVal("Z0") * tanh(gamma * x[i].getVal("Length")));
      ABCD_t(1, 1) = 1;
      break;
    case TransmissionLine:
      ABCD_t(0, 0) = cosh(gamma * x[i].getVal("Length"));
      ABCD_t(0, 1) = x[i].getVal("Z0") * sinh(gamma * x[i].getVal("Length"));
      ABCD_t(1, 0) = sinh(gamma * x[i].getVal("Length")) / x[i].getVal("Z0");
      ABCD_t(1, 1) = cosh(gamma * x[i].getVal("Length"));
      break;
    case Resistor:
      if (x[i].Rotation == 90) { // Series
        ABCD_t(0, 0) = 1;
        ABCD_t(0, 1) = x[i].getVal("R");
        ABCD_t(1, 0) = 0;
        ABCD_t(1, 1) = 1;
      } else { // Shunt
        ABCD_t(0, 0) = 1;
        ABCD_t(0, 1) = 0;
        ABCD_t(1, 0) = 1 / x[i].getVal("R");
        ABCD_t(1, 1) = 1;
      }
      break;

    default:
      ABCD.eye();
      return ABCD;
    }

    ABCD = ABCD * ABCD_t;
  }
  return ABCD;
}

void SparEngine::setNetwork(NetworkInfo nwi) {
  if (nwi.Ladder[0].Type == Term)
    nwi.Ladder.pop_front(); // Remove port 1
  if (nwi.Ladder[nwi.Ladder.size() - 1].Type == Term)
    nwi.Ladder.pop_back(); // Remove port 2
  NI = nwi;
}

void SparEngine::setSimulationSettings(SP_Analysis spa) {
  sim_settings = spa;
  sim_settings.freq = linspace(spa.fstart, spa.fstop, spa.n_points);
}

void SparEngine::run() {
  if (NI.ZS.size() == 1) {
    NI.ZS = NI.ZS * ones(sim_settings.n_points);
  }
  if (NI.ZL.size() == 1) {
    NI.ZL = NI.ZL * ones(sim_settings.n_points);
  }

  for (unsigned int i = 0; i < sim_settings.freq.size(); i++) {
    Mat S = getSparams(NI.Ladder, NI.ZS.at(i), NI.ZL.at(i),
                       sim_settings.freq.at(i));
    S11.push_back(S(0, 0));
    S21.push_back(S(1, 0));
    S22.push_back(S(1, 1));
    S12.push_back(S(0, 1));
  }
}

vector<complex<double>> SparEngine::getSij(int i, int j) {
  if ((i == 1) && (j == 1))
    return S11;
  if ((i == 2) && (j == 1))
    return S21;
  if ((i == 2) && (j == 2))
    return S22;
  if ((i == 1) && (j == 2))
    return S12;
  return vector<complex<double>>(0, 0);
}

vector<double> SparEngine::getFreq() { return sim_settings.freq; }
