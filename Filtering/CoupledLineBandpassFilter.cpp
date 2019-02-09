/***************************************************************************
                                CoupledLineBandpassFilter.cpp
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
#include "CoupledLineBandpassFilter.h"

CoupledLineBandpassFilter::CoupledLineBandpassFilter() {}

CoupledLineBandpassFilter::CoupledLineBandpassFilter(FilterSpecifications FS) {
  Specification = FS;
}

CoupledLineBandpassFilter::~CoupledLineBandpassFilter() {}

// This function synthesizes a coupled line bandpass filter
// implementation Reference: Microwave Engineering. David M. Pozar. 4th Edition.
// 2012. John Wiley and Sons.Page 430-437.
void CoupledLineBandpassFilter::synthesize() {
  LowpassPrototypeCoeffs LP_coeffs(Specification);
  std::deque<double> gi = LP_coeffs.getCoefficients();

  ComponentInfo Coupled_Lines;
  WireInfo WI;
  QStringList ConnectionNodes;

  int N = Specification.order; // Number of elements
  int posx = 0, posy = 10;
  int Nopen = 0;
  QString PreviousComponent, NextNode;
  QString PreviousNode, CurrentNode = QString("N0");

  double delta = Specification.bw / Specification.fc; // Fractional bandwidth
  double Z0 = Specification.ZS;
  double lambda0 = SPEED_OF_LIGHT / Specification.fc;
  double l4 = 0.25 * lambda0;
  double J[N + 1], Z0e[N + 1], Z0o[N + 1];

  // Add Term 1
  ComponentInfo TermSpar1(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 180, posx,
      0, CurrentNode, "gnd");
  TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
  Schematic.appendComponent(TermSpar1);
  PreviousComponent = TermSpar1.ID;
  PreviousNode = CurrentNode;
  posx += 50;

  for (int k = 0; k < N; k++) {
    CurrentNode = QString("N%1").arg(k);
    NextNode = QString("N%1").arg(k + 1);

    if (k == 0) {                                            // First element
      J[k] = sqrt(M_PI * delta / (2 * gi[k + 1])) / Z0;      // Eq. 8.121
      Z0e[k] = Z0 * (1 + J[k] * Z0 + J[k] * J[k] * Z0 * Z0); // Eq. 8.108a
      Z0o[k] = Z0 * (1 - J[k] * Z0 + J[k] * J[k] * Z0 * Z0); // Eq. 8.108b

      Coupled_Lines.Connections.clear();
      // Set connections
      ConnectionNodes.clear();
      ConnectionNodes.append(QString("%1").arg(PreviousNode));
      ConnectionNodes.append(QString("NOPEN%1").arg(Nopen)), Nopen++;
      ConnectionNodes.append(QString("%1").arg(NextNode));
      ConnectionNodes.append(QString("NOPEN%1").arg(Nopen)), Nopen++;
      Coupled_Lines.setParams(
          QString("COUPL%1").arg(++Schematic.NumberComponents[CoupledLines]),
          CoupledLines, 90, posx, posy, ConnectionNodes);
      Coupled_Lines.val["Ze"] = num2str(Z0e[k], Resistance);
      Coupled_Lines.val["Zo"] = num2str(Z0o[k], Resistance);
      Coupled_Lines.val["Length"] = ConvertLengthFromM("mm", l4);
      Schematic.appendComponent(Coupled_Lines);
      posx += 75;
      posy += 20;

      // Wire: Series capacitor to SPAR term
      Schematic.appendWire(Coupled_Lines.ID, 2, TermSpar1.ID, 0);
      PreviousComponent = Coupled_Lines.ID;
      PreviousNode = NextNode;
      continue;
    }

    J[k] = (0.5 * M_PI * delta / sqrt(gi[k] * gi[k + 1])) / Z0; // Eq. 8.121
    Z0e[k] = Z0 * (1 + J[k] * Z0 + J[k] * J[k] * Z0 * Z0);      // Eq. 8.108a
    Z0o[k] = Z0 * (1 - J[k] * Z0 + J[k] * J[k] * Z0 * Z0);      // Eq. 8.108b

    // Coupled lines
    Coupled_Lines.Connections.clear();
    ConnectionNodes[0] = QString("%1").arg(PreviousNode);
    ConnectionNodes[1] = QString("NOPEN%1").arg(Nopen), Nopen++;
    ConnectionNodes[2] = QString("%1").arg(NextNode);
    ConnectionNodes[3] = QString("NOPEN%1").arg(Nopen), Nopen++;
    Coupled_Lines.setParams(
        QString("COUPL%1").arg(++Schematic.NumberComponents[CoupledLines]),
        CoupledLines, 90, posx, posy, ConnectionNodes);
    Coupled_Lines.val["Ze"] = num2str(Z0e[k], Resistance);
    Coupled_Lines.val["Zo"] = num2str(Z0o[k], Resistance);
    Coupled_Lines.val["Length"] = ConvertLengthFromM("mm", l4);
    Schematic.appendComponent(Coupled_Lines);

    // Wire: Series capacitor to SPAR term
    Schematic.appendWire(Coupled_Lines.ID, 2, PreviousComponent, 1);

    PreviousComponent = Coupled_Lines.ID;
    PreviousNode = NextNode;
    posx += 75;
    posy += 20;
  }

  // Last short stub + C series section
  J[N] = sqrt(M_PI * delta / (2 * gi[N + 1] * gi[N])) / Z0;
  Z0e[N] = Z0 * (1 + J[N] * Z0 + J[N] * J[N] * Z0 * Z0); // Eq. 8.108a
  Z0o[N] = Z0 * (1 - J[N] * Z0 + J[N] * J[N] * Z0 * Z0); // Eq. 8.108b
  CurrentNode = QString("N%1").arg(N);
  NextNode = QString("N%1").arg(N + 1);

  // Coupled lines
  Coupled_Lines.Connections.clear();
  ConnectionNodes[0] = QString("%1").arg(PreviousNode);
  ConnectionNodes[1] = QString("NOPEN%1").arg(Nopen), Nopen++;
  ConnectionNodes[2] = QString("%1").arg(NextNode);
  ConnectionNodes[3] = QString("NOPEN%1").arg(Nopen), Nopen++;
  Coupled_Lines.setParams(
      QString("COUPL%1").arg(++Schematic.NumberComponents[CoupledLines]),
      CoupledLines, 90, posx, posy, ConnectionNodes);
  Coupled_Lines.val["Ze"] = num2str(Z0e[N], Resistance);
  Coupled_Lines.val["Zo"] = num2str(Z0o[N], Resistance);
  Coupled_Lines.val["Length"] = ConvertLengthFromM("mm", l4);
  Schematic.appendComponent(Coupled_Lines);

  posx += 50;
  posy += 10;
  Schematic.appendWire(PreviousComponent, 1, Coupled_Lines.ID, 2);

  // Add Term 2
  double k = Specification.ZS;
  if (Specification.UseZverevTables)
    (!Specification.isCLC) ? k /= gi[N + 1] : k *= gi[N + 1];
  else
    (Specification.isCLC) ? k /= gi[N + 1] : k *= gi[N + 1];

  ComponentInfo TermSpar2(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 0, posx,
      posy, NextNode, "gnd");
  TermSpar2.val["Z"] = num2str(k, Resistance);
  Schematic.appendComponent(TermSpar2);

  Schematic.appendWire(Coupled_Lines.ID, 1, TermSpar2.ID, 0);

  Schematic.clearGraphs();
  Schematic.appendGraph(QString("S[2,1]"), QPen(Qt::red, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[1,1]"), QPen(Qt::blue, 1, Qt::SolidLine));
}
