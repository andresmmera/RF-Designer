/***************************************************************************
                                CapacitivelyCoupledShuntResonatorsFilter.h
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
#ifndef CAPACITIVELYCOUPLEDSHUNTRESONATORS_H
#define CAPACITIVELYCOUPLEDSHUNTRESONATORS_H

#include "Filtering/LowpassPrototypeCoeffs.h"
#include "Filtering/Network.h"
#include "Schematic/component.h"
#include "general.h"
#include <QPen>

class CapacitivelyCoupledShuntResonatorsFilter : public Network {
public:
  CapacitivelyCoupledShuntResonatorsFilter();
  virtual ~CapacitivelyCoupledShuntResonatorsFilter();
  CapacitivelyCoupledShuntResonatorsFilter(FilterSpecifications);
  QList<ComponentInfo> getComponents();
  QList<WireInfo> getWires();
  QList<NodeInfo> getNodes();
  void synthesize();
  NetworkInfo getLadder();
  QString getQucsNetlist() { return QucsNetlist; };
  QMap<QString, QPen> displaygraphs;

private:
  struct FilterSpecifications Specification;
  QList<ComponentInfo> Components;
  QList<WireInfo> Wires;
  QList<NodeInfo> Nodes;

  QString QucsNetlist;

  QMap<ComponentType, int>
      NumberComponents;  // List for assigning IDs to the filter components
  std::deque<double> gi; // Lowpass prototype
  std::vector<std::complex<double>> Poles;
  std::vector<std::complex<double>> Zeros;

  void Synthesize_CCSRF();
};

#endif // CAPACITIVELYCOUPLEDSHUNTRESONATORS_H
