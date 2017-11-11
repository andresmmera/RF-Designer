#include "PowerCombining/PowerCombinerDesigner.h"

void PowerCombinerDesigner::MultistageWilkinson()
{
    int NStages = Specs.Nstages;
    double C[NStages], L[NStages];
    std::deque<double> Zlines = ChebyshevTaper(2*Specs.Z0, 0.05);
    if (Specs.Implementation == "Lumped LC")//CLC pi equivalent calculation
    {
        double w = 2*M_PI*Specs.freq;
        for (int i = 0; i<NStages;i++)
        {
            L[i] = Zlines[i]/w;
            C[i] = 1./(L[i]*w*w);
        }

    }

    double alpha = log(pow(0.1*Specs.alpha, 10));//Alpha is given in dB/m, then it is necessary to convert it into Np/m units
    double lambda4 = SPEED_OF_LIGHT/(4.*Specs.freq);
    std::complex<double> gamma(alpha, 2*M_PI*Specs.freq/SPEED_OF_LIGHT);//It is only considered the attenation of the metal conductor since it tends to be much higher than the dielectric
    std::deque<double> Risol = calcMultistageWilkinsonIsolators(Zlines, lambda4, gamma);

    //Build the schematic
    int posx = 0;
    int posy;//The position of the upper and the lower branches vary depending on the type of implementation
    int Ni=0;//Node index
    (Specs.Implementation == "Lumped LC") ? posy = 75 : posy = 50;
    struct ComponentInfo TL, TL_Upper, TL_Lower;
    struct ComponentInfo TermSpar;
    struct ComponentInfo Cshunt, Ground;
    struct ComponentInfo Lseries;
    struct WireInfo WI;
    NodeInfo NI, Nupper, Nlower;
    QString PreviousNode;

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
    if (Specs.Implementation == "Lumped LC" )//LC elements. Pi CLC equivalent of a lambda/4 line
    {
        //Shunt capacitor
        struct ComponentInfo Cshunt;
        Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
        Cshunt.Type = Capacitor;
        Cshunt.Orientation = vertical;
        Cshunt.parameter = 0;
        Cshunt.val.clear();
        Cshunt.val["C"] = num2str(2*C[0], Capacitance);
        Cshunt.Coordinates.clear();
        Cshunt.Coordinates.push_back(posx);
        Cshunt.Coordinates.push_back(20);
        Components.append(Cshunt);
        QucsNetlist+=QString("C:C%1 N0 gnd C=\"%2\"\n").arg(NumberComponents[Capacitor]).arg(Cshunt.val["C"]);

        Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
        Ground.Type = GND;
        Ground.Orientation = vertical;
        Ground.parameter = 0;
        Ground.Coordinates.clear();
        Ground.Coordinates.push_back(posx);
        Ground.Coordinates.push_back(60);
        Ground.val.clear();
        Components.append(Ground);

        //Capacitor to ground
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ground.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        //Node
        NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        NI.Coordinates.clear();
        NI.Coordinates.push_back(posx+25);
        NI.Coordinates.push_back(0);
        Nodes.append(NI);

        //Capacitor to ground
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = NI.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        //Term to capacitor
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = TermSpar.ID;
        WI.PortDestination = 0;
        Wires.append(WI);
        posx += 20;
    }
    else
    {//Ideal transmission lines
        //1st transmission line
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

        //Node
        NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        NI.Coordinates.clear();
        NI.Coordinates.push_back(posx+25);
        NI.Coordinates.push_back(0);
        Nodes.append(NI);

        WI.OriginID = TermSpar.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = TL.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        WI.OriginID = TL.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = NI.ID;
        WI.PortDestination = 1;
        Wires.append(WI);
        Ni++;
    }


    for (int i = 0; i < Specs.Nstages; i++)
    {
        if (Specs.Implementation == "Lumped LC" )//LC elements. Pi CLC equivalent of a lambda/4 line
        {
              double C_;
              if (i != Specs.Nstages-1) C_ = C[i] + C[i+1];
              else C_ = C[i];
             //Upper branch
              posx += 50;
              (i==0) ? PreviousNode = "N0" : PreviousNode = QString("Nupper%1").arg(i-1);

              //Series inductor
              Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
              Lseries.Type = Inductor;
              Lseries.Orientation = horizontal;
              Lseries.parameter = 0;
              Lseries.val.clear();
              Lseries.val["L"] = num2str(L[i], Inductance);
              Lseries.Coordinates.clear();
              Lseries.Coordinates.push_back(posx);
              Lseries.Coordinates.push_back(-75);
              Components.append(Lseries);
              QucsNetlist+=QString("L:L%1 %2 Nupper%3 L=\"%4\"\n")
                      .arg(NumberComponents[Inductor])
                      .arg(PreviousNode)
                      .arg(i)
                      .arg(Lseries.val["L"]);

              posx += 50;
              //Cshunt
              Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
              Cshunt.Type = Capacitor;
              Cshunt.Orientation = vertical;
              Cshunt.parameter = 0;
              Cshunt.val.clear();
              Cshunt.val["C"] = num2str(C_, Capacitance);
              Cshunt.Coordinates.clear();
              Cshunt.Coordinates.push_back(posx);
              Cshunt.Coordinates.push_back(-50);
              Components.append(Cshunt);
              QucsNetlist+=QString("C:C%1 Nupper%2 gnd C=\"%3\"\n")
                      .arg(NumberComponents[Capacitor])
                      .arg(i)
                      .arg(Cshunt.val["C"]);

              Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
              Ground.Type = GND;
              Ground.Orientation = vertical;
              Ground.parameter = 0;
              Ground.Coordinates.clear();
              Ground.Coordinates.push_back(posx);
              Ground.Coordinates.push_back(-10);
              Ground.val.clear();
              Components.append(Ground);

              //Capacitor to ground
              WI.OriginID = Cshunt.ID;
              WI.PortOrigin = 0;
              WI.DestinationID = Ground.ID;
              WI.PortDestination = 0;
              Wires.append(WI);


              if (i > 0)
              {//Connect the current section to the previous one
                  WI.OriginID = Lseries.ID;
                  WI.PortOrigin = 0;
                  WI.DestinationID = Nupper.ID;
                  WI.PortDestination = 0;
                  Wires.append(WI);
              }
              else
              {
                  if (i == 0)
                  {//Connect to the common node
                      WI.OriginID = Lseries.ID;
                      WI.PortOrigin = 0;
                      WI.DestinationID = NI.ID;
                      WI.PortDestination = 1;
                      Wires.append(WI);
                  }
              }

              //Node
              Nupper.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
              Nupper.Coordinates.clear();
              Nupper.Coordinates.push_back(posx+50);
              Nupper.Coordinates.push_back(-75);
              Nodes.append(Nupper);

              //Capacitor to node
              WI.OriginID = Cshunt.ID;
              WI.PortOrigin = 1;
              WI.DestinationID = Nupper.ID;
              WI.PortDestination = 0;
              Wires.append(WI);

              //Inductor to capacitor
              WI.OriginID = Cshunt.ID;
              WI.PortOrigin = 1;
              WI.DestinationID = Lseries.ID;
              WI.PortDestination = 1;
              Wires.append(WI);

              posx-=50;
              //Lower branch
              (i==0) ? PreviousNode = "N0" : PreviousNode = QString("Nlower%1").arg(i-1);

              Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
              Lseries.Type = Inductor;
              Lseries.Orientation = horizontal;
              Lseries.parameter = 0;
              Lseries.val.clear();
              Lseries.val["L"] = num2str(L[i], Inductance);
              Lseries.Coordinates.clear();
              Lseries.Coordinates.push_back(posx);
              Lseries.Coordinates.push_back(75);
              Components.append(Lseries);
              QucsNetlist+=QString("L:L%1 %2 Nlower%3 L=\"%4\"\n")
                      .arg(NumberComponents[Inductor])
                      .arg(PreviousNode)
                      .arg(i)
                      .arg(Lseries.val["L"]);

              posx += 50;
              //Cshunt
              Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
              Cshunt.Type = Capacitor;
              Cshunt.Orientation = vertical;
              Cshunt.parameter = 0;
              Cshunt.val.clear();
              Cshunt.val["C"] = num2str(C_, Capacitance);
              Cshunt.Coordinates.clear();
              Cshunt.Coordinates.push_back(posx);
              Cshunt.Coordinates.push_back(100);
              Components.append(Cshunt);
              QucsNetlist+=QString("C:C%1 Nlower%2 gnd C=\"%3\"\n")
                      .arg(NumberComponents[Capacitor])
                      .arg(i)
                      .arg(Cshunt.val["C"]);

              Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
              Ground.Type = GND;
              Ground.Orientation = vertical;
              Ground.parameter = 0;
              Ground.Coordinates.clear();
              Ground.Coordinates.push_back(posx);
              Ground.Coordinates.push_back(140);
              Ground.val.clear();
              Components.append(Ground);

              //Capacitor to ground
              WI.OriginID = Cshunt.ID;
              WI.PortOrigin = 0;
              WI.DestinationID = Ground.ID;
              WI.PortDestination = 0;
              Wires.append(WI);


              if (i > 0)
              {//Connect the current section to the previous one
                  WI.OriginID = Lseries.ID;
                  WI.PortOrigin = 0;
                  WI.DestinationID = Nlower.ID;
                  WI.PortDestination = 0;
                  Wires.append(WI);
              }
              else
              {//Connect to the common node
                  WI.OriginID = Lseries.ID;
                  WI.PortOrigin = 0;
                  WI.DestinationID = NI.ID;
                  WI.PortDestination = 1;
                  Wires.append(WI);
              }
              //Node
              Nlower.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
              Nlower.Coordinates.clear();
              Nlower.Coordinates.push_back(posx+50);
              Nlower.Coordinates.push_back(75);
              Nodes.append(Nlower);

              //Capacitor to node
              WI.OriginID = Cshunt.ID;
              WI.PortOrigin = 1;
              WI.DestinationID = Nlower.ID;
              WI.PortDestination = 0;
              Wires.append(WI);

              //Inductor to capacitor
              WI.OriginID = Cshunt.ID;
              WI.PortOrigin = 1;
              WI.DestinationID = Lseries.ID;
              WI.PortDestination = 1;
              Wires.append(WI);

        }
        else
        {//Ideal TL
             posx += 50;
            //Upper branch TL
            //1st transmission line
            (i==0) ? PreviousNode = "N1" : PreviousNode = QString("Nupper%1").arg(i-1);
            TL_Upper.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
            TL_Upper.Type = TransmissionLine;
            TL_Upper.Orientation = horizontal;
            TL_Upper.parameter = 0;
            TL_Upper.val.clear();
            TL_Upper.val["Z"] = num2str(Zlines[i], Resistance);
            TL_Upper.val["L"] = ConvertLengthFromM(lambda4);
            TL_Upper.Coordinates.clear();
            TL_Upper.Coordinates.push_back(posx+15);
            TL_Upper.Coordinates.push_back(-50);
            Components.append(TL_Upper);
            QucsNetlist+=QString("TLIN:Line%1 %2 Nupper%3 Z=\"%4\" L=\"%5 m\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                    .arg(NumberComponents[TransmissionLine])
                    .arg(PreviousNode)
                    .arg(i)
                    .arg(TL_Upper.val["Z"])
                    .arg(TL_Upper.val["L"])
                    .arg(Specs.alpha);

            if (i > 0)
            {//Connect the current section to the previous one
                WI.OriginID = TL_Upper.ID;
                WI.PortOrigin = 0;
                WI.DestinationID = Nupper.ID;
                WI.PortDestination = 0;
                Wires.append(WI);
            }

            //Node
            Nupper.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            Nupper.Coordinates.clear();
            Nupper.Coordinates.push_back(posx+50);
            Nupper.Coordinates.push_back(-50);
            Nodes.append(Nupper);

            WI.OriginID = TL_Upper.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = Nupper.ID;
            WI.PortDestination = 0;
            Wires.append(WI);


            //Lower branch TL
            //1st transmission line
            (i==0) ? PreviousNode = "N1" : PreviousNode = QString("Nlower%1").arg(i-1);
            TL_Lower.ID=QString("TLIN%1").arg(++NumberComponents[TransmissionLine]);
            TL_Lower.Type = TransmissionLine;
            TL_Lower.Orientation = horizontal;
            TL_Lower.parameter = 0;
            TL_Lower.val.clear();
            TL_Lower.val["Z"] = num2str(Zlines[i], Resistance);
            TL_Lower.val["L"] = ConvertLengthFromM(lambda4);
            TL_Lower.Coordinates.clear();
            TL_Lower.Coordinates.push_back(posx+15);
            TL_Lower.Coordinates.push_back(50);
            Components.append(TL_Lower);
            QucsNetlist+=QString("TLIN:Line%1 %2 Nlower%3 Z=\"%4\" L=\"%5 mm\" Alpha=\"0 dB\" Temp=\"26.85\"\n")
                    .arg(NumberComponents[TransmissionLine])
                    .arg(PreviousNode)
                    .arg(i)
                    .arg(TL_Lower.val["Z"])
                    .arg(TL_Lower.val["L"])
                    .arg(Specs.alpha);

            if (i > 0)
            {//Connect the current section to the previous one
                WI.OriginID = TL_Lower.ID;
                WI.PortOrigin = 0;
                WI.DestinationID = Nlower.ID;
                WI.PortDestination = 0;
                Wires.append(WI);
            }

            //Node
            Nlower.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
            Nlower.Coordinates.clear();
            Nlower.Coordinates.push_back(posx+50);
            Nlower.Coordinates.push_back(50);
            Nodes.append(Nlower);

            WI.OriginID = TL_Lower.ID;
            WI.PortOrigin = 1;
            WI.DestinationID = Nlower.ID;
            WI.PortDestination = 0;
            Wires.append(WI);

            if (i == 0)
            {//First section, the branches must be connected to the common node
                WI.OriginID = NI.ID;
                WI.PortOrigin = 0;
                WI.DestinationID = TL_Upper.ID;
                WI.PortDestination = 0;
                Wires.append(WI);

                WI.OriginID = NI.ID;
                WI.PortOrigin = 0;
                WI.DestinationID = TL_Lower.ID;
                WI.PortDestination = 0;
                Wires.append(WI);
            }


        }

        posx += 50;
        //Isolation resistor
        struct ComponentInfo Risolation;
        Risolation.ID=QString("R%1").arg(++NumberComponents[Resistor]);
        Risolation.Type = Resistor;
        Risolation.Orientation = vertical;
        Risolation.parameter = 0;
        Risolation.val.clear();
        Risolation.val["R"] = num2str(Risol[i], Resistance);
        Risolation.Coordinates.clear();
        Risolation.Coordinates.push_back(posx);
        Risolation.Coordinates.push_back(0);
        Components.append(Risolation);
        QucsNetlist+=QString("R:R%1 Nupper%2 Nlower%2 R=\"%3\"\n")
                .arg(NumberComponents[Resistor])
                .arg(i)
                .arg(Risolation.val["R"]);

        WI.OriginID = Risolation.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Nupper.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        WI.OriginID = Risolation.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Nlower.ID;
        WI.PortDestination = 0;
        Wires.append(WI);
        Ni+=2;
    }

    posx += 50;


    //Add the output terminals
    //Upper branch term
    TermSpar.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar.Type = Term;
    TermSpar.Orientation = horizontal;
    TermSpar.parameter = 0;
    TermSpar.val.clear();
    TermSpar.val["Z"] = num2str(Specs.Z0, Resistance);
    TermSpar.Coordinates.clear();
    TermSpar.Coordinates.push_back(posx);
    TermSpar.Coordinates.push_back(-posy);
    Components.append(TermSpar);
    QucsNetlist += QString("Pac:P2 Nupper%1 gnd Num=\"2\" Z=\"%2\" P=\"0 dBm\" f=\"1 GHz\"\n")
            .arg(Specs.Nstages-1)
            .arg(TermSpar.val["Z"]);

    WI.OriginID = TermSpar.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Nupper.ID;
    WI.PortDestination = 0;
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
    TermSpar.Coordinates.push_back(posy);
    Components.append(TermSpar);
    QucsNetlist += QString("Pac:P3 Nlower%1 gnd Num=\"3\" Z=\"%2\" P=\"0 dBm\" f=\"1 GHz\"\n")
            .arg(Specs.Nstages-1)
            .arg(TermSpar.val["Z"]);

    WI.OriginID = TermSpar.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Nlower.ID;
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


// This function calculates the isolation resistors given the impedance of the quarter wave lines
std::deque<double> PowerCombinerDesigner::calcMultistageWilkinsonIsolators(std::deque<double> Zlines, double L, std::complex<double> gamma)
{
  int NStages = Specs.Nstages;
  double Z_, R, Zaux = Zlines[NStages-1];
  std::deque<double> Risol;

  for (int i=0; i<NStages;i++)
  {
    Z_ = abs(Zaux*(Specs.Z0 + Zaux*tanh(gamma*L))/(Zaux+Specs.Z0*tanh(gamma*L)));
    Zaux  = Zlines[i];
    R = Specs.Z0*Z_/(Z_ - Specs.Z0);
    Risol.push_front(2*R);
  }
  return Risol;
}
