/***************************************************************************
                                SteppedImpedanceFilter.cpp
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

#include "SteppedImpedanceFilter.h"

SteppedImpedanceFilter::SteppedImpedanceFilter() {
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
}

SteppedImpedanceFilter::SteppedImpedanceFilter(FilterSpecifications FS) {
  Specification = FS;
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
}

SteppedImpedanceFilter::~SteppedImpedanceFilter() {}

void SteppedImpedanceFilter::synthesize() {
  LowpassPrototypeCoeffs LP_coeffs(Specification);
  gi = LP_coeffs.getCoefficients();

  Synthesize_STIF();

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

// This function synthesizes a lowpass filter using the stepped-impedance
// implementation Reference: Microwave Engineering. David M. Pozar. 4th Edition.
// 2012. John Wiley and Sons.Page 424.
void SteppedImpedanceFilter::Synthesize_STIF() {
  ComponentInfo TL;
  WireInfo WI;
  NodeInfo NI;

  int N = Specification.order; // Number of elements
  int posx = 0;
  QString PreviousComponent;
  QString PreviousNode = QString("NS"), CurrentNode;
  Components.clear();

  double TL_length;
  double Zlow = Specification.minZ;
  double Zhigh = Specification.maxZ;
  double Zline;
  double beta = 2 * M_PI * Specification.fc / SPEED_OF_LIGHT;

  // Add Term 1
  ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term,
                          180, posx, 0, "NS", "gnd");
  TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
  Components.append(TermSpar1);
  PreviousComponent = TermSpar1.ID;

  int Kcontrol = 0;
  if (!Specification.UseZverevTables)
    Kcontrol = 0;
  if (Specification.UseZverevTables && (Specification.order % 2 == 0))
    Kcontrol = 1;
  posx += 50;
  for (int k = 0; k < N; k++) {
    CurrentNode = QString("N%1").arg(k);
    if (((Specification.isCLC) && (k % 2 == Kcontrol)) ||
        ((!Specification.isCLC) && (k % 2 != Kcontrol))) {
      // Replace shunt capacitor
      Zline = Zlow;
      TL_length = gi[k + 1] * Zlow / (beta * Specification.ZS);
    } else {
      // Replace series inductor
      Zline = Zhigh;
      TL_length = gi[k + 1] * Specification.ZS / (beta * Zhigh);
    }

    // Short transmission line
    TL.Connections.clear();
    TL.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                 TransmissionLine, 90, posx, 0, QString("%1").arg(PreviousNode),
                 CurrentNode);
    TL.val["Z0"] = num2str(Zline, Resistance);
    TL.val["Length"] = ConvertLengthFromM("mm", TL_length);
    Components.append(TL);

    // Wiring
    WI.setParams(PreviousComponent, 1, TL.ID, 0);
    Wires.append(WI);

    PreviousComponent = TL.ID;
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

QList<ComponentInfo> SteppedImpedanceFilter::getComponents() {
  return Components;
}

QList<WireInfo> SteppedImpedanceFilter::getWires() { return Wires; }

QList<NodeInfo> SteppedImpedanceFilter::getNodes() { return Nodes; }
