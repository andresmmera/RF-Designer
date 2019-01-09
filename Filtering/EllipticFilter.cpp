/***************************************************************************
                                EllipticFilter.cpp
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
#include "EllipticFilter.h"

/* Reference:
 *  [1] "Elliptic Approximation and Elliptic Filter Design on Small Computers",
 Pierre Amstutz, IEEE Transactions on Circuits and Systems, vol. CAS-25, No 12,
 December 1978
*/

EllipticFilter::EllipticFilter() {
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
  QucsNetlist = "";
  virtual_nodes = 0;
}

EllipticFilter::~EllipticFilter() {
  delete Cshunt_LP;
  delete Lseries_LP;
  delete Cseries_LP;
}

EllipticFilter::EllipticFilter(FilterSpecifications FS) {
  Specification = FS;
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
  QucsNetlist = "";
  Cshunt_LP = new std::vector<double>(FS.order + 1);
  Lseries_LP = new std::vector<double>(FS.order + 1);
  Cseries_LP = new std::vector<double>(FS.order);
  virtual_nodes = 0;
}

QList<ComponentInfo> EllipticFilter::getComponents() { return Components; }

QList<WireInfo> EllipticFilter::getWires() { return Wires; }

QList<NodeInfo> EllipticFilter::getNodes() { return Nodes; }

void EllipticFilter::synthesize() {
  // Ideally, the user should be the one which controls the style of the traces
  // as well the traces to be shown However, in favour of a simpler
  // implementation, it'll be the design code responsible for this... by the
  // moment...
  displaygraphs.clear();
  displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
  displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);

  (Specification.EllipticType == "Type S") ? EllipticTypeS()
                                           : EllipticTypesABC();

  SynthesizeEllipticFilter();

  // Build Qucs netlist
  QucsNetlist.clear();
  QString codestr;
  for (int i = 0; i < Components.length(); i++) {
    codestr = Components[i].getQucs();
    if (!codestr.isEmpty())
      QucsNetlist += codestr;
  }
}

void EllipticFilter::EllipticTypeS() {
  double as = Specification.as;     // Stopband attenuation
  double ap = Specification.Ripple; // Passband ripple
  int N = Specification.order;      // Number of peaks
  double dbn = 0.23025851;          // dB -> Np conversion

  int M = 2 * N + 1;
  double u =
      (M_PI * M_PI) / (log(16 * (exp(as * dbn) - 1) / (exp(ap * dbn) - 1)));
  double w =
      (u / (2 * M_PI)) * log((exp(ap * dbn / 2) + 1) / (exp(ap * dbn / 2) - 1));
  // Resize elliptic network parameters
  std::vector<double> E(N), F(M - 1);
  E[N - 1] = tan(w);
  double a0 = 1 / tan(u * (as + log(2)) / M_PI);

  // Calculation of the natural frequencies = Sn(M*u, j*u) j \in [1, M-1]
  for (int j = 1; j < M; j++)
    F[j - 1] = Sn(M * u, j * u);

  // Calculation of a0 Eqn (4.34)
  double K = 1;
  int j = 1;
  double delta = 1, Kaux;
  while (delta > 1e-6) {
    Kaux = K * (pow(tan(w), 2) + pow(tanh(j * M * u), 2)) /
           (1 + pow(tan(w) * tanh(j * M * u), 2));
    delta = std::abs(K - Kaux);
    K = Kaux;
    j++;
  }
  a0 = tan(w) * K;
  E[N - 1] = a0;

  // Delay group at the natural frequencies
  for (int j = 0; j < N; j++)
    Cseries_LP->at(j) = F[2 * j + 1] * (1 - pow(F[j], 4)) / F[j]; // Eqn 5.7

  std::vector<double> C(N);
  C[0] = (1 / (a0 * F[N])); // Starting value for dB/dw calculation
  for (int j = 1; j < N; j++) {
    C[j] = (C[j - 1] - a0 * F[N - j - 1]) / (1 + C[j - 1] * a0 * F[N - j - 1]);
    E[N - j - 1] =
        E[N - j] + E[N - 1] * Cseries_LP->at(j - 1) /
                       (1 + pow(a0 * F[j - 1], 2)); // Ej=(Dj/Fj)*dB/dw(1/Fj)
  }

  for (int j = 0; j < N; j++) {
    Lseries_LP->at(j) =
        ((1 + pow(C[j], 2)) * E[j] / Cseries_LP->at(j) - C[j] / F[j]) / 2.;
    Cshunt_LP->at(j) = C[j] * F[j];
  }
  Lseries_LP->at(N) = Lseries_LP->at(N - 1);
  Cshunt_LP->at(N) = Cshunt_LP->at(N - 1);
  // Permutations method Eqn (3.6)
  for (int l = 0; l < 2; l++) {
    for (int k = l + 2; k < N + 1; k += 2) {
      for (int j = l; j <= k - 2; j += 2) {
        double U = Cshunt_LP->at(j) - Cshunt_LP->at(k);
        double V =
            1 / (U / ((F[k] * F[k] - F[j] * F[j]) * Lseries_LP->at(j)) - 1);
        Cshunt_LP->at(k) = U * V;
        Lseries_LP->at(k) =
            (V * V) * Lseries_LP->at(k) - (pow(V + 1, 2)) * Lseries_LP->at(j);
      }
    }
  }

  // Impedance and frequency scaling
  for (int j = 0; j < N; j++) {
    Cseries_LP->at(j) = Lseries_LP->at(j) * F[j] * F[j];
    Lseries_LP->at(j) = 1 / Lseries_LP->at(j);
  }
  Lseries_LP->pop_back();
}

