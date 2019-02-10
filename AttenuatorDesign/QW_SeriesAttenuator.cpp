/***************************************************************************
                                QW_SeriesAttenuator.cpp
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
#include "AttenuatorDesign/QW_SeriesAttenuator.h"
QW_SeriesAttenuator::QW_SeriesAttenuator() {}

QW_SeriesAttenuator::QW_SeriesAttenuator(AttenuatorDesignParameters AS) {
  Specification = AS;
}

QW_SeriesAttenuator::~QW_SeriesAttenuator() {}

void QW_SeriesAttenuator::synthesize() {
  ComponentInfo TermSpar1, TermSpar2;
  ComponentInfo Ground, Res1, Res2, Res3, TL;
  ComponentInfo Lseries, Cshunt;
  WireInfo WI;
  NodeInfo NI;

  // Design equations
  double R = Specs.Zin / (pow(10, .05 * Specs.Attenuation) - 1);
  double l4 = .25 * SPEED_OF_LIGHT / Specs.Frequency;
  double w0 = 2 * M_PI * Specs.Frequency;

  // Power dissipation
  double K = (R + Specs.Zin) * (R + Specs.Zin);
  Pdiss.R1 = Specs.Pin * Specs.Zin * R / K;
  Pdiss.R2 = Specs.Pin * Specs.Zin * Specs.Zin / K;
  Pdiss.R3 = Pdiss.R1;

  // Circuit implementation
  TermSpar1.setParams(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                      Term, 180, 0, 0, "N0", "gnd");
  TermSpar1.val["Z"] = num2str(Specs.Zin, Resistance);
  Schematic.appendComponent(TermSpar1);

  // 1st shunt resistor
  Res1.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 0, 50, 50, "N0", "NA");
  Res1.val["R"] = num2str(R, Resistance);
  Schematic.appendComponent(Res1);

  // Node
  NI.setParams(
      QString("N%1").arg(++Schematic.NumberComponents[ConnectionNodes]), 50, 0);
  Schematic.appendNode(NI);

  Schematic.appendWire(Res1.ID, 1, NI.ID, 0);
  Schematic.appendWire(TermSpar1.ID, 0, NI.ID, 0);

  // 2nd shunt resistor
  Res2.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 0, 50, 125, "NA", "gnd");
  Res2.val["R"] = num2str(Specs.Zin, Resistance);
  Schematic.appendComponent(Res2);

  Schematic.appendWire(Res1.ID, 0, Res2.ID, 1);

  Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]), GND,
                   0, 50, 175, "", "");
  Schematic.appendComponent(Ground);

  Schematic.appendWire(Res2.ID, 0, Ground.ID, 0);

  if (Specs.Lumped_TL) {
    Cshunt.setParams(
        QString("C%1").arg(++Schematic.NumberComponents[Capacitor]), Capacitor,
        0, 50, -50, QString("N0"), QString("gnd"));
    Cshunt.val["C"] = num2str(1 / (Specs.Zin * w0), Capacitance);
    Schematic.appendComponent(Cshunt);

    Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]),
                     GND, 180, 50, -70, "", "");
    Schematic.appendComponent(Ground);

    Lseries.setParams(
        QString("L%1").arg(++Schematic.NumberComponents[Inductor]), Inductor,
        -90, 100, 0, QString("N0"), QString("N1"));
    Lseries.val["L"] = num2str(Specs.Zin / w0, Inductance);
    Schematic.appendComponent(Lseries);

    Schematic.appendWire(Lseries.ID, 1, NI.ID, 0);
    Schematic.appendWire(Cshunt.ID, 0, NI.ID, 0);
    Schematic.appendWire(Cshunt.ID, 1, Ground.ID, 0);
  } else {
    TL.setParams(
        QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
        TransmissionLine, 90, 100, 0, QString("N0"), QString("N1"));
    TL.val["Z0"] = num2str(Specs.Zin, Resistance);
    TL.val["Length"] = ConvertLengthFromM("mm", l4);
    Schematic.appendComponent(TL);
    Schematic.appendWire(TL.ID, 0, NI.ID, 0);
  }

  // Node
  NI.setParams(
      QString("N%1").arg(++Schematic.NumberComponents[ConnectionNodes]), 150,
      0);
  Schematic.appendNode(NI);

  if (Specs.Lumped_TL) {
    Cshunt.Connections.clear();
    Cshunt.setParams(
        QString("C%1").arg(++Schematic.NumberComponents[Capacitor]), Capacitor,
        0, 150, -50, QString("N1"), QString("gnd"));
    Cshunt.val["C"] = num2str(1 / (Specs.Zin * w0), Capacitance);
    Schematic.appendComponent(Cshunt);

    Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]),
                     GND, 180, 150, -70, "", "");
    Schematic.appendComponent(Ground);

    Schematic.appendWire(Lseries.ID, 0, NI.ID, 0);
    Schematic.appendWire(Cshunt.ID, 0, NI.ID, 0);
    Schematic.appendWire(Cshunt.ID, 1, Ground.ID, 0);

  } else {
    Schematic.appendWire(NI.ID, 0, TL.ID, 1);
  }

  // 3rd shunt resistor
  Res3.setParams(QString("R%1").arg(++Schematic.NumberComponents[Resistor]),
                 Resistor, 0, 150, 50, "N1", "gnd");
  Res3.val["R"] = num2str(R, Resistance);
  Schematic.appendComponent(Res3);

  Ground.setParams(QString("GND%1").arg(++Schematic.NumberComponents[GND]), GND,
                   0, 150, 100, "", "");
  Schematic.appendComponent(Ground);

  Schematic.appendWire(Res3.ID, 1, NI.ID, 0);
  Schematic.appendWire(Res3.ID, 0, Ground.ID, 0);

  TermSpar2.setParams(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                      Term, 0, 200, 0, "N1", "gnd");
  TermSpar2.val["Z"] = num2str(Specs.Zin, Resistance);
  Schematic.appendComponent(TermSpar2);

  Schematic.appendWire(NI.ID, 0, TermSpar2.ID, 0);

  Schematic.clearGraphs();
  Schematic.appendGraph(QString("S[2,1]"), QPen(Qt::red, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[1,1]"), QPen(Qt::blue, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[2,2]"),
                        QPen(Qt::darkGreen, 1, Qt::SolidLine));
}
