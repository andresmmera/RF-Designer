/***************************************************************************
                                EllipticFilter.h
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
#ifndef ELLIPTICFILTER_H
#define ELLIPTICFILTER_H
#include "Schematic/Network.h"
#include "Schematic/component.h"
#include "Schematic/structures.h"
#include "general.h"
#include <QPen>

class ComponentInfo;
class WireInfo;

class EllipticFilter : public Network {
public:
  EllipticFilter();
  EllipticFilter(FilterSpecifications);
  virtual ~EllipticFilter();
  void synthesize();
  void setSemilumpedMode(bool);

private:
  struct FilterSpecifications Specification;

  std::vector<double> *Cseries_LP, *Lseries_LP,
      *Cshunt_LP;          // Elliptic network parameters
  double RL;               // Load resistance
  bool semilumped = false; // Activate semilumped implementation mode
  unsigned int virtual_nodes;

  // Lowpass prototype
  void EllipticTypeS();
  void EllipticTypesABC();
  double Sn(double, double);
  std::complex<double> Sn(double, std::complex<double>);

  //***********  Schematic synthesis ********************
  void SynthesizeEllipticFilter();
  void InsertEllipticSection(int &, QMap<QString, unsigned int> &, int, bool,
                             bool);

  void Insert_LowpassMinL_Section(int &, QMap<QString, unsigned int> &,
                                  unsigned int, bool, bool);
  void Insert_HighpassMinC_Section(int &, QMap<QString, unsigned int> &,
                                   unsigned int, bool, bool);

  void Insert_LowpassMinC_Section(int &, QMap<QString, unsigned int> &,
                                  unsigned int, bool, bool);
  void Insert_HighpassMinL_Section(int &, QMap<QString, unsigned int> &,
                                   unsigned int, bool, bool);
  void Insert_LowpassSemilumpedMinC_Section(int &,
                                            QMap<QString, unsigned int> &,
                                            unsigned int, bool, bool);
  void Insert_HighpassSemilumpedMinL_Section(int &,
                                             QMap<QString, unsigned int> &,
                                             unsigned int, bool, bool);

  void Insert_Bandpass_1_Section(int &, QMap<QString, unsigned int> &,
                                 unsigned int, bool, bool);
  void Insert_Bandpass_2_Section(int &, QMap<QString, unsigned int> &,
                                 unsigned int, bool, bool);

  void Insert_Bandstop_1_Section(int &, QMap<QString, unsigned int> &,
                                 unsigned int, bool, bool);
  void Insert_Bandstop_2_Section(int &, QMap<QString, unsigned int> &,
                                 unsigned int, bool, bool);
};
#endif
