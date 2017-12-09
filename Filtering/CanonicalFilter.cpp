#include "CanonicalFilter.h"

CanonicalFilter::CanonicalFilter()
{
    //Initialize list of components
    NumberComponents[Capacitor] = 0;
    NumberComponents[Inductor] = 0;
    NumberComponents[Term] = 0;
    NumberComponents[GND] = 0;
    NumberComponents[ConnectionNodes] = 0;
}

CanonicalFilter::CanonicalFilter(FilterSpecifications FS)
{
    Specification = FS;
    //Initialize list of components
    NumberComponents[Capacitor] = 0;
    NumberComponents[Inductor] = 0;
    NumberComponents[Term] = 0;
    NumberComponents[GND] = 0;
    NumberComponents[ConnectionNodes] = 0;
}

CanonicalFilter::~CanonicalFilter()
{

}

QList<ComponentInfo> CanonicalFilter::getComponents()
{
    return Components;
}


QList<WireInfo> CanonicalFilter::getWires()
{
    return Wires;
}

QList<NodeInfo> CanonicalFilter::getNodes()
{
    return Nodes;
}


void CanonicalFilter::synthesize()
{
    LowpassPrototypeCoeffs LP_coeffs(Specification);
    gi = LP_coeffs.getCoefficients();

    if (Specification.FilterResponse == Chebyshev && !Specification.UseZverevTables)
    {//Correct cutoff according to the ripple
        double epsilon = sqrt(pow(10.0, Specification.Ripple/10.0) - 1.0);
        Specification.fc /= cosh(acosh(1.0 / epsilon) / Specification.order);
    }

    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);

    switch (Specification.FilterType)
    {
    case Lowpass:
        SynthesizeLPF();
        break;
    case Highpass:
        SynthesizeHPF();
        break;
    case Bandpass:
        SynthesizeBPF();
        break;
    case Bandstop:
        SynthesizeBSF();
        break;
    }

    //Build Qucs netlist
    QucsNetlist.clear();
    QString codestr;
    for (int i = 0; i< Components.length(); i++)
    {
     codestr = Components[i].getQucs();
     if (!codestr.isEmpty()) QucsNetlist += codestr;
    }
}


//Synthesis of lowpass filters
void CanonicalFilter::SynthesizeLPF()
{
    ComponentInfo Cshunt, Lseries, Ground;
    WireInfo WI;
    NodeInfo NI;

    //Synthesize CLC of LCL network
    int N = Specification.order;//Number of elements
    int posx = 0;
    QString ConnectionAux = "";
    Components.clear();

    //Add Term 1
    double k=Specification.ZS;

    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, posx, 0, "N0", "gnd");
    TermSpar1.val["Z"] = num2str(k, Resistance);
    Components.append(TermSpar1);

    ConnectionAux = TermSpar1.ID;

    unsigned int Ni=0;
    int Kcontrol = 0;
    if (!Specification.UseZverevTables) Kcontrol = 0;
    if (Specification.UseZverevTables && (Specification.order % 2 ==0)) Kcontrol=1;
    posx+=50;
    for (int k=0; k<N; k++)
    {

        if (((Specification.isCLC) && (k % 2 == Kcontrol)) || ((!Specification.isCLC) && (k % 2 != Kcontrol)))
        {
            //Shunt capacitor
            //Shunt capacitor
            Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                                 posx, 50, QString("N%1").arg(Ni), "gnd");
            gi[k+1] *= 1/(2*M_PI*Specification.fc*Specification.ZS);//Lowpass to highpass transformation
            Cshunt.val["C"] = num2str(gi[k+1], Capacitance);
            Components.append(Cshunt);

            //GND
            Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx, 100, "", "");
            Components.append(Ground);

            //Node
            NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
            Nodes.append(NI);

            //Wires
            //***** Capacitor to node *****
            WI.setParams(NI.ID, 0, Cshunt.ID, 1);
            Wires.append(WI);

            //***** GND to capacitor *****
            WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
            Wires.append(WI);

            //***** Capacitor to the previous Lseries/Term *****
            WI.setParams(ConnectionAux, 1, NI.ID, 1);
            Wires.append(WI);

            ConnectionAux = NI.ID;//The series inductor of the next section must be connected to this node
        }
        else
        {
            //Series inductor
            Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                                   posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
            gi[k+1] *= Specification.ZS/(2*M_PI*Specification.fc);
            Lseries.val["L"] = num2str(gi[k+1], Inductance);
            Components.append(Lseries);

            Ni++;
            //Wiring
            WI.setParams(ConnectionAux, 0, Lseries.ID, 0);
            Wires.append(WI);
            ConnectionAux = Lseries.ID;

        }
        posx += 50;
    }
    //Add Term 2
    k=Specification.ZS;
    if (Specification.UseZverevTables) (!Specification.isCLC) ? k /=gi[N+1] : k *=gi[N+1];
    else (Specification.isCLC) ? k /=gi[N+1] : k *=gi[N+1];

    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, posx, 0, QString("N%1").arg(Ni), "gnd");
    TermSpar2.val["Z"] = num2str(k, Resistance);
    Components.append(TermSpar2);

    WI.setParams(ConnectionAux, 1, TermSpar2.ID, 0);
    Wires.append(WI);
}


