/***************************************************************************
                                CoupledLineHarmonicRejectionSIRBandpassFilter.cpp
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
#include "CoupledLineHarmonicRejectionSIRBandpassFilter.h"

CoupledLineHarmonicRejectionSIRBandpassFilter::
    CoupledLineHarmonicRejectionSIRBandpassFilter() {}

CoupledLineHarmonicRejectionSIRBandpassFilter::
    CoupledLineHarmonicRejectionSIRBandpassFilter(FilterSpecifications FS) {
  Specification = FS;
}

CoupledLineHarmonicRejectionSIRBandpassFilter::
    ~CoupledLineHarmonicRejectionSIRBandpassFilter() {}

// This function synthesizes a coupled line bandpass filter
// implementation Reference: Bandpass Filters Using Parallel Coupled Stripline
// Stepped Impedance Resonators. Mitsuo Makimoto, Sadahiko Yamasita. IEEE
// Transactions on microwave theory and techniques, vol MTT-28, No 12, December
// 1980
void CoupledLineHarmonicRejectionSIRBandpassFilter::synthesize() {
  LowpassPrototypeCoeffs LP_coeffs(Specification);
  std::deque<double> gi = LP_coeffs.getCoefficients();

  ComponentInfo Coupled_Lines, TL;

  int N = Specification.order; // Number of elements
  int posx = 0, posy = 10;
  QString PreviousComponent;

  double delta = Specification.bw / Specification.fc; // Fractional bandwidth
  double Z0 = Specification.ZS;
  double lambda0 = SPEED_OF_LIGHT / Specification.fc;
  double J[N + 1], Z0e[N + 1], Z0o[N + 1];

  double K = Specification.ImpedanceRatio;
  double Z1 = Specification.ZS / K;
  double theta = atan(sqrt(K)); // beta*L
  double beta = 2 * M_PI / lambda0;
  double len_coup = theta / beta;
  double len_TL = 2 * len_coup;

  // Add Term 1
  ComponentInfo TermSpar1(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 180, posx,
      0);
  TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
  Schematic.appendComponent(TermSpar1);
  PreviousComponent = TermSpar1.ID;
  posx += 50;

  for (int k = 0; k < N; k++) {
    if (k == 0) {                                        // First element
      J[k] = sqrt(2 * delta * theta / (gi[k + 1])) / Z0; // Eq. 25
      Z0e[k] =
          Z0 *
          ((1 + (J[k] * Z0) / sin(theta) + (J[k] * J[k] * Z0 * Z0)) /
           (1 - (J[k] * J[k] * Z0 * Z0) / (tan(theta) * tan(theta)))); // Eq. 21
      Z0o[k] =
          Z0 *
          ((1 - (J[k] * Z0) / sin(theta) + (J[k] * J[k] * Z0 * Z0)) /
           (1 - (J[k] * J[k] * Z0 * Z0) / (tan(theta) * tan(theta)))); // Eq. 21

      // Set connections
      Coupled_Lines.setParams(
          QString("COUPL%1").arg(++Schematic.NumberComponents[CoupledLines]),
          CoupledLines, 90, posx, posy);
      Coupled_Lines.val["Ze"] = num2str(Z0e[k], Resistance);
      Coupled_Lines.val["Zo"] = num2str(Z0o[k], Resistance);
      Coupled_Lines.val["Length"] = ConvertLengthFromM("mm", len_coup);
      Schematic.appendComponent(Coupled_Lines);

      posx += 75;
      posy += 10;

      TL.setParams(
          QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
          TransmissionLine, 90, posx, posy);
      TL.val["Z0"] = num2str(Z1, Resistance);
      TL.val["Length"] = ConvertLengthFromM("mm", len_TL);
      Schematic.appendComponent(TL);

      posx += 75;
      posy += 10;

      // Wire: Series capacitor to SPAR term
      Schematic.appendWire(Coupled_Lines.ID, 0, TermSpar1.ID, 0);
      // Wire: Coupled line to transmission line
      Schematic.appendWire(Coupled_Lines.ID, 2, TL.ID, 0);

      PreviousComponent = TL.ID;
      continue;
    }

    J[k] = (2 * delta * theta / sqrt(gi[k] * gi[k + 1])) / Z0; // Eq. 8.121
    Z0e[k] =
        Z0 *
        ((1 + (J[k] * Z0) / sin(theta) + (J[k] * J[k] * Z0 * Z0)) /
         (1 - (J[k] * J[k] * Z0 * Z0) / (tan(theta) * tan(theta)))); // Eq. 21
    Z0o[k] =
        Z0 *
        ((1 - (J[k] * Z0) / sin(theta) + (J[k] * J[k] * Z0 * Z0)) /
         (1 - (J[k] * J[k] * Z0 * Z0) / (tan(theta) * tan(theta)))); // Eq. 21

    // Coupled lines
    Coupled_Lines.setParams(
        QString("COUPL%1").arg(++Schematic.NumberComponents[CoupledLines]),
        CoupledLines, 90, posx, posy);
    Coupled_Lines.val["Ze"] = num2str(Z0e[k], Resistance);
    Coupled_Lines.val["Zo"] = num2str(Z0o[k], Resistance);
    Coupled_Lines.val["Length"] = ConvertLengthFromM("mm", len_coup);
    Schematic.appendComponent(Coupled_Lines);

    posx += 50;
    posy += 10;

    TL.setParams(
        QString("TLIN%1").arg(++Schematic.NumberComponents[TransmissionLine]),
        TransmissionLine, 90, posx, posy);
    TL.val["Z0"] = num2str(Z1, Resistance);
    TL.val["Length"] = ConvertLengthFromM("mm", len_TL);
    Schematic.appendComponent(TL);

    // Wire: Coupled line to previous section
    Schematic.appendWire(Coupled_Lines.ID, 0, PreviousComponent, 1);

    // Wire: Coupled line to transmission line
    Schematic.appendWire(Coupled_Lines.ID, 2, TL.ID, 0);

    PreviousComponent = TL.ID;
    posx += 75;
    posy += 10;
  }

  // Last short stub + C series section
  J[N] = sqrt(2 * delta * theta / (gi[N + 1] * gi[N])) / Z0;
  Z0e[N] =
      Z0 *
      ((1 + (J[N] * Z0) / sin(theta) + (J[N] * J[N] * Z0 * Z0)) /
       (1 - (J[N] * J[N] * Z0 * Z0) / (tan(theta) * tan(theta)))); // Eq. 21
  Z0o[N] =
      Z0 *
      ((1 - (J[N] * Z0) / sin(theta) + (J[N] * J[N] * Z0 * Z0)) /
       (1 - (J[N] * J[N] * Z0 * Z0) / (tan(theta) * tan(theta)))); // Eq. 21

  // Coupled lines
  Coupled_Lines.setParams(
      QString("COUPL%1").arg(++Schematic.NumberComponents[CoupledLines]),
      CoupledLines, 90, posx, posy);
  Coupled_Lines.val["Ze"] = num2str(Z0e[N], Resistance);
  Coupled_Lines.val["Zo"] = num2str(Z0o[N], Resistance);
  Coupled_Lines.val["Length"] = ConvertLengthFromM("mm", len_coup);
  Schematic.appendComponent(Coupled_Lines);

  posx += 50;
  posy += 10;

  // Previous component to coupled lines
  Schematic.appendWire(PreviousComponent, 1, Coupled_Lines.ID, 0);

  // Add Term 2
  double k = Specification.ZS;
  if (Specification.UseZverevTables)
    (!Specification.isCLC) ? k /= gi[N + 1] : k *= gi[N + 1];
  else
    (Specification.isCLC) ? k /= gi[N + 1] : k *= gi[N + 1];

  ComponentInfo TermSpar2(
      QString("T%1").arg(++Schematic.NumberComponents[Term]), Term, 0, posx,
      posy);
  TermSpar2.val["Z"] = num2str(k, Resistance);
  Schematic.appendComponent(TermSpar2);

  Schematic.appendWire(Coupled_Lines.ID, 2, TermSpar2.ID, 0);

  Schematic.clearGraphs();
  Schematic.appendGraph(QString("S[2,1]"), QPen(Qt::red, 1, Qt::SolidLine));
  Schematic.appendGraph(QString("S[1,1]"), QPen(Qt::blue, 1, Qt::SolidLine));
}
