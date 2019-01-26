/***************************************************************************
                                Network.h
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
#ifndef NETWORK_H
#define NETWORK_H

#include <QFile>
#include <QList>
#include <QMap>
#include <QStringList>
#include <cmath>
#include <complex>
#include <deque>
#include <list>
#include <vector>

#include "Schematic/component.h"
#include <QDebug>
#include <QtSql>

enum ResponseType {
  Butterworth,
  Chebyshev,
  Legendre,
  Elliptic,
  Blichinkoff,
  Bessel,
  LinearPhaseEqError,
  Gegenbauer
};
enum FilterClass { Lowpass, Highpass, Bandpass, Bandstop };
enum Coupling {
  CapacitativeCoupledShuntResonators,
  InductiveCoupledSeriesResonators
};
enum SemiLumpedImplementation { ONLY_INDUCTORS, INDUCTORS_AND_SHUNT_CAPS };

static const double SPEED_OF_LIGHT =
    299792458.0; // REMOVE THIS WHEN THIS TOOL BECOMES INTEGRATED IN QUCS

struct PrototypeTableProperties {
  QString ID;
  std::vector<int> N;
  std::vector<double> Ripple;
  std::vector<double> RL;
};

struct FilterSpecifications {
  FilterClass FilterType;      // Lowpass, Highpass, Bandpass, Bandstop
  ResponseType FilterResponse; // Butterworth, Chebyshev, Cauer, etc...
  QString Implementation;
  Coupling DC_Coupling; // Only for bandpass direct coupled filters
  bool isCLC;           // CLC or LCL implementation
  unsigned int order;   // Filter order
  double Ripple;        // Ripple (Chebyshev and Cauer)
  double fc; // Cutoff frequency (lowpass and highpass filters) or center
             // frequency (bandpass and bandstop)
  double bw; // Filter bandwidth
  double ZS; // Source impedance
  double ZL; // Load impedance
  double as; // Stopband attenuation
  QString EllipticType; // Type of elliptic filter
  bool UseZverevTables;
  double minZ; // Minimum synthesizable impedance (only for stepped impedance
               // filters)
  double maxZ; // Maximum synthesizable impedance (only for stepped impedance
               // filters)
  SemiLumpedImplementation SemiLumpedISettings;
  double ImpedanceRatio;
};

struct NetworkInfo {
  std::vector<std::complex<double>> ZS;
  std::vector<std::complex<double>> ZL;
  QStringList topology;
  QList<struct ComponentInfo>
      Ladder; // Contains the ID of the components and their properties
};

struct SP_Analysis {
  unsigned int n_points;
  double fstart;
  double fstop;
  std::vector<double> freq;
};

struct SchematicInfo {
  struct SP_Analysis SPAR_Settings;
  QString netlist;
  QList<struct ComponentInfo> Comps;
  QList<struct WireInfo> Wires;
  QList<struct NodeInfo> Nodes;
  QMap<QString, QPen> displayGraphs;
  QString Description;
  QVector<QPointF> ImpedanceTrace;
};

struct PowerCombinerParams {
  QString Type; // Wilkinson, branchlines, Bagley, etc.
  int Noutputs; // Number of output branches
  int Nstages;  // Number of combiner stages (broadband Wilkinson)
  std::deque<double> OutputRatio; // Splitting ratio
  QString Implementation;         // LC, microstrip, ideal TL
  double alpha;                   // Attenuation constant of the ideal TL
  QString units;                  // mm, mil, um
  double freq;                    // Center freq
  double Z0;                      // Reference impedance
};

class Network {
public:
  virtual QList<ComponentInfo> getComponents() = 0;
  virtual QList<WireInfo> getWires() = 0;
  virtual QList<NodeInfo> getNodes() = 0;
  virtual void synthesize() = 0;
};

#endif // NETWORK_H
