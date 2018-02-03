#include "PowerCombining/PowerCombinerDesigner.h"

/*
 * References:
 * [1] "Power combiners, impedance transformers and directional couplers: part II". Andrei Grebennikov. High Frequency Electronics. 2008
 * [2] “A New 3-Way Power Divider with Various Output Power Ratios,” J.-S. Lim and S.-Y. Eom, 1996 IEEE MTT-S Int. Microwave Symp. Dig., pp. 785-788."
 */
void PowerCombinerDesigner::Lim_Eom()
{
    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);

    double M = Specs.OutputRatio.at(0);
    double N = Specs.OutputRatio.at(1);
    double K = Specs.OutputRatio.at(2);

    double d1 = M+N+K;
    double d2 = N+K;
    double Z1 = Specs.Z0*sqrt(d1/d2);
    double Z2 = Specs.Z0*sqrt(d1/M);

    double Z4 = Specs.Z0*sqrt(d2/N);
    double Z5 = Specs.Z0*sqrt(d2/K);

    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, -25, -100, "N0", "gnd");
    TermSpar1.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar1);

    NodeInfo N0 (QString("N%1").arg(++NumberComponents[ConnectionNodes]), 0, -100);
    Nodes.append(N0);

    WireInfo WI(TermSpar1.ID, 0, N0.ID, 0);
    Wires.append(WI);

    ComponentInfo TL1(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 50, -100, "N0", "N1");
    TL1.val["Z0"] = num2str(Z2, Resistance);
    TL1.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL1);

    WI.setParams(TL1.ID, 0, N0.ID, 0);
    Wires.append(WI);

    //Node
    NodeInfo N1 (QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, -100);
    Nodes.append(N1);

    WI.setParams(TL1.ID, 1, N1.ID, 0);
    Wires.append(WI);

    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 100, -130, "N1", "gnd");
    TermSpar2.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar2);

    WI.setParams(TermSpar2.ID, 0, N1.ID, 0);
    Wires.append(WI);

    ComponentInfo TL2(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 150, -100, "N1", "N2");
    TL2.val["Z0"] = num2str(Z1, Resistance);
    TL2.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL2);

    WI.setParams(TL2.ID, 0, N1.ID, 0);
    Wires.append(WI);

    //Node
    NodeInfo N2(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 200, -100);
    Nodes.append(N2);

    WI.setParams(TL2.ID, 1, N2.ID, 0);
    Wires.append(WI);


    //Isolation resistor
    ComponentInfo Ri1(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 260, -80, "N2", "gnd");
    Ri1.val["R"] = num2str(Specs.Z0, Resistance);
    Components.append(Ri1);

    ComponentInfo Ground1(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 260, -40, "", "");
    Components.append(Ground1);

    WI.setParams(Ri1.ID, 1, N2.ID, 0);
    Wires.append(WI);

    WI.setParams(Ri1.ID, 0, Ground1.ID, 0);
    Wires.append(WI);

    ComponentInfo TL3(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 200, -50, "N2", "N3");
    TL3.val["Z0"] = num2str(Z2, Resistance);
    TL3.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL3);

    WI.setParams(TL3.ID, 1, N2.ID, 0);
    Wires.append(WI);

    //Node
    NodeInfo N3(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 200, 0);
    Nodes.append(N3);

    WI.setParams(TL3.ID, 0, N3.ID, 0);
    Wires.append(WI);

    ComponentInfo TL4(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 200, 50, "N3", "N4");
    TL4.val["Z0"] = num2str(Z4, Resistance);
    TL4.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL4);

    WI.setParams(TL4.ID, 1, N3.ID, 0);
    Wires.append(WI);

    //Node
    NodeInfo N4(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 200, 100);
    Nodes.append(N4);

    WI.setParams(TL4.ID, 0, N4.ID, 0);
    Wires.append(WI);

    ComponentInfo TermSpar3(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 220, 100, "N4", "gnd");
    TermSpar3.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar3);

    WI.setParams(TermSpar3.ID, 1, N4.ID, 0);
    Wires.append(WI);

    ComponentInfo TL5(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 150, 100, "N4", "N5");
    TL5.val["Z0"] = num2str(Z5, Resistance);
    TL5.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL5);

    WI.setParams(TL5.ID, 1, N4.ID, 0);
    Wires.append(WI);

    //Node
    NodeInfo N5(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, 100);
    Nodes.append(N5);

    WI.setParams(TL5.ID, 0, N5.ID, 0);
    Wires.append(WI);

    //Isolation resistor
    ComponentInfo Ri2(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 100, 150, "N5", "gnd");
    Ri2.val["R"] = num2str(Specs.Z0, Resistance);
    Components.append(Ri2);

    ComponentInfo Ground2(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 100, 200, "", "");
    Components.append(Ground2);

    WI.setParams(Ri2.ID, 1, N5.ID, 0);
    Wires.append(WI);

    WI.setParams(Ri2.ID, 0, Ground2.ID, 0);
    Wires.append(WI);

    ComponentInfo TL6(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 50, 100, "N5", "N6");
    TL6.val["Z0"] = num2str(Z4, Resistance);
    TL6.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL6);

    WI.setParams(TL6.ID, 1, N5.ID, 0);
    Wires.append(WI);

    //Node
    NodeInfo N6 (QString("N%1").arg(++NumberComponents[ConnectionNodes]), 0, 100);
    Nodes.append(N6);

    WI.setParams(TL6.ID, 0, N6.ID, 0);
    Wires.append(WI);

    ComponentInfo TermSpar4(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, -20, 100, "N6", "gnd");
    TermSpar4.val["Z0"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar4);

    WI.setParams(TermSpar4.ID, 0, N6.ID, 0);
    Wires.append(WI);

    ComponentInfo TL7(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, 50, "N7", "N6");
    TL7.val["Z0"] = num2str(Z5, Resistance);
    TL7.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL7);

    WI.setParams(TL7.ID, 0, N6.ID, 0);
    Wires.append(WI);

    //Node
    NodeInfo N7(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 0, 0);
    Nodes.append(N7);

    WI.setParams(TL7.ID, 1, N7.ID, 0);
    Wires.append(WI);

    ComponentInfo TL8(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, -50, "N7", "N0");
    TL8.val["Z0"] = num2str(Z1, Resistance);
    TL8.val["Length"] = ConvertLengthFromM(lambda4);
    Components.append(TL8);

    WI.setParams(TL8.ID, 0, N7.ID, 0);
    Wires.append(WI);

    WI.setParams(TL8.ID, 1, N0.ID, 0);
    Wires.append(WI);

    ComponentInfo TL9(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 100, 0, "N7", "N3");
    TL9.val["Z0"] = num2str(Specs.Z0, Resistance);
    TL9.val["Length"] = ConvertLengthFromM(2*lambda4);
    Components.append(TL9);

    WI.setParams(TL9.ID, 0, N7.ID, 0);
    Wires.append(WI);

    WI.setParams(TL9.ID, 1, N3.ID, 0);
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
