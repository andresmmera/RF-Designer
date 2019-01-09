/***************************************************************************
                                QuarterWaveFilters.cpp
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

#include "QuarterWaveFilters.h"

QuarterWaveFilters::QuarterWaveFilters() {
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
}

QuarterWaveFilters::QuarterWaveFilters(FilterSpecifications FS) {
  Specification = FS;
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
}

QuarterWaveFilters::~QuarterWaveFilters() {}

void QuarterWaveFilters::synthesize() {
  LowpassPrototypeCoeffs LP_coeffs(Specification);
  gi = LP_coeffs.getCoefficients();

  Synthesize_QW(Specification.FilterType);

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

void QuarterWaveFilters::Synthesize_QW(FilterClass response) {
  WireInfo WI;
  ComponentInfo QW_TL, OC_Stub, SC_Stub;
  NodeInfo NI;
  double lambda4 = SPEED_OF_LIGHT / (4. * Specification.fc);

  int N = Specification.order;
  double Z;
  double r = gi[N + 1];
  gi.pop_back();
  gi.pop_front();
  double fc = Specification.fc;
  double wc = 2 * M_PI * fc;
  double BW = Specification.bw;
  double bw = BW / fc;
  double Z0 = Specification.ZS;

  // Build schematic
  int posx = 0, Ni = 0;
  QString PreviousNode = "NS", CurrentNode;
  QString PreviousComp;
  Components.clear();

  ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term,
                          180, posx, 0, "NS", "gnd");
  TermSpar1.val["Z"] = num2str(Z0, Resistance);
  Components.append(TermSpar1);
  PreviousComp = TermSpar1.ID;
  posx -= 50;

  for (int k = 0; k < N; k++) {
    posx += 100;
    CurrentNode = QString("N%1").arg(k);
    // Quarter-wave transmission line
    QW_TL.Connections.clear();
    QW_TL.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                    TransmissionLine, 90, posx, 0,
                    QString("%1").arg(PreviousNode), CurrentNode);
    QW_TL.val["Z0"] = num2str(Z0, Resistance);
    QW_TL.val["Length"] = ConvertLengthFromM("mm", lambda4);
    Components.append(QW_TL);

    // Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx + 50, 0);
    Nodes.append(NI);

    // Wire: Connect the QW transmission line to the previous element (Term1 or
    // a node)
    WI.setParams(PreviousComp, 0, QW_TL.ID, 0);
    Wires.append(WI);

    // Wire: Connect the QW transmission line to the node
    WI.setParams(NI.ID, 0, QW_TL.ID, 1);
    Wires.append(WI);

    // Stubs
    switch (response) {
    default:
    case Bandpass:
      Z = (M_PI * Z0 * bw) / (4 * gi[k]);
      SC_Stub.Connections.clear();
      SC_Stub.setParams(
          QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
          ShortStub, 0, posx + 50, 50, CurrentNode, QString("gnd"));
      SC_Stub.val["Z0"] = num2str(Z, Resistance);
      SC_Stub.val["Length"] = ConvertLengthFromM("mm", lambda4);
      Components.append(SC_Stub);

      // Wire: Node to stub
      WI.setParams(NI.ID, 0, SC_Stub.ID, 1);
      Wires.append(WI);
      break;

    case Bandstop:
      Z = (4 * Z0) / (M_PI * bw * gi[k]);
      OC_Stub.Connections.clear();
      OC_Stub.setParams(
          QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), OpenStub,
          0, posx + 50, 50, CurrentNode, QString("NOPEN%1").arg(k));
      OC_Stub.val["Z0"] = num2str(Z, Resistance);
      OC_Stub.val["Length"] = ConvertLengthFromM("mm", lambda4);
      Components.append(OC_Stub);

      // Wire: Node to stub
      WI.setParams(NI.ID, 0, OC_Stub.ID, 1);
      Wires.append(WI);

      break;
    }
    PreviousComp = NI.ID;
    PreviousNode = CurrentNode;
  }
  posx += 100;
  // Quarter-wave transmission line
  QW_TL.Connections.clear();
  QW_TL.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                  TransmissionLine, 90, posx, 0,
                  QString("%1").arg(PreviousNode), QString("N%1").arg(N));
  QW_TL.val["Z0"] = num2str(Z0, Resistance);
  QW_TL.val["Length"] = ConvertLengthFromM("mm", lambda4);
  Components.append(QW_TL);

  ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                          posx + 50, 0, QString("N%1").arg(N), "gnd");
  TermSpar2.val["Z"] = num2str(Z0, Resistance);
  Components.append(TermSpar2);

  // Wire: Connect  QW line to the previous node
  WI.setParams(NI.ID, 0, QW_TL.ID, 0);
  Wires.append(WI);

  // Wire: Connect QW line to the SPAR term
  WI.setParams(TermSpar2.ID, 0, QW_TL.ID, 1);
  Wires.append(WI);
}

QList<ComponentInfo> QuarterWaveFilters::getComponents() { return Components; }

QList<WireInfo> QuarterWaveFilters::getWires() { return Wires; }

QList<NodeInfo> QuarterWaveFilters::getNodes() { return Nodes; }
