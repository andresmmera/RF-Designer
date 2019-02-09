/***************************************************************************
                                sparengine.h
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
#ifndef SPARENGINE_H
#define SPARENGINE_H

#include "MathOperations.h"
#include "Schematic/Network.h"
#include "Schematic/structures.h"
#include <set>
#include <string.h>

using namespace std;

class SparEngine {
  const double c0 = 299792458; // Speed of light (m/s)
public:
  SparEngine();
  Mat getSparams(QList<ComponentInfo>, complex<double>, complex<double>,
                 double);
  Mat getABCDmatrix(QList<ComponentInfo>, double);
  void setNetwork(NetworkInfo);
  void setSimulationSettings(SP_Analysis);
  QMap<QString, vector<complex<double>>> getData();
  void run();
  vector<complex<double>> getSij(int, int);
  vector<double> getFreq();

private:
  NetworkInfo NI;
  SP_Analysis sim_settings;
  vector<complex<double>> S11, S22, S21, S12;
};

#endif // SPARENGINE_H
