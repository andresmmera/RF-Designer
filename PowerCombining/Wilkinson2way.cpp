#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::Wilkinson()
{
    TwoWayWilkinsonParams WilkinsonParams = CalculateWilkinson();
    double lambda4=SPEED_OF_LIGHT/(4*Specs.freq);
    ComponentInfo TermSpar1, TermSpar2, TermSpar3;
    ComponentInfo Ground;
    WireInfo WI;
    if (Specs.Implementation == "Lumped LC")
    {//This implementation is based on the quarter wavelength transmission line Pi LC equivalent
        //Design equations
        double Z4, Z5, L2_, C2_, L3_, C3_;
        double K = Specs.OutputRatio.at(0);
        double w = 2*M_PI*Specs.freq;
        double L2 = WilkinsonParams.Z2/w;
        double C2 = 1./(L2*w*w);
        double L3 = WilkinsonParams.Z3/w;
        double C3 = 1./(L3*w*w);
        double CC = C2+C3;
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
        TermSpar1.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 50, 0, "N0", "gnd");
        TermSpar1.val["Z"] = num2str(Specs.Z0, Resistance);
        Components.append(TermSpar1);

        //Shunt capacitor
        ComponentInfo Cshunt1(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                             100, 20, "N0", "gnd");
        Cshunt1.val["C"] = num2str(CC, Capacitance);
        Components.append(Cshunt1);

        WI.setParams(TermSpar1.ID, 0, Cshunt1.ID, 1);//Term to capacitor
        Wires.append(WI);

        Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 100, 65, "", "");
        Components.append(Ground);

        WI.setParams(Cshunt1.ID, 0, Ground.ID, 0);
        Wires.append(WI);

        NodeInfo N1(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, 0);
        Nodes.append(N1);

        WI.setParams(Cshunt1.ID, 1, N1.ID, 0);
        Wires.append(WI);

        //Upper branch
        //Series inductor
        ComponentInfo Lseries1(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                               150, -75, "N0", "N1");
        Lseries1.val["L"] = num2str(L2, Inductance);
        Components.append(Lseries1);

        //Cshunt
        ComponentInfo Cshunt2(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                              200, -55, "N1", "gnd");
        Cshunt2.val["C"] = num2str(C2, Capacitance);
        Components.append(Cshunt2);

        Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 200, -10, "", "");
        Components.append(Ground);

        NodeInfo N2(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 250, -75);
        Nodes.append(N2);

        WI.setParams(N1.ID, 0, Lseries1.ID, 0);
        Wires.append(WI);

        WI.setParams(Lseries1.ID, 1, Cshunt2.ID, 1);
        Wires.append(WI);

        WI.setParams(Cshunt2.ID, 0, Ground.ID, 0);
        Wires.append(WI);

        WI.setParams(Cshunt2.ID, 1, N2.ID, 0);
        Wires.append(WI);


        //Lower branch
        //Series inductor
        ComponentInfo Lseries2(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                               150, 75, "N0", "N2");
        Lseries2.val["L"] = num2str(L3, Inductance);
        Components.append(Lseries2);


        //Cshunt
        ComponentInfo Cshunt3(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                              200, 95, "N2", "gnd");
        Cshunt3.val["C"] = num2str(C3, Capacitance);
        Components.append(Cshunt3);

        Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 200, 130, "", "");
        Components.append(Ground);

        NodeInfo N3(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 250, 75);
        Nodes.append(N3);

        WI.setParams(N1.ID, 0, Lseries2.ID, 0);
        Wires.append(WI);

        WI.setParams(Lseries2.ID, 1, Cshunt3.ID, 1);
        Wires.append(WI);

        WI.setParams(Cshunt3.ID, 0, Ground.ID, 0);
        Wires.append(WI);

        WI.setParams(Cshunt3.ID, 1, N3.ID, 0);
        Wires.append(WI);


        ComponentInfo Risolation(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 250, 0, "N1", "N2");
        Risolation.val["R"] = num2str(WilkinsonParams.R, Resistance);
        Components.append(Risolation);

        WI.setParams(Risolation.ID, 0, N3.ID, 1);
        Wires.append(WI);

        WI.setParams(Risolation.ID, 1, N2.ID, 0);
        Wires.append(WI);

        ComponentInfo Ls3, Ls4, Cp4, Cp5;
        if (Specs.OutputRatio.at(0) != 1)
        {// An unequal power ratio implies that the load impedance != Z0, so it requires matching

            //Upper branch
            Ls3.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal, 285, -75, "N1", "N3");
            Ls3.val["L"] = num2str(L2_, Inductance);
            Components.append(Ls3);

            WI.setParams(Ls3.ID, 0, N2.ID, 0);
            Wires.append(WI);

            //Cshunt
            Cp4.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical, 320, -50, "N3", "gnd");
            Cp4.val["C"] = num2str(C2_, Capacitance);
            Components.append(Cp4);

            Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 320, -10, "", "");
            Components.append(Ground);

            NodeInfo N4(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 320, -75);
            Nodes.append(N4);

            WI.setParams(Ls3.ID, 1, N4.ID, 0);
            Wires.append(WI);

            WI.setParams(Cp4.ID, 1, N4.ID, 0);
            Wires.append(WI);

            WI.setParams(Cp4.ID, 0, Ground.ID, 0);
            Wires.append(WI);

            TermSpar2.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 370, -50, "N3", "gnd");
            TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
            Components.append(TermSpar2);

            WI.setParams(N4.ID, 0, TermSpar2.ID, 0);
            Wires.append(WI);

            //Lower branch
            Ls4.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal, 285, 75, "N2", "N4");
            Ls4.val["L"] = num2str(L3_, Inductance);
            Components.append(Ls4);

            WI.setParams(Ls4.ID, 0, N3.ID, 0);
            Wires.append(WI);

            //Cshunt
            Cp5.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical, 320, 100, "N4", "gnd");
            Cp5.val["C"] = num2str(C3_, Capacitance);
            Components.append(Cp5);

            Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 320, 140, "", "");
            Components.append(Ground);

            NodeInfo N5(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 320, 75);
            Nodes.append(N5);

            WI.setParams(Ls4.ID, 1, N5.ID, 0);
            Wires.append(WI);

            WI.setParams(Cp5.ID, 1, N5.ID, 0);
            Wires.append(WI);

            WI.setParams(Cp5.ID, 0, Ground.ID, 0);
            Wires.append(WI);

            TermSpar3.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 370, 75, "N4", "gnd");
            TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
            Components.append(TermSpar3);

            WI.setParams(N5.ID, 0, TermSpar3.ID, 0);
            Wires.append(WI);
        }
        else
        {//Just put the output terms
            TermSpar2.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 280, -75, "N1", "gnd");
            TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
            Components.append(TermSpar2);

            WI.setParams(TermSpar2.ID, 0, N2.ID, 0);
            Wires.append(WI);

            TermSpar3.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 280, 75, "N2", "gnd");
            TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
            Components.append(TermSpar3);


            WI.setParams(TermSpar3.ID, 0, N3.ID, 0);
            Wires.append(WI);
        }
    }

    if (Specs.Implementation == "Ideal TL")
    {
        TermSpar1.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, 0, 0, "N0", "gnd");
        TermSpar1.val["Z"] = num2str(Specs.Z0, Resistance);
        Components.append(TermSpar1);

        //1st transmission line
        ComponentInfo TL1(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine,
                          horizontal, 50, 0, "N0", "N1");
        TL1.val["Z"] = num2str(Specs.Z0, Resistance);
        TL1.val["L"] = ConvertLengthFromM(lambda4);
        Components.append(TL1);

        WireInfo WI(TermSpar1.ID, 0, TL1.ID, 0);
        Wires.append(WI);

        //Node
        NodeInfo N1(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 100, 0);
        Nodes.append(N1);

        WI.setParams(TL1.ID, 1, N1.ID, 1);
        Wires.append(WI);

        //Upper branch TL
        //1st transmission line
        ComponentInfo TL2(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine,
                          horizontal, 135, -50, "N1", "N2");
        TL2.val["Z"] = num2str(WilkinsonParams.Z2, Resistance);
        TL2.val["L"] = ConvertLengthFromM(lambda4);
        Components.append(TL2);

        WI.setParams(TL2.ID, 0, N1.ID, 0);
        Wires.append(WI);

        NodeInfo N2(QString("N%1").arg(++NumberComponents[ConnectionNodes]), 200, -50);
        Nodes.append(N2);

        WI.setParams(TL2.ID, 1, N2.ID, 0);
        Wires.append(WI);

        //Lower branch TL
        //1st transmission line
        ComponentInfo TL3(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine,
                          horizontal, 135, 50, "N1", "N3");
        TL3.val["Z"] = num2str(WilkinsonParams.Z3, Resistance);
        TL3.val["L"] = ConvertLengthFromM(lambda4);
        Components.append(TL3);

        WI.setParams(TL3.ID, 0, N1.ID, 0);
        Wires.append(WI);

        NodeInfo N3(QString("N%1").arg(++NumberComponents[ConnectionNodes]),200,50);
        Nodes.append(N3);

        WI.setParams(TL3.ID, 1, N3.ID, 0);
        Wires.append(WI);

        //Isolation resistor
        ComponentInfo Riso(QString("R%1").arg(++NumberComponents[Resistor]), Resistor, vertical, 200, 0, "N2", "N3");
        Riso.val["R"] = num2str(WilkinsonParams.R, Resistance);
        Components.append(Riso);

        WI.setParams(Riso.ID, 1, N2.ID, 0);
        Wires.append(WI);

        WI.setParams(Riso.ID, 0, N3.ID, 0);
        Wires.append(WI);

        ComponentInfo TL4, TL5;//Auxiliar lines for matching in case of K!=1
        if (Specs.OutputRatio.at(0) != 1)
        {// An unequal power ratio implies that the load impedance != Z0, so it requires matching

            //Upper branch matching transmission line
            ComponentInfo TL4(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine,
                              horizontal, 275, -50, "N2", "N4");
            TL4.val["Z"] = num2str(sqrt(Specs.Z0*WilkinsonParams.R2), Resistance);
            TL4.val["L"] = ConvertLengthFromM(lambda4);
            Components.append(TL4);

            //Upper branch term
            TermSpar2.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 325, -50, "N4", "gnd");
            TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
            Components.append(TermSpar2);

            WI.setParams(TL4.ID, 0, N2.ID, 0);
            Wires.append(WI);

            WI.setParams(TL4.ID, 1, TermSpar2.ID, 0);
            Wires.append(WI);

            //Lower branch matching transmission line
            ComponentInfo TL5(QString("TLIN%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine,
                              horizontal, 275, 50, "N3", "N5");
            TL5.val["Z"] = num2str(sqrt(Specs.Z0*WilkinsonParams.R3), Resistance);
            TL5.val["L"] = ConvertLengthFromM(lambda4);
            Components.append(TL5);


            //Lower branch term
            TermSpar3.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 325, 50, "N5", "gnd");
            TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
            Components.append(TermSpar3);

            WI.setParams(TL5.ID, 0, N3.ID, 0);
            Wires.append(WI);

            WI.setParams(TL5.ID, 1, TermSpar3.ID, 0);
            Wires.append(WI);
        }
        else
        {//Just put the output terms
            //Upper branch term
            TermSpar2.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 250, -50, "N2", "gnd");
            TermSpar2.val["Z"] = num2str(Specs.Z0, Resistance);
            Components.append(TermSpar2);

            WI.setParams(TL2.ID, 1, TermSpar2.ID, 0);
            Wires.append(WI);

            //Lower branch term
            TermSpar3.setParams(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, 250, 50, "N3", "gnd");
            TermSpar3.val["Z"] = num2str(Specs.Z0, Resistance);
            Components.append(TermSpar3);

            WI.setParams(TL3.ID, 1, TermSpar3.ID, 0);
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

