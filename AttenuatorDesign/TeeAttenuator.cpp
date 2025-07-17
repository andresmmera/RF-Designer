/***************************************************************************
                                TeeAttenuator.cpp
                                ----------
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
#include "AttenuatorDesign/TeeAttenuator.h"

TeeAttenuator::TeeAttenuator() {}

TeeAttenuator::TeeAttenuator(AttenuatorDesignParameters AS) { Specs = AS; }

TeeAttenuator::~TeeAttenuator() {}

// Reference: RF design guide. Systems, circuits, and equations. Peter
// Vizmuller. Artech House, 1995
void TeeAttenuator::synthesize() {
  ComponentInfo TermSpar1, TermSpar2;
  ComponentInfo Ground, Res1, Res2, Res3;
  NodeInfo NI;

  // Design equations
  double L = pow(10, .1 * Specs.Attenuation);
  double R2 = (2 * sqrt(Specs.Zin * Specs.Zout * L)) / (L - 1);
  double R1 = Specs.Zin * ((L + 1) / (L - 1)) - R2;
  double R3 = Specs.Zout * ((L + 1) / (L - 1)) - R2;

  // Power dissipation
  Pdiss.R1 = Specs.Pin * R1 / Specs.Zin;
  Pdiss.R2 = Specs.Pin * (R1 - Specs.Zin) * (R1 - Specs.Zin) / (R2 * Specs.Zin);
  Pdiss.R3 = Specs.Pin * R3 * (R1 + R2 - Specs.Zin) * (R1 + R2 - Specs.Zin) /
             (Specs.Zin * R2 * R2);

  // Circuit implementation
  TermSpar1.setParams(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                      Term, 180, 0, 0);
  TermSpar1.val["Z"] = num2str(Specs.Zin, Resistance);
  Schematic.appendComponent(TermSpar1);

  // 1st series resistor
  Res1.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 90, 50, 0);
  Res1.val["R"] = num2str(R1, Resistance);
  Schematic.appendComponent(Res1);

  // Node
  NI.setParams(
      QString("N%1").arg(++Schematic.NumberComponents[ConnectionNodes]), 100,
      0);
  Schematic.appendNode(NI);

  Schematic.appendWire(TermSpar1.ID, 0, Res1.ID, 0);
  Schematic.appendWire(Res1.ID, 1, NI.ID, 0);

  // Shunt resistor
  Res2.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 0, 100, 50);
  Res2.val["R"] = num2str(R2, Resistance);
  Schematic.appendComponent(Res2);

  Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]), GND,
                   0, 100, 100);
  Schematic.appendComponent(Ground);

  Schematic.appendWire(Res2.ID, 1, NI.ID, 0);
  Schematic.appendWire(Res2.ID, 0, Ground.ID, 0);

  // 2nd series resistor
  Res3.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 90, 150, 0);
  Res3.val["R"] = num2str(R3, Resistance);
  Schematic.appendComponent(Res3);

  Schematic.appendWire(Res2.ID, 1, NI.ID, 0);
  Schematic.appendWire(Res3.ID, 0, NI.ID, 0);

  TermSpar2.setParams(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                      Term, 0, 200, 0);
  TermSpar2.val["Z"] = num2str(Specs.Zout, Resistance);
  Schematic.appendComponent(TermSpar2);

  Schematic.appendWire(TermSpar2.ID, 0, Res3.ID, 1);

  Schematic.clearGraphs();
  Schematic.appendGraph(QString("S[2,1]"), QPen(Qt::red, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[1,1]"), QPen(Qt::blue, 1, Qt::SolidLine));
}
