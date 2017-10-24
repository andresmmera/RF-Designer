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

    switch (Specification.FilterType)
    {
    case Lowpass:
        return SynthesizeLPF();
    case Highpass:
        return SynthesizeHPF();
    case Bandpass:
        return SynthesizeBPF();
    case Bandstop:
        return SynthesizeBSF();
    }

}


//Synthesis of lowpass filters
void CanonicalFilter::SynthesizeLPF()
{
    //Synthesize CLC of LCL network
    int N = Specification.order;//Number of elements
    int posx = 0;
    QString ConnectionAux = "";
    Components.clear();

    //Add Term 1
    double k=Specification.ZS;
 /*   if (Specification.UseZverevTables)
    {
        k=Specification.ZL;
    }*/
    struct ComponentInfo TermSpar;
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = k;
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    ConnectionAux = TermSpar.ID;
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(k);
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
            struct ComponentInfo Cshunt;
            Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
            Cshunt.Type = Capacitor;
            Cshunt.Orientation = vertical;
            Cshunt.parameter = 0;
            Cshunt.val.clear();
            gi[k+1] *= 1/(2*M_PI*Specification.fc*Specification.ZS);//Lowpass to highpass transformation
            Cshunt.val["C"] = gi[k+1];
            Cshunt.Coordinates.clear();
            Cshunt.Coordinates.push_back(posx);
            Cshunt.Coordinates.push_back(50);
            Components.append(Cshunt);
            QucsNetlist+=QString("C:C%1 N%2 gnd C=\"%3 F\"\n").arg(NumberComponents[Capacitor]).arg(Ni).arg(gi[k+1]);

            //GND
            struct ComponentInfo Ground;
            Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
            Ground.Type = GND;
            Ground.Orientation = vertical;
            Ground.parameter = 0;
            Ground.Coordinates.clear();
            Ground.Coordinates.push_back(posx);
            Ground.Coordinates.push_back(100);
            Ground.val.clear();
            Components.append(Ground);

            //Node
            NodeInfo NI;
            NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            NI.Coordinates.clear();
            NI.Coordinates.push_back(posx);
            NI.Coordinates.push_back(0);
            Nodes.append(NI);

            //Wires
            //***** Capacitor to node *****
            struct WireInfo WI;
            WI.OriginID = NI.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = Cshunt.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //***** GND to capacitor *****
            WI.OriginID = Ground.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Cshunt.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            //***** Capacitor to the previous Lseries *****
            if (!ConnectionAux.isEmpty())
            {
                WI.OriginID = ConnectionAux;
                WI.PortOrigin = 1;
                WI.DestinationID = NI.ID;
                WI.PortDestination = 1;
                Wires.append(WI);
            }

            ConnectionAux = NI.ID;//The series inductor of the next section must be connected to this node
        }
        else
        {
            //Series inductor
            struct ComponentInfo Lseries;
            Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lseries.Type = Inductor;
            Lseries.Orientation = horizontal;
            Lseries.parameter = 0;
            Lseries.val.clear();
            gi[k+1] *= Specification.ZS/(2*M_PI*Specification.fc);
            Lseries.val["L"] = gi[k+1];
            Lseries.Coordinates.clear();
            Lseries.Coordinates.push_back(posx);
            Lseries.Coordinates.push_back(0);
            Components.append(Lseries);
            QucsNetlist+=QString("L:L%1 N%2 N%3 L=\"%4 H\"\n").arg(NumberComponents[Inductor]).arg(Ni).arg(Ni+1).arg(gi[k+1]);
            Ni++;
            //Wiring
            struct WireInfo WI;
            WI.OriginID = ConnectionAux;//Node
            WI.PortOrigin = 0;
            WI.DestinationID = Lseries.ID;
            WI.PortDestination = 0;
            Wires.append(WI);
            ConnectionAux = Lseries.ID;

        }
        posx += 50;
    }
    //Add Term 2
    k=Specification.ZS;
    if (Specification.UseZverevTables) (!Specification.isCLC) ? k /=gi[N+1] : k *=gi[N+1];
    else (Specification.isCLC) ? k /=gi[N+1] : k *=gi[N+1];
  //  if (Specification.UseZverevTables) k=Specification.ZS;
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = horizontal;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = k;
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    //********************** Network description for simulation ****************************
    QucsNetlist += QString("Pac:P2 N%1 gnd Num=2 Z=\"%2 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Ni).arg(k);

    struct WireInfo WI;
    WI.OriginID = ConnectionAux;//Node
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);
}


