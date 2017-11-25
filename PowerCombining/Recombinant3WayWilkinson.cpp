#include "PowerCombining/PowerCombinerDesigner.h"

/*
 * References:
 * [1] "Power combiners, impedance transformers and directional couplers: part II". Andrei Grebennikov. High Frequency Electronics. 2008
 * [2] "A recombinant in-phase power divider", IEEE Trans. Microwave Theory Tech., vol. MTT-39, Aug. 1991, pp. 1438-1440
 */
void PowerCombinerDesigner::Recombinant3WayWilkinson()
{
    struct ComponentInfo TermSpar1, TermSpar2, TermSpar3, TermSpar4, Ground;
    struct ComponentInfo Ri1, Ri2, Ri3;
    struct ComponentInfo TL1, TL2, TL3, TL4, TL5, TL6, TL7;
    struct WireInfo WI;
    NodeInfo N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11;

    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);
    double Z1 = Specs.Z0*0.72;
    double Z2 = Specs.Z0*0.8;
    double Z4 = Specs.Z0*1.6;
    double R1 = Specs.Z0, R2 = Specs.Z0*2;

    TermSpar1.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar1.Type = Term;
    TermSpar1.Orientation = vertical;
    TermSpar1.parameter = 0;
    TermSpar1.val.clear();
    TermSpar1.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar1.Coordinates.clear();
    TermSpar1.Coordinates.push_back(0);
    TermSpar1.Coordinates.push_back(0);
    Components.append(TermSpar1);
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);


    TL1.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL1.Type = TransmissionLine;
    TL1.Orientation = horizontal;
    TL1.parameter = 0;
    TL1.val.clear();
    TL1.val["Z"] = num2str(Z1, Resistance);
    TL1.val["L"] = ConvertLengthFromM(lambda4);
    TL1.Coordinates.clear();
    TL1.Coordinates.push_back(50);
    TL1.Coordinates.push_back(0);
    Components.append(TL1);
    QucsNetlist+=QString("TLIN:Line%1 N0 N1 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL1.val["Z"])
            .arg(TL1.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TermSpar1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    N1.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N1.Coordinates.clear();
    N1.Coordinates.push_back(100);
    N1.Coordinates.push_back(0);
    Nodes.append(N1);

    WI.OriginID = TL1.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL2.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL2.Type = TransmissionLine;
    TL2.Orientation = horizontal;
    TL2.parameter = 0;
    TL2.val.clear();
    TL2.val["Z"] = num2str(Z2, Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    TL2.Coordinates.clear();
    TL2.Coordinates.push_back(125);
    TL2.Coordinates.push_back(-75);
    Components.append(TL2);
    QucsNetlist+=QString("TLIN:Line%1 N1 N2 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL2.val["Z"])
            .arg(TL2.val["L"])
            .arg(Specs.alpha);

    N2.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N2.Coordinates.clear();
    N2.Coordinates.push_back(175);
    N2.Coordinates.push_back(-75);
    Nodes.append(N2);

    WI.OriginID = TL2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL2.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL3.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL3.Type = TransmissionLine;
    TL3.Orientation = horizontal;
    TL3.parameter = 0;
    TL3.val.clear();
    TL3.val["Z"] = num2str(Z2, Resistance);
    TL3.val["L"] = ConvertLengthFromM(lambda4);
    TL3.Coordinates.clear();
    TL3.Coordinates.push_back(125);
    TL3.Coordinates.push_back(75);
    Components.append(TL3);
    QucsNetlist+=QString("TLIN:Line%1 N1 N3 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL3.val["Z"])
            .arg(TL3.val["L"])
            .arg(Specs.alpha);

    N3.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N3.Coordinates.clear();
    N3.Coordinates.push_back(175);
    N3.Coordinates.push_back(75);
    Nodes.append(N3);

    WI.OriginID = TL3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL3.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N3.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    //Isolation resistor
    Ri1.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Ri1.Type = Resistor;
    Ri1.Orientation = vertical;
    Ri1.parameter = 0;
    Ri1.val.clear();
    Ri1.val["R"] = num2str(R1, Resistance);
    Ri1.Coordinates.clear();
    Ri1.Coordinates.push_back(175);
    Ri1.Coordinates.push_back(0);
    Components.append(Ri1);
    QucsNetlist+=QString("R:R%1 N2 N3 R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Ri1.val["R"]);

    WI.OriginID = Ri1.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Ri1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N3.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL4.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL4.Type = TransmissionLine;
    TL4.Orientation = horizontal;
    TL4.parameter = 0;
    TL4.val.clear();
    TL4.val["Z"] = num2str(Z2, Resistance);
    TL4.val["L"] = ConvertLengthFromM(lambda4);
    TL4.Coordinates.clear();
    TL4.Coordinates.push_back(225);
    TL4.Coordinates.push_back(-125);
    Components.append(TL4);
    QucsNetlist+=QString("TLIN:Line%1 N4 N2 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL4.val["Z"])
            .arg(TL4.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL4.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    N4.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N4.Coordinates.clear();
    N4.Coordinates.push_back(275);
    N4.Coordinates.push_back(-125);
    Nodes.append(N4);

    TermSpar2.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar2.Type = Term;
    TermSpar2.Orientation = horizontal;
    TermSpar2.parameter = 0;
    TermSpar2.val.clear();
    TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar2.Coordinates.clear();
    TermSpar2.Coordinates.push_back(300);
    TermSpar2.Coordinates.push_back(-125);
    Components.append(TermSpar2);
    QucsNetlist += QString("Pac:P2 N4 gnd Num=2 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = TL4.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TermSpar2.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL5.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL5.Type = TransmissionLine;
    TL5.Orientation = horizontal;
    TL5.parameter = 0;
    TL5.val.clear();
    TL5.val["Z"] = num2str(Z4, Resistance);
    TL5.val["L"] = ConvertLengthFromM(lambda4);
    TL5.Coordinates.clear();
    TL5.Coordinates.push_back(225);
    TL5.Coordinates.push_back(-50);
    Components.append(TL5);
    QucsNetlist+=QString("TLIN:Line%1 N2 N5 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL5.val["Z"])
            .arg(TL5.val["L"])
            .arg(Specs.alpha);

    N5.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N5.Coordinates.clear();
    N5.Coordinates.push_back(275);
    N5.Coordinates.push_back(-50);
    Nodes.append(N5);

    WI.OriginID = TL5.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL5.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N5.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    //Isolation resistor
    Ri2.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Ri2.Type = Resistor;
    Ri2.Orientation = vertical;
    Ri2.parameter = 0;
    Ri2.val.clear();
    Ri2.val["R"] = num2str(R2, Resistance);
    Ri2.Coordinates.clear();
    Ri2.Coordinates.push_back(275);
    Ri2.Coordinates.push_back(-85);
    Components.append(Ri2);
    QucsNetlist+=QString("R:R%1 N5 N4 R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Ri2.val["R"]);

    WI.OriginID = Ri2.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    WI.OriginID = Ri2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N5.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    N8.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N8.Coordinates.clear();
    N8.Coordinates.push_back(275);
    N8.Coordinates.push_back(0);
    Nodes.append(N8);

    TermSpar3.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar3.Type = Term;
    TermSpar3.Orientation = horizontal;
    TermSpar3.parameter = 0;
    TermSpar3.val.clear();
    TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar3.Coordinates.clear();
    TermSpar3.Coordinates.push_back(300);
    TermSpar3.Coordinates.push_back(0);
    Components.append(TermSpar3);
    QucsNetlist += QString("Pac:P3 N5 gnd Num=3 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = N5.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N8.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TermSpar3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N8.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL6.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL6.Type = TransmissionLine;
    TL6.Orientation = horizontal;
    TL6.parameter = 0;
    TL6.val.clear();
    TL6.val["Z"] = num2str(Z2, Resistance);
    TL6.val["L"] = ConvertLengthFromM(lambda4);
    TL6.Coordinates.clear();
    TL6.Coordinates.push_back(225);
    TL6.Coordinates.push_back(125);
    Components.append(TL6);
    QucsNetlist+=QString("TLIN:Line%1 N6 N3 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL6.val["Z"])
            .arg(TL6.val["L"])
            .arg(Specs.alpha);

    N6.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N6.Coordinates.clear();
    N6.Coordinates.push_back(275);
    N6.Coordinates.push_back(125);
    Nodes.append(N6);

    WI.OriginID = TL6.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N3.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL6.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N6.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL7.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL7.Type = TransmissionLine;
    TL7.Orientation = horizontal;
    TL7.parameter = 0;
    TL7.val.clear();
    TL7.val["Z"] = num2str(Z4, Resistance);
    TL7.val["L"] = ConvertLengthFromM(lambda4);
    TL7.Coordinates.clear();
    TL7.Coordinates.push_back(225);
    TL7.Coordinates.push_back(50);
    Components.append(TL7);
    QucsNetlist+=QString("TLIN:Line%1 N3 N5 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL7.val["Z"])
            .arg(TL7.val["L"])
            .arg(Specs.alpha);

    N7.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N7.Coordinates.clear();
    N7.Coordinates.push_back(275);
    N7.Coordinates.push_back(50);
    Nodes.append(N7);

    WI.OriginID = TL7.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N3.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL7.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N7.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Isolation resistor
    Ri3.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Ri3.Type = Resistor;
    Ri3.Orientation = vertical;
    Ri3.parameter = 0;
    Ri3.val.clear();
    Ri3.val["R"] = num2str(R2, Resistance);
    Ri3.Coordinates.clear();
    Ri3.Coordinates.push_back(275);
    Ri3.Coordinates.push_back(85);
    Components.append(Ri3);
    QucsNetlist+=QString("R:R%1 N6 N5 R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Ri3.val["R"]);


    WI.OriginID = Ri3.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N7.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Ri3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N6.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = N7.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N8.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TermSpar4.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar4.Type = Term;
    TermSpar4.Orientation = horizontal;
    TermSpar4.parameter = 0;
    TermSpar4.val.clear();
    TermSpar4.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar4.Coordinates.clear();
    TermSpar4.Coordinates.push_back(300);
    TermSpar4.Coordinates.push_back(125);
    Components.append(TermSpar4);
    QucsNetlist += QString("Pac:P4 N6 gnd Num=4 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = TermSpar4.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N6.ID;
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
