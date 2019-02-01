/***************************************************************************
                                TeeAttenuator.cpp
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
#include "AttenuatorDesign/AttenuatorDesigner.h"

// Reference: RF design guide. Systems, circuits, and equations. Peter
// Vizmuller. Artech House, 1995
void AttenuatorDesigner::QW_SeriesAttenuator() {
  ComponentInfo TermSpar1, TermSpar2;
  ComponentInfo Ground, Res1, Res2, Res3, TL;
  ComponentInfo Lseries, Cshunt;
  WireInfo WI;
  NodeInfo NI;

  Components.clear();
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
  TermSpar1.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 180,
                      0, 0, "N0", "gnd");
  TermSpar1.val["Z"] = num2str(Specs.Zin, Resistance);
  Components.append(TermSpar1);

  // 1st shunt resistor
  Res1.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 0,
                 50, 50, "N0", "NA");
  Res1.val["R"] = num2str(R, Resistance);
  Components.append(Res1);

  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 50, 0);
  Nodes.append(NI);

  WI.setParams(Res1.ID, 1, NI.ID, 0);
  Wires.append(WI);

  WI.setParams(TermSpar1.ID, 0, NI.ID, 0);
  Wires.append(WI);

  // 2nd shunt resistor
  Res2.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 0,
                 50, 125, "NA", "gnd");
  Res2.val["R"] = num2str(Specs.Zin, Resistance);
  Components.append(Res2);

  WI.setParams(Res1.ID, 0, Res2.ID, 1);
  Wires.append(WI);

  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 0, 50,
                   175, "", "");
  Components.append(Ground);

  WI.setParams(Res2.ID, 0, Ground.ID, 0);
  Wires.append(WI);

  if (Specs.Lumped_TL) {

    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, 50, -50, QString("N0"), QString("gnd"));
    Cshunt.val["C"] = num2str(1 / (Specs.Zin * w0), Capacitance);
    Components.append(Cshunt);

    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 180,
                     50, -70, "", "");
    Components.append(Ground);

    Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, -90, 100, 0, QString("N0"), QString("N1"));
    Lseries.val["L"] = num2str(Specs.Zin / w0, Inductance);
    Components.append(Lseries);

    WI.setParams(Lseries.ID, 1, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 0, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 1, Ground.ID, 0);
    Wires.append(WI);

  } else {
    TL.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                 TransmissionLine, 90, 100, 0, QString("N0"), QString("N1"));
    TL.val["Z0"] = num2str(Specs.Zin, Resistance);
    TL.val["Length"] = ConvertLengthFromM("mm", l4);
    Components.append(TL);

    WI.setParams(TL.ID, 0, NI.ID, 0);
    Wires.append(WI);
  }

  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 150, 0);
  Nodes.append(NI);

  if (Specs.Lumped_TL) {
    Cshunt.Connections.clear();
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, 150, -50, QString("N1"), QString("gnd"));
    Cshunt.val["C"] = num2str(1 / (Specs.Zin * w0), Capacitance);
    Components.append(Cshunt);

    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 180,
                     150, -70, "", "");
    Components.append(Ground);

    WI.setParams(Lseries.ID, 0, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 0, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 1, Ground.ID, 0);
    Wires.append(WI);

  } else {
    WI.setParams(NI.ID, 0, TL.ID, 1);
    Wires.append(WI);
  }

  // 3rd shunt resistor
  Res3.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 0,
                 150, 50, "N1", "gnd");
  Res3.val["R"] = num2str(R, Resistance);
  Components.append(Res3);

  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 0, 150,
                   100, "", "");
  Components.append(Ground);

  WI.setParams(Res3.ID, 1, NI.ID, 0);
  Wires.append(WI);

  WI.setParams(Res3.ID, 0, Ground.ID, 0);
  Wires.append(WI);

  TermSpar2.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                      200, 0, "N1", "gnd");
  TermSpar2.val["Z"] = num2str(Specs.Zin, Resistance);
  Components.append(TermSpar2);

  WI.setParams(NI.ID, 0, TermSpar2.ID, 0);
  Wires.append(WI);

  displaygraphs.clear();
  displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
  displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
  displaygraphs[QString("S[2,2]")] = QPen(Qt::darkGreen, 1, Qt::SolidLine);
}
