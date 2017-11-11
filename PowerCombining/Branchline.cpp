#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::Branchline()
{
    struct ComponentInfo TermSpar1, TermSpar2, TermSpar3, Ground;
    struct ComponentInfo Riso;
    struct ComponentInfo TL1, TL2, TL3, TL4;
    struct WireInfo WI;

    double K = Specs.OutputRatio;
    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);
    double ZA = Specs.Z0*sqrt(K/(K+1));
    double ZB = Specs.Z0*sqrt(K);

    TermSpar1.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar1.Type = Term;
    TermSpar1.Orientation = vertical;
    TermSpar1.parameter = 0;
    TermSpar1.val.clear();
    TermSpar1.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar1.Coordinates.clear();
    TermSpar1.Coordinates.push_back(0);
    TermSpar1.Coordinates.push_back(-50);
    Components.append(TermSpar1);
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    TermSpar2.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar2.Type = Term;
    TermSpar2.Orientation = horizontal;
    TermSpar2.parameter = 0;
    TermSpar2.val.clear();
    TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar2.Coordinates.clear();
    TermSpar2.Coordinates.push_back(100);
    TermSpar2.Coordinates.push_back(-50);
    Components.append(TermSpar2);
    QucsNetlist += QString("Pac:P2 N1 gnd Num=2 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    TermSpar3.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar3.Type = Term;
    TermSpar3.Orientation = horizontal;
    TermSpar3.parameter = 0;
    TermSpar3.val.clear();
    TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar3.Coordinates.clear();
    TermSpar3.Coordinates.push_back(100);
    TermSpar3.Coordinates.push_back(50);
    Components.append(TermSpar3);
    QucsNetlist += QString("Pac:P3 N2 gnd Num=3 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    Riso.ID=QString("R%1").arg(++NumberComponents[Resistor]);
    Riso.Type = Resistor;
    Riso.Orientation = vertical;
    Riso.parameter = 0;
    Riso.val.clear();
    Riso.val["R"] = num2str(Specs.Z0, Resistance);
    Riso.Coordinates.clear();
    Riso.Coordinates.push_back(0);
    Riso.Coordinates.push_back(75);
    Components.append(Riso);
    QucsNetlist+=QString("R:R%1 N3 gnd R=\"%2\"\n").arg(NumberComponents[Resistor]).arg(Riso.val["R"]);

    Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
    Ground.Type = GND;
    Ground.Orientation = vertical;
    Ground.parameter = 0;
    Ground.Coordinates.clear();
    Ground.Coordinates.push_back(0);
    Ground.Coordinates.push_back(120);
    Ground.val.clear();
    Components.append(Ground);


    TL1.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL1.Type = TransmissionLine;
    TL1.Orientation = horizontal;
    TL1.parameter = 0;
    TL1.val.clear();
    TL1.val["Z"] = num2str(ZA, Resistance);
    TL1.val["L"] = ConvertLengthFromM(lambda4);
    TL1.Coordinates.clear();
    TL1.Coordinates.push_back(50);
    TL1.Coordinates.push_back(-50);
    Components.append(TL1);
    QucsNetlist+=QString("TLIN:Line%1 N0 N1 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL1.val["Z"])
            .arg(TL1.val["L"])
            .arg(Specs.alpha);

    TL2.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL2.Type = TransmissionLine;
    TL2.Orientation = horizontal;
    TL2.parameter = 0;
    TL2.val.clear();
    TL2.val["Z"] = num2str(ZA, Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    TL2.Coordinates.clear();
    TL2.Coordinates.push_back(50);
    TL2.Coordinates.push_back(50);
    Components.append(TL2);
    QucsNetlist+=QString("TLIN:Line%1 N3 N2 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL2.val["Z"])
            .arg(TL2.val["L"])
            .arg(Specs.alpha);

    TL3.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL3.Type = TransmissionLine;
    TL3.Orientation = vertical;
    TL3.parameter = 0;
    TL3.val.clear();
    TL3.val["Z"] = num2str(ZB, Resistance);
    TL3.val["L"] = ConvertLengthFromM(lambda4);
    TL3.Coordinates.clear();
    TL3.Coordinates.push_back(0);
    TL3.Coordinates.push_back(0);
    Components.append(TL3);
    QucsNetlist+=QString("TLIN:Line%1 N0 N3 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL3.val["Z"])
            .arg(TL3.val["L"])
            .arg(Specs.alpha);

    TL4.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL4.Type = TransmissionLine;
    TL4.Orientation = vertical;
    TL4.parameter = 0;
    TL4.val.clear();
    TL4.val["Z"] = num2str(ZB, Resistance);
    TL4.val["L"] = ConvertLengthFromM(lambda4);
    TL4.Coordinates.clear();
    TL4.Coordinates.push_back(100);
    TL4.Coordinates.push_back(0);
    Components.append(TL4);
    QucsNetlist+=QString("TLIN:Line%1 N1 N2 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL4.val["Z"])
            .arg(TL4.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TermSpar1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TermSpar2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL1.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = TermSpar2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL4.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = TermSpar3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL4.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TermSpar3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL2.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = TermSpar1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL3.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    WI.OriginID = Riso.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = TL2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Riso.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = TL3.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = Riso.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Ground.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,1]")] = QPen(Qt::red, 1, Qt::DashLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,2]")] = QPen(Qt::black, 1, Qt::DotLine);
}
