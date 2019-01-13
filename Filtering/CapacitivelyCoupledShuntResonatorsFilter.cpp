/***************************************************************************
                                CapacitivelyCoupledShuntResonatorsFilter.cpp
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
#include "CapacitivelyCoupledShuntResonatorsFilter.h"

CapacitivelyCoupledShuntResonatorsFilter::
    CapacitivelyCoupledShuntResonatorsFilter() {
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
}

CapacitivelyCoupledShuntResonatorsFilter::
    CapacitivelyCoupledShuntResonatorsFilter(FilterSpecifications FS) {
  Specification = FS;
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
}

CapacitivelyCoupledShuntResonatorsFilter::
    ~CapacitivelyCoupledShuntResonatorsFilter() {}

void CapacitivelyCoupledShuntResonatorsFilter::synthesize() {
  LowpassPrototypeCoeffs LP_coeffs(Specification);
  gi = LP_coeffs.getCoefficients();

  Synthesize_CCSRF();

  // Build Qucs netlist
  QucsNetlist.clear();
  QString codestr;
  for (int i = 0; i < Components.length(); i++) {
    codestr = Components[i].getQucs();
    if (!codestr.isEmpty())
      QucsNetlist += codestr;
  }

  // Ideally, the user should be the one which controls the style of the traces
  // as well the traces to be shown However, in favour of a simpler
  // implementation, it'll be the design code responsible for this... by the
  // moment...
  displaygraphs.clear();
  displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
  displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
}

// This function synthesizes a capacitively coupled resonators bandpass filter
// implementation Reference: Microwave Engineering. David M. Pozar. 4th Edition.
// 2012. John Wiley and Sons.Page 443-448.
void CapacitivelyCoupledShuntResonatorsFilter::Synthesize_CCSRF() {
  ComponentInfo SC_Stub, Cseries;
  WireInfo WI;
  NodeInfo NI;

  int N = Specification.order; // Number of elements
  int posx = 0;
  QString PreviousComponent, NextNode;
  QString PreviousNode = QString("NS"), CurrentNode;
  Components.clear();

  double delta = Specification.bw / Specification.fc; // Fractional bandwidth
  double w0 = 2 * M_PI * Specification.fc;
  double Z0 = Specification.ZS;
  double lambda0 = SPEED_OF_LIGHT / Specification.fc;
  double J[N + 1], C[N + 1], deltaC[N], l[N];

  // Add Term 1
  ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term,
                          180, posx, 0, "N0", "gnd");
  TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
  Components.append(TermSpar1);
  PreviousComponent = TermSpar1.ID;

  posx += 50;

  for (int k = 0; k < N; k++) {
    CurrentNode = QString("N%1").arg(k);

    if (k == 0) { // First element
      J[k] = sqrt(M_PI * delta / (4 * gi[k + 1])) / Z0;
      C[k] = J[k] / (w0 * sqrt(1 - Z0 * Z0 * J[k] * J[k]));
      // Series capacitor
      NextNode = QString("N%1").arg(k + 1);
      Cseries.Connections.clear();
      Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                        Capacitor, 90, posx, 0, CurrentNode, NextNode);
      Cseries.val["C"] = num2str(C[k], Capacitance);
      Components.append(Cseries);
      posx += 50;

      // Wire: Series capacitor to SPAR term
      WI.setParams(Cseries.ID, 0, TermSpar1.ID, 0);
      Wires.append(WI);
      PreviousComponent = Cseries.ID;
      continue;
    }

    J[k] = (0.25 * M_PI * delta / sqrt(gi[k] * gi[k + 1])) / Z0;
    C[k] = J[k] / w0;
    deltaC[k - 1] = -C[k - 1] - C[k];
    l[k - 1] = lambda0 / 4 + (Z0 * w0 * deltaC[k - 1] / (2 * M_PI)) * lambda0;
    if (l[k - 1] < 0)
      l[k - 1] += lambda0 / 4;

    // Short stub
    SC_Stub.Connections.clear();
    SC_Stub.setParams(
        QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), ShortStub,
        0, posx, 25, CurrentNode, QString("gnd"));
    SC_Stub.val["Z0"] = num2str(Z0, Resistance);
    SC_Stub.val["Length"] = ConvertLengthFromM("mm", l[k - 1]);
    Components.append(SC_Stub);
    posx += 50;

    // Wire: Series capacitor to SPAR term
    WI.setParams(SC_Stub.ID, 1, PreviousComponent, 1);
    Wires.append(WI);

    // Series capacitor
    NextNode = QString("N%1").arg(k + 1);
    Cseries.Connections.clear();
    Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 90, posx, 0, CurrentNode, NextNode);
    Cseries.val["C"] = num2str(C[k], Capacitance);
    Components.append(Cseries);

    // Wire: Series capacitor to transmission line
    WI.setParams(SC_Stub.ID, 1, Cseries.ID, 0);
    Wires.append(WI);

    PreviousComponent = Cseries.ID;
    posx += 50;
  }

  // Last short stub + C series section
  J[N] = sqrt(M_PI * delta / (4 * gi[N + 1] * gi[N])) / Z0;
  C[N] = J[N] / (w0 * sqrt(1 - Z0 * Z0 * J[N] * J[N]));
  deltaC[N - 1] = -C[N] - C[N - 1];
  l[N - 1] = lambda0 / 4 + (Z0 * w0 * deltaC[N - 1] / (2 * M_PI)) * lambda0;
  if (l[N - 1] < 0)
    l[N - 1] += lambda0 / 4;
  CurrentNode = QString("N%1").arg(N);

  // Short stub
  SC_Stub.Connections.clear();
  SC_Stub.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                    ShortStub, 0, posx, 25, CurrentNode, QString("gnd"));
  SC_Stub.val["Z0"] = num2str(Z0, Resistance);
  SC_Stub.val["Length"] = ConvertLengthFromM("mm", l[N - 1]);
  Components.append(SC_Stub);
  posx += 50;

  // Series capacitor
  NextNode = QString("N%1").arg(N + 1);
  Cseries.Connections.clear();
  Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                    Capacitor, 90, posx, 0, CurrentNode, NextNode);
  Cseries.val["C"] = num2str(C[N], Capacitance);
  Components.append(Cseries);
  posx += 50;

  WI.setParams(SC_Stub.ID, 1, PreviousComponent, 1);
  Wires.append(WI);

  WI.setParams(SC_Stub.ID, 1, Cseries.ID, 0);
  Wires.append(WI);

  // Add Term 2
  double k = Specification.ZS;
  if (Specification.UseZverevTables)
    (!Specification.isCLC) ? k /= gi[N + 1] : k *= gi[N + 1];
  else
    (Specification.isCLC) ? k /= gi[N + 1] : k *= gi[N + 1];

  ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                          posx, 0, QString("N%1").arg(N + 1), "gnd");
  TermSpar2.val["Z"] = num2str(k, Resistance);
  Components.append(TermSpar2);

  WI.setParams(TermSpar2.ID, 0, Cseries.ID, 1);
  Wires.append(WI);
}

QList<ComponentInfo> CapacitivelyCoupledShuntResonatorsFilter::getComponents() {
  return Components;
}

QList<WireInfo> CapacitivelyCoupledShuntResonatorsFilter::getWires() {
  return Wires;
}

QList<NodeInfo> CapacitivelyCoupledShuntResonatorsFilter::getNodes() {
  return Nodes;
}
