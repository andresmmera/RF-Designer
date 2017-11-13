#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::Gysel()
{
    struct ComponentInfo TermSpar, Riso, Ground;
    struct ComponentInfo TL1, TL2, TL3, TL4, TL5;
    struct WireInfo WI;
    NodeInfo N1,N2;

    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);
    double lambda2=lambda4*2;
    double Zbranch = 2*Specs.Z0/sqrt(Specs.Noutputs);

    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(0);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);


    TL1.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL1.Type = TransmissionLine;
    TL1.Orientation = vertical;
    TL1.parameter = 0;
    TL1.val.clear();
    TL1.val["Z"] = num2str(sqrt(2)*Specs.Z0, Resistance);
    TL1.val["L"] = ConvertLengthFromM(lambda4);
    TL1.Coordinates.clear();
    TL1.Coordinates.push_back(0);
    TL1.Coordinates.push_back(-50);
    Components.append(TL1);
    QucsNetlist+=QString("TLIN:Line%1 N0 N1 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL1.val["Z"])
            .arg(TL1.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TL2.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL2.Type = TransmissionLine;
    TL2.Orientation = vertical;
    TL2.parameter = 0;
    TL2.val.clear();
    TL2.val["Z"] = num2str(sqrt(2)*Specs.Z0, Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    TL2.Coordinates.clear();
    TL2.Coordinates.push_back(0);
    TL2.Coordinates.push_back(50);
    Components.append(TL2);
    QucsNetlist+=QString("TLIN:Line%1 N0 N4 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL2.val["Z"])
            .arg(TL2.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL2.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL3.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL3.Type = TransmissionLine;
    TL3.Orientation = horizontal;
    TL3.parameter = 0;
    TL3.val.clear();
    TL3.val["Z"] = num2str(Specs.Z0, Resistance);
    TL3.val["L"] = ConvertLengthFromM(lambda4);
    TL3.Coordinates.clear();
    TL3.Coordinates.push_back(100);
    TL3.Coordinates.push_back(-75);
    Components.append(TL3);
    QucsNetlist+=QString("TLIN:Line%1 N1 N2 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL3.val["Z"])
            .arg(TL3.val["L"])
            .arg(Specs.alpha);


    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(0);
    TermSpar.Coordinates.push_back(-75);
    Components.append(TermSpar);
    QucsNetlist += QString("Pac:P2 N1 gnd Num=2 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = TL1.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    TL4.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL4.Type = TransmissionLine;
    TL4.Orientation = horizontal;
    TL4.parameter = 0;
    TL4.val.clear();
    TL4.val["Z"] = num2str(Specs.Z0, Resistance);
    TL4.val["L"] = ConvertLengthFromM(lambda4);
    TL4.Coordinates.clear();
    TL4.Coordinates.push_back(100);
    TL4.Coordinates.push_back(75);
    Components.append(TL4);
    QucsNetlist+=QString("TLIN:Line%1 N4 N3 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL4.val["Z"])
            .arg(TL4.val["L"])
            .arg(Specs.alpha);


    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(0);
    TermSpar.Coordinates.push_back(75);
    Components.append(TermSpar);
    QucsNetlist += QString("Pac:P3 N4 gnd Num=3 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);


    WI.OriginID = TL2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL4.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    //Node
    N1.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N1.Coordinates.clear();
    N1.Coordinates.push_back(150);
    N1.Coordinates.push_back(-75);
    Nodes.append(N1);


    TL5.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL5.Type = TransmissionLine;
    TL5.Orientation = vertical;
    TL5.parameter = 0;
    TL5.val.clear();
    TL5.val["Z"] = num2str(Specs.Z0/sqrt(2), Resistance);
    TL5.val["L"] = ConvertLengthFromM(lambda2);
    TL5.Coordinates.clear();
    TL5.Coordinates.push_back(150);
    TL5.Coordinates.push_back(0);
    Components.append(TL5);
    QucsNetlist+=QString("TLIN:Line%1 N2 N3 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL5.val["Z"])
            .arg(TL5.val["L"])
            .arg(Specs.alpha);


    //Node
    N2.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    N2.Coordinates.clear();
    N2.Coordinates.push_back(150);
    N2.Coordinates.push_back(75);
    Nodes.append(N2);


    Riso.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Riso.Type = Resistor;
    Riso.Orientation = vertical;
    Riso.parameter = 0;
    Riso.val.clear();
    Riso.val["R"] = num2str(Specs.Z0, Resistance);
    Riso.Coordinates.clear();
    Riso.Coordinates.push_back(200);
    Riso.Coordinates.push_back(100);
    Components.append(Riso);
    QucsNetlist+=QString("R:R%1 N3 gnd R=\"%2\"\n").arg(NumberComponents[Resistor]).arg(Riso.val["R"]);

    Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
    Ground.Type = GND;
    Ground.Orientation = vertical;
    Ground.parameter = 0;
    Ground.Coordinates.clear();
    Ground.Coordinates.push_back(200);
    Ground.Coordinates.push_back(125);
    Ground.val.clear();
    Components.append(Ground);


    WI.OriginID = Riso.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Ground.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = Riso.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = TL5.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = TL4.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N2.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = TL5.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = TL3.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    Riso.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Riso.Type = Resistor;
    Riso.Orientation = vertical;
    Riso.parameter = 0;
    Riso.val.clear();
    Riso.val["R"] = num2str(Specs.Z0, Resistance);
    Riso.Coordinates.clear();
    Riso.Coordinates.push_back(200);
    Riso.Coordinates.push_back(-50);
    Components.append(Riso);
    QucsNetlist+=QString("R:R%1 N2 gnd R=\"%2\"\n").arg(NumberComponents[Resistor]).arg(Riso.val["R"]);

    Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
    Ground.Type = GND;
    Ground.Orientation = vertical;
    Ground.parameter = 0;
    Ground.Coordinates.clear();
    Ground.Coordinates.push_back(200);
    Ground.Coordinates.push_back(0);
    Ground.val.clear();
    Components.append(Ground);


    WI.OriginID = Riso.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Ground.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = Riso.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = N1.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,1]")] = QPen(Qt::red, 1, Qt::DashLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,2]")] = QPen(Qt::black, 1, Qt::DotLine);

}