//Synthesis of highpass filters
void CanonicalFilter::SynthesizeHPF()
{
    ComponentInfo Lshunt, Cseries, Ground;
    //Synthesize CLC of LCL network
    int N = Specification.order;//Number of elements
    int posx = 0;
    QString ConnectionAux = "";
    Components.clear();

    WireInfo WI;
    NodeInfo NI;

    //Add Term 1
    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, posx, 0, "N0", "gnd");
    TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
    Components.append(TermSpar1);

    ConnectionAux = TermSpar1.ID;
    unsigned int Ni=0;//Node index
    posx+=50;
    for (int k=0; k<N; k++)
    {

        if (((Specification.isCLC) && (k % 2 == 0)) || ((!Specification.isCLC) && (k % 2 != 0)))
        {
            //Shunt inductor
            Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, vertical, posx, 50,
                             QString("N%1").arg(Ni), "gnd");
            gi[k+1] = Specification.ZS/(2*M_PI*Specification.fc*gi[k+1]);
            Lshunt.val["L"] = num2str(gi[k+1], Inductance);
            Components.append(Lshunt);


            //GND
            Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx, 100, "", "");
            Components.append(Ground);

            //Node
            NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
            Nodes.append(NI);

            //Wires
            //***** Capacitor to node *****
            WI.setParams(NI.ID, 1, Lshunt.ID, 1);
            Wires.append(WI);

            //***** GND to capacitor *****
            WI.setParams(Ground.ID, 0, Lshunt.ID, 0);
            Wires.append(WI);

            //***** Capacitor to the previous Lseries/Term *****
            WI.setParams(ConnectionAux, 1, NI.ID, 1);
            Wires.append(WI);

            ConnectionAux = NI.ID;//The series inductor of the next section must be connected to this node
        }
        else
        {
            //Series capacitor
            Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, horizontal, posx, 0,
                              QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
            gi[k+1] = 1/(2*M_PI*Specification.fc*gi[k+1]*Specification.ZS);
            Cseries.val["C"] = num2str(gi[k+1], Capacitance);
            Components.append(Cseries);

            Ni++;

            //Wiring
            WI.setParams(ConnectionAux, 0, Cseries.ID, 0);
            Wires.append(WI);
            ConnectionAux = Cseries.ID;
        }
        posx += 50;
    }
    //Add Term 2
    double k=Specification.ZL;
    Specification.isCLC ? k /=gi[N+1] : k *=gi[N+1];

    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, posx, 0, QString("N%1").arg(Ni), "gnd");
    TermSpar2.val["Z"] = num2str(k, Resistance);
    Components.append(TermSpar2);

    WI.setParams(ConnectionAux, 1, TermSpar2.ID, 0);
    Wires.append(WI);
}