void EllipticFilter::EllipticTypesABC() {
  double as = Specification.as;     // Stopband attenuation
  double ap = Specification.Ripple; // Passband ripple
  int M = Specification.order;      // Number of peaks
  double RS = Specification.ZS;     // Source impedance
  double dbn = 0.23025851;          // dB -> Np conversion

  int N = 2 * M;

  double u = M_PI * M_PI / log(16 * (exp(as * dbn) - 1) / (exp(ap * dbn) - 1));
  double W =
      (u / (2 * M_PI)) * log((exp(ap * dbn / 2) + 1) / (exp(ap * dbn / 2) - 1));

  std::vector<double> E(N);
  std::vector<double> R(M);
  std::vector<double> S(M);
  std::vector<double> B(M + 1);
  std::vector<double> F(M + 1);
  std::vector<double> D(M + 1);

  for (int j = 0; j < N; j++)
    E[j] = Sn(M * u, (j + 1 - M) * u / 2);

  // Calculation of a0 Eqn (4.34)
  double K = 1;
  int j = 1;
  double delta = 1, Kaux, a0;

  while (delta > 1e-6) {
    Kaux = K * (pow(tan(W), 2) + pow(tanh(j * M * u), 2)) /
           (1 + pow(tan(W) * tanh(j * M * u), 2));
    delta = abs(K - Kaux);
    K = Kaux;
    j = j + 1;
  }
  a0 = tan(W) * K;

  std::vector<std::complex<double>> RS_(M);
  std::complex<double> i = std::complex<double>(0, 1);
  // Calculation of the natural frequencies for the Type S
  for (int j = 0; j < M; j++)
    RS_[j] = i * Sn(M * u, i * W + (M + 1 - 2 * (j + 1)) * u / 2);

  for (int i = 0; i < M; i++)
    R[i] = real(RS_[i]), S[i] = imag(RS_[i]);

  double E8, E0;
  int IT;
  if (Specification.EllipticType == "Type A") {
    E8 = E[N - 1];
    IT = 1;
  } else {
    IT = 2;
    E8 = -E[0];
  }

  (Specification.EllipticType == "Type C") ? E0 = -E[0] : E0 = E[N - 1];

  // The normalized passband and stopband edges for a type S
  // characteristic are given by Eq. 4.15
  double FP =
      Sn(N * u, N * u / 2); // Normalized passband edge. Originally, it is
  // estimated as: sqrt((E(N)+E0)/(1+E(N)*E8));

  // Calculation of the attenuation peaks
  for (int j = IT; j <= M; j++)
    D[j - 1] = (E[2 * j - 2] + E8) / (1 + E0 * E[2 * j - 2]);

  double TQ = 0, T0 = 0;

  int I = 1;

  for (int i = 0; i < M; i++)
    F[i] = sqrt(1 / D[i]);
  for (int j = 0; j < M; j++) {
    double W = (a0 * a0 + pow(E[2 * j], 2)) / (1 + pow(a0 * E[2 * j], 2));
    double U =
        sqrt((E0 * E0 + 2 * E0 * S[j] + W) / (1 + 2 * E8 * S[j] + W * E8 * E8));
    double V = ((1 + E0 * E8) * S[j] + E0 + E8 * W) /
               (1 + 2 * E8 * S[j] + W * E8 * E8);
    R[j] = sqrt((U - V) / 2);
    S[j] = sqrt((U + V) / 2);
    I = -I;
    W = I * R[j] / S[j];
    TQ = (TQ + W) / (1 - TQ * W);
    if (Specification.EllipticType == "Type A") {
      U = (F[1] - S[j]) / R[j];
      V = (F[1] + S[j]) / R[j];
      W = I * (V - U) / (1 + U * V);
      T0 = (T0 + W) / (1 - T0 * W);
    }
    B[0] = B[0] + R[j];
  }

  if (Specification.EllipticType == "Type A")
    T0 = T0 / (1 + sqrt(1 + T0 * T0));

  std::vector<double> DB(M + 1, 0), TB(M + 1, 0), C(M + 1, 0);
  // Calculation of the derivative of the phase at F(K)
  for (int k = IT - 1; k < M; k++) {
    DB[k] = 0;
    TB[k] = T0;
    I = 1;
    for (int j = 0; j < M; j++) {
      DB[k] = DB[k] + 1 / (R[j] + pow(F[k] - S[j], 2) / R[j]);
      DB[k] = DB[k] + 1 / (R[j] + pow(F[k] + S[j], 2) / R[j]);
      I = -I;
      W = (F[k] - I * S[j]) / R[j];
      TB[k] = (TB[k] + W) / (1 - TB[k] * W);
    }
  }

  D[M] = D[M - 1];
  F[M] = F[M - 1];
  DB[M] = DB[M - 1];
  TB[M] = TB[M - 1];

  for (int j = 0; j < M + 1 - IT; j += 2) {
    TB[M - j - 1] = -1 / TB[M - j - 1];
  }

  for (int j = IT - 1; j <= M; j++) {
    B[j] = (1 + pow(TB[j], 2)) * DB[j] / (4 * D[j]) - TB[j] * F[j] / 2;
    C[j] = TB[j] / F[j];
  }
  RL = RS / W;

  // Permutations method Eq 3.6
  for (int l = 0; l < 2; l++) {
    for (int k = l + 2; k < M + 1; k += 2) {
      for (int j = l; j <= k - 2; j += 2) {
        double U = C[j] - C[k];
        double V = 1 / (U / (B[j] * (D[k] - D[j])) - 1);
        C[k] = U * V;
        B[k] = (B[k] - B[j]) * V * V - B[j] * (V + V + 1);
      }
    }
  }

  if (Specification.EllipticType != "Type C")
    W = pow((1 - TQ * T0) / (TQ + T0), 2); // Types A and B have RL != RS
  else
    W = 1;

  for (int j = 0; j < M + 1; j += 2) {
    B[j] = B[j] * W;
    C[j] = C[j] * W;
  }

  RL = RS / W; // Load resistance

  // Create netlist
  int Ci = 1, Li = 1, Ni = 0;

  if (Specification.EllipticType != "Type A") {
    Lseries_LP->at(0) = FP / B[0];
    Li = Li + 1;
    Ni = Ni + 1;
  }

  double V = 0, w, L_, C_;

  for (int j = IT - 1; j < M - 1; j++) {
    V = V * C[j];
    // Calculation of the capacitor of the resonator
    w = F[j] / FP;
    L_ = FP / B[j];
    C_ = 1 / (w * w * L_);
    Cseries_LP->at(j) = C_;
    Lseries_LP->at(j) = FP / B[j];
    Cshunt_LP->at(j) = FP * C[j];

    K = K - 2;
    Ci = Ci + 2;
    Ni = Ni + 1;
    Li = Li + 1;
  }

  w = F[M - 1] / FP;
  L_ = FP / B[M - 1];
  C_ = 1 / (w * w * L_);
  Cseries_LP->at(M - 1) = C_;
  Lseries_LP->at(M - 1) = FP / B[M - 1];
  Cshunt_LP->at(M - 1) = FP * C[M - 1];
  Cshunt_LP->at(M) = FP * C[M];
}

std::complex<double> EllipticFilter::Sn(double u, std::complex<double> z) {
  std::complex<double> x = tanh(z);
  for (int j = 1; j < 10; j++)
    x = x * (tanh(j * u - z) * tanh(j * u + z));
  return x;
}

double EllipticFilter::Sn(double u, double z) {
  double x = tanh(z);
  for (int j = 1; j < 10; j++)
    x = x * (tanh(j * u - z) * tanh(j * u + z));
  return x;
}

/* This function takes the prototype values calculated at EllipticTypeS() and
 * EllipticTypesABC() and put that in schematic format According to that
 * functions the synthesis starts form the load end.
 */
void EllipticFilter::SynthesizeEllipticFilter() {
  WireInfo WI;
  NodeInfo NI;
  unsigned int N = Specification.order;

  // Synthesize CLC of LCL network
  int posx = 0; // Index used for painting. It indicates the current x position
  QMap<QString, unsigned int>
      UnconnectedComponents; // The UnconnectedComponents map contains one or
  // more (componentID, pin number) pair so that the
  // next iteration can wire the current elliptic
  // section to the previos one
  Components.clear();

  unsigned int M = 2 * N + 1;
  double l = (0.5 * (N + 1)) * 2.;
  unsigned int K = M - l - N % 2 - 1;

  if ((Specification.FilterType == Lowpass) ||
      (Specification.FilterType == Highpass))
    posx = N * 50;
  else
    posx = N * 300;

  if (Specification.EllipticType == "Type S")
    RL = Specification.ZS;

  // Change the load impedance according to the filter type
  if ((Specification.FilterType == Lowpass) && (!Specification.isCLC))
    RL = Specification.ZS * Specification.ZS / RL;
  if ((Specification.FilterType == Highpass) && (Specification.isCLC))
    RL = Specification.ZS * Specification.ZS / RL;

  // Add Term 1 (Load)
  ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                          posx, 0, "N0", "gnd");
  TermSpar1.val["Z"] = num2str(RL, Resistance);
  Components.append(TermSpar1);
  UnconnectedComponents[TermSpar1.ID] = 0;
  posx -= 100;
  unsigned int Ni = 0;

  // M/2 sections starting from the load
  for (unsigned int j = 0; j < N; j += 2) {
    InsertEllipticSection(posx, Ni, UnconnectedComponents, j, true, false);
    Ni++;

    if ((Specification.FilterType == Lowpass) ||
        (Specification.FilterType == Highpass))
      posx -= 150;
    else
      posx -= 300;
  }

  // Central shunt capacitor
  InsertEllipticSection(posx, Ni, UnconnectedComponents, N, false, true);

  // Now draw the last M/2 sections (after the central capacitor)
  for (unsigned int j = l + 2; j <= M; j += 2) {
    InsertEllipticSection(posx, Ni, UnconnectedComponents, K, false, false);
    K = K - 2;
    Ni++;

    if ((Specification.FilterType == Lowpass) ||
        (Specification.FilterType == Highpass))
      posx -= 150;
    else
      posx -= 300;
  }

  posx += 100;
  // Add Term 2 (Source)
  ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term,
                          180, posx, 0, QString("N%1").arg(Ni), "gnd");
  TermSpar2.val["Z"] = num2str(Specification.ZS, Resistance);
  Components.append(TermSpar2);

  // Connect the last components to the load (taking into account the pinout)
  QMap<QString, unsigned int>::const_iterator i =
      UnconnectedComponents.constBegin();
  while (i != UnconnectedComponents.constEnd()) {
    WI.setParams(TermSpar2.ID, 0, i.key(), i.value());
    Wires.append(WI);
    ++i;
  }
}

// This function just handles the type of elliptic section to implement. That
// could be done at SynthesizeEllipticFilter() but that'd be a complete mesh
void EllipticFilter::InsertEllipticSection(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, int j, bool flip,
    bool CentralSection) {
  if (Specification.FilterType == Lowpass && Specification.isCLC)
    Insert_LowpassMinL_Section(posx, Ni, UnconnectedComponents, j, flip,
                               CentralSection);
  if (Specification.FilterType == Highpass && !Specification.isCLC)
    Insert_HighpassMinC_Section(posx, Ni, UnconnectedComponents, j, flip,
                                CentralSection);
  if (Specification.FilterType == Lowpass && !Specification.isCLC)
    Insert_LowpassMinC_Section(posx, Ni, UnconnectedComponents, j, flip,
                               CentralSection);
  if (Specification.FilterType == Highpass && Specification.isCLC)
    Insert_HighpassMinL_Section(posx, Ni, UnconnectedComponents, j, flip,
                                CentralSection);
  if (Specification.FilterType == Bandpass && Specification.isCLC)
    Insert_Bandpass_1_Section(posx, Ni, UnconnectedComponents, j, flip,
                              CentralSection);
  if (Specification.FilterType == Bandpass && !Specification.isCLC)
    Insert_Bandpass_2_Section(posx, Ni, UnconnectedComponents, j, flip,
                              CentralSection);
  if (Specification.FilterType == Bandstop && Specification.isCLC)
    Insert_Bandstop_1_Section(posx, Ni, UnconnectedComponents, j, flip,
                              CentralSection);
  if (Specification.FilterType == Bandstop && !Specification.isCLC)
    Insert_Bandstop_2_Section(posx, Ni, UnconnectedComponents, j, flip,
                              CentralSection);
}

