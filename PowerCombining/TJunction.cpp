#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::TJunction() {
  // Design equations
  double K = Specs.OutputRatio.at(0) * Specs.OutputRatio.at(0);
  int index_t2 = 2, index_t3 = 3;
  double lambda4 = SPEED_OF_LIGHT / (4 * Specs.freq);

  ComponentInfo TL4, TL5; // Auxiliar lines for matching in case of K!=1

  ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term,
                          180, 0, 0, "N0", "gnd");
  TermSpar1.val["Z0"] = num2str(Specs.Z0, Resistance);
  Components.append(TermSpar1);

  ComponentInfo TL1(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                    TransmissionLine, 90, 25, 0, "N0", "N1");
  TL1.val["Z0"] = num2str(Specs.Z0, Resistance);
  TL1.val["Length"] = ConvertLengthFromM(lambda4);
  Components.append(TL1);

  WireInfo WI(TermSpar1.ID, 0, TL1.ID, 0);
  Wires.append(WI);

  ComponentInfo TL2(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                    TransmissionLine, 90, 100, -50, "N1", "N2");
  TL2.val["Z0"] = num2str(Specs.Z0 * (K + 1), Resistance);
  TL2.val["Length"] = ConvertLengthFromM(lambda4);
  Components.append(TL2);

  WI.setParams(TL2.ID, 0, TL1.ID, 1);
  Wires.append(WI);

  ComponentInfo TL3(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                    TransmissionLine, 90, 100, 50, "N1", "N3");
  TL3.val["Z0"] = num2str(Specs.Z0 * (K + 1) / K, Resistance);
  TL3.val["Length"] = ConvertLengthFromM(lambda4);
  Components.append(TL3);

  WI.setParams(TL3.ID, 0, TL1.ID, 1);
  Wires.append(WI);

  int xpos_term = 150;

  if (K != 1) { // Requires matching

    TL4.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                  TransmissionLine, 90, 175, -50, "N2", "N4");
    TL4.val["Z0"] =
        num2str(sqrt(2 * Specs.Z0 * Specs.Z0 * (K + 1)), Resistance);
    TL4.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL4);

    WI.setParams(TL4.ID, 0, TL2.ID, 1);
    Wires.append(WI);

    TL5.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                  TransmissionLine, 90, 175, 50, "N3", "N5");
    TL5.val["Z0"] =
        num2str(sqrt(2 * Specs.Z0 * Specs.Z0 * (K + 1) / K), Resistance);
    TL5.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL5);

    WI.setParams(TL5.ID, 0, TL3.ID, 1);
    Wires.append(WI);

    xpos_term += 25;
    index_t2 = 5;
    index_t3 = 4;
  }

  ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                          xpos_term + 25, -50, QString("N%1").arg(index_t2),
                          "gnd");
  TermSpar2.val["Z0"] = num2str(Specs.Z0, Resistance);
  Components.append(TermSpar2);

  QString dst;
  (index_t2 == 5) ? dst = TL4.ID : dst = TL2.ID;
  WI.setParams(TermSpar2.ID, 0, dst, 1);
  Wires.append(WI);

  ComponentInfo TermSpar3(QString("T%1").arg(++NumberComponents[Term]), Term, 0,
                          xpos_term + 25, 50, QString("N%1").arg(index_t3),
                          "gnd");

  TermSpar3.val["Z0"] = num2str(Specs.Z0, Resistance);
  Components.append(TermSpar3);

  (index_t2 == 5) ? dst = TL5.ID : dst = TL3.ID;
  WI.setParams(TermSpar3.ID, 0, dst, 1);
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
