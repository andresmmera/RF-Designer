#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::TJunction()
{
    struct ComponentInfo TermSpar;
    struct ComponentInfo TL1, TL2, TL3, TL4, TL5;
    struct WireInfo WI;
    double K = Specs.OutputRatio*Specs.OutputRatio;
    int index_t2=2, index_t3=3;
    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);
    int posx = 0;

    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    posx += 25;
    TL1.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL1.Type = TransmissionLine;
    TL1.Orientation = horizontal;
    TL1.parameter = 0;
    TL1.val.clear();
    TL1.val["Z"] = num2str(Specs.Z0, Resistance);
    TL1.val["L"] = ConvertLengthFromM(lambda4);
    TL1.Coordinates.clear();
    TL1.Coordinates.push_back(posx);
    TL1.Coordinates.push_back(0);
    Components.append(TL1);
    QucsNetlist+=QString("TLIN:Line%1 N0 N1 Z=\"%2\" L=\"%3 m\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL1.val["Z"])
            .arg(TL1.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TermSpar.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL1.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


    posx += 75;
    TL2.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL2.Type = TransmissionLine;
    TL2.Orientation = horizontal;
    TL2.parameter = 0;
    TL2.val.clear();
    TL2.val["Z"] = num2str(Specs.Z0*(K+1), Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    TL2.Coordinates.clear();
    TL2.Coordinates.push_back(posx);
    TL2.Coordinates.push_back(-50);
    Components.append(TL2);
    QucsNetlist+=QString("TLIN:Line%1 N1 N2 Z=\"%2\" L=\"%3 m\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL2.val["Z"])
            .arg(TL2.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL2.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL1.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    TL3.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL3.Type = TransmissionLine;
    TL3.Orientation = horizontal;
    TL3.parameter = 0;
    TL3.val.clear();
    TL3.val["Z"] = num2str(Specs.Z0*(K+1)/K, Resistance);
    TL3.val["L"] = ConvertLengthFromM(lambda4);
    TL3.Coordinates.clear();
    TL3.Coordinates.push_back(posx);
    TL3.Coordinates.push_back(50);
    Components.append(TL3);
    QucsNetlist+=QString("TLIN:Line%1 N1 N3 Z=\"%2\" L=\"%3 m\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL3.val["Z"])
            .arg(TL3.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL3.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL1.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    posx += 50;

    if (K!=1)
    {//Requires matching

        posx+=25;
        TL4.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
        TL4.Type = TransmissionLine;
        TL4.Orientation = horizontal;
        TL4.parameter = 0;
        TL4.val.clear();
        TL4.val["Z"] = num2str(sqrt(2*Specs.Z0*Specs.Z0*(K+1)), Resistance);
        TL4.val["L"] = ConvertLengthFromM(lambda4);
        TL4.Coordinates.clear();
        TL4.Coordinates.push_back(posx);
        TL4.Coordinates.push_back(-50);
        Components.append(TL4);
        QucsNetlist+=QString("TLIN:Line%1 N3 N5 Z=\"%2\" L=\"%3 m\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                .arg(NumberComponents[TransmissionLine])
                .arg(TL4.val["Z"])
                .arg(TL4.val["L"])
                .arg(Specs.alpha);

        WI.OriginID = TL4.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = TL2.ID;
        WI.PortDestination = 1;
        Wires.append(WI);


        TL5.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
        TL5.Type = TransmissionLine;
        TL5.Orientation = horizontal;
        TL5.parameter = 0;
        TL5.val.clear();
        TL5.val["Z"] = num2str(sqrt(2*Specs.Z0*Specs.Z0*(K+1)/K), Resistance);
        TL5.val["L"] = ConvertLengthFromM(lambda4);
        TL5.Coordinates.clear();
        TL5.Coordinates.push_back(posx);
        TL5.Coordinates.push_back(50);
        Components.append(TL5);
        QucsNetlist+=QString("TLIN:Line%1 N2 N4 Z=\"%2\" L=\"%3 m\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                .arg(NumberComponents[TransmissionLine])
                .arg(TL5.val["Z"])
                .arg(TL5.val["L"])
                .arg(Specs.alpha);

        WI.OriginID = TL5.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = TL3.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        posx+=25;
        index_t2 = 5;
        index_t3 = 4;

    }

    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = horizontal;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(-50);
    Components.append(TermSpar);
    QucsNetlist += QString("Pac:P2 N%1 gnd Num=2 Z=\"%2 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n")
            .arg(index_t2)
            .arg(Specs.Z0);

    WI.OriginID = TermSpar.ID;
    WI.PortOrigin = 0;
    (index_t2 == 5)? WI.DestinationID =TL4.ID : WI.DestinationID = TL2.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = horizontal;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(50);
    Components.append(TermSpar);
    QucsNetlist += QString("Pac:P3 N%1 gnd Num=3 Z=\"%2 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n")
            .arg(index_t3)
            .arg(Specs.Z0);

    WI.OriginID = TermSpar.ID;
    WI.PortOrigin = 0;
    (index_t2 == 5)? WI.DestinationID =TL5.ID : WI.DestinationID = TL3.ID;
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
