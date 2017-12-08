#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::DoubleBoxBranchline()
{

    double K = Specs.OutputRatio.at(0);
    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);
    double r=1;
    double t = sqrt((1+K)*r);
    double ZA = Specs.Z0*sqrt(r*(t*t -r))/(t-r);
    double ZD = Specs.Z0*sqrt(r*(t*t -r))/(t-1);
    double ZB = Specs.Z0*sqrt(r-(r*r)/(t*t));

    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 0, -50, "N0", "gnd");
    TermSpar1.val["Z"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar1);

    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 200, -50, "N2", "gnd");
    TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar2);

    ComponentInfo TermSpar3(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 200, 50, "N3", "gnd");
    TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
    Components.append(TermSpar3);

    ComponentInfo Riso(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 0, 75, "N5", "gnd");
    Riso.val["R"] = num2str(Specs.Z0, Resistance);
    Components.append(Riso);

    ComponentInfo Ground(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 0, 120, "", "");
    Components.append(Ground);

    ComponentInfo TL1(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 50, -50, "N0", "N1");
    TL1.val["Z"] = num2str(ZB, Resistance);
    TL1.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL1);

    //Node
    NodeInfo N1(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, -50);
    Nodes.append(N1);

    ComponentInfo TL2(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 50, 50, "N4", "N5");
    TL2.val["Z"] = num2str(ZB, Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL2);


    //Node
    NodeInfo N2(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, 50);
    Nodes.append(N2);

    ComponentInfo TL3(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, 0, "N0", "N5");
    TL3.val["Z"] = num2str(ZA, Resistance);
    TL3.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL3);

    ComponentInfo TL4(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 100, 0, "N1", "N4");
    TL4.val["Z"] = num2str(ZB, Resistance);
    TL4.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL4);

    ComponentInfo TL5(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 150, -50, "N1", "N2");
    TL5.val["Z"] = num2str(ZB, Resistance);
    TL5.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL5);

    ComponentInfo TL6(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 150, 50, "N3", "N4");
    TL6.val["Z"] = num2str(ZB, Resistance);
    TL6.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL6);

    ComponentInfo TL7(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 200, 0, "N2", "N3");
    TL7.val["Z"] = num2str(ZD, Resistance);
    TL7.val["L"] = ConvertLengthFromM(lambda4);
    Components.append(TL7);

    WireInfo WI(TermSpar1.ID, 0, TL1.ID, 0);
    Wires.append(WI);

    WI.setParams(TermSpar2.ID, 0, TL5.ID, 1);
    Wires.append(WI);

    WI.setParams(TermSpar2.ID, 0, TL7.ID, 1);
    Wires.append(WI);

    WI.setParams(TermSpar3.ID, 0, TL7.ID, 0);
    Wires.append(WI);

    WI.setParams(TermSpar3.ID, 0, TL6.ID, 1);
    Wires.append(WI);

    WI.setParams(TermSpar1.ID, 0, TL3.ID, 1);
    Wires.append(WI);

    WI.setParams(Riso.ID, 1, TL2.ID, 0);
    Wires.append(WI);

    WI.setParams(Riso.ID, 1, TL3.ID, 0);
    Wires.append(WI);

    WI.setParams(Riso.ID, 0, Ground.ID, 0);
    Wires.append(WI);

    //Connect transmission lines to the central nodes

    WI.setParams(TL1.ID, 1, N1.ID, 0);
    Wires.append(WI);

    WI.setParams(TL5.ID, 0, N1.ID, 0);
    Wires.append(WI);

    WI.setParams(TL4.ID, 1, N1.ID, 0);
    Wires.append(WI);

    WI.setParams(TL2.ID, 1, N2.ID, 0);
    Wires.append(WI);

    WI.setParams(TL4.ID, 0, N2.ID, 0);
    Wires.append(WI);

    WI.setParams(TL6.ID, 0, N2.ID, 0);
    Wires.append(WI);

    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,1]")] = QPen(Qt::red, 1, Qt::DashLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,2]")] = QPen(Qt::black, 1, Qt::DotLine);
}