// Draw and generate the netlist of a lowpass min L elliptic section
void EllipticFilter::Insert_LowpassMinL_Section(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, unsigned int j,
    bool flip, bool CentralSection) {
  ComponentInfo Cshunt, Ground, Lseries, Cseries;
  NodeInfo NI;
  WireInfo WI;
  QMapIterator<QString, unsigned int> mapIT(UnconnectedComponents);

  if (CentralSection) {
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, posx + 50, 50, QString("N%1").arg(Ni),
                     "gnd");
    Cshunt.val["C"] = num2str(
        Cshunt_LP->at(j) * 1 / (2 * M_PI * Specification.fc * Specification.ZS),
        Capacitance);
    Components.append(Cshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx + 50, 100, "", "");
    Components.append(Ground);

    //***** GND to capacitor *****
    WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
    Wires.append(WI);

    // UnconnectedComponents[Cshunt.ID] = 0;

    // Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx + 50, 0);
    Nodes.append(NI);

    // Wires
    //***** Capacitor to node *****
    WI.setParams(NI.ID, 0, Cshunt.ID, 1);
    Wires.append(WI);

    // Connect components from the previous section
    while (mapIT.hasNext()) {
      mapIT.next();
      WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
      WI.OriginID = NI.ID;
      Wires.append(WI);
    }
    UnconnectedComponents.clear();
    UnconnectedComponents[NI.ID] = 0;
    posx -= 100;

    return;
  }
  // Scale lowpass prototype values
  Cshunt_LP->at(j) *= 1 / (2 * M_PI * Specification.fc * Specification.ZS);
  Cseries_LP->at(j) *= 1 / (2 * M_PI * Specification.fc * Specification.ZS);
  Lseries_LP->at(j) *= Specification.ZS / (2 * M_PI * Specification.fc);

  // Shunt capacitor
  (flip) ? posx += 50 : posx += 50;
  if (!flip)
    Ni++;

  if (Cshunt_LP->at(j) != 0) {
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, vertical, posx, 50, QString("N%1").arg(Ni),
                     "gnd");
    Cshunt.val["C"] = num2str(Cshunt_LP->at(j), Capacitance);
    Components.append(Cshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx, 100, "", "");
    Components.append(Ground);
  }
  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx,
               0);
  Nodes.append(NI);

  //********************************************************************************

  (flip) ? posx -= 50 : posx += 50;

  if (!flip)
    Ni -= 1;

  // Series inductor
  Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                    -90, posx, 0, QString("N%1").arg(Ni),
                    QString("N%1").arg(Ni + 1));
  Lseries.val["L"] = num2str(Lseries_LP->at(j), Inductance);
  Components.append(Lseries);

  if (Cseries_LP->at(j) != 0) {
    // Series capacitor
    Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 90, posx, -80, QString("N%1").arg(Ni),
                      QString("N%1").arg(Ni + 1));
    Cseries.val["C"] = num2str(Cseries_LP->at(j), Capacitance);
    Components.append(Cseries);
  }

  if (flip) {
    // Wires
    if (Cshunt_LP->at(j) != 0) {
      //***** Capacitor to node *****
      WI.setParams(NI.ID, 0, Cshunt.ID, 1);
      Wires.append(WI);

      //***** GND to shunt cap *****
      WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
      Wires.append(WI);
    }

    //***** Inductor to node *****
    WI.setParams(NI.ID, 0, Lseries.ID, 0);
    Wires.append(WI);

    if (Cseries_LP->at(j) != 0) {
      //***** Series cap to node *****
      WI.setParams(NI.ID, 0, Cseries.ID, 1);
      Wires.append(WI);
    }
  } else {
    // Wires
    if (Cshunt_LP->at(j) != 0) {
      //***** Capacitor to node *****
      WI.setParams(NI.ID, 0, Cshunt.ID, 1);
      Wires.append(WI);

      //***** GND to shunt cap *****
      WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
      Wires.append(WI);
    }

    //***** Inductor to node *****
    WI.setParams(NI.ID, 0, Lseries.ID, 1);
    Wires.append(WI);

    if (Cseries_LP->at(j) != 0) {
      //***** Series cap to node *****
      WI.setParams(NI.ID, 0, Cseries.ID, 0);
      Wires.append(WI);
    }
  }
  //***** Connect components from the previous section *****
  if (flip) {
    //***** Inductor to node *****
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 1, mapIT.key(), mapIT.value());
    Wires.append(WI);

    //***** Series cap to node *****
    if (mapIT.hasNext()) {
      mapIT.next();
      WI.setParams(NI.ID, 1, mapIT.key(), mapIT.value());
      Wires.append(WI);
    }
    posx += 50;
    UnconnectedComponents.clear();
    UnconnectedComponents[Lseries.ID] = 1;
    if (Cseries_LP->at(j) != 0)
      UnconnectedComponents[Cseries.ID] = 0;
  } else {
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(Lseries.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);

    if (Cseries_LP->at(j) != 0) {
      if (mapIT.hasNext())
        mapIT.next();
      WI.setParams(mapIT.key(), mapIT.value(), Cseries.ID, 1);
      Wires.append(WI);
    }
    UnconnectedComponents.clear(); // Remove previous section elements
    UnconnectedComponents[NI.ID];

    posx -= 50;
  }
}

// Draw and generate the netlist of a lowpass min L elliptic section
void EllipticFilter::Insert_HighpassMinC_Section(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, unsigned int j,
    bool flip, bool CentralSection) {
  ComponentInfo Lshunt, Ground, Lseries, Cseries;
  NodeInfo NI;
  WireInfo WI;

  double Kl = Specification.ZS / (2 * M_PI * Specification.fc);
  double Kc = 1 / (2 * M_PI * Specification.fc * Specification.ZS);
  QMapIterator<QString, unsigned int> mapIT(UnconnectedComponents);

  if (CentralSection) {
    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     vertical, posx + 50, 50, QString("N%1").arg(Ni), "gnd");
    Lshunt.val["L"] = num2str(Kl / Cshunt_LP->at(j), Inductance);
    Components.append(Lshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx + 50, 100, "", "");
    Components.append(Ground);

    //***** GND to capacitor *****
    WI.setParams(Ground.ID, 0, Lshunt.ID, 0);
    Wires.append(WI);

    // Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx + 50, 0);
    Nodes.append(NI);

    // Wires
    //***** Capacitor to node *****
    WI.setParams(NI.ID, 1, Lshunt.ID, 1);
    Wires.append(WI);

    // Connect components from the previous section
    while (mapIT.hasNext()) {
      mapIT.next();
      WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
      WI.OriginID = NI.ID;
      Wires.append(WI);
    }
    UnconnectedComponents.clear(); // Remove previous section elements
    UnconnectedComponents[NI.ID] = 0;
    posx -= 100;

    return;
  }
  // Scale lowpass prototype values
  double Lshunt_HP = Kl / Cshunt_LP->at(j);
  double Cseries_HP = Kc / Lseries_LP->at(j);
  double Lseries_HP = Kl / Cseries_LP->at(j);

  // Shunt capacitor
  (flip) ? posx += 50 : posx += 50;
  if (!flip)
    Ni++;

  if (Cshunt_LP->at(j) != 0) {
    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     0, posx, 50, QString("N%1").arg(Ni), "gnd");
    Lshunt.val["L"] = num2str(Lshunt_HP, Inductance);
    Components.append(Lshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx, 100, "", "");
    Components.append(Ground);
  }
  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx,
               0);
  Nodes.append(NI);

  //********************************************************************************

  (flip) ? posx -= 50 : posx += 50;

  if (!flip)
    Ni -= 1;

  // Series inductor
  if (Cshunt_LP->at(j) != 0) {
    Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, -90, posx, -80, QString("N%1").arg(Ni),
                      QString("N%1").arg(Ni + 1));
    Lseries.val["L"] = num2str(Lseries_HP, Inductance);
    Components.append(Lseries);
  }

  // Series capacitor
  Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                    Capacitor, 90, posx, 0, QString("N%1").arg(Ni),
                    QString("N%1").arg(Ni + 1));
  Cseries.val["C"] = num2str(Cseries_HP, Capacitance);
  Components.append(Cseries);

  if (flip) {
    // Wires
    if (Cshunt_LP->at(j) != 0) {
      //***** Capacitor to node *****
      WI.setParams(NI.ID, 0, Lshunt.ID, 1);
      Wires.append(WI);

      //***** GND to shunt cap *****
      WI.setParams(Ground.ID, 0, Lshunt.ID, 0);
      Wires.append(WI);

      //***** Inductor to node *****
      WI.setParams(NI.ID, 0, Lseries.ID, 0);
      Wires.append(WI);
    }

    //***** Series cap to node *****
    WI.setParams(NI.ID, 0, Cseries.ID, 1);
    Wires.append(WI);

  } else {
    // Wires
    if (Cshunt_LP->at(j) != 0) {
      //***** Capacitor to node *****
      WI.setParams(NI.ID, 0, Lshunt.ID, 1);
      Wires.append(WI);

      //***** GND to shunt cap *****
      WI.setParams(Ground.ID, 0, Lshunt.ID, 0);
      Wires.append(WI);

      //***** Series inductor to node *****
      WI.setParams(NI.ID, 0, Lseries.ID, 1);
      Wires.append(WI);
    }

    //***** Series capacitor to node *****
    WI.setParams(NI.ID, 0, Cseries.ID, 0);
    Wires.append(WI);
  }
  //***** Connect components from the previous section *****
  if (flip) {
    //***** Inductor to node *****
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 1, mapIT.key(), mapIT.value());
    Wires.append(WI);

    if (mapIT.hasNext()) {
      //***** Series cap to node *****
      mapIT.next();
      WI.setParams(NI.ID, 1, mapIT.key(), mapIT.value());
      Wires.append(WI);
    }
    posx += 50;

    UnconnectedComponents.clear();
    if (Cshunt_LP->at(j) != 0)
      UnconnectedComponents[Lseries.ID] = 1;
    UnconnectedComponents[Cseries.ID] = 0;
  } else {
    if (Cseries_LP->at(j) != 0) {
      if (mapIT.hasNext())
        mapIT.next();
      WI.setParams(mapIT.key(), mapIT.value(), Lseries.ID, 0);
      Wires.append(WI);
    }
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(mapIT.key(), mapIT.value(), Cseries.ID, 1);
    Wires.append(WI);

    UnconnectedComponents.clear(); // Remove previous section elements
    UnconnectedComponents[NI.ID] = 0;

    posx -= 50;
  }
}