//Synthesis of highpass filters
void CanonicalFilter::SynthesizeHPF()
{
    //Synthesize CLC of LCL network
    int N = Specification.order;//Number of elements
    int posx = 0;
    QString ConnectionAux = "";
    Components.clear();

    NetworkInfo NWI;

    //Add Term 1
    struct ComponentInfo TermSpar;
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = Specification.ZS;
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    ConnectionAux = TermSpar.ID;
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specification.ZS);
    unsigned int Ni=0;//Node index
    posx+=50;
    for (int k=0; k<N; k++)
    {

        if (((Specification.isCLC) && (k % 2 == 0)) || ((!Specification.isCLC) && (k % 2 != 0)))
        {
            //Shunt inductor
            struct ComponentInfo Lshunt;
            Lshunt.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lshunt.Type = Inductor;
            Lshunt.Orientation = vertical;
            Lshunt.parameter = 0;
            Lshunt.val.clear();
            gi[k+1] = Specification.ZS/(2*M_PI*Specification.fc*gi[k+1]);
            Lshunt.val["L"] = gi[k+1];
            Lshunt.Coordinates.clear();
            Lshunt.Coordinates.push_back(posx);
            Lshunt.Coordinates.push_back(50);
            Components.append(Lshunt);
            QucsNetlist+=QString("L:L%1 N%2 gnd L=\"%3 H\"\n").arg(NumberComponents[Inductor]).arg(Ni).arg(gi[k+1]);


            //GND
            struct ComponentInfo Ground;
            Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
            Ground.Type = GND;
            Ground.Orientation = vertical;
            Ground.parameter = 0;
            Ground.Coordinates.clear();
            Ground.Coordinates.push_back(posx);
            Ground.Coordinates.push_back(100);
            Ground.val.clear();
            Components.append(Ground);

            //Node
            NodeInfo NI;
            NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            NI.Coordinates.clear();
            NI.Coordinates.push_back(posx);
            NI.Coordinates.push_back(0);
            Nodes.append(NI);

            //Wires
            //***** Capacitor to node *****
            struct WireInfo WI;
            WI.OriginID = NI.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = Lshunt.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //***** GND to capacitor *****
            WI.OriginID = Ground.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Lshunt.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            //***** Capacitor to the previous Lseries *****
            if (!ConnectionAux.isEmpty())
            {
                WI.OriginID = ConnectionAux;
                WI.PortOrigin = 1;
                WI.DestinationID = NI.ID;
                WI.PortDestination = 1;
                Wires.append(WI);
            }

            ConnectionAux = NI.ID;//The series inductor of the next section must be connected to this node
        }
        else
        {
            //Series capacitor
            struct ComponentInfo Cseries;
            Cseries.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
            Cseries.Type = Capacitor;
            Cseries.Orientation = horizontal;
            Cseries.parameter = 0;
            Cseries.val.clear();
            gi[k+1] = 1/(2*M_PI*Specification.fc*gi[k+1]*Specification.ZS);
            Cseries.val["C"] = gi[k+1];
            Cseries.Coordinates.clear();
            Cseries.Coordinates.push_back(posx);
            Cseries.Coordinates.push_back(0);
            Components.append(Cseries);
            QucsNetlist+=QString("C:C%1 N%2 N%3 C=\"%4 F\"\n").arg(NumberComponents[Capacitor]).arg(Ni).arg(Ni+1).arg(gi[k+1]);
            Ni++;

            //Wiring
            struct WireInfo WI;
            WI.OriginID = ConnectionAux;//Node
            WI.PortOrigin = 0;
            WI.DestinationID = Cseries.ID;
            WI.PortDestination = 0;
            Wires.append(WI);
            ConnectionAux = Cseries.ID;
        }
        posx += 50;
    }
    //Add Term 2
    double k=Specification.ZL;
    Specification.isCLC ? k /=gi[N+1] : k *=gi[N+1];
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = horizontal;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = k;
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);

    //********************** Network description for simulation ****************************
    k=Specification.ZL;
    Specification.isCLC ? k /=gi[N+1] : k *=gi[N+1];
    //***************************************************************************************
    QucsNetlist += QString("Pac:P2 N%1 gnd Num=2 Z=\"%2 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Ni).arg(k);

    struct WireInfo WI;
    WI.OriginID = ConnectionAux;//Node
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);
}

