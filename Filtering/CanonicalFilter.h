/***************************************************************************
                                CanonicalFilter.h
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
#ifndef CANONICALFILTER_H
#define CANONICALFILTER_H

#include "Filtering/LowpassPrototypeCoeffs.h"
#include "Filtering/Network.h"
#include "Schematic/component.h"
#include "general.h"
#include <QPen>
class CanonicalFilter : public Network {
public:
  CanonicalFilter();
  virtual ~CanonicalFilter();
  CanonicalFilter(FilterSpecifications);
  QList<ComponentInfo> getComponents();
  QList<WireInfo> getWires();
  QList<NodeInfo> getNodes();
  void synthesize();
  NetworkInfo getLadder();
  QString getQucsNetlist() { return QucsNetlist; }
  QMap<QString, QPen> displaygraphs;
  void setSemilumpedMode(bool);

private:
  struct FilterSpecifications Specification;
  QList<ComponentInfo> Components;
  QList<WireInfo> Wires;
  QList<NodeInfo> Nodes;
  bool semilumped = false; // Activate semilumped implementation mode

  QString QucsNetlist;

  QMap<ComponentType, int>
      NumberComponents; // List for assigning IDs to the filter components
  std::vector<std::complex<double>> Poles;
  std::vector<std::complex<double>> Zeros;
  std::deque<double> gi;

  //***********  Schematic synthesis ********************
  void SynthesizeLPF();           // Lowpass
  void SynthesizeSemilumpedLPF(); // Semilumped Lowpass implementation
  void SynthesizeHPF();           // Highpass
  void SynthesizeBPF();           // Bandpass
  void SynthesizeBSF();           // Bandstop
};

#endif // CANONICALFILTER_H