void EllipticFilter::Insert_LowpassMinC_Section(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, unsigned int j,
    bool flip, bool CentralSection) {
  ComponentInfo Lshunt, Ground, Lseries, Cshunt;
  NodeInfo NI;
  WireInfo WI;

  double Kl = Specification.ZS / (2 * M_PI * Specification.fc);
  double Kc = 1 / (2 * M_PI * Specification.fc * Specification.ZS);
  int corr = 1;
  QMapIterator<QString, unsigned int> mapIT(UnconnectedComponents);

  if (CentralSection) {
    Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, -90, posx + 50, 0, QString("N%1").arg(Ni),
                      QString("N%1").arg(Ni + 1));
    Lseries.val["L"] = num2str(Kl * Cshunt_LP->at(j), Inductance);
    Components.append(Lseries);

    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(Lseries.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);

    UnconnectedComponents.clear(); // Remove previous section elements
    UnconnectedComponents[Lseries.ID] = 1;
    posx -= 100;
    Ni++;

    return;
  }
  // Scale lowpass prototype values
  double Lshunt_LP_MINC = Kl * Cseries_LP->at(j);
  double Cshunt_LP_MINC = Kc * Lseries_LP->at(j);
  double Lseries_LP_MINC = Kl * Cshunt_LP->at(j);

  // Shunt capacitor
  (flip) ? posx += 50 : posx += 50;
  // if (!flip) Ni++;

  if (Lseries_LP_MINC != 0) {
    Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, -90, posx, 0, QString("N%1").arg(Ni),
                      QString("N%1").arg(Ni + 1));
    Lseries.val["L"] = num2str(Lseries_LP_MINC, Inductance);
    Components.append(Lseries);
  }

  // Node
  if (flip)
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx - 50, 0);
  else
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx + 50, 0);
  Nodes.append(NI);

  //********************************************************************************

  (flip) ? posx -= 50 : posx += 50;
  if (!flip)
    corr = 0;
  else
    corr = 1;

  //  if (!flip)  Ni-=1;

  // Shunt inductor
  if (Lshunt_LP_MINC != 0) {
    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     0, posx, 30, QString("N%1").arg(Ni + corr),
                     QString("N%1R").arg(Ni + corr));
    Lshunt.val["L"] = num2str(Lshunt_LP_MINC, Inductance);
    Components.append(Lshunt);
  }

  // Series capacitor
  if (Lseries_LP_MINC != 0) {
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, posx, 100, QString("N%1R").arg(Ni + corr),
                     "gnd");
  } else {
    Ni--;
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, posx, 100, QString("N%1").arg(Ni + corr),
                     "gnd");
  }
  Cshunt.val["C"] = num2str(Cshunt_LP_MINC, Capacitance);
  Components.append(Cshunt);

  // GND
  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical,
                   posx, 140, "", "");
  Components.append(Ground);

  //***** Inductor to node *****
  if (Lshunt_LP_MINC != 0) {
    WI.setParams(NI.ID, 0, Lshunt.ID, 1);
    Wires.append(WI);
  }

  //***** GND to shunt cap *****
  WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
  Wires.append(WI);

  if (Lshunt_LP_MINC != 0) {
    //***** Inductor to capacitor *****
    WI.setParams(Lshunt.ID, 0, Cshunt.ID, 1);
  } else {
    //***** Capacitor to node *****
    WI.setParams(NI.ID, 0, Cshunt.ID, 1);
  }
  Wires.append(WI);

  // Series inductor to node
  if (Lseries_LP_MINC != 0) {
    WI.setParams(NI.ID, 0, Lseries.ID, flip);
    Wires.append(WI);
  }
  //***** Connect components from the previous section *****
  if (flip) {
    //***** Inductor to node *****
    if (mapIT.hasNext())
      mapIT.next();
    if (Lseries_LP_MINC != 0)
      WI.setParams(Lseries.ID, 0, mapIT.key(), mapIT.value());
    else
      WI.setParams(NI.ID, 1, mapIT.key(), mapIT.value());

    Wires.append(WI);

    posx += 50;

    UnconnectedComponents.clear();
    if (Lseries_LP_MINC != 0)
      UnconnectedComponents[Lseries.ID] = 1;
    else
      UnconnectedComponents[NI.ID] = 0;
  } else {
    //***** Inductor to node *****
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);
    if (j == Specification.order)
      Ni++;

    UnconnectedComponents.clear();
    if (Lseries_LP_MINC != 0)
      UnconnectedComponents[Lseries.ID] = 1;
    else
      UnconnectedComponents[NI.ID] = 0;

    if (j == Specification.order)
      Ni--;

    posx -= 50;
  }
}

