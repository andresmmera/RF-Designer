#include "DirectCoupledFilters.h"

DirectCoupledFilters::DirectCoupledFilters()
{
    //Initialize list of components
    NumberComponents[Capacitor] = 0;
    NumberComponents[Inductor] = 0;
    NumberComponents[Term] = 0;
    NumberComponents[GND] = 0;
    NumberComponents[ConnectionNodes] = 0;
}

DirectCoupledFilters::DirectCoupledFilters(FilterSpecifications FS)
{
    Specification = FS;
    //Initialize list of components
    NumberComponents[Capacitor] = 0;
    NumberComponents[Inductor] = 0;
    NumberComponents[Term] = 0;
    NumberComponents[GND] = 0;
    NumberComponents[ConnectionNodes] = 0;
}



void DirectCoupledFilters::synthesize()
{
  LowpassPrototypeCoeffs LP_coeffs(Specification);
  gi = LP_coeffs.getCoefficients();
  if (Specification.DC_Coupling == Capacitative) Synthesize_Capacitative_Coupled_Shunt_Resonators();
  if (Specification.DC_Coupling == Inductive) Synthesize_Inductive_Coupled_Series_Resonators();
}





void DirectCoupledFilters::Synthesize_Capacitative_Coupled_Shunt_Resonators()
{
    int N = Specification.order;
    std::deque<double> L(N), Cp(N);
    double r = gi[N+1];
    gi.pop_back();
    gi.pop_front();
    double fc = Specification.fc;
    double wc=2*M_PI*fc;
    double BW=Specification.bw;
    double Z0 = Specification.ZS;

    for (int i =0; i < N; i++) L[i] = 10e-9;


    double R1 = 1;
    double RN = Specification.ZL/Z0;
    double w0 = 1.0;
    double f1 = (fc-BW)/(2*M_PI*fc);
    double f2 = (fc+BW)/(2*M_PI*fc);
    double f1d = w0/(2*M_PI);
    double f0 = w0/(2*M_PI);
    double  wd = ((f0/f1)-(f0/f2))*(f0/f1d);

    std::deque<double>  Lrk(N), Crk(N), Cs(N+1);

    for (int i = 0; i <N; i++)
    {
       Lrk[i] = ((wc*L[i])/Z0);
       Crk[i] = 1./((w0*w0)*Lrk[i]);
    }

    //Series capacitors
    Cs[0] = (1/w0)*sqrt((wd*Crk[0]/(R1*gi[0])/(1-(wd*Crk[0]*R1/gi[0]))));
    for (int i = 1; i < N; i++)
    {
       Cs[i] = wd*sqrt((Crk[i-1]*Crk[i])/(gi[i-1]*gi[i]));
    }
    Cs[N] = (1/w0)*sqrt((wd*Crk[N-1]*r/(RN*gi[N-1]))/(1-(wd*Crk[N-1]*RN/gi[N-1])));

    double Cs1_ = Cs[0]/(1+pow(w0*Cs[0]*R1,2));
    double Csn_n1 = Cs[N]/(1+pow(w0*Cs[N]*RN, 2));

    Cp[0] = Crk[0]-Cs1_-Cs[1];
    for (int i = 1; i< N-1; i++)
    {
       Cp[i] = Crk[i]-Cs[i]-Cs[i+1];
    }
    Cp[N-1] = Crk[N-1]-Csn_n1-Cs[N-1];

    //Scale
    for (int i = 0; i < N+1; i++)
    {
       if (i < N) Cp[i] = Cp[i]/(2*M_PI*fc*Z0);
       Cs[i] = Cs[i]/(2*M_PI*fc*Z0);
    }

    //Build schematic
    int posx = 0, Ni = 0;
    QString ConnectionAux = "";
    Components.clear();
    double k=Specification.ZS;
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

    posx += 50;
    //Series capacitor
    struct ComponentInfo Cseries;
    Cseries.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
    Cseries.Type = Capacitor;
    Cseries.Orientation = horizontal;
    Cseries.parameter = 0;
    Cseries.val.clear();
    Cseries.val["C"] = Cs[0];
    Cseries.Coordinates.clear();
    Cseries.Coordinates.push_back(posx);
    Cseries.Coordinates.push_back(0);
    Components.append(Cseries);
    QucsNetlist+=QString("C:C%1 N0 N1 C=\"%2 F\"\n").arg(NumberComponents[Capacitor]).arg(Cseries.val["C"]);
    Ni++;


    //Wires
    //***** Port to capacitor *****
    struct WireInfo WI;
    WI.OriginID = TermSpar.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = Cseries.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

    for (int k=0; k<N; k++)
    {
        posx += 50;
       //Shunt resonator
        //Shunt inductor
        struct ComponentInfo Lshunt;
        Lshunt.ID=QString("L%1").arg(++NumberComponents[Inductor]);
        Lshunt.Type = Inductor;
        Lshunt.Orientation = vertical;
        Lshunt.parameter = 0;
        Lshunt.val.clear();
        Lshunt.val["L"] = L[k];
        Lshunt.Coordinates.clear();
        Lshunt.Coordinates.push_back(posx);
        Lshunt.Coordinates.push_back(50);
        Components.append(Lshunt);
        QucsNetlist+=QString("L:L%1 N%2 gnd L=\"%3 H\"\n").arg(NumberComponents[Inductor]).arg(Ni).arg(Lshunt.val["L"]);


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


        WI.OriginID = Lshunt.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ground.ID;
        WI.PortDestination = 0;
        Wires.append(WI);



        posx+=25;
        //Node
        NodeInfo NI;
        NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        NI.Coordinates.clear();
        NI.Coordinates.push_back(posx);
        NI.Coordinates.push_back(0);
        Nodes.append(NI);

        //Node to the previous series capacitor
        WI.OriginID = NI.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Cseries.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        posx += 25;
        //Shunt capacitor
        struct ComponentInfo Cshunt;
        Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
        Cshunt.Type = Capacitor;
        Cshunt.Orientation = vertical;
        Cshunt.parameter = 0;
        Cshunt.val.clear();
        Cshunt.val["C"] = Cp[k];
        Cshunt.Coordinates.clear();
        Cshunt.Coordinates.push_back(posx);
        Cshunt.Coordinates.push_back(50);
        Components.append(Cshunt);
        QucsNetlist+=QString("C:C%1 N%2 gnd C=\"%3 F\"\n").arg(NumberComponents[Capacitor]).arg(Ni).arg(Cshunt.val["C"]);

        //GND
        Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
        Ground.Type = GND;
        Ground.Orientation = vertical;
        Ground.parameter = 0;
        Ground.Coordinates.clear();
        Ground.Coordinates.push_back(posx);
        Ground.Coordinates.push_back(100);
        Ground.val.clear();
        Components.append(Ground);

        posx += 50;
        //Series capacitor
        Cseries.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
        Cseries.Type = Capacitor;
        Cseries.Orientation = horizontal;
        Cseries.parameter = 0;
        Cseries.val.clear();
        Cseries.val["C"] = Cs[k+1];
        Cseries.Coordinates.clear();
        Cseries.Coordinates.push_back(posx);
        Cseries.Coordinates.push_back(0);
        Components.append(Cseries);
        QucsNetlist+=QString("C:C%1 N%2 N%3 C=\"%4 F\"\n").arg(NumberComponents[Capacitor]).arg(Ni).arg(Ni+1).arg(Cseries.val["C"]);
        Ni++;

        WI.OriginID = NI.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Lshunt.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        WI.OriginID = NI.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Cshunt.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        WI.OriginID = NI.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Cseries.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ground.ID;
        WI.PortDestination = 0;
        Wires.append(WI);
    }

    posx += 50;
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = horizontal;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = Specification.ZL;
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(0);
    Components.append(TermSpar);
    ConnectionAux = TermSpar.ID;
    QucsNetlist += QString("Pac:P2 N%1 gnd Num=2 Z=\"%2 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Ni).arg(TermSpar.val["Z"]);


    WI.OriginID = Cseries.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = TermSpar.ID;
    WI.PortDestination = 0;
    Wires.append(WI);
}



void DirectCoupledFilters::Synthesize_Inductive_Coupled_Series_Resonators()
{
    int N = Specification.order;
    std::deque<double> L(N+2), Lrk(N+2), Crk(N), M(N+2), Lp(N+2);
    double r = gi[N];
    gi.pop_back();
    double fc = Specification.fc;
    double wc=2*M_PI*fc;
    double BW=Specification.bw;
    double Z0 = Specification.ZS;
    double R1 = 1;
    double RN = Specification.ZL/Z0;

    for (int i =0; i < N+2; i++) L[i] = 10e-9;

    double w0 = 1.0;
    double f1 = (fc-BW)/(2*M_PI*fc);
    double f2 = (fc+BW)/(2*M_PI*fc);
    double f1d = 1/(2*M_PI);
    double f0 = w0/(2*M_PI);
    double w_ = ((f0/f1)-(f0/f2))*(f0/f1d);

    for (int i = 0; i < N+2; i++) Lrk[i] = (wc*L[i])/Z0;

    Crk[0] = (1 + (w_*Lrk[0])/(gi[0]*R1))/pow(Lrk[0]*w0,2);
    for (int i = 1; i < N-1; i++)
    {
      Crk[i] = 1/pow(Lrk[i]*w0,2);
    }
    Crk[N-1] = (1 + (w_*Lrk[N]*r)/(gi[N-1]*RN))/pow(Lrk[N-1]*w0,2);

    M[0] = sqrt((w_*(R1*R1 + pow(w0*Lrk[0],2)))/(pow(Crk[0]*gi[0]*R1*w0,2)))/w0;
    for (int i = 1; i < N-1;i++)
    {
      M[i] = (w_*sqrt((Lrk[i]*Lrk[i+1])/(gi[i]*gi[i+1])))/w0;
    }
    M[N+1] = sqrt((w_*r*(RN*RN + pow(w0*Lrk[N+1],2)))/(pow(Crk[N-1]*gi[N-1]*RN*w0,2)))/w0;


    std::deque<double> Ls(N+2);
    Ls[0] = Lrk[0]-M[0];
    Ls[1] = Lrk[0] - M[0] - M[1];

    for (int i = 1; i < N; i++)
    {
         Ls[i] = Lrk[i] - M[i] - M[i+1];
    }
    Ls[N+1] = Lrk[N] - M[N+1];

    //Impedance and frequency scaling
    for (int i = 0; i < N+2; i++)
    {
       Ls[i] = Ls[i]*Z0/(2*M_PI*fc);
       if (i < N)
       {
           if (i < N-1) Lp[i] = M[i]*Z0/(2*M_PI*fc);
           Crk[i] = Crk[i]/(2*M_PI*fc*Z0);
       }
    }

    //Create schematic and Qucs netlist
}

QList<ComponentInfo> DirectCoupledFilters::getComponents()
{
    return Components;
}


QList<WireInfo> DirectCoupledFilters::getWires()
{
    return Wires;
}

QList<NodeInfo> DirectCoupledFilters::getNodes()
{
    return Nodes;
}