//Synthesis of bandpass filters
void CanonicalFilter::SynthesizeBPF()
{
    ComponentInfo Cshunt, Lshunt, Ground1, Ground2, Cseries, Lseries;
    WireInfo WI;
    NodeInfo NI;
    //Synthesize CLC of LCL network
    int N = Specification.order;//Number of elements
    int posx = 0;
    QString ConnectionAux = "";
    Components.clear();

    //Add Term 1
    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, posx, 0, "N0", "gnd");
    TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
    Components.append(TermSpar1);

    ConnectionAux = TermSpar1.ID;

    unsigned int Ni=0;
    double wc = 2*M_PI*Specification.fc;
    double delta = 2*M_PI*Specification.bw;
    double w0 = sqrt(wc*wc - .25*delta*delta);

    posx+=50;
    for (int k=0; k<N; k++)
    {

        if (((Specification.isCLC) && (k % 2 == 0)) || ((!Specification.isCLC) && (k % 2 != 0)))
        {
            //Shunt capacitor
            Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical, posx-25, 50,
                              QString("N%1").arg(Ni), "gnd");
            Cshunt.val["C"] = num2str(gi[k+1]/(delta*Specification.ZS), Capacitance);
            Components.append(Cshunt);

            //GND
            Ground1.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx-25, 100, "", "");
            Components.append(Ground1);

            //Shunt inductor
            Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, vertical, posx+25, 50,
                             QString("N%1").arg(Ni), "gnd");
            Lshunt.val["L"] = num2str(Specification.ZS*delta/(w0*w0*gi[k+1]), Inductance);
            Components.append(Lshunt);

            //GND
            Ground2.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx+25, 100, "", "");
            Components.append(Ground2);

            //Node
            NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
            Nodes.append(NI);

            //Wires
            //***** Capacitor to node *****
            WI.setParams(NI.ID, 1, Cshunt.ID, 1);
            Wires.append(WI);

            //***** Inductor to node *****
            WI.setParams(NI.ID, 1, Lshunt.ID, 1);
            Wires.append(WI);

            //***** GND to capacitor *****
            WI.setParams(Ground1.ID, 0, Cshunt.ID, 0);
            Wires.append(WI);

            //***** GND to inductor *****
            WI.setParams(Ground2.ID, 0, Lshunt.ID, 0);
            Wires.append(WI);

            //***** Capacitor to the previous Lseries/Term *****
            WI.setParams(ConnectionAux, 1,  NI.ID, 1);
            Wires.append(WI);


            ConnectionAux = NI.ID;//The series inductor of the next section must be connected to this node
        }
        else
        {   if (k==0) posx+=50;//First element
            //Series inductor
            Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal, posx-30, 0,
                              QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
            Lseries.val["L"] = num2str(gi[k+1]*Specification.ZS/(delta), Inductance);
            Components.append(Lseries);

            //Series capacitor
            Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, horizontal, posx+30, 0,
                              QString("N%1").arg(Ni+1), QString("N%1").arg(Ni+2));
            Cseries.val["C"] = num2str(delta/(w0*w0*Specification.ZS*gi[k+1]), Capacitance);
            Components.append(Cseries);

            Ni+=2;

            //Wiring
            WI.setParams(ConnectionAux, 0, Lseries.ID, 0);
            Wires.append(WI);

            WI.setParams(Lseries.ID, 1, Cseries.ID, 0);
            Wires.append(WI);
            ConnectionAux = Cseries.ID;
        }
        posx += 100;
    }
    //Add Term 2
    double k=Specification.ZL;
    Specification.isCLC ? k /=gi[N+1] : k *=gi[N+1];

    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, posx, 0, QString("N%1").arg(Ni), "gnd");
    TermSpar2.val["Z"] = num2str(k, Resistance);
    Components.append(TermSpar2);

    WI.setParams(ConnectionAux, 1, TermSpar2.ID, 0);
    Wires.append(WI);

}

