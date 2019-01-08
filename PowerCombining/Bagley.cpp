#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::Bagley() {

  double lambda4 = SPEED_OF_LIGHT / (4 * Specs.freq);
  double lambda2 = lambda4 * 2;
  double Zbranch = 2 * Specs.Z0 / sqrt(Specs.Noutputs);

  ComponentInfo TermSpar(QString("T%1").arg(++NumberComponents[Term]), Term, 90,
                         (Specs.Noutputs - 1) * 50, 0, "N0", "gnd");
  TermSpar.val["Z0"] = num2str(Specs.Z0, Resistance);
  Components.append(TermSpar);

  ComponentInfo TL1(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                    TransmissionLine, 0, (Specs.Noutputs - 1) * 100, 50, "N0",
                    QString("N%1").arg(Specs.Noutputs));
  TL1.val["Z0"] = num2str(Zbranch, Resistance);
  TL1.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
  Components.append(TL1);

  ComponentInfo TL2(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                    TransmissionLine, 0, 0, 50, "N0", "N1");
  TL2.val["Z0"] = num2str(Zbranch, Resistance);
  TL2.val["Length"] = ConvertLengthFromM(Specs.units, lambda4);
  Components.append(TL2);

  WireInfo WI(TL1.ID, 1, TermSpar.ID, 0);
  Wires.append(WI);

  WI.setParams(TL2.ID, 1, TermSpar.ID, 0);
  Wires.append(WI);

  TermSpar.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 90, 0,
                     100, "N1", "gnd");
  TermSpar.val["Z0"] = num2str(Specs.Z0, Resistance);
  Components.append(TermSpar);

  WI.setParams(TermSpar.ID, 0, TL2.ID, 0);
  Wires.append(WI);

  ComponentInfo TL;
  int posx = -50;
  for (int i = 1; i < Specs.Noutputs; i++) {
    posx += 100;
    TL.setParams(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]),
                 TransmissionLine, 90, posx, 100, QString("N%1").arg(i),
                 QString("N%1").arg(i + 1));
    TL.val["Z0"] = num2str(Zbranch, Resistance);
    TL.val["Length"] = ConvertLengthFromM(Specs.units, lambda2);
    Components.append(TL);

    WI.setParams(TermSpar.ID, 0, TL.ID, 0);
    Wires.append(WI);

    TermSpar.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, 90,
                       posx + 50, 100, QString("N%1").arg(i + 1), "gnd");
    TermSpar.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar);

    WI.setParams(TermSpar.ID, 0, TL.ID, 1);
    Wires.append(WI);
  }

  WI.setParams(TL1.ID, 0, TermSpar.ID, 0);
  Wires.append(WI);

  // Ideally, the user should be the one which controls the style of the traces
  // as well the traces to be shown However, in favour of a simpler
  // implementation, it'll be the design code responsible for this... by the
  // moment...
  displaygraphs.clear();

  // Input matching
  displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
  // Forward transmission graphs
  for (int i = 2; i <= Specs.Noutputs; i++) {
    displaygraphs[QString("S[%1,1]").arg(i)] = QPen(Qt::red, 1, Qt::DashLine);
  }

  // Isolation between consecutive output ports
  for (int i = 3; i <= Specs.Noutputs; i++) {
    displaygraphs[QString("S[%1,%2]").arg(i - 1).arg(i)] =
        QPen(Qt::black, 1, Qt::DotLine);
  }
}
