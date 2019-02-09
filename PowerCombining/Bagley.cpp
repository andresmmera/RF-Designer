/***************************************************************************
                                Bagley.cpp
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

void PowerCombinerDesigner::Bagley() {

  double lambda4 = SPEED_OF_LIGHT / (4 * Specs.freq);
  double lambda2 = lambda4 * 2;
  double Zbranch = 2 * Specs.Z0 / sqrt(Specs.Noutputs);

  ComponentInfo TermSpar(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                         Term, 90, (Specs.Noutputs - 1) * 50, 0, "N0", "gnd");
  TermSpar.val["Z0"] = num2str(Specs.Z0, Resistance);
  Schematic.appendComponent(TermSpar);

  ComponentInfo TL1(
      QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
      TransmissionLine, 0, (Specs.Noutputs - 1) * 100, 50, "N0",
      QString("N%1").arg(Specs.Noutputs));
  TL1.val["Z0"] = num2str(Zbranch, Resistance);
  TL1.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
  Schematic.appendComponent(TL1);

  ComponentInfo TL2(
      QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
      TransmissionLine, 0, 0, 50, "N0", "N1");
  TL2.val["Z0"] = num2str(Zbranch, Resistance);
  TL2.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
  Schematic.appendComponent(TL2);

  Schematic.appendWire(TL1.ID, 1, TermSpar.ID, 0);
  Schematic.appendWire(TL2.ID, 1, TermSpar.ID, 0);

  TermSpar.Connections.clear();
  TermSpar.setParams(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                     Term, 90, 0, 100, "N1", "gnd");
  TermSpar.val["Z0"] = num2str(Specs.Z0, Resistance);
  Schematic.appendComponent(TermSpar);

  Schematic.appendWire(TermSpar.ID, 0, TL2.ID, 0);

  ComponentInfo TL;
  int posx = -50;
  for (int i = 1; i < Specs.Noutputs; i++) {
    posx += 100;
    TL.Connections.clear();
    TL.setParams(
        QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
        TransmissionLine, 90, posx, 100, QString("N%1").arg(i),
        QString("N%1").arg(i + 1));
    TL.val["Z0"] = num2str(Zbranch, Resistance);
    TL.val["Length"] = ConvertLengthFromM(Specs.units, lambda2);
    Schematic.appendComponent(TL);

    Schematic.appendWire(TermSpar.ID, 0, TL.ID, 0);

    TermSpar.Connections.clear();
    TermSpar.setParams(QString("T%1").arg(++Schematic.NumberComponents[Term]),
                       Term, 90, posx + 50, 100, QString("N%1").arg(i + 1),
                       "gnd");
    TermSpar.val["Z0"] = num2str(Specs.Z0, Resistance);
    Schematic.appendComponent(TermSpar);
    Schematic.appendWire(TermSpar.ID, 0, TL.ID, 1);
  }

  Schematic.appendWire(TL1.ID, 0, TermSpar.ID, 0);

  Schematic.clearGraphs();
  // Input matching
  Schematic.appendGraph(QString("S[1,1]"), QPen(Qt::blue, 1, Qt::SolidLine));
  // Forward transmission graphs
  for (int i = 2; i <= Specs.Noutputs; i++) {
    Schematic.appendGraph(QString("S[%1,1]").arg(i),
                          QPen(Qt::red, 1, Qt::DashLine));
  }

  // Isolation between consecutive output ports
  for (int i = 3; i <= Specs.Noutputs; i++) {
    Schematic.appendGraph(QString("S[%1,%2]").arg(i - 1).arg(i),
                          QPen(Qt::black, 1, Qt::DotLine));
  }
}
