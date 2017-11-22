#include "PowerCombining/PowerCombinerDesigner.h"

/*
 * References:
 * [1] "Power combiners, impedance transformers and directional couplers: part II". Andrei Grebennikov. High Frequency Electronics. 2008
 * [2] “A New 3-Way Power Divider with Various Output Power Ratios,” J.-S. Lim and S.-Y. Eom, 1996 IEEE MTT-S Int. Microwave Symp. Dig., pp. 785-788."
 */
void PowerCombinerDesigner::Lim_Eom()
{
    struct ComponentInfo TermSpar1, TermSpar2, TermSpar3, TermSpar4, Ground;
    struct ComponentInfo Risolation;
    struct ComponentInfo TL1, TL2, TL3, TL4, TL5, TL6, TL7, TL8, TL9;
    struct WireInfo WI;
    NodeInfo N0, N1, N2, N3, N4, N5, N6, N7;

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

    TermSpar1.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar1.Type = Term;
    TermSpar1.Orientation = vertical;
    TermSpar1.parameter = 0;
    TermSpar1.val.clear();
    TermSpar1.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar1.Coordinates.clear();
    TermSpar1.Coordinates.push_back(-20);
    TermSpar1.Coordinates.push_back(-100);
    Components.append(TermSpar1);
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    N0.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N0.Coordinates.clear();
    N0.Coordinates.push_back(0);
    N0.Coordinates.push_back(-100);
    Nodes.append(N0);

    WI.OriginID = TermSpar1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N0.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL1.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL1.Type = TransmissionLine;
    TL1.Orientation = horizontal;
    TL1.parameter = 0;
    TL1.val.clear();
    TL1.val["Z"] = num2str(Z2, Resistance);
    TL1.val["L"] = ConvertLengthFromM(lambda4);
    TL1.Coordinates.clear();
    TL1.Coordinates.push_back(50);
    TL1.Coordinates.push_back(-100);
    Components.append(TL1);
    QucsNetlist+=QString("TLIN:Line%1 N0 N1 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL1.val["Z"])
            .arg(TL1.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N0.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Node
    N1.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N1.Coordinates.clear();
    N1.Coordinates.push_back(100);
    N1.Coordinates.push_back(-100);
    Nodes.append(N1);

    WI.OriginID = TL1.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TermSpar2.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar2.Type = Term;
    TermSpar2.Orientation = vertical;
    TermSpar2.parameter = 0;
    TermSpar2.val.clear();
    TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar2.Coordinates.clear();
    TermSpar2.Coordinates.push_back(100);
    TermSpar2.Coordinates.push_back(-130);
    Components.append(TermSpar2);
    QucsNetlist += QString("Pac:P2 N1 gnd Num=2 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = TermSpar2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL2.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL2.Type = TransmissionLine;
    TL2.Orientation = horizontal;
    TL2.parameter = 0;
    TL2.val.clear();
    TL2.val["Z"] = num2str(Z1, Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    TL2.Coordinates.clear();
    TL2.Coordinates.push_back(150);
    TL2.Coordinates.push_back(-100);
    Components.append(TL2);
    QucsNetlist+=QString("TLIN:Line%1 N1 N2 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL2.val["Z"])
            .arg(TL2.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    //Node
    N2.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N2.Coordinates.clear();
    N2.Coordinates.push_back(200);
    N2.Coordinates.push_back(-100);
    Nodes.append(N2);

    WI.OriginID = TL2.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    //Isolation resistor
    Risolation.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Risolation.Type = Resistor;
    Risolation.Orientation = vertical;
    Risolation.parameter = 0;
    Risolation.val.clear();
    Risolation.val["R"] = num2str(Specs.Z0, Resistance);
    Risolation.Coordinates.clear();
    Risolation.Coordinates.push_back(260);
    Risolation.Coordinates.push_back(-80);
    Components.append(Risolation);
    QucsNetlist+=QString("R:R%1 N2 gnd R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Risolation.val["R"]);


    Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
    Ground.Type = GND;
    Ground.Orientation = vertical;
    Ground.parameter = 0;
    Ground.Coordinates.clear();
    Ground.Coordinates.push_back(260);
    Ground.Coordinates.push_back(-40);
    Ground.val.clear();
    Components.append(Ground);

    WI.OriginID = Risolation.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Risolation.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Ground.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL3.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL3.Type = TransmissionLine;
    TL3.Orientation = vertical;
    TL3.parameter = 0;
    TL3.val.clear();
    TL3.val["Z"] = num2str(Z2, Resistance);
    TL3.val["L"] = ConvertLengthFromM(lambda4);
    TL3.Coordinates.clear();
    TL3.Coordinates.push_back(200);
    TL3.Coordinates.push_back(-50);
    Components.append(TL3);
    QucsNetlist+=QString("TLIN:Line%1 N3 N2 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL3.val["Z"])
            .arg(TL3.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL3.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Node
    N3.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N3.Coordinates.clear();
    N3.Coordinates.push_back(200);
    N3.Coordinates.push_back(0);
    Nodes.append(N3);

    WI.OriginID = TL3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N3.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL4.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL4.Type = TransmissionLine;
    TL4.Orientation = vertical;
    TL4.parameter = 0;
    TL4.val.clear();
    TL4.val["Z"] = num2str(Z4, Resistance);
    TL4.val["L"] = ConvertLengthFromM(lambda4);
    TL4.Coordinates.clear();
    TL4.Coordinates.push_back(200);
    TL4.Coordinates.push_back(50);
    Components.append(TL4);
    QucsNetlist+=QString("TLIN:Line%1 N3 N4 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL4.val["Z"])
            .arg(TL4.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL4.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N3.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Node
    N4.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N4.Coordinates.clear();
    N4.Coordinates.push_back(200);
    N4.Coordinates.push_back(100);
    Nodes.append(N4);

    WI.OriginID = TL4.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TermSpar3.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar3.Type = Term;
    TermSpar3.Orientation = horizontal;
    TermSpar3.parameter = 0;
    TermSpar3.val.clear();
    TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar3.Coordinates.clear();
    TermSpar3.Coordinates.push_back(220);
    TermSpar3.Coordinates.push_back(100);
    Components.append(TermSpar3);
    QucsNetlist += QString("Pac:P3 N4 gnd Num=3 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = TermSpar3.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL5.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL5.Type = TransmissionLine;
    TL5.Orientation = horizontal;
    TL5.parameter = 0;
    TL5.val.clear();
    TL5.val["Z"] = num2str(Z5, Resistance);
    TL5.val["L"] = ConvertLengthFromM(lambda4);
    TL5.Coordinates.clear();
    TL5.Coordinates.push_back(150);
    TL5.Coordinates.push_back(100);
    Components.append(TL5);
    QucsNetlist+=QString("TLIN:Line%1 N5 N4 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL5.val["Z"])
            .arg(TL5.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL5.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Node
    N5.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N5.Coordinates.clear();
    N5.Coordinates.push_back(100);
    N5.Coordinates.push_back(100);
    Nodes.append(N5);

    WI.OriginID = TL5.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N5.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Isolation resistor
    Risolation.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Risolation.Type = Resistor;
    Risolation.Orientation = vertical;
    Risolation.parameter = 0;
    Risolation.val.clear();
    Risolation.val["R"] = num2str(Specs.Z0, Resistance);
    Risolation.Coordinates.clear();
    Risolation.Coordinates.push_back(100);
    Risolation.Coordinates.push_back(150);
    Components.append(Risolation);
    QucsNetlist+=QString("R:R%1 N5 gnd R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Risolation.val["R"]);

    Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
    Ground.Type = GND;
    Ground.Orientation = vertical;
    Ground.parameter = 0;
    Ground.Coordinates.clear();
    Ground.Coordinates.push_back(100);
    Ground.Coordinates.push_back(200);
    Ground.val.clear();
    Components.append(Ground);

    WI.OriginID = Risolation.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N5.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Risolation.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Ground.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL6.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL6.Type = TransmissionLine;
    TL6.Orientation = horizontal;
    TL6.parameter = 0;
    TL6.val.clear();
    TL6.val["Z"] = num2str(Z4, Resistance);
    TL6.val["L"] = ConvertLengthFromM(lambda4);
    TL6.Coordinates.clear();
    TL6.Coordinates.push_back(50);
    TL6.Coordinates.push_back(100);
    Components.append(TL6);
    QucsNetlist+=QString("TLIN:Line%1 N6 N5 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL6.val["Z"])
            .arg(TL6.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL6.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N5.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Node
    N6.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N6.Coordinates.clear();
    N6.Coordinates.push_back(0);
    N6.Coordinates.push_back(100);
    Nodes.append(N6);

    WI.OriginID = TL6.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N6.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TermSpar4.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar4.Type = Term;
    TermSpar4.Orientation = vertical;
    TermSpar4.parameter = 0;
    TermSpar4.val.clear();
    TermSpar4.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar4.Coordinates.clear();
    TermSpar4.Coordinates.push_back(-20);
    TermSpar4.Coordinates.push_back(100);
    Components.append(TermSpar4);
    QucsNetlist += QString("Pac:P4 N6 gnd Num=4 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = TermSpar4.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N6.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL7.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL7.Type = TransmissionLine;
    TL7.Orientation = vertical;
    TL7.parameter = 0;
    TL7.val.clear();
    TL7.val["Z"] = num2str(Z5, Resistance);
    TL7.val["L"] = ConvertLengthFromM(lambda4);
    TL7.Coordinates.clear();
    TL7.Coordinates.push_back(0);
    TL7.Coordinates.push_back(50);
    Components.append(TL7);
    QucsNetlist+=QString("TLIN:Line%1 N7 N6 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL7.val["Z"])
            .arg(TL7.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL7.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N6
            .ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Node
    N7.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N7.Coordinates.clear();
    N7.Coordinates.push_back(0);
    N7.Coordinates.push_back(0);
    Nodes.append(N7);

    WI.OriginID = TL7.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N7.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL8.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL8.Type = TransmissionLine;
    TL8.Orientation = vertical;
    TL8.parameter = 0;
    TL8.val.clear();
    TL8.val["Z"] = num2str(Z1, Resistance);
    TL8.val["L"] = ConvertLengthFromM(lambda4);
    TL8.Coordinates.clear();
    TL8.Coordinates.push_back(0);
    TL8.Coordinates.push_back(-50);
    Components.append(TL8);
    QucsNetlist+=QString("TLIN:Line%1 N7 N0 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL8.val["Z"])
            .arg(TL8.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL8.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N7.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL8.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N0.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL9.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL9.Type = TransmissionLine;
    TL9.Orientation = horizontal;
    TL9.parameter = 0;
    TL9.val.clear();
    TL9.val["Z"] = num2str(Specs.Z0, Resistance);
    TL9.val["L"] = ConvertLengthFromM(2*lambda4);
    TL9.Coordinates.clear();
    TL9.Coordinates.push_back(100);
    TL9.Coordinates.push_back(0);
    Components.append(TL9);
    QucsNetlist+=QString("TLIN:Line%1 N7 N3 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL9.val["Z"])
            .arg(TL9.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL9.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N7.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL9.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N3.ID;
    WI.PortDestination = 0;
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
