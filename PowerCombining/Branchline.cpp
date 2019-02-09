/***************************************************************************
                                Branchline.cpp
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
#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::Branchline() {
  // Design equations
  double K = Specs.OutputRatio.at(0);
  double lambda4 = SPEED_OF_LIGHT / (4 * Specs.freq);
  double ZA = Specs.Z0 * sqrt(K / (K + 1));
  double ZB = Specs.Z0 * sqrt(K);

  ComponentInfo TermSpar1(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 0, 0, -50,
      "N0", "gnd");
  TermSpar1.val["Z0"] = num2str(Specs.Z0, Resistance);
  Schematic.appendComponent(TermSpar1);

  ComponentInfo TermSpar2(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 0, 100, -50,
      "N1", "gnd");
  TermSpar2.val["Z0"] = num2str(Specs.Z0, Resistance);
  Schematic.appendComponent(TermSpar2);

  ComponentInfo TermSpar3(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 0, 100, 50,
      "N2", "gnd");
  TermSpar3.val["Z0"] = num2str(Specs.Z0, Resistance);
  Schematic.appendComponent(TermSpar3);

  ComponentInfo Riso(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                     Resistor, 0, 0, 75, "N3", "gnd");
  Riso.val["R"] = num2str(Specs.Z0, Resistance);
  Schematic.appendComponent(Riso);

  ComponentInfo Ground(QString("GND%1").arg(++Schematic.NumberComponents[GND]),
                       GND, 0, 0, 120, "", "");
  Schematic.appendComponent(Ground);

  ComponentInfo TL1(
      QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
      TransmissionLine, 90, 50, -50, "N0", "N1");
  TL1.val["Z0"] = num2str(ZA, Resistance);
  TL1.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
  Schematic.appendComponent(TL1);

  ComponentInfo TL2(
      QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
      TransmissionLine, 90, 50, 50, "N2", "N3");
  TL2.val["Z0"] = num2str(ZA, Resistance);
  TL2.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
  Schematic.appendComponent(TL2);

  ComponentInfo TL3(
      QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
      TransmissionLine, 0, 0, 0, "N0", "N3");
  TL3.val["Z0"] = num2str(ZB, Resistance);
  TL3.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
  Schematic.appendComponent(TL3);

  ComponentInfo TL4(
      QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
      TransmissionLine, 0, 100, 0, "N1", "N2");
  TL4.val["Z0"] = num2str(ZB, Resistance);
  TL4.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
  Schematic.appendComponent(TL4);

  Schematic.appendWire(TermSpar1.ID, 0, TL1.ID, 0);
  Schematic.appendWire(TermSpar2.ID, 0, TL1.ID, 1);
  Schematic.appendWire(TermSpar2.ID, 0, TL4.ID, 1);
  Schematic.appendWire(TermSpar3.ID, 0, TL4.ID, 0);
  Schematic.appendWire(TermSpar3.ID, 0, TL2.ID, 1);
  Schematic.appendWire(TermSpar1.ID, 0, TL3.ID, 1);
  Schematic.appendWire(Riso.ID, 1, TL2.ID, 0);
  Schematic.appendWire(Riso.ID, 1, TL3.ID, 0);
  Schematic.appendWire(Riso.ID, 0, Ground.ID, 0);

  Schematic.clearGraphs();
  Schematic.appendGraph(QString("S[2,1]"), QPen(Qt::red, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[3,1]"), QPen(Qt::red, 1, Qt::DashLine));
  Schematic.appendGraph(QString("S[1,1]"), QPen(Qt::blue, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[3,2]"), QPen(Qt::black, 1, Qt::DotLine));
}