void EllipticFilter::Insert_HighpassMinL_Section(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, unsigned int j,
    bool flip, bool CentralSection) {
  ComponentInfo Lshunt, Ground, Cseries, Cshunt;
  NodeInfo NI;
  WireInfo WI;

  double Kl = Specification.ZS / (2 * M_PI * Specification.fc);
  double Kc = 1 / (2 * M_PI * Specification.fc * Specification.ZS);
  int corr = 1;
  QMapIterator<QString, unsigned int> mapIT(UnconnectedComponents);

  if (CentralSection) {
    Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 90, posx + 50, 0, QString("N%1").arg(Ni),
                      QString("N%1").arg(Ni + 1));
    Cseries.val["C"] = num2str(Kc / Cshunt_LP->at(j), Capacitance);
    Components.append(Cseries);

    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(Cseries.ID, 1, mapIT.key(), mapIT.value());
    Wires.append(WI);

    UnconnectedComponents.clear(); // Remove previous section elements
    UnconnectedComponents[Cseries.ID] = 0;
    posx -= 100;
    Ni++;

    return;
  }
  // Scale lowpass prototype values
  double Lshunt_HP_MINL = Kl / Lseries_LP->at(j);
  double Cshunt_HP_MINL = Kc / Cseries_LP->at(j);
  double Cseries_HP_MINL = Kc / Cshunt_LP->at(j);

  // Shunt capacitor
  (flip) ? posx += 50 : posx += 50;

  if (Cshunt_LP->at(j) != 0) {
    Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 90, posx, 0, QString("N%1").arg(Ni),
                      QString("N%1").arg(Ni + 1));
    Cseries.val["C"] = num2str(Cseries_HP_MINL, Capacitance);
    Components.append(Cseries);
  }
  // Node
  if (flip)
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx - 50, 0);
  else
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx + 50, 0);
  Nodes.append(NI);

  (flip) ? posx -= 50 : posx += 50;

  if (!flip)
    corr = 0;
  else
    corr = 1;

  // Shunt inductor
  if (Cseries_LP->at(j) != 0) {
    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     0, posx, 30, QString("N%1").arg(Ni + corr),
                     QString("N%1R").arg(Ni + corr));
  } else {
    Ni--;
    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     0, posx, 30, QString("N%1").arg(Ni + corr), "gnd");
  }
  Lshunt.val["L"] = num2str(Lshunt_HP_MINL, Inductance);
  Components.append(Lshunt);

  // Series capacitor
  if (Cseries_LP->at(j) != 0) {
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, posx, 100, QString("N%1R").arg(Ni + corr),
                     "gnd");
    Cshunt.val["C"] = num2str(Cshunt_HP_MINL, Capacitance);
    Components.append(Cshunt);
  }

  // GND
  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical,
                   posx, 140, "", "");
  Components.append(Ground);

  //***** Inductor to node *****
  WI.setParams(NI.ID, 0, Lshunt.ID, 1);
  Wires.append(WI);

  //***** GND to shunt cap *****
  if (Cseries_LP->at(j) != 0) {
    WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
    Wires.append(WI);
  }

  if (Cseries_LP->at(j) != 0) {
    //***** Inductor to capacitor *****
    WI.setParams(Lshunt.ID, 0, Cshunt.ID, 1);
  } else {
    //***** Inductor to node *****
    WI.setParams(Ground.ID, 0, Lshunt.ID, 0);
  }
  Wires.append(WI);

  // Series inductor to node
  if (Cshunt_LP->at(j) != 0) {
    WI.setParams(NI.ID, 0, Cseries.ID, !flip);
    Wires.append(WI);
  }
  //***** Connect components from the previous section *****
  if (flip) {
    //***** Inductor to node *****
    if (mapIT.hasNext())
      mapIT.next();
    if (Cshunt_LP->at(j) != 0)
      WI.setParams(Cseries.ID, 1, mapIT.key(), mapIT.value());
    else
      WI.setParams(NI.ID, 1, mapIT.key(), mapIT.value());

    Wires.append(WI);

    posx += 50;

    UnconnectedComponents.clear();
    if (Cshunt_LP->at(j) != 0)
      UnconnectedComponents[Cseries.ID] = 0;
    else
      UnconnectedComponents[NI.ID] = 0;
  } else {
    //***** Inductor to node *****
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);
    if (j == Specification.order)
      Ni++;

    UnconnectedComponents.clear();
    if (Cshunt_LP->at(j) != 0)
      UnconnectedComponents[Cseries.ID] = 0;
    else
      UnconnectedComponents[NI.ID] = 0;
    if (j == Specification.order)
      Ni--;
    posx -= 50;
  }
}

void EllipticFilter::Insert_Bandpass_1_Section(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, unsigned int j,
    bool flip, bool CentralSection) {
  ComponentInfo Lshunt, Ground, Cseries1, Cseries2, Lseries1, Lseries2, Cshunt;
  NodeInfo NI;
  WireInfo WI;
  QMapIterator<QString, unsigned int> mapIT(UnconnectedComponents);

  double Kl = Specification.ZS / (2 * M_PI * Specification.fc);
  double Kc = 1 / (2 * M_PI * Specification.fc * Specification.ZS);

  double delta = Specification.bw / Specification.fc;

  if (CentralSection) {
    double Cshunt_BP_T1 = Kc * Cshunt_LP->at(j) / delta;
    double Lshunt_BP_T1 = Kl * delta / Cshunt_LP->at(j);

    // Node
    NI.setParams(QString("N%1").arg(NumberComponents[ConnectionNodes]++),
                 posx + 25, 0);
    Nodes.append(NI);

    // Shunt resonator
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, posx + 50, 50, QString("%1").arg(NI.ID),
                     "gnd");
    Cshunt.val["C"] = num2str(Cshunt_BP_T1, Capacitance);
    Components.append(Cshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx + 50, 100, "", "");
    Components.append(Ground);

    WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     0, posx, 50, QString("%1").arg(NI.ID), "gnd");
    Lshunt.val["L"] = num2str(Lshunt_BP_T1, Inductance);
    Components.append(Lshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx, 100, "", "");
    Components.append(Ground);

    WI.setParams(Lshunt.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    WI.setParams(Lshunt.ID, 1, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 1, NI.ID, 0);
    Wires.append(WI);

    // Connect components from previous sections
    while (mapIT.hasNext()) {
      mapIT.next();
      WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
      Wires.append(WI);
    }

    UnconnectedComponents.clear(); // Remove previous section elements
    UnconnectedComponents[NI.ID] = 0;
    posx -= 200;
    Ni++;

    return;
  }

  // Scale lowpass prototype values
  double Cshunt_BP_T1 = Kc * Cshunt_LP->at(j) / delta;
  double Lshunt_BP_T1 = Kl * delta / Cshunt_LP->at(j);

  double Cseries1_BP_T1 = Kc * Cseries_LP->at(j) / delta;
  double Lseries1_BP_T1 = Kl * delta / Cseries_LP->at(j);

  double Cseries2_BP_T1 = Kc * delta / Lseries_LP->at(j);
  double Lseries2_BP_T1 = Kl * Lseries_LP->at(j) / delta;

  (flip) ? posx += 50 : posx += 50;

  // Node
  NI.setParams(QString("N%1").arg(NumberComponents[ConnectionNodes]++),
               posx - 25, 0);
  Nodes.append(NI);

  if (Cshunt_BP_T1 != 0) {
    // Shunt resonator
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, posx - 50, 50, QString("%1").arg(NI.ID),
                     "gnd");
    Cshunt.val["C"] = num2str(Cshunt_BP_T1, Capacitance);
    Components.append(Cshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx - 50, 100, "", "");
    Components.append(Ground);

    WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     0, posx, 50, QString("%1").arg(NI.ID), "gnd");
    Lshunt.val["L"] = num2str(Lshunt_BP_T1, Inductance);
    Components.append(Lshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx, 100, "", "");
    Components.append(Ground);

    WI.setParams(Lshunt.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    WI.setParams(Lshunt.ID, 1, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 1, NI.ID, 0);
    Wires.append(WI);
  }

  if (flip)
    posx -= 200;

  if (!flip) {

    if (Cseries1_BP_T1 != 0) {
      Cseries1.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
          posx + 75, -50,
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 2));
      Cseries1.val["C"] = num2str(Cseries1_BP_T1, Capacitance);
      Components.append(Cseries1);

      Lseries1.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
          posx + 75, -100,
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 2));
      Lseries1.val["L"] = num2str(Lseries1_BP_T1, Inductance);
      Components.append(Lseries1);
    }

    Cseries2.setParams(
        QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
        posx + 40, 0, QString("NV%1").arg(virtual_nodes),
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 2));
    Cseries2.val["C"] = num2str(Cseries2_BP_T1, Capacitance);
    Components.append(Cseries2);

    Lseries2.setParams(
        QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
        posx + 110, 0, QString("NV%1").arg(virtual_nodes++),
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 1));
    Lseries2.val["L"] = num2str(Lseries2_BP_T1, Inductance);
    Components.append(Lseries2);
  } else {
    if (Cseries1_BP_T1 != 0) {
      Cseries1.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
          posx + 75, -50,
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
          QString("N%1").arg(NumberComponents[ConnectionNodes]));
      Cseries1.val["C"] = num2str(Cseries1_BP_T1, Capacitance);
      Components.append(Cseries1);

      Lseries1.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
          posx + 75, -100,
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
          QString("N%1").arg(NumberComponents[ConnectionNodes]));
      Lseries1.val["L"] = num2str(Lseries1_BP_T1, Inductance);
      Components.append(Lseries1);
    }
    Cseries2.setParams(
        QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, -90,
        posx + 40, 0, QString("NV%1").arg(virtual_nodes),
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 1));
    Cseries2.val["C"] = num2str(Cseries2_BP_T1, Capacitance);
    Components.append(Cseries2);

    Lseries2.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                       Inductor, -90, posx + 110, 0,
                       QString("NV%1").arg(virtual_nodes++),
                       QString("N%1").arg(NumberComponents[ConnectionNodes]));
    Lseries2.val["L"] = num2str(Lseries2_BP_T1, Inductance);
    Components.append(Lseries2);
  }

  //***** Connect components from the previous section *****
  if (flip) {
    while (mapIT.hasNext()) {
      mapIT.next();
      WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
      Wires.append(WI);
    }
    posx += 250;
  } else { // Here it connects all the two resonators to the previous node
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(mapIT.key(), mapIT.value(), Lseries2.ID, 0);
    Wires.append(WI);
    if (Cseries1_BP_T1 != 0) {
      if (mapIT.hasNext())
        mapIT.next();
      WI.setParams(mapIT.key(), mapIT.value(), Cseries1.ID, 1);
      Wires.append(WI);
      if (mapIT.hasNext())
        mapIT.next();
      WI.setParams(mapIT.key(), mapIT.value(), Lseries1.ID, 0);
      Wires.append(WI);

      posx += 50;
    }
  }

  if (flip) {
    WI.setParams(Lseries2.ID, 1, Cseries2.ID, 0);
    Wires.append(WI);

    WI.setParams(NI.ID, 1, Lseries2.ID, 0);
    Wires.append(WI);

    UnconnectedComponents.clear();

    if (Cseries1_BP_T1 != 0) {
      WI.setParams(NI.ID, 1, Lseries1.ID, 0);
      Wires.append(WI);

      WI.setParams(NI.ID, 1, Cseries1.ID, 1);
      Wires.append(WI);

      UnconnectedComponents[Cseries1.ID] = 0;
      UnconnectedComponents[Lseries1.ID] = 1;
    }
    UnconnectedComponents[Cseries2.ID] = 1;
  } else {
    WI.setParams(Lseries2.ID, 1, Cseries2.ID, 1);
    Wires.append(WI);

    if (Cseries1_BP_T1 != 0) {
      WI.setParams(NI.ID, 1, Lseries1.ID, 1);
      Wires.append(WI);

      WI.setParams(NI.ID, 1, Cseries1.ID, 0);
      Wires.append(WI);
    }
    WI.setParams(NI.ID, 1, Cseries2.ID, 0);
    Wires.append(WI);

    UnconnectedComponents.clear();
    UnconnectedComponents[NI.ID] = 0;
  }

  if (j == Specification.order - 1)
    Ni--;
}

