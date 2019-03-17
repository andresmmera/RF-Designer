/***************************************************************************
                                io.h
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

#ifndef IO_H
#define IO_H

#include "SPAR/MathOperations.h"
#include "general.h"
#include <complex>
#include <fstream>
#include <locale>
#include <queue>

using namespace std;
enum terminal { SOURCE, LOAD };

class IO {
public:
  IO();
  ~IO();
  int loadS1Pdata(std::string, terminal);
  int loadS2Pdata(std::string);
  int ResampleImpedances();
  vector<double> getFrequency();
  void set_constant_ZS_vs_freq(complex<double>);
  void set_constant_ZL_vs_freq(complex<double>);
  void set_matching_band(double, double);
  string tmp_path; // Path to a temporary directory for data dumping
  bool Two_Port_Matching;

  // ZS and ZL are the source and load impedances, respectively whereas fS and
  // fL indicates the frequencies where ZS and ZL were sampled
  vector<complex<double>> ZS, ZL;
  vector<complex<double>> Zin_maxg,
      Zout_maxg;       // Optimal Zin and Zout for achieving maximum gain on a
                       // two-port device
  vector<double> freq; // More often than not, ZS and ZL are sampled at
                       // different frecuencies, so it is necessary to have
  // common frequency vector for pairing ZS and ZL.
  S2P_DATA getS2P();

private:
  vector<double> fS, fL, fAMP; // Frequencies at which the input source, load
                               // and amplifier data are sampled

  double fmatching_min, fmatching_max;
  int getFreqIndex(double);

  vector<double> f_matching;
  double getS2PfreqScale(string line);

  int Nsamples; // Impedance samples within matching band

  string Num2String(int x);
  string Num2String(double x);
  S2P_DATA DeviceS2P;

  void generateConstant_s1p(string, complex<double>);
};

#endif // IO_H
