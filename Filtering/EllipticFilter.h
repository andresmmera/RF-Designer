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
#include "Filtering/Network.h"
#include "general.h"
#include <QPen>

class EllipticFilter : public Network {
public:
  EllipticFilter();
  EllipticFilter(FilterSpecifications);
  virtual ~EllipticFilter();
  QList<ComponentInfo> getComponents();
  QList<WireInfo> getWires();
  QList<NodeInfo> getNodes();
  void synthesize();
  NetworkInfo getLadder();
  QString getQucsNetlist() { return QucsNetlist; }
  QMap<QString, QPen> displaygraphs;

private:
  struct FilterSpecifications Specification;
  QList<ComponentInfo> Components;
  QList<WireInfo> Wires;
  QList<NodeInfo> Nodes;

  QString QucsNetlist;

  QMap<ComponentType, int>
      NumberComponents; // List for assigning IDs to the filter components
  std::vector<double> *Cseries_LP, *Lseries_LP,
      *Cshunt_LP; // Elliptic network parameters
  double RL;      // Load resistance
  std::vector<std::complex<double>> Poles;
  std::vector<std::complex<double>> Zeros;
  unsigned int virtual_nodes;

  // Lowpass prototype
  void EllipticTypeS();
  void EllipticTypesABC();
  double Sn(double, double);
  std::complex<double> Sn(double, std::complex<double>);

  //***********  Schematic synthesis ********************
  void SynthesizeEllipticFilter();
  void InsertEllipticSection(int &, unsigned int &,
                             QMap<QString, unsigned int> &, int, bool, bool);

  void Insert_LowpassMinL_Section(int &, unsigned int &,
                                  QMap<QString, unsigned int> &, unsigned int,
                                  bool, bool);
  void Insert_HighpassMinC_Section(int &, unsigned int &,
                                   QMap<QString, unsigned int> &, unsigned int,
                                   bool, bool);

  void Insert_LowpassMinC_Section(int &, unsigned int &,
                                  QMap<QString, unsigned int> &, unsigned int,
                                  bool, bool);
  void Insert_HighpassMinL_Section(int &, unsigned int &,
                                   QMap<QString, unsigned int> &, unsigned int,
                                   bool, bool);

  void Insert_Bandpass_1_Section(int &, unsigned int &,
                                 QMap<QString, unsigned int> &, unsigned int,
                                 bool, bool);
  void Insert_Bandpass_2_Section(int &, unsigned int &,
                                 QMap<QString, unsigned int> &, unsigned int,
                                 bool, bool);

  void Insert_Bandstop_1_Section(int &, unsigned int &,
                                 QMap<QString, unsigned int> &, unsigned int,
                                 bool, bool);
  void Insert_Bandstop_2_Section(int &, unsigned int &,
                                 QMap<QString, unsigned int> &, unsigned int,
                                 bool, bool);
};
#endif