void EllipticFilter::Insert_Bandpass_2_Section(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, unsigned int j,
    bool flip, bool CentralSection) {
  ComponentInfo Ground, Cshunt1, Cshunt2, Lshunt1, Lshunt2, Lseries, Cseries;
  NodeInfo NI;
  WireInfo WI;

  double Kl = Specification.ZS / (2 * M_PI * Specification.fc);
  double Kc = 1 / (2 * M_PI * Specification.fc * Specification.ZS);
  double delta = Specification.bw / Specification.fc;

  QMapIterator<QString, unsigned int> mapIT(UnconnectedComponents);

  if (CentralSection) {
    double Cseries_BP_T2 = Kc * delta / Cshunt_LP->at(j);
    double Lseries_BP_T2 = Kl * Cshunt_LP->at(j) / delta;

    // Series resonator
    Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, -90, posx + 50, 0,
                      QString("N%1").arg(NumberComponents[ConnectionNodes]),
                      QString("NV%1").arg(++virtual_nodes));
    Lseries.val["L"] = num2str(Lseries_BP_T2, Inductance);
    Components.append(Lseries);

    Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 90, posx - 25, 0,
                      QString("N%1").arg(NumberComponents[ConnectionNodes] + 1),
                      QString("NV%1").arg(virtual_nodes));
    Cseries.val["C"] = num2str(Cseries_BP_T2, Capacitance);
    Components.append(Cseries);

    WI.setParams(Lseries.ID, 1, Cseries.ID, 1);
    Wires.append(WI);

    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(mapIT.key(), mapIT.value(), Lseries.ID, 0);
    Wires.append(WI);

    UnconnectedComponents.clear();
    UnconnectedComponents[Cseries.ID] = 0;

    posx -= 200;
    Ni++;

    return;
  }

  // Scale lowpass prototype values
  double Cseries_BP_T2 = Kc * delta / Cshunt_LP->at(j);
  double Lseries_BP_T2 = Kl * Cshunt_LP->at(j) / delta;

  double Cshunt1_BP_T2 = Kc * delta / Cseries_LP->at(j);
  double Lshunt1_BP_T2 = Kl * Cseries_LP->at(j) / delta;

  double Cshunt2_BP_T2 = Kc * Lseries_LP->at(j) / delta;
  double Lshunt2_BP_T2 = Kl * delta / Lseries_LP->at(j);

  // Shunt capacitor
  (flip) ? posx += 50 : posx += 50;

  // Series resonator
  if (Lseries_BP_T2 != 0) {
    if (!flip) {

      Lseries.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
          posx - 0, 0,
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 1),
          QString("NV%1").arg(++virtual_nodes));
      Lseries.val["L"] = num2str(Lseries_BP_T2, Inductance);
      Components.append(Lseries);

      Cseries.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
          posx - 75, 0, QString("NV%1").arg(virtual_nodes++),
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 2));
      Cseries.val["C"] = num2str(Cseries_BP_T2, Capacitance);
      Components.append(Cseries);

    } else {
      Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                        Inductor, -90, posx - 0, 0,
                        QString("N%1").arg(NumberComponents[ConnectionNodes]),
                        QString("NV%1").arg(++virtual_nodes));
      Lseries.val["L"] = num2str(Lseries_BP_T2, Inductance);
      Components.append(Lseries);

      Cseries.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
          posx - 75, 0, QString("NV%1").arg(virtual_nodes++),
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 1));
      Cseries.val["C"] = num2str(Cseries_BP_T2, Capacitance);
      Components.append(Cseries);
    }
  } else {
    NumberComponents[ConnectionNodes]--;
  }

  if (flip)
    posx -= 200;

  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
               posx + 75, 0);
  Nodes.append(NI);

  if (Lseries_BP_T2 != 0) {
    if (!flip) {
      WI.setParams(NI.ID, 0, Lseries.ID, 0);
      Wires.append(WI);

      WI.setParams(Lseries.ID, 1, Cseries.ID, 1);
      Wires.append(WI);
    } else {
      WI.setParams(Lseries.ID, 1, Cseries.ID, 1);
      Wires.append(WI);

      WI.setParams(Cseries.ID, 0, NI.ID, 0);
      Wires.append(WI);
    }
  } else {
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);
  }

  if (Lseries_BP_T2 !=
      0) { // If Lshunt -> 0 and Cshunt->+infty we should skip these components.
    Cshunt1.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 0, posx + 75, 50,
                      QString("N%1").arg(NumberComponents[ConnectionNodes]),
                      QString("NV%1").arg(virtual_nodes));
    Cshunt1.val["C"] = num2str(Cshunt1_BP_T2, Capacitance);

    Components.append(Cshunt1);

    Lshunt1.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, 0, posx + 75, 100,
                      QString("NV%1").arg(virtual_nodes),
                      QString("NV%1").arg(virtual_nodes + 1));
    Lshunt1.val["L"] = num2str(Lshunt1_BP_T2, Inductance);
    Components.append(Lshunt1);
    virtual_nodes++;

    WI.setParams(Cshunt1.ID, 0, Lshunt1.ID, 1);
    Wires.append(WI);
  }

  if (Lseries_BP_T2 != 0) {
    Cshunt2.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 0, posx + 40, 150,
                      QString("NV%1").arg(virtual_nodes), "gnd");
    Cshunt2.val["C"] = num2str(Cshunt2_BP_T2, Capacitance);
    Components.append(Cshunt2);
  } else {
    Cshunt2.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 0, posx + 40, 150,
                      QString("N%1").arg(NumberComponents[ConnectionNodes]),
                      "gnd");
    Cshunt2.val["C"] = num2str(Cshunt2_BP_T2, Capacitance);
    Components.append(Cshunt2);
  }
  // GND
  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical,
                   posx + 40, 200, "", "");
  Components.append(Ground);

  WI.setParams(Cshunt2.ID, 0, Ground.ID, 0);
  Wires.append(WI);

  if (Lseries_BP_T2 != 0) {
    Lshunt2.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, 0, posx + 110, 150,
                      QString("NV%1").arg(virtual_nodes++), "gnd");
    Lshunt2.val["L"] = num2str(Lshunt2_BP_T2, Inductance);
    Components.append(Lshunt2);
  } else {
    Lshunt2.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, 0, posx + 110, 150,
                      QString("N%1").arg(NumberComponents[ConnectionNodes]),
                      "gnd");
    Lshunt2.val["L"] = num2str(Lshunt2_BP_T2, Inductance);
    Components.append(Lshunt2);
  }

  // GND
  Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical,
                   posx + 110, 200, "", "");
  Components.append(Ground);

  WI.setParams(Lshunt2.ID, 0, Ground.ID, 0);
  Wires.append(WI);

  if (Lseries_BP_T2 != 0) {
    WI.setParams(Lshunt1.ID, 0, Lshunt2.ID, 1);
    Wires.append(WI);

    WI.setParams(Lshunt1.ID, 0, Cshunt2.ID, 1);
    Wires.append(WI);

    WI.setParams(Cshunt1.ID, 1, NI.ID, 1);
    Wires.append(WI);
  } else {
    WI.setParams(NI.ID, 1, Cshunt2.ID, 1);
    Wires.append(WI);

    WI.setParams(NI.ID, 1, Lshunt2.ID, 1);
    Wires.append(WI);
  }

  //***** Connect components from the previous section *****
  if (flip) {
    if (Lseries_BP_T2 != 0) {
      if (mapIT.hasNext())
        mapIT.next();
      WI.setParams(mapIT.key(), mapIT.value(), Lseries.ID, 0);
      Wires.append(WI);
    }
    posx += 250;
  } else {
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);

    posx += 50;
  }

  UnconnectedComponents.clear();
  if (flip) {
    UnconnectedComponents[NI.ID] = 0;
  } else {
    UnconnectedComponents[Cseries.ID] = 0;
  }

  if (((Specification.EllipticType == "Type B") ||
       (Specification.EllipticType == "Type C")) &&
      (j == Specification.order - 1))
    Ni--;
}