//Synthesis of bandpass filters
void CanonicalFilter::SynthesizeBPF()
{
    //Synthesize CLC of LCL network
    int N = Specification.order;//Number of elements
    int posx = 0;
    QString ConnectionAux = "";
    Components.clear();

    //Add Term 1
    struct ComponentInfo TermSpar;
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = Specification.ZS;
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    ConnectionAux = TermSpar.ID;
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specification.ZS);
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
            struct ComponentInfo Cshunt;
            Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
            Cshunt.Type = Capacitor;
            Cshunt.Orientation = vertical;
            Cshunt.parameter = 0;
            Cshunt.val.clear();
            Cshunt.val["C"] = gi[k+1]/(delta*Specification.ZS);
            Cshunt.Coordinates.clear();
            Cshunt.Coordinates.push_back(posx-25);
            Cshunt.Coordinates.push_back(50);
            Components.append(Cshunt);

            //GND
            struct ComponentInfo Ground1;
            Ground1.ID=QString("GND%1").arg(++NumberComponents[GND]);
            Ground1.Type = GND;
            Ground1.Orientation = vertical;
            Ground1.parameter = 0;
            Ground1.Coordinates.clear();
            Ground1.Coordinates.push_back(posx-25);
            Ground1.Coordinates.push_back(100);
            Ground1.val.clear();
            Components.append(Ground1);


            //Shunt inductor
            struct ComponentInfo Lshunt;
            Lshunt.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lshunt.Type = Inductor;
            Lshunt.Orientation = vertical;
            Lshunt.parameter = 0;
            Lshunt.val.clear();
            Lshunt.val["L"] = Specification.ZS*delta/(w0*w0*gi[k+1]);
            Lshunt.Coordinates.clear();
            Lshunt.Coordinates.push_back(posx+25);
            Lshunt.Coordinates.push_back(50);
            Components.append(Lshunt);

            //GND
            struct ComponentInfo Ground2;
            Ground2.ID=QString("GND%1").arg(++NumberComponents[GND]);
            Ground2.Type = GND;
            Ground2.Orientation = vertical;
            Ground2.parameter = 0;
            Ground2.Coordinates.clear();
            Ground2.Coordinates.push_back(posx+25);
            Ground2.Coordinates.push_back(100);
            Ground2.val.clear();
            Components.append(Ground2);

            QucsNetlist+=QString("C:C%1 N%2 gnd C=\"%3 F\"\n").arg(NumberComponents[Capacitor]).arg(Ni).arg(Cshunt.val["C"]);
            QucsNetlist+=QString("L:L%1 N%2 gnd L=\"%3 H\"\n").arg(NumberComponents[Inductor]).arg(Ni).arg(Lshunt.val["L"]);


            //Node
            NodeInfo NI;
            NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            NI.Coordinates.clear();
            NI.Coordinates.push_back(posx);
            NI.Coordinates.push_back(0);
            Nodes.append(NI);

            //Wires
            //***** Capacitor to node *****
            struct WireInfo WI;
            WI.OriginID = NI.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = Cshunt.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //***** Inductor to node *****
            WI.OriginID = NI.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = Lshunt.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //***** GND to capacitor *****
            WI.OriginID = Ground1.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Cshunt.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            //***** GND to inductor *****
            WI.OriginID = Ground2.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Lshunt.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            //***** Capacitor to the previous Lseries *****
            if (!ConnectionAux.isEmpty())
            {
                WI.OriginID = ConnectionAux;
                WI.PortOrigin = 1;
                WI.DestinationID = NI.ID;
                WI.PortDestination = 1;
                Wires.append(WI);
            }

            ConnectionAux = NI.ID;//The series inductor of the next section must be connected to this node
        }
        else
        {   if (k==0) posx+=50;//First element
            //Series inductor
            struct ComponentInfo Lseries;
            Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lseries.Type = Inductor;
            Lseries.Orientation = horizontal;
            Lseries.parameter = 0;
            Lseries.val.clear();
            Lseries.val["L"] = gi[k+1]*Specification.ZS/(delta);
            Lseries.Coordinates.clear();
            Lseries.Coordinates.push_back(posx-30);
            Lseries.Coordinates.push_back(0);
            Components.append(Lseries);

            //Series capacitor
            struct ComponentInfo Cseries;
            Cseries.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
            Cseries.Type = Capacitor;
            Cseries.Orientation = horizontal;
            Cseries.parameter = 0;
            Cseries.val.clear();
            Cseries.val["C"] = delta/(w0*w0*Specification.ZS*gi[k+1]);
            Cseries.Coordinates.clear();
            Cseries.Coordinates.push_back(posx+30);
            Cseries.Coordinates.push_back(0);
            Components.append(Cseries);

            QucsNetlist+=QString("L:L%1 N%2 N%3 L=\"%4 H\"\n").arg(NumberComponents[Inductor]).arg(Ni).arg(Ni+1).arg(Lseries.val["L"]);
            QucsNetlist+=QString("C:C%1 N%2 N%3 C=\"%4 F\"\n").arg(NumberComponents[Capacitor]).arg(Ni+1).arg(Ni+2).arg(Cseries.val["C"]);
            Ni+=2;

            //Wiring
            struct WireInfo WI;
            WI.OriginID = ConnectionAux;//Node
            WI.PortOrigin = 0;
            WI.DestinationID = Lseries.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            WI.OriginID = Lseries.ID;//Node
            WI.PortOrigin = 1;
            WI.DestinationID = Cseries.ID;
            WI.PortDestination = 0;
            Wires.append(WI);
            ConnectionAux = Cseries.ID;
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
    TermSpar.val["Z"] = k;
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);

    //********************** Network description for simulation ****************************
    k=Specification.ZL;
    Specification.isCLC ? k /=gi[N+1] : k *=gi[N+1];
    //***************************************************************************************
    QucsNetlist += QString("Pac:P2 N%1 gnd Num=2 Z=\"%2 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Ni).arg(k);

    struct WireInfo WI;
    WI.OriginID = ConnectionAux;//Node
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

}

