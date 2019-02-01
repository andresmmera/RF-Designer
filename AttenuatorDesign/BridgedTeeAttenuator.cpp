/***************************************************************************
                                BridgedTeeAttenuator.cpp
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
void AttenuatorDesigner::BridgedTeeAttenuator() {
  ComponentInfo TermSpar1, TermSpar2;
  ComponentInfo Ground, Res1, Res2, Res3, Res4;
  WireInfo WI;
  NodeInfo NI;

  Components.clear();
  // Design equations
  double L = pow(10, .05 * Specs.Attenuation);
  double R1 = Specs.Zin * (L - 1);
  double R4 = Specs.Zin / (L - 1);

  // Power dissipation calculation
  double K = R1 * R4 + Specs.Zin * (2 * R4 + Specs.Zin);
  K *= K;
  Pdiss.R1 = Specs.Pin * (4 * R1 * R4 * R4 * Specs.Zin) / (K);
  Pdiss.R2 = Specs.Pin * (R1 * R4 + Specs.Zin * Specs.Zin) *
             (R1 * R4 + Specs.Zin * Specs.Zin) / K;
  Pdiss.R3 = 0;
  Pdiss.R4 = 4 * R4 * Specs.Zin * Specs.Zin / K;

  // Circuit implementation
  TermSpar1.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 180,
                      0, 0, "N0", "gnd");
  TermSpar1.val["Z"] = num2str(Specs.Zin, Resistance);
  Components.append(TermSpar1);

  // Series resistor
  Res1.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 90,
                 100, 0, "N0", "N1");
  Res1.val["R"] = num2str(R1, Resistance);
  Components.append(Res1);

  // 1st shunt resistor
  Res2.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 0,
                 50, 50, "N0", "NA");
  Res2.val["R"] = num2str(Specs.Zin, Resistance);
  Components.append(Res2);

  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 50, 0);
  Nodes.append(NI);

  WI.setParams(TermSpar1.ID, 0, NI.ID, 0);
  Wires.append(WI);

  WI.setParams(Res2.ID, 1, NI.ID, 0);
  Wires.append(WI);

  WI.setParams(Res1.ID, 0, NI.ID, 0);
  Wires.append(WI);

  // 2nd shunt resistor
  Res3.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 0,
                 150, 50, "N1", "NA");
  Res3.val["R"] = num2str(Specs.Zin, Resistance);
  Components.append(Res3);

  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100,
               80);
  Nodes.append(NI);

  // 3rd shunt resistor
  Res4.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 0,
                 100, 120, "NA", "gnd");
  Res4.val["R"] = num2str(R4, Resistance);
  Components.append(Res4);

  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 0, 100,
                   170, "", "");
  Components.append(Ground);

  WI.setParams(Res2.ID, 0, NI.ID, 0);
  Wires.append(WI);

  WI.setParams(Res3.ID, 0, NI.ID, 0);
  Wires.append(WI);

  WI.setParams(Res4.ID, 1, NI.ID, 0);
  Wires.append(WI);

  WI.setParams(Res4.ID, 0, Ground.ID, 0);
  Wires.append(WI);

  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 150, 0);
  Nodes.append(NI);

  WI.setParams(Res1.ID, 1, NI.ID, 0);
  Wires.append(WI);

  WI.setParams(Res3.ID, 1, NI.ID, 0);
  Wires.append(WI);

  TermSpar2.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                      200, 0, "N1", "gnd");
  TermSpar2.val["Z"] = num2str(Specs.Zout, Resistance);
  Components.append(TermSpar2);

  WI.setParams(TermSpar2.ID, 0, NI.ID, 0);
  Wires.append(WI);

  displaygraphs.clear();
  displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
  displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
}