void EllipticFilter::Insert_Bandstop_2_Section(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, unsigned int j,
    bool flip, bool CentralSection) {
  ComponentInfo Lshunt, Ground, Cseries1, Cseries2, Lseries1, Lseries2, Cshunt;
  NodeInfo NI;
  WireInfo WI;
  QMapIterator<QString, unsigned int> mapIT(UnconnectedComponents);

  double Kl = Specification.ZS / (2 * M_PI * Specification.fc);
  double Kc = 1 / (2 * M_PI * Specification.fc * Specification.ZS);
  double delta = Specification.bw / Specification.fc;

  if (CentralSection) {
    double Cshunt_BS_T1 = Kc * Cshunt_LP->at(j) * delta;
    double Lshunt_BS_T1 = Kl / (delta * Cshunt_LP->at(j));

    // Series resonator
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                     Capacitor, 0, posx + 25, 100,
                     QString("N%1").arg(NumberComponents[ConnectionNodes]),
                     QString("NV%1").arg(++virtual_nodes));
    Cshunt.val["C"] = num2str(Cshunt_BS_T1, Capacitance);
    Components.append(Cshunt);

    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     0, posx + 25, 50, QString("NV%1").arg(virtual_nodes),
                     "gnd");
    Lshunt.val["L"] = num2str(Lshunt_BS_T1, Inductance);
    Components.append(Lshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx + 25, 150, "", "");
    Components.append(Ground);

    // Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
                 posx + 25, 0);
    Nodes.append(NI);

    WI.setParams(Lshunt.ID, 0, Cshunt.ID, 1);
    Wires.append(WI);

    WI.setParams(Lshunt.ID, 1, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    while (mapIT.hasNext()) {
      mapIT.next();
      WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
      Wires.append(WI);
    }

    UnconnectedComponents.clear(); // Remove previous section elements
    UnconnectedComponents[NI.ID] = 0;
    posx -= 200;
    Ni++;

    return;
  }

  // Scale lowpass prototype values
  double Cseries1_BS_T2 = Kc / (delta * Lseries_LP->at(j));
  double Lseries1_BS_T2 = Kl * Lseries_LP->at(j) * delta;

  double Cseries2_BS_T2 = Kc * delta * Cseries_LP->at(j);
  double Lseries2_BS_T2 = Kl / (Cseries_LP->at(j) * delta);

  double Cshunt_BS_T2 = Kc * Cshunt_LP->at(j) * delta;
  double Lshunt_BS_T2 = Kl / (delta * Cshunt_LP->at(j));
  // Shunt capacitor
  (flip) ? posx += 50 : posx += 50;

  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
               posx - 25, 0);
  Nodes.append(NI);

  if (Cshunt_BS_T2 != 0) {
    if (flip) {
      // Shunt resonator
      Cshunt.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 0,
          posx - 25, 100,
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
          QString("NV%1").arg(++virtual_nodes));
      Cshunt.val["C"] = num2str(Cshunt_BS_T2, Capacitance);
      Components.append(Cshunt);
    } else {
      // Shunt resonator
      Cshunt.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 0,
          posx - 25, 100,
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
          QString("NV%1").arg(++virtual_nodes));
      Cshunt.val["C"] = num2str(Cshunt_BS_T2, Capacitance);
      Components.append(Cshunt);
    }

    Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor,
                     vertical, posx - 25, 50,
                     QString("NV%1").arg(virtual_nodes), "gnd");
    Lshunt.val["L"] = num2str(Lshunt_BS_T2, Inductance);
    Components.append(Lshunt);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx - 25, 150, "", "");
    Components.append(Ground);

    WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    WI.setParams(Lshunt.ID, 1, NI.ID, 0);
    Wires.append(WI);

    WI.setParams(Cshunt.ID, 1, Lshunt.ID, 0);
    Wires.append(WI);
  }

  if (flip)
    posx -= 200;

  if (flip) {
    Cseries1.setParams(
        QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
        posx + 75, -50,
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
        QString("N%1").arg(NumberComponents[ConnectionNodes]));
    Cseries1.val["C"] = num2str(Cseries1_BS_T2, Capacitance);
    Components.append(Cseries1);

    Lseries1.setParams(
        QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
        posx + 75, -100,
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
        QString("N%1").arg(NumberComponents[ConnectionNodes]));
    Lseries1.val["L"] = num2str(Lseries1_BS_T2, Inductance);
    Components.append(Lseries1);

    if (Cshunt_BS_T2 != 0) {
      Lseries2.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
          posx + 110, 0, QString("NV%1").arg(++virtual_nodes),
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 1));
      Lseries2.val["L"] = num2str(Lseries2_BS_T2, Inductance);
      Components.append(Lseries2);

      Cseries2.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                         Capacitor, 90, posx + 40, 0,
                         QString("N%1").arg(NumberComponents[ConnectionNodes]),
                         QString("NV%1").arg(virtual_nodes));
      Cseries2.val["C"] = num2str(Cseries2_BS_T2, Capacitance);
      Components.append(Cseries2);
    }
  } else {
    Cseries1.setParams(
        QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
        posx + 75, -50,
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 2),
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 1));
    Cseries1.val["C"] = num2str(Cseries1_BS_T2, Capacitance);
    Components.append(Cseries1);

    Lseries1.setParams(
        QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
        posx + 75, -100,
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 2),
        QString("N%1").arg(NumberComponents[ConnectionNodes] - 1));
    Lseries1.val["L"] = num2str(Lseries1_BS_T2, Inductance);
    Components.append(Lseries1);

    if (Cshunt_BS_T2 != 0) {
      Lseries2.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
          posx + 110, 0, QString("NV%1").arg(++virtual_nodes),
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 2));
      Lseries2.val["L"] = num2str(Lseries2_BS_T2, Inductance);
      Components.append(Lseries2);

      Cseries2.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
          posx + 40, 0,
          QString("N%1").arg(NumberComponents[ConnectionNodes] - 1),
          QString("NV%1").arg(virtual_nodes));
      Cseries2.val["C"] = num2str(Cseries2_BS_T2, Capacitance);
      Components.append(Cseries2);
    }
  }
  //***** Connect components from the previous section *****
  if (flip) {
    while (mapIT.hasNext()) {
      mapIT.next();
      WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
      Wires.append(WI);
    }
    posx += 250;
  } else { // Here it connects all the two resonators to the previous node
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(mapIT.key(), mapIT.value(), Lseries1.ID, 0, Qt::red);
    Wires.append(WI);
    if (Cshunt_BS_T2 != 0) {
      if (mapIT.hasNext())
        mapIT.next();
      WI.setParams(mapIT.key(), mapIT.value(), Lseries2.ID, 0, Qt::red);
      Wires.append(WI);
    }
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(mapIT.key(), mapIT.value(), Cseries1.ID, 1);
    Wires.append(WI);
    posx += 50;
  }

  UnconnectedComponents.clear();
  if (flip) {

    UnconnectedComponents[Cseries1.ID] = 0;
    UnconnectedComponents[Lseries1.ID] = 1;

    if (Cshunt_BS_T2 != 0) {
      WI.setParams(Lseries2.ID, 1, Cseries2.ID, 1);
      Wires.append(WI);

      WI.setParams(NI.ID, 1, Lseries2.ID, 0);
      Wires.append(WI);

      UnconnectedComponents[Cseries2.ID] = 0;
    }
    WI.setParams(NI.ID, 1, Lseries1.ID, 0);
    Wires.append(WI);

    WI.setParams(NI.ID, 1, Cseries1.ID, 1);
    Wires.append(WI);
  } else {
    if (Cshunt_BS_T2 != 0) {
      WI.setParams(Lseries2.ID, 1, Cseries2.ID, 1);
      Wires.append(WI);

      WI.setParams(NI.ID, 1, Cseries2.ID, 0);
      Wires.append(WI);
    }

    WI.setParams(NI.ID, 1, Lseries1.ID, 1);
    Wires.append(WI);

    WI.setParams(NI.ID, 1, Cseries1.ID, 0);
    Wires.append(WI);

    UnconnectedComponents[NI.ID] = 0;
  }

  if (j == Specification.order - 1)
    Ni--;
}

