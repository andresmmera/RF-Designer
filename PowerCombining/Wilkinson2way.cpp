#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::Wilkinson()
{
    TwoWayWilkinsonParams WilkinsonParams = CalculateWilkinson();
    if (Specs.Implementation == "Lumped LC")
    {//This implementation is based on the quarter wavelength transmission line Pi LC equivalent
        double Z4, Z5, L2, L2_, C2, C2_, L3, L3_, C3, C3_, CC, w, K;
        int posx = 50;
        K = Specs.OutputRatio;
        w = 2*M_PI*Specs.freq;
        L2 = WilkinsonParams.Z2/w;
        C2 = 1./(L2*w*w);
        L3 = WilkinsonParams.Z3/w;
        C3 = 1./(L3*w*w);
        CC = C2+C3;
        if (WilkinsonParams.R2 != WilkinsonParams.R3)//Unequal output power rate => requires matching to Z0
        {
           Z4 = Specs.Z0*sqrt(K);
           Z5 = Specs.Z0/sqrt(K);
           L2_ = Z4/w;
           L3_ = Z5/w;
           C2_ = 1./(L2_*w*w);
           C3_ = 1./(L3_*w*w);
           //Embed the first capacitor of the Pi quarter wave equivalent in the last C of the Wilkinson structure
           C2 += C2_;
           C3 += C3_;
        }

        //Build the circuit and the netlist

        struct ComponentInfo TermSpar;
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

        posx += 50;
        //Shunt capacitor
        struct ComponentInfo Cshunt;
        Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
        Cshunt.Type = Capacitor;
        Cshunt.Orientation = vertical;
        Cshunt.parameter = 0;
        Cshunt.val.clear();
        Cshunt.val["C"] = num2str(CC, Capacitance);
        Cshunt.Coordinates.clear();
        Cshunt.Coordinates.push_back(posx);
        Cshunt.Coordinates.push_back(20);
        Components.append(Cshunt);
        QucsNetlist+=QString("C:C%1 N0 gnd C=\"%2\"\n").arg(NumberComponents[Capacitor]).arg(Cshunt.val["C"]);

        //Wires
        //***** Term to capacitor *****
        struct WireInfo WI;
        WI.OriginID = TermSpar.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Cshunt.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        //GND
        struct ComponentInfo Ground;
        Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
        Ground.Type = GND;
        Ground.Orientation = vertical;
        Ground.parameter = 0;
        Ground.Coordinates.clear();
        Ground.Coordinates.push_back(posx);
        Ground.Coordinates.push_back(65);
        Ground.val.clear();
        Components.append(Ground);

        //Shunt capacitor to ground
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ground.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        posx += 50;

        //Node
        NodeInfo NI;
        NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        NI.Coordinates.clear();
        NI.Coordinates.push_back(posx);
        NI.Coordinates.push_back(0);
        Nodes.append(NI);

        //Shunt capacitor to node
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = NI.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        posx += 50;

        //Upper branch
        //Series inductor
        struct ComponentInfo Lseries;
        Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
        Lseries.Type = Inductor;
        Lseries.Orientation = horizontal;
        Lseries.parameter = 0;
        Lseries.val.clear();
        Lseries.val["L"] = num2str(L2, Inductance);
        Lseries.Coordinates.clear();
        Lseries.Coordinates.push_back(posx);
        Lseries.Coordinates.push_back(-75);
        Components.append(Lseries);
        QucsNetlist+=QString("L:L%1 N0 N1 L=\"%2\"\n").arg(NumberComponents[Inductor]).arg(Lseries.val["L"]);
        posx += 50;

        //Cshunt
        Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
        Cshunt.Type = Capacitor;
        Cshunt.Orientation = vertical;
        Cshunt.parameter = 0;
        Cshunt.val.clear();
        Cshunt.val["C"] = num2str(C2, Capacitance);
        Cshunt.Coordinates.clear();
        Cshunt.Coordinates.push_back(posx);
        Cshunt.Coordinates.push_back(-55);
        Components.append(Cshunt);
        QucsNetlist+=QString("C:C%1 N1 gnd C=\"%2\"\n").arg(NumberComponents[Capacitor]).arg(Cshunt.val["C"]);

        Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
        Ground.Type = GND;
        Ground.Orientation = vertical;
        Ground.parameter = 0;
        Ground.Coordinates.clear();
        Ground.Coordinates.push_back(posx);
        Ground.Coordinates.push_back(-10);
        Ground.val.clear();
        Components.append(Ground);

        posx += 50;
        NodeInfo UpperNode;
        UpperNode.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        UpperNode.Coordinates.clear();
        UpperNode.Coordinates.push_back(posx);
        UpperNode.Coordinates.push_back(-75);
        Nodes.append(UpperNode);

        //Node to upper-branch series inductor
        WI.OriginID = NI.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Lseries.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        //Upper-branch series inductor to shunt capacitor
        WI.OriginID = Lseries.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Cshunt.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        //Shunt capacitor to ground
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ground.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        //Shunt capacitor to node
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = UpperNode.ID;
        WI.PortDestination = 1;
        Wires.append(WI);


        //Lower branch
        posx -= 100;

        //Series inductor
        Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
        Lseries.Type = Inductor;
        Lseries.Orientation = horizontal;
        Lseries.parameter = 0;
        Lseries.val.clear();
        Lseries.val["L"] = num2str(L3, Inductance);
        Lseries.Coordinates.clear();
        Lseries.Coordinates.push_back(posx);
        Lseries.Coordinates.push_back(75);
        Components.append(Lseries);
        QucsNetlist+=QString("L:L%1 N0 N2 L=\"%2\"\n").arg(NumberComponents[Inductor]).arg(Lseries.val["L"]);
        posx += 50;

        //Cshunt
        Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
        Cshunt.Type = Capacitor;
        Cshunt.Orientation = vertical;
        Cshunt.parameter = 0;
        Cshunt.val.clear();
        Cshunt.val["C"] = num2str(C3, Capacitance);
        Cshunt.Coordinates.clear();
        Cshunt.Coordinates.push_back(posx);
        Cshunt.Coordinates.push_back(95);
        Components.append(Cshunt);
        QucsNetlist+=QString("C:C%1 N2 gnd C=\"%2\"\n").arg(NumberComponents[Capacitor]).arg(Cshunt.val["C"]);

        Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
        Ground.Type = GND;
        Ground.Orientation = vertical;
        Ground.parameter = 0;
        Ground.Coordinates.clear();
        Ground.Coordinates.push_back(posx);
        Ground.Coordinates.push_back(140);
        Ground.val.clear();
        Components.append(Ground);

        posx += 50;
        NodeInfo LowerNode;
        LowerNode.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        LowerNode.Coordinates.clear();
        LowerNode.Coordinates.push_back(posx);
        LowerNode.Coordinates.push_back(75);
        Nodes.append(LowerNode);

        //Node to lower-branch series inductor
        WI.OriginID = NI.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Lseries.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        //Lower-branch series inductor to shunt capacitor
        WI.OriginID = Lseries.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Cshunt.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        //Shunt capacitor to ground
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ground.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        //Shunt capacitor to node
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = LowerNode.ID;
        WI.PortDestination = 1;
        Wires.append(WI);


        //Isolation resistor
        struct ComponentInfo Risolation;
        Risolation.ID=QString("R%1").arg(++NumberComponents[Resistor]);
        Risolation.Type = Resistor;
        Risolation.Orientation = vertical;
        Risolation.parameter = 0;
        Risolation.val.clear();
        Risolation.val["R"] = num2str(WilkinsonParams.R, Resistance);
        Risolation.Coordinates.clear();
        Risolation.Coordinates.push_back(posx);
        Risolation.Coordinates.push_back(0);
        Components.append(Risolation);
        QucsNetlist+=QString("R:R%1 N1 N2 R=\"%2\"\n").arg(NumberComponents[Resistor]).arg(Risolation.val["R"]);

        //Isolation resistor to nodes
        WI.OriginID = Risolation.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = LowerNode.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        WI.OriginID = Risolation.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = UpperNode.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        if (Specs.OutputRatio != 1)
        {// An unequal power ratio implies that the load impedance != Z0, so it requires matching

          //Upper branch
            posx += 35;
            //Series inductor
            Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lseries.Type = Inductor;
            Lseries.Orientation = horizontal;
            Lseries.parameter = 0;
            Lseries.val.clear();
            Lseries.val["L"] = num2str(L2_, Inductance);
            Lseries.Coordinates.clear();
            Lseries.Coordinates.push_back(posx);
            Lseries.Coordinates.push_back(-75);
            Components.append(Lseries);
            QucsNetlist+=QString("L:L%1 N1 N3 L=\"%2\"\n").arg(NumberComponents[Inductor]).arg(Lseries.val["L"]);

            WI.OriginID = Lseries.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = UpperNode.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            posx += 50;
            //Cshunt
            Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
            Cshunt.Type = Capacitor;
            Cshunt.Orientation = vertical;
            Cshunt.parameter = 0;
            Cshunt.val.clear();
            Cshunt.val["C"] = num2str(C2_, Capacitance);
            Cshunt.Coordinates.clear();
            Cshunt.Coordinates.push_back(posx);
            Cshunt.Coordinates.push_back(-50);
            Components.append(Cshunt);
            QucsNetlist+=QString("C:C%1 N3 gnd C=\"%2\"\n").arg(NumberComponents[Capacitor]).arg(Cshunt.val["C"]);

            Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
            Ground.Type = GND;
            Ground.Orientation = vertical;
            Ground.parameter = 0;
            Ground.Coordinates.clear();
            Ground.Coordinates.push_back(posx);
            Ground.Coordinates.push_back(-10);
            Ground.val.clear();
            Components.append(Ground);

            NodeInfo UpperNode_Matching;
            UpperNode_Matching.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            UpperNode_Matching.Coordinates.clear();
            UpperNode_Matching.Coordinates.push_back(posx);
            UpperNode_Matching.Coordinates.push_back(-75);
            Nodes.append(UpperNode_Matching);

            //Inductor to matching section node
            WI.OriginID = Lseries.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = UpperNode_Matching.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //Capacitor to matching section node
            WI.OriginID = Cshunt.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = UpperNode_Matching.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //Capacitor to ground
            WI.OriginID = Cshunt.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Ground.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            posx += 50;
            TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
            TermSpar.Type = Term;
            TermSpar.Orientation = horizontal;
            TermSpar.parameter = 0;
            TermSpar.val.clear();
            TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
            TermSpar.Coordinates.clear();
            TermSpar.Coordinates.push_back(posx);
            TermSpar.Coordinates.push_back(-75);
            Components.append(TermSpar);
            QucsNetlist += QString("Pac:P2 N3 gnd Num=2 Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(TermSpar.val["Z"]);

            //Node to output term
            WI.OriginID = UpperNode_Matching.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = TermSpar.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            posx -= 100;
            //Lower branch
            Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lseries.Type = Inductor;
            Lseries.Orientation = horizontal;
            Lseries.parameter = 0;
            Lseries.val.clear();
            Lseries.val["L"] = num2str(L3_, Inductance);
            Lseries.Coordinates.clear();
            Lseries.Coordinates.push_back(posx);
            Lseries.Coordinates.push_back(75);
            Components.append(Lseries);
            QucsNetlist+=QString("L:L%1 N2 N4 L=\"%2\"\n").arg(NumberComponents[Inductor]).arg(Lseries.val["L"]);

            WI.OriginID = Lseries.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = LowerNode.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            posx += 50;
            //Cshunt
            Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
            Cshunt.Type = Capacitor;
            Cshunt.Orientation = vertical;
            Cshunt.parameter = 0;
            Cshunt.val.clear();
            Cshunt.val["C"] = num2str(C3_, Capacitance);
            Cshunt.Coordinates.clear();
            Cshunt.Coordinates.push_back(posx);
            Cshunt.Coordinates.push_back(100);
            Components.append(Cshunt);
            QucsNetlist+=QString("C:C%1 N4 gnd C=\"%2\"\n").arg(NumberComponents[Capacitor]).arg(Cshunt.val["C"]);

            Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
            Ground.Type = GND;
            Ground.Orientation = vertical;
            Ground.parameter = 0;
            Ground.Coordinates.clear();
            Ground.Coordinates.push_back(posx);
            Ground.Coordinates.push_back(140);
            Ground.val.clear();
            Components.append(Ground);

            NodeInfo LowerNode_Matching;
            LowerNode_Matching.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            LowerNode_Matching.Coordinates.clear();
            LowerNode_Matching.Coordinates.push_back(posx);
            LowerNode_Matching.Coordinates.push_back(75);
            Nodes.append(LowerNode_Matching);

            //Inductor to matching section node
            WI.OriginID = Lseries.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = LowerNode_Matching.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //Capacitor to matching section node
            WI.OriginID = Cshunt.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = LowerNode_Matching.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //Capacitor to ground
            WI.OriginID = Cshunt.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Ground.ID;
            WI.PortDestination = 1;
            Wires.append(WI);


            posx += 50;
            TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
            TermSpar.Type = Term;
            TermSpar.Orientation = horizontal;
            TermSpar.parameter = 0;
            TermSpar.val.clear();
            TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
            TermSpar.Coordinates.clear();
            TermSpar.Coordinates.push_back(posx);
            TermSpar.Coordinates.push_back(75);
            Components.append(TermSpar);
            QucsNetlist += QString("Pac:P3 N4 gnd Num=3 Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(TermSpar.val["Z"]);

            //Node to output term
            WI.OriginID = LowerNode_Matching.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = TermSpar.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

        }
        else
        {//Just put the output terms
            posx += 35;
            TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
            TermSpar.Type = Term;
            TermSpar.Orientation = horizontal;
            TermSpar.parameter = 0;
            TermSpar.val.clear();
            TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
            TermSpar.Coordinates.clear();
            TermSpar.Coordinates.push_back(posx);
            TermSpar.Coordinates.push_back(-75);
            Components.append(TermSpar);
            QucsNetlist += QString("Pac:P2 N1 gnd Num=2 Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(TermSpar.val["Z"]);

            //Upper branch node to output term
            WI.OriginID = TermSpar.ID;
            WI.PortOrigin = 1;
            WI.DestinationID =  UpperNode.ID;
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
            TermSpar.Coordinates.push_back(75);
            Components.append(TermSpar);
            QucsNetlist += QString("Pac:P3 N2 gnd Num=3 Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(TermSpar.val["Z"]);

            //Upper branch node to output term
            WI.OriginID = TermSpar.ID;
            WI.PortOrigin = 1;
            WI.DestinationID =  LowerNode.ID;
            WI.PortDestination = 1;
            Wires.append(WI);
        }


    }

    if (Specs.Implementation == "Ideal TL")
    {
        int posx = 0;
        double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);
        struct ComponentInfo TermSpar;
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
        QucsNetlist = QString("Pac:P1 N0 gnd Num=\"1\" Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specs.Z0);

        posx += 50;
        //1st transmission line
        struct ComponentInfo TL, TL_Upper, TL_Lower;
        TL.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
        TL.Type = TransmissionLine;
        TL.Orientation = horizontal;
        TL.parameter = 0;
        TL.val.clear();
        TL.val["Z"] = num2str(Specs.Z0, Resistance);
        TL.val["L"] = ConvertLengthFromM(lambda4);
        TL.Coordinates.clear();
        TL.Coordinates.push_back(posx);
        TL.Coordinates.push_back(0);
        Components.append(TL);
        QucsNetlist+=QString("TLIN:Line%1 N0 N1 Z=\"%2\" L=\"%3 m\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                .arg(NumberComponents[TransmissionLine])
                .arg(TL.val["Z"])
                .arg(TL.val["L"])
                .arg(Specs.alpha);

        posx += 25;

        struct WireInfo WI;
        WI.OriginID = TermSpar.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = TL.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        posx += 25;
        //Node
        NodeInfo Ncommon, NUpper, NLower;
        Ncommon.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        Ncommon.Coordinates.clear();
        Ncommon.Coordinates.push_back(posx);
        Ncommon.Coordinates.push_back(0);
        Nodes.append(Ncommon);

        WI.OriginID = TL.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Ncommon.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        posx += 25;
        //Upper branch TL
        //1st transmission line
        TL_Upper.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
        TL_Upper.Type = TransmissionLine;
        TL_Upper.Orientation = horizontal;
        TL_Upper.parameter = 0;
        TL_Upper.val.clear();
        TL_Upper.val["Z"] = num2str(WilkinsonParams.Z2, Resistance);
        TL_Upper.val["L"] = ConvertLengthFromM(lambda4);
        TL_Upper.Coordinates.clear();
        TL_Upper.Coordinates.push_back(posx+15);
        TL_Upper.Coordinates.push_back(-50);
        Components.append(TL_Upper);
        QucsNetlist+=QString("TLIN:Line%1 N1 N2 Z=\"%2\" L=\"%3 m\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                .arg(NumberComponents[TransmissionLine])
                .arg(TL_Upper.val["Z"])
                .arg(TL_Upper.val["L"])
                .arg(Specs.alpha);

        WI.OriginID = TL_Upper.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ncommon.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        NUpper.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        NUpper.Coordinates.clear();
        NUpper.Coordinates.push_back(posx+50);
        NUpper.Coordinates.push_back(-50);
        Nodes.append(NUpper);

        WI.OriginID = TL_Upper.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = NUpper.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        //Lower branch TL
        //1st transmission line
        TL_Lower.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
        TL_Lower.Type = TransmissionLine;
        TL_Lower.Orientation = horizontal;
        TL_Lower.parameter = 0;
        TL_Lower.val.clear();
        TL_Lower.val["Z"] = num2str(WilkinsonParams.Z3, Resistance);
        TL_Lower.val["L"] = ConvertLengthFromM(lambda4);
        TL_Lower.Coordinates.clear();
        TL_Lower.Coordinates.push_back(posx+15);
        TL_Lower.Coordinates.push_back(50);
        Components.append(TL_Lower);
        QucsNetlist+=QString("TLIN:Line%1 N1 N3 Z=\"%2\" L=\"%3 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                .arg(NumberComponents[TransmissionLine])
                .arg(TL_Lower.val["Z"])
                .arg(TL_Lower.val["L"])
                .arg(Specs.alpha);

        WI.OriginID = TL_Lower.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ncommon.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        NLower.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        NLower.Coordinates.clear();
        NLower.Coordinates.push_back(posx+50);
        NLower.Coordinates.push_back(50);
        Nodes.append(NLower);

        WI.OriginID = TL_Lower.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = NLower.ID;
        WI.PortDestination = 1;
        Wires.append(WI);


        posx += 50;
        //Isolation resistor
        struct ComponentInfo Risolation;
        Risolation.ID=QString("R%1").arg(++NumberComponents[Resistor]);
        Risolation.Type = Resistor;
        Risolation.Orientation = vertical;
        Risolation.parameter = 0;
        Risolation.val.clear();
        Risolation.val["R"] = num2str(WilkinsonParams.R, Resistance);
        Risolation.Coordinates.clear();
        Risolation.Coordinates.push_back(posx);
        Risolation.Coordinates.push_back(0);
        Components.append(Risolation);
        QucsNetlist+=QString("R:R%1 N2 N3 R=\"%2\"\n").arg(NumberComponents[Resistor]).arg(Risolation.val["R"]);

        WI.OriginID = Risolation.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = NUpper.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        WI.OriginID = Risolation.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = NLower.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        posx += 50;
        if (Specs.OutputRatio != 1)
        {// An unequal power ratio implies that the load impedance != Z0, so it requires matching

            //Upper branch matching transmission line
            TL.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
            TL.Type = TransmissionLine;
            TL.Orientation = horizontal;
            TL.parameter = 0;
            TL.val.clear();
            TL.val["Z"] = num2str(sqrt(Specs.Z0*WilkinsonParams.R2), Resistance);
            TL.val["L"] = ConvertLengthFromM(lambda4);
            TL.Coordinates.clear();
            TL.Coordinates.push_back(posx);
            TL.Coordinates.push_back(-50);
            Components.append(TL);
            QucsNetlist+=QString("TLIN:Line%1 N2 N4 Z=\"%2\" L=\"%3\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                    .arg(NumberComponents[TransmissionLine])
                    .arg(TL.val["Z"])
                    .arg(TL.val["L"])
                    .arg(Specs.alpha);

            //Upper branch term
            TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
            TermSpar.Type = Term;
            TermSpar.Orientation = horizontal;
            TermSpar.parameter = 0;
            TermSpar.val.clear();
            TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
            TermSpar.Coordinates.clear();
            TermSpar.Coordinates.push_back(posx+50);
            TermSpar.Coordinates.push_back(-50);
            Components.append(TermSpar);
            QucsNetlist += QString("Pac:P2 N4 gnd Num=\"2\" Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(TermSpar.val["Z"]);

            WI.OriginID = TL.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = NUpper.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            WI.OriginID = TL.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = TermSpar.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //Lower branch matching transmission line
            TL.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
            TL.Type = TransmissionLine;
            TL.Orientation = horizontal;
            TL.parameter = 0;
            TL.val.clear();
            TL.val["Z"] = num2str(sqrt(Specs.Z0*WilkinsonParams.R3), Resistance);
            TL.val["L"] = ConvertLengthFromM(lambda4);
            TL.Coordinates.clear();
            TL.Coordinates.push_back(posx);
            TL.Coordinates.push_back(50);
            Components.append(TL);
            QucsNetlist+=QString("TLIN:Line%1 N3 N5 Z=\"%2\" L=\"%3\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                    .arg(NumberComponents[TransmissionLine])
                    .arg(TL.val["Z"])
                    .arg(TL.val["L"])
                    .arg(Specs.alpha);

            //Lower branch term
            TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
            TermSpar.Type = Term;
            TermSpar.Orientation = horizontal;
            TermSpar.parameter = 0;
            TermSpar.val.clear();
            TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
            TermSpar.Coordinates.clear();
            TermSpar.Coordinates.push_back(posx+50);
            TermSpar.Coordinates.push_back(50);
            Components.append(TermSpar);
            QucsNetlist += QString("Pac:P3 N5 gnd Num=\"3\" Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(TermSpar.val["Z"]);

            WI.OriginID = TL.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = NLower.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            WI.OriginID = TL.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = TermSpar.ID;
            WI.PortDestination = 1;
            Wires.append(WI);
        }
        else
        {//Just put the output terms
            //Upper branch term
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
            QucsNetlist += QString("Pac:P2 N2 gnd Num=\"2\" Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(TermSpar.val["Z"]);

            WI.OriginID = TL_Upper.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = TermSpar.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //Lower branch term
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
            QucsNetlist += QString("Pac:P3 N3 gnd Num=\"3\" Z=\"%1\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(TermSpar.val["Z"]);

            WI.OriginID = TL_Lower.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = TermSpar.ID;
            WI.PortDestination = 1;
            Wires.append(WI);


        }

    }

    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,1]")] = QPen(Qt::red, 1, Qt::DashLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);
    displaygraphs[QString("S[3,2]")] = QPen(Qt::black, 1, Qt::DotLine);
}

