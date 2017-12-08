#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::Gysel()
{
    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);
    double lambda2=lambda4*2;

    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 0, 0, "N0", "gnd");
    TermSpar1.val["Z"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar1);

    ComponentInfo TL1(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, -50, "N0", "N1");
    TL1.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL1.val["Z"] = num2str(sqrt(2)*Specs.Z0, Resistance);
    TL1.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL1);

    WireInfo WI(TL1.ID, 0, TermSpar1.ID, 0);
    Wires.append(WI);

    ComponentInfo TL2(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, 50, "N0", "N4");
    TL2.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL2.val["Z"] = num2str(sqrt(2)*Specs.Z0, Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL2);

    WI.setParams(TL2.ID, 1, TermSpar1.ID, 0);
    Wires.append(WI);

    ComponentInfo TL3(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 100, -75, "N1", "N2");
    TL3.val["Z"] = num2str(Specs.Z0, Resistance);
    TL3.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL3);

    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 0, -75, "N1", "gnd");
    TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar2);

    WI.setParams(TL1.ID, 1, TermSpar2.ID, 0);
    Wires.append(WI);

    WI.setParams(TL3.ID, 0, TermSpar2.ID, 0);
    Wires.append(WI);

    ComponentInfo TL4(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 100, 75, "N3", "N4");
    TL4.val["Z"] = num2str(Specs.Z0, Resistance);
    TL4.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL4);

    ComponentInfo TermSpar3(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 0, 75, "N4", "gnd");
    TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar3);

    WI.setParams(TL2.ID, 0, TermSpar3.ID, 0);
    Wires.append(WI);

    WI.setParams(TL4.ID, 0, TermSpar3.ID, 0);
    Wires.append(WI);


    //Node
    NodeInfo N1(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 150, -75);
    Nodes.append(N1);

    ComponentInfo TL5(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 150, 0, "N2", "N3");
    TL5.val["Z"] = num2str(Specs.Z0/sqrt(2), Resistance);
    TL5.val["L"] = ConvertLengthFromM(lambda2);
    Components.append(TL5);

    //Node
    NodeInfo N2 (QString("N%1").arg(++NumberComponents[ConnectionNodes]), 150, 75);
    Nodes.append(N2);

    ComponentInfo Ri1(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 200, 100, "N3", "gnd");
    Ri1.val["R"] = num2str(Specs.Z0, Resistance);
    Components.append(Ri1);

    ComponentInfo Ground1(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 200, 125, "", "");
    Components.append(Ground1);

    WI.setParams(Ri1.ID, 0, Ground1.ID, 0);
    Wires.append(WI);

    WI.setParams(Ri1.ID, 1, N2.ID, 0);
    Wires.append(WI);

    WI.setParams(TL5.ID, 0, N2.ID, 0);
    Wires.append(WI);

    WI.setParams(TL4.ID, 1, N2.ID, 0);
    Wires.append(WI);

    WI.setParams(TL5.ID, 1, TL3.ID, 1);
    Wires.append(WI);

    ComponentInfo Ri2(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 200, -50, "N2", "gnd");
    Ri2.val["R"] = num2str(Specs.Z0, Resistance);
    Components.append(Ri2);

    ComponentInfo Ground2(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 200, 0, "", "");
    Components.append(Ground2);

    WI.setParams(Ri2.ID, 0, Ground2.ID, 0);
    Wires.append(WI);

    WI.setParams(Ri2.ID, 1, N1.ID, 0);
    Wires.append(WI);

    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,1]")] = QPen(Qt::red, 1, Qt::DashLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,2]")] = QPen(Qt::black, 1, Qt::DotLine);

}