//Synthesis of bandstop filters
void CanonicalFilter::SynthesizeBSF()
{
    //Synthesize CLC of LCL network
    int N = Specification.order;//Number of elements
    int posx = 0;
    QString ConnectionAux = "";
    Components.clear();

    //Add Term 1
    struct ComponentInfo TermSpar;
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = vertical;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = Specification.ZS;
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    ConnectionAux = TermSpar.ID;
    QucsNetlist.clear();
    QucsNetlist = QString("Pac:P1 N0 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specification.ZS);
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
            struct ComponentInfo Cshunt;
            Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
            Cshunt.Type = Capacitor;
            Cshunt.Orientation = vertical;
            Cshunt.parameter = 0;
            Cshunt.val.clear();
            Cshunt.val["C"] = gi[k+1]*delta/(w0*w0*Specification.ZS);
            Cshunt.Coordinates.clear();
            Cshunt.Coordinates.push_back(posx);
            Cshunt.Coordinates.push_back(100);
            Components.append(Cshunt);

            //GND
            struct ComponentInfo Ground1;
            Ground1.ID=QString("GND%1").arg(++NumberComponents[GND]);
            Ground1.Type = GND;
            Ground1.Orientation = vertical;
            Ground1.parameter = 0;
            Ground1.Coordinates.clear();
            Ground1.Coordinates.push_back(posx);
            Ground1.Coordinates.push_back(150);
            Ground1.val.clear();
            Components.append(Ground1);


            //Shunt inductor
            struct ComponentInfo Lshunt;
            Lshunt.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lshunt.Type = Inductor;
            Lshunt.Orientation = vertical;
            Lshunt.parameter = 0;
            Lshunt.val.clear();
            Lshunt.val["L"] = Specification.ZS/(delta*gi[k+1]);
            Lshunt.Coordinates.clear();
            Lshunt.Coordinates.push_back(posx);
            Lshunt.Coordinates.push_back(50);
            Components.append(Lshunt);
            QucsNetlist+=QString("L:L%1 N%2 N%3 L=\"%4 H\"\n").arg(NumberComponents[Inductor]).arg(Ni).arg(Ni+1).arg(Lshunt.val["L"]);
            QucsNetlist+=QString("C:C%1 N%2 gnd C=\"%3 F\"\n").arg(NumberComponents[Capacitor]).arg(Ni+1).arg(Cshunt.val["C"]);
            Ni++;

            //Node
            NodeInfo NI;
            NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            NI.Coordinates.clear();
            NI.Coordinates.push_back(posx);
            NI.Coordinates.push_back(0);
            Nodes.append(NI);

            //Wires
            //***** Inductor to node *****
            struct WireInfo WI;
            WI.OriginID = NI.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = Lshunt.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //***** Capacitor to indutcot *****
            WI.OriginID = Lshunt.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Cshunt.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            //***** GND to capacitor *****
            WI.OriginID = Ground1.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Cshunt.ID;
            WI.PortDestination = 0;
            Wires.append(WI);


            //***** Capacitor to the previous Lseries *****
            if (!ConnectionAux.isEmpty())
            {
                WI.OriginID = ConnectionAux;
                WI.PortOrigin = 1;
                WI.DestinationID = NI.ID;
                WI.PortDestination = 1;
                Wires.append(WI);
            }

            ConnectionAux = NI.ID;//The series inductor of the next section must be connected to this node
        }
        else
        {   if (k==0) posx+=50;//First element

            //Node
            NodeInfo Node1;
            Node1.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            Node1.Coordinates.clear();
            Node1.Coordinates.push_back(posx);
            Node1.Coordinates.push_back(0);
            Nodes.append(Node1);
            posx+= 50;

            //Series inductor
            struct ComponentInfo Lseries;
            Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
            Lseries.Type = Inductor;
            Lseries.Orientation = horizontal;
            Lseries.parameter = 0;
            Lseries.val.clear();
            Lseries.val["L"] = gi[k+1]*Specification.ZS*delta/(w0*w0);
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
            Cseries.val["C"] = 1/(gi[k+1]*delta*Specification.ZS);
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
            struct WireInfo WI;
            //Intermediate series inductance => Connect port 0 to the previous Cshunt and port 1 to the next Cshunt
            WI.OriginID = ConnectionAux;//Node
            WI.PortOrigin = 0;
            WI.DestinationID = Node1.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            WI.OriginID = Node1.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Lseries.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            WI.OriginID = Node1.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Cseries.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            WI.OriginID = Node2.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Lseries.ID;
            WI.PortDestination = 1;
            Wires.append(WI);

            WI.OriginID = Node2.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = Cseries.ID;
            WI.PortDestination = 1;
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
    TermSpar.val["Z"] = k;
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

    struct WireInfo WI;
    WI.OriginID = ConnectionAux;//Node
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);
}





























