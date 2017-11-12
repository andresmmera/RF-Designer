#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::Bagley()
{
    struct ComponentInfo TermSpar, Ground;
    struct ComponentInfo TL1, TL2, TL;
    struct WireInfo WI;

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
    TermSpar.Coordinates.push_back((Specs.Noutputs-1)*50);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);


    TL1.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL1.Type = TransmissionLine;
    TL1.Orientation = vertical;
    TL1.parameter = 0;
    TL1.val.clear();
    TL1.val["Z"] = num2str(Zbranch, Resistance);
    TL1.val["L"] = ConvertLengthFromM(lambda4);
    TL1.Coordinates.clear();
    TL1.Coordinates.push_back((Specs.Noutputs-1)*100);
    TL1.Coordinates.push_back(50);
    Components.append(TL1);
    QucsNetlist+=QString("TLIN:Line%1 N0 N%2 Z=\"%3\" L=\"%4 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(Specs.Noutputs)
            .arg(TL1.val["Z"])
            .arg(TL1.val["L"])
            .arg(Specs.alpha);

    TL2.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
    TL2.Type = TransmissionLine;
    TL2.Orientation = vertical;
    TL2.parameter = 0;
    TL2.val.clear();
    TL2.val["Z"] = num2str(Zbranch, Resistance);
    TL2.val["L"] = ConvertLengthFromM(lambda4);
    TL2.Coordinates.clear();
    TL2.Coordinates.push_back(0);
    TL2.Coordinates.push_back(50);
    Components.append(TL2);
    QucsNetlist+=QString("TLIN:Line%1 N0 N1 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
            .arg(NumberComponents[TransmissionLine])
            .arg(TL2.val["Z"])
            .arg(TL2.val["L"])
            .arg(Specs.alpha);

    WI.OriginID = TL1.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    WI.OriginID = TL2.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(0);
    TermSpar.Coordinates.push_back(100);
    Components.append(TermSpar);
    QucsNetlist += QString("Pac:P2 N1 gnd Num=2 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

    WI.OriginID = TermSpar.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TL2.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    int posx = -50;
    for (int i = 1; i < Specs.Noutputs; i++)
    {
        posx+=100;
        TL.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
        TL.Type = TransmissionLine;
        TL.Orientation = horizontal;
        TL.parameter = 0;
        TL.val.clear();
        TL.val["Z"] = num2str(Zbranch, Resistance);
        TL.val["L"] = ConvertLengthFromM(lambda2);
        TL.Coordinates.clear();
        TL.Coordinates.push_back(posx);
        TL.Coordinates.push_back(100);
        Components.append(TL);
        QucsNetlist+=QString("TLIN:Line%1 N%2 N%3 Z=\"%4\" L=\"%5 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                .arg(NumberComponents[TransmissionLine])
                .arg(i)
                .arg(i+1)
                .arg(TL.val["Z"])
                .arg(TL.val["L"])
                .arg(Specs.alpha);

        WI.OriginID = TermSpar.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = TL.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
        TermSpar.Type = Term;
        TermSpar.Orientation = vertical;
        TermSpar.parameter = 0;
        TermSpar.val.clear();
        TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
        TermSpar.Coordinates.clear();
        TermSpar.Coordinates.push_back(posx + 50);
        TermSpar.Coordinates.push_back(100);
        Components.append(TermSpar);
        QucsNetlist += QString("Pac:P%1 N%2 gnd Num=%1 Z=\"%3 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n")
                .arg(NumberComponents[Term])
                .arg(i+1)
                .arg(Specs.Z0);

        WI.OriginID = TermSpar.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = TL.ID;
        WI.PortDestination = 1;
        Wires.append(WI);
    }

    WI.OriginID = TL1.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();

    //Input matching
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
    //Forward transmission graphs
    for (int i = 2; i <= Specs.Noutputs; i++)
    {
        displaygraphs[QString("S[%1,1]").arg(i)] = QPen(Qt::red, 1, Qt::DashLine);
    }

    //Isolation between consecutive output ports
    for (int i = 3; i <= Specs.Noutputs; i++)
    {
       displaygraphs[QString("S[%1,%2]").arg(i-1).arg(i)] = QPen(Qt::black, 1, Qt::DotLine);
    }


}
