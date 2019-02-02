/***************************************************************************
                                ReflectionAttenuator.cpp
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

void AttenuatorDesigner::ReflectionAttenuator() {
  ComponentInfo TermSpar1, TermSpar2;
  ComponentInfo Ground, Res1, Res2, Coup;
  WireInfo WI;
  QStringList ConnectionNodes;

  Components.clear();
  // Design equations
  double L = pow(10, -.05 * Specs.Attenuation);
  double Ri = Specs.Zin * (1 - L) / (1 + L);

  // Power dissipation calculation
  Pdiss.R1 = .5 * Specs.Pin * (1 - pow(10, -0.1 * Specs.Attenuation));
  Pdiss.R2 = Pdiss.R1;

  // Circuit implementation
  TermSpar1.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 180,
                      0, 0, "N0", "gnd");
  TermSpar1.val["Z"] = num2str(Specs.Zin, Resistance);
  Components.append(TermSpar1);

  // 1st shunt resistor
  Res1.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 0,
                 50, 100, "NR1", "gnd");
  Res1.val["R"] = num2str(Ri, Resistance);
  Components.append(Res1);

  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 0, 50,
                   150, "", "");
  Components.append(Ground);

  WI.setParams(Res1.ID, 2, Ground.ID, 0);
  Wires.append(WI);

  // Coupler
  ConnectionNodes.clear();
  ConnectionNodes.append(QString("N0"));
  ConnectionNodes.append(QString("NR1"));
  ConnectionNodes.append(QString("NR2"));
  ConnectionNodes.append(QString("N1"));
  Coup.setParams(QString("COUP%1").arg(++NumberComponents[Coupler]), Coupler, 0,
                 100, 25, ConnectionNodes);
  Coup.val["k"] = num2str(0.7071, NoUnits);
  Coup.val["phi"] = num2str(90, NoUnits);
  Components.append(Coup);

  WI.setParams(Res1.ID, 1, Coup.ID, 2);
  Wires.append(WI);

  // 2nd shunt resistor
  Res2.setParams(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, 0,
                 150, 100, "NR2", "gnd");
  Res2.val["R"] = num2str(Ri, Resistance);
  Components.append(Res2);

  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 0, 150,
                   150, "", "");
  Components.append(Ground);

  WI.setParams(Res2.ID, 1, Coup.ID, 3);
  Wires.append(WI);

  WI.setParams(Res2.ID, 0, Ground.ID, 0);
  Wires.append(WI);

  TermSpar2.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                      200, 0, "N1", "gnd");
  TermSpar2.val["Z"] = num2str(Specs.Zout, Resistance);
  Components.append(TermSpar2);

  WI.setParams(TermSpar1.ID, 0, Coup.ID, 0);
  Wires.append(WI);

  WI.setParams(TermSpar2.ID, 0, Coup.ID, 1);
  Wires.append(WI);

  displaygraphs.clear();
  displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
  displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
}
