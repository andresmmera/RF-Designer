#include "PowerCombining/PowerCombinerDesigner.h"

/*
 * References:
 * [1] "Power combiners, impedance transformers and directional couplers: part II". Andrei Grebennikov. High Frequency Electronics. 2008
 * [2] "New 3N way hybrid power dividers", IEEE Trans. Microwave Theory Tech., vol. MTT-25, Dec. 1977, pp. 1008-1012
 */
void PowerCombinerDesigner::Wilkinson3Way_ImprovedIsolation()
{
    struct ComponentInfo TermSpar1, TermSpar2, TermSpar3, TermSpar4, Ground;
    struct ComponentInfo Ri1, Ri2, Ri3, Ri4;
    struct ComponentInfo TL1, TL2, TL3, TL4, TL5, TL6;
    struct WireInfo WI;
    NodeInfo N0, N1, N2, N3, N4, N5, N6;

    double Z1 = 2.28*Specs.Z0;
    double Z2 = 1.316*Specs.Z0;
    double R1 = 1.319*Specs.Z0;
    double R2 = 4*Specs.Z0;

    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);

    TermSpar1.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar1.Type = Term;
    TermSpar1.Orientation = vertical;
    TermSpar1.parameter = 0;
    TermSpar1.val.clear();
    TermSpar1.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar1.Coordinates.clear();
    TermSpar1.Coordinates.push_back(-25);
    TermSpar1.Coordinates.push_back(0);
    Components.append(TermSpar1);
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    N0.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N0.Coordinates.clear();
    N0.Coordinates.push_back(0);
    N0.Coordinates.push_back(0);
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

    N1.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N1.Coordinates.clear();
    N1.Coordinates.push_back(100);
    N1.Coordinates.push_back(0);
    Nodes.append(N1);


    WI.OriginID = TL1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N0.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

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
    TL2.val["Z"] = num2str(Z1, Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    TL2.Coordinates.clear();
    TL2.Coordinates.push_back(50);
    TL2.Coordinates.push_back(-100);
    Components.append(TL2);
    QucsNetlist+=QString("TLIN:Line%1 N0 N2 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL2.val["Z"])
            .arg(TL2.val["L"])
            .arg(Specs.alpha);


    N2.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N2.Coordinates.clear();
    N2.Coordinates.push_back(100);
    N2.Coordinates.push_back(-100);
    Nodes.append(N2);

    WI.OriginID = TL2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N0.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    WI.OriginID = TL2.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N2.ID;
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
    Ri1.Coordinates.push_back(100);
    Ri1.Coordinates.push_back(-50);
    Components.append(Ri1);
    QucsNetlist+=QString("R:R%1 N2 N1 R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Ri1.val["R"]);

    WI.OriginID = N2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Ri1.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = N1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Ri1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL3.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL3.Type = TransmissionLine;
    TL3.Orientation = horizontal;
    TL3.parameter = 0;
    TL3.val.clear();
    TL3.val["Z"] = num2str(Z1, Resistance);
    TL3.val["L"] = ConvertLengthFromM(lambda4);
    TL3.Coordinates.clear();
    TL3.Coordinates.push_back(50);
    TL3.Coordinates.push_back(100);
    Components.append(TL3);
    QucsNetlist+=QString("TLIN:Line%1 N0 N3 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL3.val["Z"])
            .arg(TL3.val["L"])
            .arg(Specs.alpha);


    N3.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N3.Coordinates.clear();
    N3.Coordinates.push_back(100);
    N3.Coordinates.push_back(100);
    Nodes.append(N3);

    WI.OriginID = TL3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N0.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL3.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N3.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    //Isolation resistor
    Ri2.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Ri2.Type = Resistor;
    Ri2.Orientation = vertical;
    Ri2.parameter = 0;
    Ri2.val.clear();
    Ri2.val["R"] = num2str(R1, Resistance);
    Ri2.Coordinates.clear();
    Ri2.Coordinates.push_back(100);
    Ri2.Coordinates.push_back(50);
    Components.append(Ri2);
    QucsNetlist+=QString("R:R%1 N3 N1 R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Ri1.val["R"]);


    WI.OriginID = Ri2.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    WI.OriginID = Ri2.ID;
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
    TL4.Coordinates.push_back(150);
    TL4.Coordinates.push_back(0);
    Components.append(TL4);
    QucsNetlist+=QString("TLIN:Line%1 N1 N4 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL4.val["Z"])
            .arg(TL4.val["L"])
            .arg(Specs.alpha);


    N4.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N4.Coordinates.clear();
    N4.Coordinates.push_back(200);
    N4.Coordinates.push_back(0);
    Nodes.append(N4);


    WI.OriginID = TL4.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    WI.OriginID = TL4.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TermSpar2.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar2.Type = Term;
    TermSpar2.Orientation = horizontal;
    TermSpar2.parameter = 0;
    TermSpar2.val.clear();
    TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar2.Coordinates.clear();
    TermSpar2.Coordinates.push_back(225);
    TermSpar2.Coordinates.push_back(0);
    Components.append(TermSpar2);
    QucsNetlist += QString("Pac:P2 N4 gnd Num=2 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = TermSpar2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL5.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL5.Type = TransmissionLine;
    TL5.Orientation = horizontal;
    TL5.parameter = 0;
    TL5.val.clear();
    TL5.val["Z"] = num2str(Z2, Resistance);
    TL5.val["L"] = ConvertLengthFromM(lambda4);
    TL5.Coordinates.clear();
    TL5.Coordinates.push_back(150);
    TL5.Coordinates.push_back(-100);
    Components.append(TL5);
    QucsNetlist+=QString("TLIN:Line%1 N2 N5 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL5.val["Z"])
            .arg(TL5.val["L"])
            .arg(Specs.alpha);


    N5.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N5.Coordinates.clear();
    N5.Coordinates.push_back(200);
    N5.Coordinates.push_back(-100);
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
    Ri3.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Ri3.Type = Resistor;
    Ri3.Orientation = vertical;
    Ri3.parameter = 0;
    Ri3.val.clear();
    Ri3.val["R"] = num2str(R2, Resistance);
    Ri3.Coordinates.clear();
    Ri3.Coordinates.push_back(200);
    Ri3.Coordinates.push_back(-50);
    Components.append(Ri3);
    QucsNetlist+=QString("R:R%1 N4 N5 R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Ri1.val["R"]);

    TermSpar3.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar3.Type = Term;
    TermSpar3.Orientation = horizontal;
    TermSpar3.parameter = 0;
    TermSpar3.val.clear();
    TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar3.Coordinates.clear();
    TermSpar3.Coordinates.push_back(225);
    TermSpar3.Coordinates.push_back(-100);
    Components.append(TermSpar3);
    QucsNetlist+= QString("Pac:P3 N5 gnd Num=3 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);


    WI.OriginID = Ri3.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N5.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Ri3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TermSpar3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N5.ID;
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
    TL6.Coordinates.push_back(150);
    TL6.Coordinates.push_back(100);
    Components.append(TL6);
    QucsNetlist+=QString("TLIN:Line%1 N3 N6 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL6.val["Z"])
            .arg(TL6.val["L"])
            .arg(Specs.alpha);


    N6.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N6.Coordinates.clear();
    N6.Coordinates.push_back(200);
    N6.Coordinates.push_back(100);
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



    //Isolation resistor
    Ri4.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Ri4.Type = Resistor;
    Ri4.Orientation = vertical;
    Ri4.parameter = 0;
    Ri4.val.clear();
    Ri4.val["R"] = num2str(R2, Resistance);
    Ri4.Coordinates.clear();
    Ri4.Coordinates.push_back(200);
    Ri4.Coordinates.push_back(50);
    Components.append(Ri4);
    QucsNetlist+=QString("R:R%1 N4 N6 R=\"%2\"\n")
            .arg(NumberComponents[Resistor])
            .arg(Ri1.val["R"]);

    TermSpar4.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar4.Type = Term;
    TermSpar4.Orientation = horizontal;
    TermSpar4.parameter = 0;
    TermSpar4.val.clear();
    TermSpar4.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar4.Coordinates.clear();
    TermSpar4.Coordinates.push_back(225);
    TermSpar4.Coordinates.push_back(100);
    Components.append(TermSpar4);
    QucsNetlist += QString("Pac:P4 N6 gnd Num=4 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = N6.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TermSpar4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Ri4.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N6.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Ri4.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N4.ID;
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
