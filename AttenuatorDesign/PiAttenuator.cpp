/***************************************************************************
                                PiAttenuator.cpp
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
#include "AttenuatorDesign/PiAttenuator.h"

PiAttenuator::PiAttenuator() {}

PiAttenuator::PiAttenuator(AttenuatorDesignParameters AS) { Specs = AS; }

PiAttenuator::~PiAttenuator() {}

// Reference: RF design guide. Systems, circuits, and equations. Peter
// Vizmuller. Artech House, 1995
void PiAttenuator::synthesize() {
  ComponentInfo TermSpar1, TermSpar2;
  ComponentInfo Ground, Res1, Res2, Res3;
  NodeInfo NI;

  // Design equations
  double L = pow(10, .1 * Specs.Attenuation);
  double R2 = (.5 * (L - 1)) * sqrt(Specs.Zin * Specs.Zout / L);
  double R1 = 1 / (((L + 1) / (Specs.Zin * (L - 1))) - (1 / R2));
  double R3 = 1 / (((L + 1) / (Specs.Zout * (L - 1))) - (1 / R2));

  // Power dissipation calculation
  Pdiss.R1 = Specs.Pin * Specs.Zin / R1;
  Pdiss.R2 = Specs.Pin * R2 * (R1 - Specs.Zin) * (R1 - Specs.Zin) /
             (R1 * R1 * Specs.Zin);
  double K = R1 * R2 - Specs.Zin * (R1 + R2);
  Pdiss.R3 = Specs.Pin * K * K / (R1 * R1 * R3 * Specs.Zin);

  // Circuit implementation
  TermSpar1.setParams(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                      Term, 180, 0, 0, "N0", "gnd");
  TermSpar1.val["Z"] = num2str(Specs.Zin, Resistance);
  Schematic.appendComponent(TermSpar1);

  // 1st shunt resistor
  Res1.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 0, 50, 50, "N0", "gnd");
  Res1.val["R"] = num2str(R1, Resistance);
  Schematic.appendComponent(Res1);

  Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]), GND,
                   0, 50, 100, "", "");
  Schematic.appendComponent(Ground);

  // Node
  NI.setParams(
      QString("N%1").arg(++Schematic.NumberComponents[ConnectionNodes]), 50, 0);
  Schematic.appendNode(NI);

  Schematic.appendWire(TermSpar1.ID, 0, NI.ID, 0);
  Schematic.appendWire(Res1.ID, 1, NI.ID, 0);
  Schematic.appendWire(Res1.ID, 2, Ground.ID, 0);

  // Series resistor
  Res2.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 90, 100, 0, "N0", "N1");
  Res2.val["R"] = num2str(R2, Resistance);
  Schematic.appendComponent(Res2);

  Schematic.appendWire(Res2.ID, 0, NI.ID, 0);

  // 2nd shunt resistor
  Res3.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 0, 150, 50, "N1", "gnd");
  Res3.val["R"] = num2str(R3, Resistance);
  Schematic.appendComponent(Res3);

  // Node
  NI.setParams(
      QString("N%1").arg(++Schematic.NumberComponents[ConnectionNodes]), 150,
      0);
  Schematic.appendNode(NI);

  Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]), GND,
                   0, 150, 100, "", "");
  Schematic.appendComponent(Ground);

  Schematic.appendWire(Res2.ID, 1, NI.ID, 0);
  Schematic.appendWire(Res3.ID, 1, NI.ID, 0);
  Schematic.appendWire(Res3.ID, 0, Ground.ID, 0);

  TermSpar2.setParams(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                      Term, 0, 200, 0, "N1", "gnd");
  TermSpar2.val["Z"] = num2str(Specs.Zout, Resistance);
  Schematic.appendComponent(TermSpar2);

  Schematic.appendWire(TermSpar2.ID, 0, NI.ID, 0);

  Schematic.clearGraphs();
  Schematic.appendGraph(QString("S[2,1]"), QPen(Qt::red, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[1,1]"), QPen(Qt::blue, 1, Qt::SolidLine));
}
