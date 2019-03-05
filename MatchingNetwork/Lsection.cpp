/***************************************************************************
                                Lsection.cpp
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
#include "MatchingNetwork/Lsection.h"

Lsection::Lsection() {}

Lsection::Lsection(MatchingNetworkDesignParameters AS) { Specs = AS; }

Lsection::~Lsection() {}

// Reference: RF design guide. Systems, circuits, and equations. Peter
// Vizmuller. Artech House, 1995
void Lsection::synthesize() {
  // Port 1
  ComponentInfo TermSpar1(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 180, 0, 0);
  TermSpar1.val["Z"] = num2str(Specs.Zin.real(), Resistance);
  Schematic.appendComponent(TermSpar1);

  // Port 2
  ComponentInfo TermSpar2(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 0, 200, 0);
  TermSpar2.val["Z"] = num2str(Specs.Zout, Resistance);

  ComponentInfo Shunt, Series, Ground;
  NodeInfo NI;

  // Design equations
  double w0 = 2.0 * M_PI * Specs.freqStart;
  double L, C, X, B;

  double Z0 = Specs.Zin.real();
  double RL = Specs.Zout.real();
  double XL = Specs.Zout.imag();

  if (Z0 > RL) {
    // ZS -------- X -- ZL
    //       |
    //       B
    //       |
    //      ---

    // Solution 1
    if (Specs.Solution == 1) {
      X = sqrt(RL * (Z0 - RL)) - XL;
      B = sqrt((Z0 - RL) / RL) / Z0;
    } else {
      // Solution 2
      X = -sqrt(RL * (Z0 - RL)) - XL;
      B = -sqrt((Z0 - RL) / RL) / Z0;
    }

    // Shunt element
    if (B > 0) // Capacitor
    {
      C = B / w0;
      // Lumped capacitor
      Shunt.setParams(
          QString("C%1").arg(++Schematic.NumberComponents[Capacitor]),
          Capacitor, 0.0, 50, 50);
      Shunt.val["C"] = num2str(C, Capacitance);
    } else { // Inductor
      L = -1 / (w0 * B);
      Shunt.setParams(
          QString("L%1").arg(++Schematic.NumberComponents[Inductor]), Inductor,
          0, 50, 50);
      Shunt.val["L"] = num2str(L, Inductance);
    }
    Schematic.appendComponent(Shunt);
    // GND
    Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]),
                     GND, 0, 50, 100);
    Schematic.appendComponent(Ground);

    // Node
    NI.setParams(
        QString("N%1").arg(++Schematic.NumberComponents[ConnectionNodes]), 50,
        0);
    Schematic.appendNode(NI);

    // Series element
    if (X < 0) // Capacitor
    {
      C = -1 / (w0 * X);
      Series.setParams(
          QString("C%1").arg(++Schematic.NumberComponents[Capacitor]),
          Capacitor, 270, 100, 0);
      Series.val["C"] = num2str(C, Capacitance);
    } else { // Inductor
      L = X / w0;
      Series.setParams(
          QString("L%1").arg(++Schematic.NumberComponents[Inductor]), Inductor,
          -90, 100, 0);
      Series.val["L"] = num2str(L, Inductance);
    }
    Schematic.appendComponent(Series);

    // Wires
    Schematic.appendWire(TermSpar1.ID, 0, NI.ID, 0);
    Schematic.appendWire(Shunt.ID, 1, NI.ID, 0);
    Schematic.appendWire(Shunt.ID, 0, Ground.ID, 0);
    Schematic.appendWire(NI.ID, 0, Series.ID, 1);
    Schematic.appendWire(Series.ID, 0, TermSpar2.ID, 0);

  } else {
    // Z0 < RL
    // ZS --- X  ------- ZL
    //              |
    //              B
    //              |
    //             ---

    // Solution 1
    if (Specs.Solution == 1) {
      B = (XL + sqrt(RL / Z0) * sqrt(RL * RL + XL * XL - Z0 * RL)) /
          (RL * RL + XL * XL);
      X = 1 / B + XL * Z0 / RL - Z0 / (B * RL);
    } else {
      // Solution 2
      B = (XL - sqrt(RL / Z0) * sqrt(RL * RL + XL * XL - Z0 * RL)) /
          (RL * RL + XL * XL);
      X = 1 / B + XL * Z0 / RL - Z0 / (B * RL);
    }

    // Series element
    if (X < 0) { // Capacitor
      C = -1 / (w0 * X);
      Series.setParams(
          QString("C%1").arg(++Schematic.NumberComponents[Capacitor]),
          Capacitor, 270, 50, 0);
      Series.val["C"] = num2str(C, Capacitance);
    } else { // Inductor
      L = X / w0;
      Series.setParams(
          QString("L%1").arg(++Schematic.NumberComponents[Inductor]), Inductor,
          -90, 50, 0);
      Series.val["L"] = num2str(L, Inductance);
    }
    Schematic.appendComponent(Series);

    // Node
    NI.setParams(
        QString("N%1").arg(++Schematic.NumberComponents[ConnectionNodes]), 100,
        0);
    Schematic.appendNode(NI);

    // Shunt element
    if (B > 0) { // Capacitor
      C = B / w0;
      Shunt.setParams(
          QString("C%1").arg(++Schematic.NumberComponents[Capacitor]),
          Capacitor, 0.0, 100, 50);
      Shunt.val["C"] = num2str(C, Capacitance);
    } else { // Inductor
      L = -1 / (w0 * B);
      Shunt.setParams(
          QString("L%1").arg(++Schematic.NumberComponents[Inductor]), Inductor,
          0, 100, 50);
      Shunt.val["L"] = num2str(L, Inductance);
    }
    Schematic.appendComponent(Shunt);
    // GND
    Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]),
                     GND, 0, 100, 100);
    Schematic.appendComponent(Ground);

    // Wires
    Schematic.appendWire(TermSpar1.ID, 0, Series.ID, 1);
    Schematic.appendWire(Series.ID, 0, NI.ID, 0);
    Schematic.appendWire(NI.ID, 0, Shunt.ID, 1);
    Schematic.appendWire(Shunt.ID, 0, Ground.ID, 0);
    Schematic.appendWire(NI.ID, 0, TermSpar2.ID, 0);
  }

  Schematic.appendComponent(
      TermSpar2); // The S-parameter port2 was already instantiated. However, it
                  // is added to the Schematic here to build the ladder
                  // correctly

  Schematic.clearGraphs();
  Schematic.appendGraph(QString("S[2,1]"), QPen(Qt::red, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[1,1]"), QPen(Qt::blue, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[2,2]"), QPen(Qt::green, 1, Qt::SolidLine));
}
