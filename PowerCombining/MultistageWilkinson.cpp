/***************************************************************************
                                MultistageWilkinson.cpp
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

void PowerCombinerDesigner::MultistageWilkinson() {
  int NStages = Specs.Nstages;
  double C[NStages], L[NStages];
  std::deque<double> Zlines = ChebyshevTaper(2 * Specs.Z0, 0.05);
  ComponentInfo TermSpar1, TermSpar2, TermSpar3;
  ComponentInfo Ground;
  ComponentInfo Cshunt, Lseries;
  ComponentInfo TL, TL_Upper, TL_Lower;
  WireInfo WI;
  NodeInfo NI, Nupper, Nlower;

  QString PreviousNode; // Auxiliar variable for connecting the current stage to
                        // the previous one in the for loop

  if (Specs.Implementation == "Lumped LC") // CLC pi equivalent calculation
  {
    double w = 2 * M_PI * Specs.freq;
    for (int i = 0; i < NStages; i++) {
      L[i] = Zlines[i] / w;
      C[i] = 1. / (L[i] * w * w);
    }
  }

  double alpha = log(
      pow(0.1 * Specs.alpha, 10)); // Alpha is given in dB/m, then it is
                                   // necessary to convert it into Np/m units
  double lambda4 = SPEED_OF_LIGHT / (4. * Specs.freq);
  std::complex<double> gamma(
      alpha, 2 * M_PI * Specs.freq /
                 SPEED_OF_LIGHT); // It is only considered the attenation of the
                                  // metal conductor since it tends to be much
                                  // higher than the dielectric
  std::deque<double> Risol =
      calcMultistageWilkinsonIsolators(Zlines, lambda4, gamma);

  // Build the schematic
  int posx = 0;
  int posy;   // The position of the upper and the lower branches vary depending
              // on the type of implementation
  int Ni = 0; // Node index
  (Specs.Implementation == "Lumped LC") ? posy = 75 : posy = 50;

  TermSpar1.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 180,
                      posx, 0, "N0", "gnd");
  TermSpar1.val["Z0"] = num2str(Specs.Z0, Resistance);
  Components.append(TermSpar1);

  posx += 50;
  if (Specs.Implementation ==
      "Lumped LC") // LC elements. Pi CLC equivalent of a lambda/4 line
  {
    // Shunt capacitor
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, posx, 20, "N0", "gnd");
    Cshunt.val["C"] = num2str(2 * C[0], Capacitance);
    Components.append(Cshunt);

    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 0,
                     posx, 60, "", "");
    Components.append(Ground);

    WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx + 25, 0);
    Nodes.append(NI);

    WI.setParams(Cshunt.ID, 1, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 1, TermSpar1.ID, 0);
    Wires.append(WI);
    posx += 20;
  } else { // Ideal transmission lines
    // 1st transmission line
    TL.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                 TransmissionLine, 90, posx, 0, "N0", "N1");
    TL.val["Z0"] = num2str(Specs.Z0, Resistance);
    TL.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
    Components.append(TL);

    // Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx + 25, 0);
    Nodes.append(NI);

    WI.setParams(TermSpar1.ID, 0, TL.ID, 0);
    Wires.append(WI);

    WI.setParams(TL.ID, 1, NI.ID, 0);
    Wires.append(WI);
    Ni++;
  }

  for (int i = 0; i < Specs.Nstages; i++) {
    if (Specs.Implementation ==
        "Lumped LC") // LC elements. Pi CLC equivalent of a lambda/4 line
    {
      double C_;
      if (i != Specs.Nstages - 1)
        C_ = C[i] + C[i + 1];
      else
        C_ = C[i];
      // Upper branch
      posx += 50;
      (i == 0) ? PreviousNode = "N0"
               : PreviousNode = QString("Nupper%1").arg(i - 1);

      Lseries.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90, posx,
          -75, QString("%1").arg(PreviousNode), QString("Nupper%1").arg(i));
      Lseries.val["L"] = num2str(L[i], Inductance);
      Components.append(Lseries);

      posx += 50;

      // Cshunt
      Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                       Capacitor, 0, posx, -50, QString("Nupper%1").arg(i),
                       "gnd");
      Cshunt.val["C"] = num2str(C_, Capacitance);
      Components.append(Cshunt);

      Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 0,
                       posx, -10, "", "");
      Components.append(Ground);

      // Capacitor to ground
      WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
      Wires.append(WI);

      if (i > 0) { // Connect the current section to the previous one
        WI.setParams(Lseries.ID, 1, Nupper.ID, 0);
        Wires.append(WI);
      } else {
        if (i == 0) { // Connect to the common node
          WI.setParams(Lseries.ID, 1, NI.ID, 0);
          Wires.append(WI);
        }
      }

      // Node
      Nupper.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                       posx + 50, -75);
      Nodes.append(Nupper);

      // Capacitor to node
      WI.setParams(Cshunt.ID, 1, Nupper.ID, 0);
      Wires.append(WI);

      // Inductor to capacitor
      WI.setParams(Cshunt.ID, 1, Lseries.ID, 0);
      Wires.append(WI);

      posx -= 50;
      // Lower branch
      (i == 0) ? PreviousNode = "N0"
               : PreviousNode = QString("Nlower%1").arg(i - 1);

      Lseries.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90, posx,
          75, QString("%1").arg(PreviousNode), QString("Nlower%1").arg(i));
      Lseries.val["L"] = num2str(L[i], Inductance);
      Components.append(Lseries);

      posx += 50;
      // Cshunt
      Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                       Capacitor, 0, posx, 100, QString("Nlower%1").arg(i),
                       "gnd");
      Components.append(Cshunt);

      Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, 0,
                       posx, 140, "", "");
      Components.append(Ground);

      // Capacitor to ground
      WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
      Wires.append(WI);

      if (i > 0) { // Connect the current section to the previous one
        WI.setParams(Lseries.ID, 1, Nlower.ID, 0);
        Wires.append(WI);
      } else { // Connect to the common node
        WI.setParams(Lseries.ID, 1, NI.ID, 0);
        Wires.append(WI);
      }
      // Node
      Nlower.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                       posx + 50, 75);
      Nodes.append(Nlower);

      // Capacitor to node
      WI.setParams(Cshunt.ID, 1, Nlower.ID, 0);
      Wires.append(WI);

      // Inductor to capacitor
      WI.setParams(Cshunt.ID, 1, Lseries.ID, 0);
      Wires.append(WI);

    } else { // Ideal TL
      posx += 50;
      // Upper branch TL
      // 1st transmission line
      (i == 0) ? PreviousNode = "N1"
               : PreviousNode = QString("Nupper%1").arg(i - 1);

      TL_Upper.setParams(
          QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
          TransmissionLine, 90, posx + 15, -50, QString("%1").arg(PreviousNode),
          QString("Nupper%1").arg(i));
      TL_Upper.val["Z0"] = num2str(Zlines[i], Resistance);
      TL_Upper.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
      Components.append(TL_Upper);

      if (i > 0) { // Connect the current section to the previous one
        WI.setParams(TL_Upper.ID, 0, Nupper.ID, 0);
        Wires.append(WI);
      }

      // Node
      Nupper.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                       posx + 50, -50);
      Nodes.append(Nupper);

      WI.setParams(TL_Upper.ID, 1, Nupper.ID, 0);
      Wires.append(WI);

      // Lower branch TL
      // 1st transmission line
      (i == 0) ? PreviousNode = "N1"
               : PreviousNode = QString("Nlower%1").arg(i - 1);
      TL_Lower.setParams(
          QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
          TransmissionLine, 90, posx + 15, 50, QString("%1").arg(PreviousNode),
          QString("Nlower%1").arg(i));
      TL_Lower.val["Z0"] = num2str(Zlines[i], Resistance);
      TL_Lower.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
      Components.append(TL_Lower);

      if (i > 0) { // Connect the current section to the previous one
        WI.setParams(TL_Lower.ID, 0, Nlower.ID, 0);
        Wires.append(WI);
      }

      // Node
      Nlower.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                       posx + 50, 50);
      Nodes.append(Nlower);

      WI.setParams(TL_Lower.ID, 1, Nlower.ID, 0);
      Wires.append(WI);

      if (i == 0) { // First section, the branches must be connected to the
                    // common node
        WI.setParams(NI.ID, 0, TL_Upper.ID, 0);
        Wires.append(WI);

        WI.setParams(NI.ID, 0, TL_Lower.ID, 0);
        Wires.append(WI);
      }
    }

    posx += 50;
    // Isolation resistor
    ComponentInfo Riso(QString("R%1").arg(++NumberComponents[Resistor]),
                       Resistor, 0, posx, 0, QString("Nupper%1").arg(i),
                       QString("Nlower%1").arg(i));
    Riso.val["R"] = num2str(Risol[i], Resistance);
    Components.append(Riso);

    WI.setParams(Riso.ID, 1, Nupper.ID, 0);
    Wires.append(WI);

    WI.setParams(Riso.ID, 0, Nlower.ID, 0);
    Wires.append(WI);
    Ni += 2;
  }

  posx += 50;

  // Add the output terminals
  // Upper branch term
  TermSpar2.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                      posx, -posy, QString("Nupper%1").arg(Specs.Nstages - 1),
                      "gnd");
  TermSpar2.val["Z0"] = num2str(Specs.Z0, Resistance);
  Components.append(TermSpar2);

  WI.setParams(TermSpar2.ID, 0, Nupper.ID, 0);
  Wires.append(WI);

  // Lower branch term
  TermSpar3.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                      posx, posy, QString("Nlower%1").arg(Specs.Nstages - 1),
                      "gnd");
  TermSpar3.val["Z0"] = num2str(Specs.Z0, Resistance);
  Components.append(TermSpar3);

  WI.setParams(TermSpar3.ID, 0, Nlower.ID, 0);
  Wires.append(WI);

  // Ideally, the user should be the one which controls the style of the traces
  // as well the traces to be shown However, in favour of a simpler
  // implementation, it'll be the design code responsible for this... by the
  // moment...
  displaygraphs.clear();
  displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
  displaygraphs[QString("S[3,1]")] = QPen(Qt::red, 1, Qt::DashLine);
  displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
  displaygraphs[QString("S[3,2]")] = QPen(Qt::black, 1, Qt::DotLine);
}

// This function calculates the isolation resistors given the impedance of the
// quarter wave lines
std::deque<double> PowerCombinerDesigner::calcMultistageWilkinsonIsolators(
    std::deque<double> Zlines, double L, std::complex<double> gamma) {
  int NStages = Specs.Nstages;
  double Z_, R, Zaux = Zlines[NStages - 1];
  std::deque<double> Risol;

  for (int i = 0; i < NStages; i++) {
    Z_ = abs(Zaux * (Specs.Z0 + Zaux * tanh(gamma * L)) /
             (Zaux + Specs.Z0 * tanh(gamma * L)));
    Zaux = Zlines[i];
    R = Specs.Z0 * Z_ / (Z_ - Specs.Z0);
    Risol.push_front(2 * R);
  }
  return Risol;
}
