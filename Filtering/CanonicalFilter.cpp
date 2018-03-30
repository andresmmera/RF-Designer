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

   if ((Specification.FilterResponse == Chebyshev) && (!Specification.UseZverevTables))
    {//Correct cutoff according to the ripple
        double epsilon = sqrt(pow(10.0, Specification.Ripple/10.0) - 1.0);

        switch (Specification.FilterType)
        {
         case Lowpass:
            Specification.fc /= cosh(acosh(1.0 / epsilon) / Specification.order);
            break;
         case Highpass:
            Specification.fc *= cosh(acosh(1.0 / epsilon) / Specification.order);
            break;
        case Bandpass:
        case Bandstop:
            break;
        }
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
            Cshunt.Connections.clear();
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
            Lseries.Connections.clear();
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
            Lshunt.Connections.clear();
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
            Cseries.Connections.clear();
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
            Cshunt.Connections.clear();
            Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical, posx-25, 50,
                              QString("N%1").arg(Ni), "gnd");
            Cshunt.val["C"] = num2str(gi[k+1]/(delta*Specification.ZS), Capacitance);
            Components.append(Cshunt);

            //GND
            Ground1.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx-25, 100, "", "");
            Components.append(Ground1);

            //Shunt inductor
            Lshunt.Connections.clear();
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
            Lseries.Connections.clear();
            Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal, posx-30, 0,
                              QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
            Lseries.val["L"] = num2str(gi[k+1]*Specification.ZS/(delta), Inductance);
            Components.append(Lseries);

            //Series capacitor
            Cseries.Connections.clear();
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
    ComponentInfo Cshunt, Lshunt, Ground1, Lseries, Cseries;
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
            Cshunt.Connections.clear();
            Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical, posx, 100,
                             QString("N%1").arg(Ni+1), "gnd");
            Cshunt.val["C"] = num2str(gi[k+1]*delta/(w0*w0*Specification.ZS), Capacitance);
            Components.append(Cshunt);

            //GND
            Ground1.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx, 150, "", "");
            Components.append(Ground1);

            //Shunt inductor
            Lshunt.Connections.clear();
            Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, vertical, posx, 50,
                             QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
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
            Lseries.Connections.clear();
            Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal, posx, 30,
                              QString("N%1").arg(Ni-1), QString("N%1").arg(Ni+1));
            Lseries.val["L"] = num2str(gi[k+1]*Specification.ZS*delta/(w0*w0), Inductance);
            Components.append(Lseries);

            //Series capacitor
            Cseries.Connections.clear();
            Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]),  Capacitor, horizontal, posx, -30,
                              QString("N%1").arg(Ni-1), QString("N%1").arg(Ni+1));
            Cseries.val["C"] = num2str(1/(gi[k+1]*delta*Specification.ZS), Capacitance);
            Components.append(Cseries);
            Ni++;

            //Node
            posx+= 50;
            Node2.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
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

    int last_node;
    ((Specification.order % 2 == 0) && (Specification.isCLC)) ? last_node = Ni : last_node = Ni-1;
    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, posx, 0, QString("N%1").arg(last_node), "gnd");
    TermSpar2.val["Z"] = num2str(k, Resistance);
    Components.append(TermSpar2);

    WI.setParams(ConnectionAux, 1, TermSpar2.ID, 0);
    Wires.append(WI);
}





