void EllipticFilter::Insert_Bandstop_1_Section(
    int &posx, unsigned int &Ni,
    QMap<QString, unsigned int> &UnconnectedComponents, unsigned int j,
    bool flip, bool CentralSection) {
  ComponentInfo Ground, Cshunt1, Cshunt2, Lshunt1, Lshunt2, Lseries, Cseries;
  NodeInfo NI;
  WireInfo WI;
  QMapIterator<QString, unsigned int> mapIT(UnconnectedComponents);

  double Kl = Specification.ZS / (2 * M_PI * Specification.fc);
  double Kc = 1 / (2 * M_PI * Specification.fc * Specification.ZS);
  double delta = Specification.bw / Specification.fc;

  if (CentralSection) {
    double Cseries_BS_T1 = Kc / (delta * Cshunt_LP->at(j));
    double Lseries_BS_T1 = Kl * Cshunt_LP->at(j) * delta;

    // Series resonator
    Cseries.setParams(
        QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
        posx + 25, -50, QString("N%1").arg(NumberComponents[ConnectionNodes]),
        QString("N%1").arg(NumberComponents[ConnectionNodes] + 1));
    Cseries.val["C"] = num2str(Cseries_BS_T1, Capacitance);
    Components.append(Cseries);

    Lseries.setParams(
        QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
        posx + 25, 0, QString("N%1").arg(NumberComponents[ConnectionNodes]),
        QString("N%1").arg(NumberComponents[ConnectionNodes] + 1));
    Lseries.val["L"] = num2str(Lseries_BS_T1, Inductance);
    Components.append(Lseries);

    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(mapIT.key(), mapIT.value(), Cseries.ID, 1);
    Wires.append(WI);

    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(mapIT.key(), mapIT.value(), Lseries.ID, 0);
    Wires.append(WI);

    UnconnectedComponents.clear();
    UnconnectedComponents[Lseries.ID] = 1;
    UnconnectedComponents[Cseries.ID] = 0;

    posx -= 200;
    Ni++;

    return;
  }

  // Scale lowpass prototype values
  double Cseries_BS_T1 = Kc / (delta * Cshunt_LP->at(j));
  double Lseries_BS_T1 = Kl * Cshunt_LP->at(j) * delta;

  double Cshunt1_BS_T1 = Kc * Lseries_LP->at(j) * delta;
  double Lshunt1_BS_T1 = Kl / (delta * Lseries_LP->at(j));

  double Cshunt2_BS_T1 = Kc / (Cseries_LP->at(j) * delta);
  double Lshunt2_BS_T1 = Kl * delta * Cseries_LP->at(j);

  // Shunt capacitor
  (flip) ? posx += 50 : posx += 50;

  if (Lseries_BS_T1 != 0) {
    if (flip) {
      // Series resonator
      Cseries.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
          posx - 25, -50, QString("N%1").arg(NumberComponents[ConnectionNodes]),
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 1));
      Cseries.val["C"] = num2str(Cseries_BS_T1, Capacitance);
      Components.append(Cseries);

      Lseries.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
          posx - 25, 0, QString("N%1").arg(NumberComponents[ConnectionNodes]),
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 1));
      Lseries.val["L"] = num2str(Lseries_BS_T1, Inductance);
      Components.append(Lseries);
    } else {
      // Series resonator
      Cseries.setParams(
          QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, 90,
          posx - 25, -50,
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 1),
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 2));
      Cseries.val["C"] = num2str(Cseries_BS_T1, Capacitance);
      Components.append(Cseries);

      Lseries.setParams(
          QString("L%1").arg(++NumberComponents[Inductor]), Inductor, -90,
          posx - 25, 0,
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 1),
          QString("N%1").arg(NumberComponents[ConnectionNodes] + 2));
      Lseries.val["L"] = num2str(Lseries_BS_T1, Inductance);
      Components.append(Lseries);
    }
  } else {
    NumberComponents[ConnectionNodes]--;
  }

  if (flip)
    posx -= 200;

  // Node
  NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]),
               posx + 75, 0);
  Nodes.append(NI);

  if (Lseries_BS_T1 != 0) {
    if (!flip) {
      WI.setParams(Lseries.ID, 0, NI.ID, 0);
      Wires.append(WI);

      WI.setParams(NI.ID, 0, Cseries.ID, 1);
      Wires.append(WI);
    } else {
      WI.setParams(Lseries.ID, 1, NI.ID, 0);
      Wires.append(WI);

      WI.setParams(NI.ID, 0, Cseries.ID, 0);
      Wires.append(WI);
    }
  } else {
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);
  }

  Cshunt1.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                    Capacitor, 0, posx + 75, 50,
                    QString("N%1").arg(NumberComponents[ConnectionNodes]),
                    QString("NV%1").arg(++virtual_nodes));
  Cshunt1.val["C"] = num2str(Cshunt1_BS_T1, Capacitance);
  Components.append(Cshunt1);

  if (Lseries_BS_T1 != 0) {
    Lshunt1.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, 0, posx + 75, 100,
                      QString("NV%1").arg(virtual_nodes),
                      QString("NV%1").arg(virtual_nodes + 1));
  } else {
    Lshunt1.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, 0, posx + 75, 100,
                      QString("NV%1").arg(virtual_nodes), "gnd");
  }
  virtual_nodes++;
  Lshunt1.val["L"] = num2str(Lshunt1_BS_T1, Inductance);
  Components.append(Lshunt1);

  WI.setParams(Cshunt1.ID, 0, Lshunt1.ID, 1);
  Wires.append(WI);

  WI.setParams(Cshunt1.ID, 1, NI.ID, 1);
  Wires.append(WI);

  if (Lseries_BS_T1 != 0) {
    Cshunt2.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),
                      Capacitor, 0, posx + 40, 150,
                      QString("NV%1").arg(virtual_nodes), "gnd");
    Cshunt2.val["C"] = num2str(Cshunt2_BS_T1, Capacitance);
    Components.append(Cshunt2);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx + 40, 200, "", "");
    Components.append(Ground);

    WI.setParams(Cshunt2.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    Lshunt2.setParams(QString("L%1").arg(++NumberComponents[Inductor]),
                      Inductor, 0, posx + 110, 150,
                      QString("NV%1").arg(virtual_nodes), "gnd");
    Lshunt2.val["L"] = num2str(Lshunt2_BS_T1, Inductance);
    Components.append(Lshunt2);

    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx + 110, 200, "", "");
    Components.append(Ground);

    WI.setParams(Lshunt2.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    WI.setParams(Lshunt1.ID, 0, Lshunt2.ID, 1);
    Wires.append(WI);

    WI.setParams(Lshunt1.ID, 0, Cshunt2.ID, 1);
    Wires.append(WI);
  } else { // Cshunt -> +infty and Lshunt -> 0
    // GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND,
                     vertical, posx + 75, 200, "", "");
    Components.append(Ground);

    WI.setParams(Lshunt1.ID, 0, Ground.ID, 0);
    Wires.append(WI);
  }

  //***** Connect components from the previous section *****
  if (flip) {
    if (Lseries_BS_T1 != 0) {
      if (mapIT.hasNext())
        mapIT.next();
      WI.setParams(mapIT.key(), mapIT.value(), Cseries.ID, 1);
      Wires.append(WI);

      if (mapIT.hasNext())
        mapIT.next();
      WI.setParams(Lseries.ID, 0, mapIT.key(), mapIT.value());
      Wires.append(WI);
    }

    posx += 250;
  } else {
    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);

    if (mapIT.hasNext())
      mapIT.next();
    WI.setParams(NI.ID, 0, mapIT.key(), mapIT.value());
    Wires.append(WI);
    posx += 50;
  }

  UnconnectedComponents.clear();
  if (flip) {
    UnconnectedComponents[NI.ID] = 0;
  } else {
    UnconnectedComponents[Lseries.ID] = 1;
    UnconnectedComponents[Cseries.ID] = 0;
  }

  if (((Specification.EllipticType == "Type B") ||
       (Specification.EllipticType == "Type C")) &&
      (j == Specification.order - 1))
    Ni--;
}
