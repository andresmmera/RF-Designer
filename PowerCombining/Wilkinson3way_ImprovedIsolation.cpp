#include "PowerCombining/PowerCombinerDesigner.h"

/*
 * References:
 * [1] "Power combiners, impedance transformers and directional couplers: part II". Andrei Grebennikov. High Frequency Electronics. 2008
 * [2] "New 3N way hybrid power dividers", IEEE Trans. Microwave Theory Tech., vol. MTT-25, Dec. 1977, pp. 1008-1012
 */
void PowerCombinerDesigner::Wilkinson3Way_ImprovedIsolation()
{
    double Z1 = 2.28*Specs.Z0;
    double Z2 = 1.316*Specs.Z0;
    double R1 = 1.319*Specs.Z0;
    double R2 = 4*Specs.Z0;

    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);

    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, -25, 0, "N0", "gnd");
    TermSpar1.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar1);

    NodeInfo N0(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 0, 0);
    Nodes.append(N0);

    WireInfo WI(TermSpar1.ID, 0, N0.ID, 0);
    Wires.append(WI);

    ComponentInfo TL1(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 50, 0, "N0", "N1");
    TL1.val["Z0"] = num2str(Z1, Resistance);
    TL1.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL1);

    NodeInfo N1(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, 0);
    Nodes.append(N1);

    WI.setParams(TL1.ID, 0, N0.ID, 0);
    Wires.append(WI);

    WI.setParams(TL1.ID, 1, N1.ID, 0);
    Wires.append(WI);

    ComponentInfo TL2(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 50, -100, "N0", "N2");
    TL2.val["Z0"] = num2str(Z1, Resistance);
    TL2.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL2);

    NodeInfo N2 (QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, -100);
    Nodes.append(N2);

    WI.setParams(TL2.ID, 0, N0.ID, 0);
    Wires.append(WI);

    WI.setParams(TL2.ID, 1, N2.ID, 0);
    Wires.append(WI);


    //Isolation resistor
    ComponentInfo Ri1(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 100, -50, "N1", "N2");
    Ri1.val["R"] = num2str(R1, Resistance);
    Components.append(Ri1);

    WI.setParams(N2.ID, 0, Ri1.ID, 1);
    Wires.append(WI);

    WI.setParams(N1.ID, 0, Ri1.ID, 0);
    Wires.append(WI);

    ComponentInfo TL3(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 50, 100, "N0", "N3");
    TL3.val["Z0"] = num2str(Z1, Resistance);
    TL3.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL3);

    NodeInfo N3(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, 100);
    Nodes.append(N3);

    WI.setParams(TL3.ID, 0, N0.ID, 0);
    Wires.append(WI);

    WI.setParams(TL3.ID, 1, N3.ID, 0);
    Wires.append(WI);


    //Isolation resistor
    ComponentInfo Ri2(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 100, 50, "N1", "N3");
    Ri2.val["R"] = num2str(R1, Resistance);
    Components.append(Ri2);

    WI.setParams(Ri2.ID, 1, N1.ID, 0);
    Wires.append(WI);

    WI.setParams(Ri2.ID, 0, N3.ID, 0);
    Wires.append(WI);

    ComponentInfo TL4(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 150, 0, "N1", "N4");
    TL4.val["Z0"] = num2str(Z2, Resistance);
    TL4.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL4);


    NodeInfo N4(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 200, 0);
    Nodes.append(N4);

    WI.setParams(TL4.ID, 0, N1.ID, 0);
    Wires.append(WI);

    WI.setParams(TL4.ID, 1, N4.ID, 0);
    Wires.append(WI);

    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 225, 0, "N4", "gnd");
    TermSpar2.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar2);

    WI.setParams(TermSpar2.ID, 0, N4.ID, 0);
    Wires.append(WI);

    ComponentInfo TL5(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 150, -100, "N2", "N5");
    TL5.val["Z0"] = num2str(Z2, Resistance);
    TL5.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL5);


    NodeInfo N5(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 200, -100);
    Nodes.append(N5);

    WI.setParams(TL5.ID, 0, N2.ID, 0);
    Wires.append(WI);

    WI.setParams(TL5.ID, 1, N5.ID, 0);
    Wires.append(WI);

    //Isolation resistor
    ComponentInfo Ri3(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 200, -50, "N4", "N5");
    Ri3.val["R"] = num2str(R2, Resistance);
    Components.append(Ri3);

    ComponentInfo TermSpar3(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 225, -100, "N5", "gnd");
    TermSpar3.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar3);

    WI.setParams(Ri3.ID, 1, N5.ID, 0);
    Wires.append(WI);

    WI.setParams(Ri3.ID, 0, N4.ID, 0);
    Wires.append(WI);

    WI.setParams(TermSpar3.ID, 0, N5.ID, 0);
    Wires.append(WI);

    ComponentInfo TL6(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 150, 100, "N3", "N6");
    TL6.val["Z0"] = num2str(Z2, Resistance);
    TL6.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL6);

    NodeInfo N6(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 200, 100);
    Nodes.append(N6);

    WI.setParams(TL6.ID, 0, N3.ID, 0);
    Wires.append(WI);

    WI.setParams(TL6.ID, 1, N6.ID, 0);
    Wires.append(WI);

    //Isolation resistor
    ComponentInfo Ri4(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 200, 50, "N4", "N6");
    Ri4.val["R"] = num2str(R2, Resistance);
    Components.append(Ri4);

    ComponentInfo TermSpar4(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 225, 100, "N6", "gnd");
    TermSpar4.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar4);

    WI.setParams(N6.ID, 0, TermSpar4.ID, 0);
    Wires.append(WI);

    WI.setParams(Ri4.ID, 0, N6.ID, 0);
    Wires.append(WI);

    WI.setParams(Ri4.ID, 1, N4.ID, 0);
    Wires.append(WI);

    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,1]")] = QPen(Qt::red, 1, Qt::DashLine);
    displaygraphs[QString("S[4,1]")] = QPen(Qt::red, 1, Qt::DotLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,2]")] = QPen(Qt::black, 1, Qt::DotLine);
    displaygraphs[QString("S[4,2]")] = QPen(Qt::black, 1, Qt::SolidLine);
    displaygraphs[QString("S[4,3]")] = QPen(Qt::black, 1, Qt::DashLine);


}
