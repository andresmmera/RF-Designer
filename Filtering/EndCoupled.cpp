/***************************************************************************
                                EndCoupled.cpp
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
#include "EndCoupled.h"

EndCoupled::EndCoupled() {
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
}

EndCoupled::EndCoupled(FilterSpecifications FS) {
  Specification = FS;
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
}

EndCoupled::~EndCoupled() {}

void EndCoupled::synthesize() {
  LowpassPrototypeCoeffs LP_coeffs(Specification);
  gi = LP_coeffs.getCoefficients();

  Synthesize_ECF();

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

// This function synthesizes an end-coupled bandpass filter
// implementation Reference: Microstrip filters for RF/Microwave Applications.
// Jia-Sheng Hong. M. J. Lancaster. 2001. John Wiley and Sons. Pages 121-123.
void EndCoupled::Synthesize_ECF() {
  ComponentInfo TL, Cseries;
  WireInfo WI;
  NodeInfo NI;

  int N = Specification.order; // Number of elements
  int posx = 0;
  QString PreviousComponent;
  QString PreviousNode = QString("NS"), CurrentNode;
  Components.clear();

  double TL_length, theta, Baux = 0;
  double bw = Specification.bw / Specification.fc; // Fractional bandwidth
  double w0 = 2 * M_PI * Specification.fc;
  double Zline;
  double B, J, C, Z0 = Specification.ZS;
  double beta = 2 * M_PI * Specification.fc / SPEED_OF_LIGHT;
  double lambda_g0 = SPEED_OF_LIGHT / Specification.fc;

  // Add Term 1
  ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term,
                          180, posx, 0, "NS", "gnd");
  TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
  Components.append(TermSpar1);
  PreviousComponent = TermSpar1.ID;

  posx += 50;
  for (int k = 0; k <= N; k++) {
    CurrentNode = QString("N%1").arg(k);

    if (k == 0) { // First element
      J = sqrt(.5 * M_PI * bw / (gi[0] * gi[1]));
    } else {
      if (k == N) { // Last element
        J = sqrt(.5 * M_PI * bw / (gi[N] * gi[N + 1]));
      } else { // Resonator in the middle
        J = .5 * M_PI * bw / sqrt(gi[k] * gi[k + 1]);
      }
    }

    B = J / (1 - J * J);
    theta = M_PI - .5 * (atan(2 * Baux) + atan(2 * B));

    Baux = B;
    J /= Z0;
    B /= Z0;

    C = B / w0;
    TL_length = theta * lambda_g0 / (2 * M_PI); // - delta_e1 - delta_e2;

    if (k > 0) {
      // Transmission line
      TL.Connections.clear();
      TL.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                   TransmissionLine, 90, posx, 0,
                   QString("%1").arg(PreviousNode), QString("Nk%1").arg(k));
      TL.val["Z0"] = num2str(Z0, Resistance);
      TL.val["Length"] = ConvertLengthFromM("mm", TL_length);
      Components.append(TL);

      // Wire: TL to previous capacitor
      WI.setParams(PreviousComponent, 1, TL.ID, 0);
      Wires.append(WI);

      PreviousComponent = TL.ID;

      posx += 50;
    }

    // Series capacitor
    Cseries.Connections.clear();
    Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 90, posx, 0, QString("Nk%1").arg(k),
                      CurrentNode);
    Cseries.val["C"] = num2str(C, Capacitance);
    Components.append(Cseries);

    // Wire: Series capacitor to transmission line
    WI.setParams(Cseries.ID, 0, PreviousComponent, 1);
    Wires.append(WI);

    PreviousComponent = Cseries.ID;
    posx += 50;
  }

  // Add Term 2
  double k = Specification.ZS;
  if (Specification.UseZverevTables)
    (!Specification.isCLC) ? k /= gi[N + 1] : k *= gi[N + 1];
  else
    (Specification.isCLC) ? k /= gi[N + 1] : k *= gi[N + 1];

  ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                          posx, 0, QString("N%1").arg(N), "gnd");
  TermSpar2.val["Z"] = num2str(k, Resistance);
  Components.append(TermSpar2);

  WI.setParams(TermSpar2.ID, 0, PreviousComponent, 1);
  Wires.append(WI);
}

QList<ComponentInfo> EndCoupled::getComponents() { return Components; }

QList<WireInfo> EndCoupled::getWires() { return Wires; }

QList<NodeInfo> EndCoupled::getNodes() { return Nodes; }