//Synthesis of bandstop filters
void CanonicalFilter::SynthesizeBSF()
{
    ComponentInfo Cshunt, Lshunt, Ground1, Ground2;
    WireInfo WI;
    NodeInfo NI, Node1, Node2;
    //Synthesize CLC of LCL network
    int N = Specification.order;//Number of elements
    int posx = 0;
    QString ConnectionAux = "";
    Components.clear();

    //Add Term 1
    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, posx, 0, "N0", "gnd");
    TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
    Components.append(TermSpar1);

    ConnectionAux = TermSpar1.ID;

    unsigned int Ni=0;
    double wc = 2*M_PI*Specification.fc;
    double delta = 2*M_PI*Specification.bw;
    double w0 = sqrt(wc*wc - .25*delta*delta);

    posx+=50;
    for (int k=0; k<N; k++)
    {

        if (((Specification.isCLC) && (k % 2 == 0)) || ((!Specification.isCLC) && (k % 2 != 0)))
        {
            //Shunt capacitor
            Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical, posx, 100,
                             QString("N%1").arg(Ni+1), "gnd");
            Cshunt.val["C"] = num2str(gi[k+1]*delta/(w0*w0*Specification.ZS), Capacitance);
            Components.append(Cshunt);

            //GND
            Ground1.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx, 150, "", "");
            Components.append(Ground1);

            //Shunt inductor
            Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, vertical, posx, 50,
                             QString().arg("N%1").argNi(), QString("N%1").arg(Ni+1));
            Lshunt.val["L"] = num2str(Specification.ZS/(delta*gi[k+1]), Inductance);
            Components.append(Lshunt);
            Ni++;

            //Node
            NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
            Nodes.append(NI);

            //Wires
            //***** Inductor to node *****
            WI.setParams(NI.ID, 1, Lshunt.ID, 1);
            Wires.append(WI);

            //***** Capacitor to inductor *****
            WI.setParams(Lshunt.ID, 0, Cshunt.ID, 1);
            Wires.append(WI);

            //***** GND to capacitor *****
            WI.setParams(Ground1.ID, 0, Cshunt.ID, 0);
            Wires.append(WI);


            //***** Capacitor to the previous Lseries *****
            if (!ConnectionAux.isEmpty())
            {
                WI.setParams(ConnectionAux, 1, NI.ID, 1);
                Wires.append(WI);
            }

            ConnectionAux = NI.ID;//The series inductor of the next section must be connected to this node
        }
        else
        {   if (k==0) posx+=50;//First element

            //Node
            Node1.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
            Nodes.append(Node1);
            posx+= 50;

            //Series inductor
            struct ComponentInfo Lseries;
            Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lseries.Type = Inductor;
            Lseries.Orientation = horizontal;
            Lseries.parameter = 0;
            Lseries.val.clear();
            Lseries.val["L"] = num2str(gi[k+1]*Specification.ZS*delta/(w0*w0), Inductance);
            Lseries.Coordinates.clear();
            Lseries.Coordinates.push_back(posx);
            Lseries.Coordinates.push_back(30);
            Components.append(Lseries);

            //Series capacitor
            struct ComponentInfo Cseries;
            Cseries.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
            Cseries.Type = Capacitor;
            Cseries.Orientation = horizontal;
            Cseries.parameter = 0;
            Cseries.val.clear();
            Cseries.val["C"] = num2str(1/(gi[k+1]*delta*Specification.ZS), Capacitance);
            Cseries.Coordinates.clear();
            Cseries.Coordinates.push_back(posx);
            Cseries.Coordinates.push_back(-30);
            Components.append(Cseries);
            QucsNetlist+=QString("C:C%1 N%2 N%3 C=\"%4 F\"\n").arg(NumberComponents[Capacitor]).arg(Ni-1).arg(Ni+1).arg(Cseries.val["C"]);
            QucsNetlist+=QString("L:L%1 N%2 N%3 L=\"%4 H\"\n").arg(NumberComponents[Inductor]).arg(Ni-1).arg(Ni+1).arg(Lseries.val["L"]);
            Ni++;

            //Node
            posx+= 50;
            NodeInfo Node2;
            Node2.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            Node2.Coordinates.clear();
            Node2.Coordinates.push_back(posx);
            Node2.Coordinates.push_back(0);
            Nodes.append(Node2);


            //Wiring
            //Intermediate series inductance => Connect port 0 to the previous Cshunt and port 1 to the next Cshunt
            WI.setParams(ConnectionAux, 0, Node1.ID, 0);
            Wires.append(WI);

            WI.setParams(Node1.ID, 0, Lseries.ID, 0);
            Wires.append(WI);

            WI.setParams(Node1.ID, 0, Cseries.ID, 0);
            Wires.append(WI);

            WI.setParams(Node2.ID, 0, Lseries.ID, 1);
            Wires.append(WI);

            WI.setParams(Node2.ID, 0, Cseries.ID, 1);
            Wires.append(WI);
            ConnectionAux = Node2.ID;
        }
        posx += 100;
    }
    //Add Term 2
    double k=Specification.ZL;
    Specification.isCLC ? k /=gi[N+1] : k *=gi[N+1];
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = horizontal;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(k, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);

    //********************** Network description for simulation ****************************
    k=Specification.ZL;
    Specification.isCLC ? k /=gi[N+1] : k *=gi[N+1];
    //***************************************************************************************
    int last_node;
    ((Specification.order % 2 == 0) && (Specification.isCLC)) ? last_node = Ni : last_node = Ni-1;
    QucsNetlist += QString("Pac:P2 N%1 gnd Num=2 Z=\"%2 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(last_node).arg(k);

    WI.setParams(ConnectionAux, 1, TermSpar.ID, 0);
    Wires.append(WI);
}





























