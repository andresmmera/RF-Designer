#include "EllipticFilter.h"

/* Reference:
 *  [1] "Elliptic Approximation and Elliptic Filter Design on Small Computers", Pierre Amstutz,
    IEEE Transactions on Circuits and Systems, vol. CAS-25, No 12, December 1978
*/


EllipticFilter::EllipticFilter()
{
    //Initialize list of components
    NumberComponents[Capacitor] = 0;
    NumberComponents[Inductor] = 0;
    NumberComponents[Term] = 0;
    NumberComponents[GND] = 0;
    NumberComponents[ConnectionNodes] = 0;
    QucsNetlist="";

}


EllipticFilter::~EllipticFilter()
{
    delete Cshunt_LP;
    delete Lseries_LP;
    delete Cseries_LP;
}

EllipticFilter::EllipticFilter(FilterSpecifications FS)
{
    Specification = FS;
    //Initialize list of components
    NumberComponents[Capacitor] = 0;
    NumberComponents[Inductor] = 0;
    NumberComponents[Term] = 0;
    NumberComponents[GND] = 0;
    NumberComponents[ConnectionNodes] = 0;
    QucsNetlist="";
    Cshunt_LP = new std::vector<double>(FS.order+1);
    Lseries_LP = new std::vector<double>(FS.order+1);
    Cseries_LP = new std::vector<double>(FS.order);
}


QList<ComponentInfo> EllipticFilter::getComponents()
{
    return Components;
}


QList<WireInfo> EllipticFilter::getWires()
{
    return Wires;
}

QList<NodeInfo> EllipticFilter::getNodes()
{
    return Nodes;
}


void EllipticFilter::synthesize()
{
    //Ideally, the user should be the one which controls the style of the traces as well the traces to be shown
    //However, in favour of a simpler implementation, it'll be the design code responsible for this... by the moment...
    displaygraphs.clear();
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);

    if (Specification.EllipticType=="Type S")
        EllipticTypeS();
    else
        EllipticTypesABC();
    switch (Specification.FilterType)
    {
    case Lowpass:
        SynthesizeLPF();
        /* case Highpass:
        return SynthesizeHPF();
    case Bandpass:
        return SynthesizeBPF();
    case Bandstop:
        return SynthesizeBSF();*/
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


void EllipticFilter::EllipticTypeS()
{
    double as = Specification.as;//Stopband attenuation
    double ap = Specification.Ripple;//Passband ripple
    int N = Specification.order;//Number of peaks
    double RS = Specification.ZS;//Source impedance
    double fc = Specification.fc;//Cutoff frequency
    double dbn = 0.23025851;//dB -> Np conversion

    int M = 2*N+1;
    double u = (M_PI*M_PI)/(log(16*(exp(as*dbn)-1)/(exp(ap*dbn)-1)));
    double w = (u/(2*M_PI))*log((exp(ap*dbn/2)+1)/(exp(ap*dbn/2)-1));
    //Resize elliptic network parameters
    std::vector<double> E(N), F(M-1);
    E[N-1] = tan(w);
    double a0 = 1/tan(u*(as+log(2))/M_PI);

    //Calculation of the natural frequencies = Sn(M*u, j*u) j \in [1, M-1]
    for (int j=1; j < M; j++) F[j-1] = Sn(M*u, j*u);

    //Calculation of a0 Eqn (4.34)
    double K = 1;
    int j=1;
    double delta = 1, Kaux;
    while (delta > 1e-6) {
        Kaux = K*(pow(tan(w),2) + pow(tanh(j*M*u),2))/(1+pow(tan(w)*tanh(j*M*u),2));
        delta = std::abs(K-Kaux);
        K=Kaux;
        j++;
    }
    a0 = tan(w)*K;
    E[N-1] = a0;

    //Delay group at the natural frequencies
    for (int j = 0; j<N; j++) Cseries_LP->at(j) = F[2*j+1]*(1-pow(F[j],4))/F[j];//Eqn 5.7

    std::vector<double> C(N);
    C[0] = (1/(a0*F[N]));//Starting value for dB/dw calculation
    for (int j = 1; j < N; j++)
    {
        C[j] = (C[j-1]-a0*F[N-j-1])/(1+C[j-1]*a0*F[N-j-1]);
        E[N-j-1] = E[N-j]+E[N-1]*Cseries_LP->at(j-1)/(1+pow(a0*F[j-1],2));//Ej=(Dj/Fj)*dB/dw(1/Fj)
    }

    for (int j = 0; j < N; j++)
    {
        Lseries_LP->at(j) = ((1+pow(C[j],2))*E[j]/Cseries_LP->at(j) - C[j]/F[j])/2.;
        Cshunt_LP->at(j) = C[j]*F[j];
    }
    Lseries_LP->at(N) = Lseries_LP->at(N-1);
    Cshunt_LP->at(N) = Cshunt_LP->at(N-1);
    //Permutations method Eqn (3.6)
    for (int l = 0; l <2; l++)
    {
        for (int k = l+2; k < N+1; k += 2)
        {
            for (int j= l; j<=k-2; j+=2)
            {
                double U = Cshunt_LP->at(j)-Cshunt_LP->at(k);
                double V = 1/(U/((F[k]*F[k]-F[j]*F[j])*Lseries_LP->at(j))-1);
                Cshunt_LP->at(k) = U*V;
                Lseries_LP->at(k) = (V*V)*Lseries_LP->at(k) - (pow(V + 1,2))*Lseries_LP->at(j);
            }
        }
    }

    //Impedance and frequency scaling
    for (int j=0; j<N; j++)
    {
        Cseries_LP->at(j) = Lseries_LP->at(j)*F[j]*F[j];
        Lseries_LP->at(j) = 1/Lseries_LP->at(j);
    }
}

void EllipticFilter::EllipticTypesABC()
{
    double as = Specification.as;//Stopband attenuation
    double ap = Specification.Ripple;//Passband ripple
    int M = Specification.order;//Number of peaks
    double RS = Specification.ZS;//Source impedance
    double fc = Specification.fc;//Cutoff frequency
    double dbn = 0.23025851;//dB -> Np conversion

    int N=2*M;
    double Kc = 1/(2*M_PI*fc*RS);//Scale factor for capacitors
    double Kl = RS/(2*M_PI*fc);//Scale factor for inductors

    double u = M_PI*M_PI/log(16*(exp(as*dbn)-1)/(exp(ap*dbn)-1));
    double W = (u/(2*M_PI))*log((exp(ap*dbn/2)+1)/(exp(ap*dbn/2)-1));

    std::vector<double> E(N);
    std::vector<double> R(M);
    std::vector<double> S(M);
    std::vector<double> B(M+1);
    std::vector<double>F(M+1);
    std::vector<double>D(M+1);

    for (int j = 0; j < N; j++) E[j] = Sn(M*u, (j+1-M)*u/2);

    //Calculation of a0 Eqn (4.34)
    double K = 1;
    int j=1;
    double delta = 1, Kaux, a0;

    while (delta > 1e-6)
    {
        Kaux = K*(pow(tan(W),2) + pow(tanh(j*M*u),2))/(1+pow(tan(W)*tanh(j*M*u),2));
        delta = abs(K-Kaux);
        K=Kaux;
        j=j+1;
    }
    a0 = tan(W)*K;

    std::vector<std::complex<double> > RS_(M);
    std::complex<double> i = std::complex<double>(0, 1);
    //Calculation of the natural frequencies for the Type S
    for (int j = 0; j < M; j++) RS_[j] = i*Sn(M*u, i*W+(M+1-2*(j+1))*u/2);

    for (int i =0; i < M;i++) R[i] = real(RS_[i]), S[i] = imag(RS_[i]);

    double E8, E0;
    int IT;
    if (Specification.EllipticType == "Type A")
    {
        E8 = E[N-1];
        IT = 1;
    }
    else
    {
        IT = 2;
        E8 = -E[0];
    }

    (Specification.EllipticType == "Type C") ? E0 = -E[0] : E0 = E[N-1];

    // The normalized passband and stopband edges for a type S
    // characteristic are given by Eq. 4.15
    double FP = Sn(N*u, N*u/2);//Normalized passband edge. Originally, it is estimated as: sqrt((E(N)+E0)/(1+E(N)*E8));
    double FS = 1/FP;//Normalized stopband edge

    //Calculation of the attenuation peaks
    for (int j = IT; j <= M; j++) D[j-1] = (E[2*j-2]+E8)/(1+E0*E[2*j-2]);


    double SR = 0, TQ = 0, T0 = 0;

    int I = 1;

    for (int i =0; i < M;i++)F[i] = sqrt(1/D[i]);
    for (int j= 0; j<M; j++)
    {
        double W = (a0*a0 + pow(E[2*j],2))/(1+pow(a0*E[2*j], 2));
        double U = sqrt((E0*E0 + 2*E0*S[j]+W)/(1+ 2*E8*S[j]+W*E8*E8));
        double V = ((1+E0*E8)*S[j]+E0+E8*W)/(1+ 2*E8*S[j]+W*E8*E8);
        R[j] = sqrt((U-V)/2);
        S[j] = sqrt((U+V)/2);
        I = -I;
        W = I*R[j]/S[j];
        TQ = (TQ+W)/(1-TQ*W);
        if (Specification.EllipticType == "Type A")
        {
            U=(F[1]-S[j])/R[j];
            V=(F[1]+S[j])/R[j];
            W=I*(V-U)/(1+U*V);
            T0= (T0+W)/(1-T0*W);
        }
        B[0] = B[0]+R[j];
    }

    if (Specification.EllipticType == "Type A")  T0=T0/(1+sqrt(1+T0*T0));

    std::vector<double> DB(M+1,0), TB(M+1,0), C(M+1,0);
    //Calculation of the derivative of the phase at F(K)
    for (int k = IT-1; k<M; k++)
    {
        DB[k]=0;
        TB[k]=T0;
        I=1;
        for (int j=0; j<M; j++)
        {
            DB[k]=DB[k]+ 1/(R[j]+pow(F[k]-S[j], 2)/R[j]);
            DB[k]=DB[k]+ 1/(R[j]+pow(F[k]+S[j], 2)/R[j]);
            I=-I;
            W=(F[k]-I*S[j])/R[j];
            TB[k]=(TB[k]+W)/(1-TB[k]*W);
        }
    }

    D[M]=D[M-1];
    F[M]=F[M-1];
    DB[M]=DB[M-1];
    TB[M]=TB[M-1];

    for (int j = 0; j <M+1-IT; j+=2)
    {
        TB[M-j-1] = -1/TB[M-j-1];
    }


    for (int j = IT-1; j <= M; j++)
    {
        B[j]=(1+pow(TB[j],2))*DB[j]/(4*D[j])-TB[j]*F[j]/2;
        C[j]=TB[j]/F[j];
    }
    RL = RS/W;

    //Permutations method Eq 3.6
    for (int l=0; l < 2; l++)
    {
        for (int k=l+2; k < M+1; k+=2)
        {
            for (int j=l; j <= k-2; j+=2)
            {
                double U = C[j]-C[k];
                double V=1/(U/(B[j]*(D[k]-D[j]))-1);
                C[k]=U*V;
                B[k] = (B[k]-B[j])*V*V-B[j]*(V+V+1);
            }
        }
    }


    if (Specification.EllipticType != "Type C")
        W = pow((1-TQ*T0)/(TQ+T0),2);//Types A and B have RL != RS
    else
        W=1;

    for (int j=0; j < M+1; j+=2)
    {
        B[j]=B[j]*W;
        C[j]=C[j]*W;
    }

    RL = RS/W;//Load resistance

    //Create netlist
    int Ci = 1, Li = 1, Ni = 0;

    if (Specification.EllipticType != "Type A")
    {
        Lseries_LP->at(0) = FP/B[0];
        Li=Li+1;
        Ni = Ni+1;
    }

    double V=0, w, L_, C_;

    for (int j=IT-1; j < M-1; j++)
    {
        V=V*C[j];
        //Calculation of the capacitor of the resonator
        w = F[j]/FP;
        L_ = FP/B[j];
        C_ = 1/(w*w*L_);
        Cseries_LP->at(j) = C_;
        Lseries_LP->at(j) = FP/B[j];
        Cshunt_LP->at(j) = FP*C[j];

        K = K-2;
        Ci = Ci+2;
        Ni = Ni+1;
        Li = Li+1;
    }

    w = F[M-1]/FP;
    L_ = FP/B[M-1];
    C_ = 1/(w*w*L_);
    Cseries_LP->at(M-1) = C_;
    Lseries_LP->at(M-1) = FP/B[M-1];
    Cshunt_LP->at(M-1) = FP*C[M-1];
    Cshunt_LP->at(M) = FP*C[M];
}

std::complex<double> EllipticFilter::Sn(double u, std::complex<double> z)
{
    std::complex<double> x = tanh(z);
    for (int j = 1; j < 10; j++) x = x*(tanh(j*u-z)*tanh(j*u+z));
    return x;
}


double EllipticFilter::Sn(double u, double z)
{
    double x = tanh(z);
    for (int j = 1; j < 10; j++) x = x*(tanh(j*u-z)*tanh(j*u+z));
    return x;
}


void EllipticFilter::SynthesizeLPF()
{
    if (Specification.EllipticType == "Type S")
        (Specification.isCLC) ? SynthesizeLPF_TypeS_MinL() : SynthesizeLPF_TypeS_MinC();
    else
        (Specification.isCLC) ? SynthesizeLPF_TypesABC_MinL() : SynthesizeLPF_TypesABC_MinC();
}




void EllipticFilter::SynthesizeLPF_TypeS_MinL()
{
    ComponentInfo Cshunt, Cseries, Lseries, Ground;
    WireInfo WI;
    NodeInfo NI;
    //Impedance and frequency scaling
    double RS = Specification.ZS;
    double fc = Specification.fc;
    int N = Specification.order;
    for (int j=0; j<N; j++)
    {
        Cshunt_LP->at(j) *= 1/(2*M_PI*fc*RS);
        Cseries_LP->at(j) *= 1/(2*M_PI*fc*RS);
        Lseries_LP->at(j) *= RS/(2*M_PI*fc);
    }
    Cshunt_LP->at(N) *= 1/(2*M_PI*fc*RS);

    //Synthesize CLC of LCL network
    int posx = 0;//Index used for painting. It indicates the current x position
    QStringList ConnectionAux;
    Components.clear();

    //Add Term 1
    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, posx, 0, "N0", "gnd");
    TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
    Components.append(TermSpar1);
    ConnectionAux.append(TermSpar1.ID);
    posx+=50;
    unsigned int Ci = 1, Li = 1, Ni = 0;

    for (int j = 0; j < N; j+=2)
    {
        //Shunt capacitor
        Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                             posx+50, 50, QString("N%1").arg(Ni), "gnd");
        Cshunt.val["C"] = num2str(Cshunt_LP->at(j), Capacitance);
        Components.append(Cshunt);

        //GND
        Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx+50, 100, "", "");
        Components.append(Ground);

        //Node
        NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx+50, 0);
        Nodes.append(NI);

        //********************************************************************************

        posx+=100;//Move position to the right

        //Series inductor
        Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                               posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
        Lseries.val["L"] = num2str(Lseries_LP->at(j), Inductance);
        Components.append(Lseries);

        //Series capacitor
        Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, horizontal, posx, -80,
                          QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
        Cseries.val["C"] = num2str(Cseries_LP->at(j), Capacitance);
        Components.append(Cseries);

        //Wires
        //***** Capacitor to node *****
        WI.setParams(NI.ID, 0, Cshunt.ID, 1);
        Wires.append(WI);

        //***** GND to shunt cap *****
        WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
        Wires.append(WI);

        //***** Inductor to node *****
        WI.setParams(NI.ID, 0, Lseries.ID, 0);
        Wires.append(WI);

        //***** Series cap to node *****
        WI.setParams(NI.ID, 0, Cseries.ID, 0);
        Wires.append(WI);

        //***** Connect components from the previous section *****
        for (int i = 0; i < ConnectionAux.size(); i++)
        {
            WI.setParams(NI.ID, 0, ConnectionAux.at(i), 1);
            Wires.append(WI);
        }
        ConnectionAux.clear();//Remove previous section elements
        // Add components to be wired in the next step
        ConnectionAux.append(Cseries.ID);
        ConnectionAux.append(Lseries.ID);

        Ci+=2;
        Li++;
        Ni++;
    }

    //******************* Central shunt capacitor **************************
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                         posx+50, 50, QString("N%1").arg(Ni), "gnd");
    Cshunt.val["C"] = num2str(Cshunt_LP->at(N), Capacitance);
    Components.append(Cshunt);
    Ci=Ci+1;

    //GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx+50, 100, "", "");
    Components.append(Ground);

    //Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx+50, 0);
    Nodes.append(NI);

    //Wires
    //***** Capacitor to node *****
    WI.setParams(NI.ID, 1, Cshunt.ID, 1);
    Wires.append(WI);

    //***** GND to capacitor *****
    WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
    Wires.append(WI);
    //***********************************************************************

    //***** Connect components from the previous section *****
    for (int i = 0; i < ConnectionAux.size(); i++)
    {
        WI.setParams(NI.ID, 0, ConnectionAux.at(i), 1);
        WI.OriginID = NI.ID;
        Wires.append(WI);
    }
    ConnectionAux.clear();//Remove previous section elements
    // Add components to be wired in the next step


    int M = 2*N+1;
    double l = (0.5*(N+1))*2.;
    int K = M-l-N%2-1;
    posx += 100;
    for (int j = l+2; j <= M;j+=2)
    {
        //************************** Series capacitor **********************************
        Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, horizontal, posx, -80,
                          QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
        Cseries.val["C"] = num2str(Cseries_LP->at(K), Capacitance);
        Components.append(Cseries);

        //*************************** Series inductor *********************************
        Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                               posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
        Lseries.val["L"] = num2str(Lseries_LP->at(K), Inductance);
        Components.append(Lseries);
        posx += 50;

        //****************************** Shunt capacitor**********************************
        Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                             posx, 50, QString("N%1").arg(Ni+1), "gnd");
        Cshunt.val["C"] = num2str(Cshunt_LP->at(K), Capacitance);
        Components.append(Cshunt);

        //GND
        Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx, 100, "", "");
        Components.append(Ground);

        //Node
        NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
        Nodes.append(NI);

        //Wires
        //***** Capacitor to node *****
        WI.setParams(NI.ID, 1, Cshunt.ID, 1);
        Wires.append(WI);

        //***** GND to capacitor *****
        WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
        Wires.append(WI);

        //***** Inductor to node *****
        WI.setParams(NI.ID, 0, Lseries.ID, 1);
        Wires.append(WI);

        //***** Series cap to node *****
        WI.setParams(NI.ID, 0, Cseries.ID, 1);
        Wires.append(WI);

        //***** Connect components to the previous node *****
        WI.setParams(QString("N%1").arg(NumberComponents[ConnectionNodes]-1), 0, Cseries.ID, 0);
        Wires.append(WI);

        WI.setParams(QString("N%1").arg(NumberComponents[ConnectionNodes]-1), 0, Lseries.ID, 0);
        Wires.append(WI);

        K = K-2;
        Ci = Ci+2;
        Ni = Ni+1;
        Li = Li+1;
        posx+=50;
    }
    posx+=50;
    //Add Term 2
    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, posx, 0, QString("N%1").arg(Ni), "gnd");
    TermSpar2.val["Z"] = num2str(Specification.ZS, Resistance);
    Components.append(TermSpar2);

    //Connect last node to the load
    WI.setParams(QString("N%1").arg(NumberComponents[ConnectionNodes]), 0, TermSpar2.ID, 0);
    Wires.append(WI);
}

void EllipticFilter::SynthesizeLPF_TypeS_MinC()
{
    ComponentInfo Cshunt, Lseries, Lshunt, Ground;
    WireInfo WI;
    NodeInfo NI;

    //Impedance and frequency scaling
    double RS = Specification.ZS;
    double fc = Specification.fc;
    int N = Specification.order;
    std::vector<double> Lseries_LP(N+1), Cshunt_LP(N), Lshunt_LP(N);

    //Convert convenitional topology to minimum C
    for (int j=0; j<N; j++)
    {
        Lseries_LP[j] = this->Cshunt_LP->at(j);
        Cshunt_LP[j] = this->Lseries_LP->at(j);
        Lshunt_LP[j] = this->Cseries_LP->at(j);
    }
    Lseries_LP[N] = this->Cshunt_LP->at(N);

    //Impedance and frequency scaling
    for (int j=0; j<N; j++)
    {
        Lseries_LP[j] *= RS/(2*M_PI*fc);
        Cshunt_LP[j] *= 1/(2*M_PI*fc*RS);
        Lshunt_LP[j] *= RS/(2*M_PI*fc);
    }
    Lseries_LP[N] *= RS/(2*M_PI*fc);

    //Synthesize CLC of LCL network
    int posx = 0;//Index used for painting. It indicates the current x position
    QString ConnectionAux;
    Components.clear();

    //Add Term 1
    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, posx, 0, "N0", "gnd");
    TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
    Components.append(TermSpar1);
    ConnectionAux = TermSpar1.ID;

    posx+=50;
    unsigned int Ci = 1, Li = 1, Ni = 0;
    int Kaux = 1;
    for (int j = 0; j < N; j+=2)
    {
        //******************* Series inductance **************************
        //Series inductor
        Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                               posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+Kaux));
        Lseries.val["L"] = num2str(Lseries_LP[j], Inductance);
        Components.append(Lseries);
        Ni+=Kaux;
        Kaux=2;
        posx+=50;//Move position to the right

        //Node
        NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
        Nodes.append(NI);

        //Shunt inductor
        Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, vertical, posx, 50,
                         QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
        Lshunt.val["L"] = num2str(Lshunt_LP[j], Inductance);
        Components.append(Lshunt);

        //Shunt capacitor
        Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                             posx, 110, QString("N%1").arg(Ni+1), "gnd");
        Cshunt.val["C"] = num2str(Cshunt_LP[j], Capacitance);
        Components.append(Cshunt);

        //GND
        Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx, 160, "", "");
        Components.append(Ground);

        //Wires
        //***** Inductor to node *****
        WI.setParams(NI.ID, 0, Lseries.ID, 1);
        Wires.append(WI);

        //***** Inductor to node *****
        WI.setParams(NI.ID, 0, Lshunt.ID, 1);
        Wires.append(WI);

        //***** Shunt cap to inductor *****
        WI.setParams(Lshunt.ID, 0, Cshunt.ID, 1);
        Wires.append(WI);

        //***** GND to shunt cap *****
        WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
        Wires.append(WI);

        //***** Connect components from the previous section *****
        if (!ConnectionAux.isEmpty())
        {
            WI.setParams(Lseries.ID, 0, ConnectionAux, 1);
            Wires.append(WI);
        }
        ConnectionAux.clear();//Remove previous section elements
        // Add components to be wired in the next step
        ConnectionAux = NI.ID;

        Ci+=2;
        Li++;
        posx += 50;
    }

    //******************* Central series inductance **************************
    //Series inductor
    Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                           posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+2));
    Lseries.val["L"] = num2str(Lseries_LP[N], Inductance);
    Components.append(Lseries);
    Ni+=2;
    posx+= 50;
    //Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
    Nodes.append(NI);

    //Wires
    //***** Node to series inductor *****
    WI.setParams(ConnectionAux, 1, Lseries.ID, 0);
    Wires.append(WI);

    //***** Inductor to node *****
    WI.setParams(Lseries.ID, 1, NI.ID, 0);
    Wires.append(WI);

    ConnectionAux = NI.ID;
    //***********************************************************************

    int M = 2*N+1;
    double l = (0.5*(N+1))*2.;
    int K = M-l-N%2-1;
    for (int j = l+2; j <= M;j+=2)
    {
        //*************************** Shunt inductor *********************************
        //Shunt inductor
        Lshunt.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, vertical, posx, 50,
                         QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
        Lshunt.val["L"] = num2str(Lshunt_LP[K], Inductance);
        Components.append(Lshunt);

        //************************** Shunt capacitor **********************************
        Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                             posx, 110, QString("N%1").arg(Ni+1), "gnd");
        Cshunt.val["C"] = num2str(Cshunt_LP[K], Capacitance);
        Components.append(Cshunt);

        //GND
        Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx, 160, "", "");
        Components.append(Ground);

        posx += 50;
        //****************************** Series inductor **********************************
        Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                               posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+2));
        Lseries.val["L"] = num2str(Lseries_LP[K], Inductance);
        Components.append(Lseries);

        //Node
        NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx+50, 0);
        Nodes.append(NI);

        //Wires
        //***** Node to inductor *****
        WI.setParams(ConnectionAux, 1, Lshunt.ID, 1);
        Wires.append(WI);

        //***** Inductor to capacitor *****
        WI.setParams(Lshunt.ID, 0, Cshunt.ID, 1);
        Wires.append(WI);

        //***** Capacitor to GND *****
        WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
        Wires.append(WI);

        //***** Node to series inductor *****
        WI.setParams(ConnectionAux, 1, Lseries.ID, 0);
        Wires.append(WI);

        //***** Series inductor to new node *****
        WI.setParams(Lseries.ID, 1, NI.ID, 0);
        Wires.append(WI);

        ConnectionAux = NI.ID;

        K = K-2;
        Ci = Ci+2;
        Li = Li+1;
        Ni+=2;
        posx+=50;
    }
    posx+=50;
    //Add Term 2
    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, posx, 0, QString("N%1").arg(Ni), "gnd");
    TermSpar2.val["Z"] = num2str(Specification.ZS, Resistance);
    Components.append(TermSpar2);

    //Connect last node to the load
    WI.setParams(QString("N%1").arg(NumberComponents[ConnectionNodes]), 0, TermSpar2.ID, 0);
    Wires.append(WI);
}


void EllipticFilter::SynthesizeLPF_TypesABC_MinL()
{
    double RS = Specification.ZS;
    double fc = Specification.fc;
    int M = Specification.order;
    double Kc = 1/(2*M_PI*fc*RS);//Scale factor for capacitors
    double Kl = RS/(2*M_PI*fc);//Scale factor for inductors
    int posx = 0, posT2;
    int Kcell = 100;//Elliptic cell width in the schematic representation
    int index_beginning = Kcell;
    int index_end = (M)*Kcell;
    //Create netlist
    int Ci = 1, Li = 1, Ni = 0;
    QucsNetlist="";
    QStringList ConnectionAux, ConnectionAuxR, ConnectionAuxL;
    Components.clear();

    //Add Term 1

    ComponentInfo TermSpar1(QString("T%1").arg(++NumberComponents[Term]), Term, vertical, posx, 0, "N1", "gnd");
    TermSpar1.val["Z"] = num2str(Specification.ZS, Resistance);
    Components.append(TermSpar1);

    //********************** Term 2 ***************************
    if (Specification.EllipticType != "Type A")
    {//RS != RL
        posT2=(M+0.5)*Kcell;
    }
    else
    {
        posT2=(M+1)*Kcell;
    }
    //Add Term 2
    ComponentInfo TermSpar2(QString("T%1").arg(++NumberComponents[Term]), Term, horizontal, posT2, 0, "N0", "gnd");
    TermSpar2.val["Z"] = num2str(RL, Resistance);
    Components.append(TermSpar2);


    ComponentInfo Cshunt;
    ComponentInfo Cseries;
    ComponentInfo Lseries, Cshunt_odd;
    ComponentInfo Ground;
    NodeInfo NI;
    WireInfo WI;

    int IT=1;
    if (Specification.EllipticType != "Type A")
    {
        Lseries_LP->at(0) *= Kl;
        posx = index_end;
        //Series inductor
        Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                             posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+2));
        Lseries.val["L"] = num2str(Lseries_LP->at(0), Inductance);
        Components.append(Lseries);

        //Wire inductor to the output port
        WI.setParams(TermSpar2.ID, 0, Lseries.ID, 1);
        Wires.append(WI);

        ConnectionAuxR.append(Lseries.ID);

        index_end-=Kcell;
        Li++;
        Ni++;
        IT = 2;
    }

    int sign;
    for (int j=IT-1; j < M-1; j++)
    {
        (j % 2 == 0) ? posx = index_end : posx = index_beginning;

        Cseries_LP->at(j) *= Kc;
        Lseries_LP->at(j) *= Kl;
        Cshunt_LP->at(j) *= Kc;

        (j % 2 == 0) ? sign = 1 : sign = -1;

        //Shunt capacitor
        Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                             posx+sign*0.5*Kcell, 50, QString("N%1").arg(Ni), "gnd");
        Cshunt.val["C"] = num2str(Cshunt_LP->at(j), Capacitance);
        Components.append(Cshunt);

        //GND
        Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx+sign*0.5*Kcell, 100, "", "");
        Components.append(Ground);

        //Node
        NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx+sign*0.5*Kcell, 0);
        Nodes.append(NI);

        //Series inductor
        Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                             posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+2));
        Lseries.val["L"] = num2str(Lseries_LP->at(j), Inductance);
        Components.append(Lseries);

        //Series capacitor
        Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, horizontal, posx, -80,
                          QString("N%1").arg(Ni), QString("N%1").arg(Ni+2));
        Cseries.val["C"] = num2str(Cseries_LP->at(j), Capacitance);
        Components.append(Cseries);

        //Wiring
        //***** Shunt capacitor to node *****
        WI.setParams(Cshunt.ID, 1, NI.ID, 0);
        Wires.append(WI);

        //***** Shunt capacitor to GND *****
        WI.setParams(Cshunt.ID, 0, Ground.ID, 0);
        Wires.append(WI);

        //***** Series capacitor to node *****
        WI.setParams(NI.ID, 0, Cseries.ID, (sign == 1));
        Wires.append(WI);

        //***** Series inductor to node *****
        WI.setParams(Lseries.ID, (sign == 1), NI.ID, 0);
        Wires.append(WI);

        //Wire elements from previous sections
        if (j % 2 == 0)
        {
            for (int i = 0; i < ConnectionAuxR.size(); i++)
            {
                WI.setParams(NI.ID, 0, ConnectionAuxR.at(i), 0);
                Wires.append(WI);
            }
            ConnectionAuxR.clear();
            ConnectionAuxR.append(Lseries.ID);
            ConnectionAuxR.append(Cseries.ID);
        }
        else
        {
            for (int i = 0; i < ConnectionAuxL.size(); i++)
            {
                WI.setParams(NI.ID, 0, ConnectionAuxL.at(i), 1);
                Wires.append(WI);
            }
            ConnectionAuxL.clear();
            ConnectionAuxL.append(Lseries.ID);
            ConnectionAuxL.append(Cseries.ID);
        }

        //Wiring output port
        if ((Specification.EllipticType == "Type A") && (j == IT-1))
        {
            WI.setParams(TermSpar2.ID, 0, NI.ID, 1);
            Wires.append(WI);
        }

        //Wiring input port, N>2
        if (((Specification.EllipticType == "Type A") &&(j == IT) || ((Specification.EllipticType != "Type A") && (j == IT-1))))
        {
            WI.setParams(TermSpar1.ID, 0, NI.ID, 1);
            Wires.append(WI);
        }
        Ci = Ci+2;
        Ni = Ni+1;
        Li = Li+1;
        (j % 2 == 0) ? index_end-=Kcell : index_beginning+=Kcell;
    }
    (Specification.order > 1) ? posx=index_end : posx=posT2-1.5*Kcell;
    Cseries_LP->at(M-1) *= Kc;
    Lseries_LP->at(M-1) *= Kl;
    Cshunt_LP->at(M-1) *= Kc;
    Cshunt_LP->at(M) *= Kc;

    //Central section
    //Shunt capacitor
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                         posx+0.5*Kcell, 50, QString("N%1").arg(Ni), QString("gnd"));
    Cshunt.val["C"] = num2str(Cshunt_LP->at(M-1), Capacitance);
    Components.append(Cshunt);

    //GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx+0.5*Kcell, 100, "", "");
    Components.append(Ground);

    //Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx+0.5*Kcell, 0);
    Nodes.append(NI);

    //Wire central section to the next section on the right
    for (int i = 0; i < ConnectionAuxR.size(); i++)
    {
        WI.setParams(NI.ID, 0, ConnectionAuxR.at(i), 0);
        Wires.append(WI);
    }

    //***** Shunt capacitor to node *****
    WI.setParams(NI.ID, 0, Cshunt.ID, 1);
    Wires.append(WI);

    //***** Shunt capacitor to GND *****
    WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
    Wires.append(WI);

    if (Specification.order == 1)
    {//Wire the output port
        WI.setParams(TermSpar2.ID, 0, NI.ID, 0);
        Wires.append(WI);
    }


    //Series inductor
    Lseries.setParams(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal,
                         posx, 0, QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
    Lseries.val["L"] = num2str(Lseries_LP->at(M-1), Inductance);
    Components.append(Lseries);

    //Series capacitor
    Cseries.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, horizontal, posx, -80,
                      QString("N%1").arg(Ni), QString("N%1").arg(Ni+1));
    Cseries.val["C"] = num2str(Cseries_LP->at(M-1), Capacitance);
    Components.append(Cseries);


    //Central shunt capacitor
    Cshunt.setParams(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical,
                         posx-0.5*Kcell, 50, QString("N%1").arg(M), QString("gnd"));
    Cshunt.val["C"] = num2str(Cshunt_LP->at(M), Capacitance);
    Components.append(Cshunt);

    //GND
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, posx-0.5*Kcell, 100, "", "");
    Components.append(Ground);

    //Wiring

    //***** Series capacitor to node *****
    WI.setParams(NI.ID, 0, Cseries.ID, 1);
    Wires.append(WI);

    //***** Series inductor to node *****
    WI.setParams(Lseries.ID, 1, NI.ID, 0);
    Wires.append(WI);

    //Node
    NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx-0.5*Kcell, 0);
    Nodes.append(NI);

    //***** Shunt capacitor to node *****
    WI.setParams(NI.ID, 0, Cshunt.ID, 1);
    Wires.append(WI);

    //***** Shunt capacitor to GND *****
    WI.setParams(Ground.ID, 0, Cshunt.ID, 0);
    Wires.append(WI);

    //***** Series capacitor to node *****
    WI.setParams(NI.ID, 0, Cseries.ID, 0);
    Wires.append(WI);

    //***** Series inductor to node *****
    WI.setParams(Lseries.ID, 0, NI.ID, 0);
    Wires.append(WI);

    //Wire previous section (on the left)
    for (int i = 0; i < ConnectionAuxL.size(); i++)
    {
        WI.setParams(NI.ID, 0, ConnectionAuxL.at(i), 1);
        Wires.append(WI);
    }

    if (Specification.order <= 2)
    {//Wire the input port
        WI.setParams(TermSpar1.ID, 0, NI.ID, 0);
        Wires.append(WI);
    }
}

void EllipticFilter::SynthesizeLPF_TypesABC_MinC()
{
    double RS = Specification.ZS;
    double fc = Specification.fc;
    int N = Specification.order;

    std::vector<double> Lseries_LP(N+1), Cshunt_LP(N+1), Lshunt_LP(N);

    //Convert convenitional topology to minimum C
    for (int j=0; j<N; j++)
    {
        Lseries_LP[j] = this->Cshunt_LP->at(j);
        Cshunt_LP[j] = this->Lseries_LP->at(j);
        Lshunt_LP[j] = this->Cseries_LP->at(j);
    }
    Lseries_LP[N] = this->Cshunt_LP->at(N);
    Cshunt_LP[N] = this->Lseries_LP->at(N);

    //Impedance and frequency scaling
    for (int j=0; j<N; j++)
    {
        Lseries_LP[j] *= RS/(2*M_PI*fc);
        Cshunt_LP[j] *= 1/(2*M_PI*fc*RS);
        Lshunt_LP[j] *= RS/(2*M_PI*fc);
    }
    Cshunt_LP[N] *= 1/(2*M_PI*fc*RS);
    Lseries_LP[N] *= RS/(2*M_PI*fc);


    int posx = 0, posT2;
    int Kcell = 100;//Elliptic cell width in the schematic representation
    int index_beginning = Kcell;
    int index_end = N*Kcell;
    //Create netlist
    int Ci = 1, Li = 1, Ni = 0;
    QucsNetlist="";
    QStringList ConnectionAux, ConnectionAuxR, ConnectionAuxL;
    Components.clear();

    //Add Term 1
    struct ComponentInfo TermSpar1, TermSpar2;
    TermSpar1.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar1.Type = Term;
    TermSpar1.Orientation = vertical;
    TermSpar1.parameter = 0;
    TermSpar1.val.clear();
    TermSpar1.val["Z"] = Specification.ZS;
    TermSpar1.Coordinates.clear();
    TermSpar1.Coordinates.push_back(posx);
    TermSpar1.Coordinates.push_back(0);
    Components.append(TermSpar1);
    ConnectionAux.append(TermSpar1.ID);
    QucsNetlist = QString("Pac:P2 _NL1 gnd Num=1 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specification.ZS);

    //********************** Term 2 ***************************
    if (Specification.EllipticType != "Type A")
    {//RS != RL
        posT2=(N+0.5)*Kcell;
    }
    else
    {
        posT2=(N+1)*Kcell;
    }
    //Add Term 2
    TermSpar2.ID=QString("T%1").arg(++NumberComponents[Term]);
    TermSpar2.Type = Term;
    TermSpar2.Orientation = horizontal;
    TermSpar2.parameter = 0;
    TermSpar2.val.clear();
    TermSpar2.val["Z"] = Specification.ZS*Specification.ZS/RL;
    TermSpar2.Coordinates.clear();
    TermSpar2.Coordinates.push_back(posT2);
    TermSpar2.Coordinates.push_back(0);
    Components.append(TermSpar2);
    QucsNetlist += QString("Pac:P1 _NR1 gnd Num=2 Z=\"%1 Ohm\" P=\"0 dBm\" f=\"1 GHz\"\n").arg(Specification.ZS*Specification.ZS/RL);
    //*************************************************************

    struct ComponentInfo Cshunt;
    struct ComponentInfo Lshunt;
    struct ComponentInfo Lseries, Cshunt_odd;
    struct ComponentInfo Ground;
    struct NodeInfo NI;
    struct WireInfo WI;

    int IT=1;
    Ni=1;
    if (Specification.EllipticType != "Type A")
    {
        posx = index_end;
        //*************************** Shunt capacitor *********************************
        Cshunt_odd.ID=QString("L%1").arg(++NumberComponents[Capacitor]);
        Cshunt_odd.Type = Capacitor;
        Cshunt_odd.Orientation = vertical;
        Cshunt_odd.parameter = 0;
        Cshunt_odd.val.clear();
        Cshunt_odd.val["C"] = Lseries_LP.at(0);
        Cshunt_odd.Coordinates.clear();
        Cshunt_odd.Coordinates.push_back(posx);
        Cshunt_odd.Coordinates.push_back(90);
        Components.append(Cshunt_odd);
        QucsNetlist+=QString("C:C%1 N1 gnd L=\"%3 H\"\n").arg(Ci).arg(Cshunt_LP.at(0));
        //**********************************************************************************

        //GND
        Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
        Ground.Type = GND;
        Ground.Orientation = vertical;
        Ground.parameter = 0;
        Ground.Coordinates.clear();
        Ground.Coordinates.push_back(posx);
        Ground.Coordinates.push_back(130);
        Ground.val.clear();
        Components.append(Ground);

        //Wire inductor to the output port
        WI.OriginID = Cshunt_odd.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ground.ID;
        WI.PortDestination = 1;
        Wires.append(WI);

        index_end-=Kcell;
        Ci++;
        IT = 2;
    }

int NLi=1, NRi=1;
    for (int j=IT-1; j < N-1; j++)
    {
        (j % 2 == 0) ? posx = index_end : posx = index_beginning;

        //******************* Series inductor **************************
        Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
        Lseries.Type = Inductor;
        Lseries.Orientation = horizontal;
        Lseries.parameter = 0;
        Lseries.val.clear();
        Lseries.val["L"] = Lseries_LP.at(j);
        Lseries.Coordinates.clear();
        //Put the series inductor in the right or in the left side of the node depending on the iteration index...
        int sign;
        (j % 2 == 0) ? sign = 1: sign = -1;
        Lseries.Coordinates.push_back(posx+sign*0.5*Kcell);
        Lseries.Coordinates.push_back(0);
        Components.append(Lseries);

        //Node
        NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
        NI.Coordinates.clear();
        NI.Coordinates.push_back(posx);
        NI.Coordinates.push_back(0);
        Nodes.append(NI);

        //********************************************************************************
        //*************************** Shunt inductor *********************************
        Lshunt.ID=QString("L%1").arg(++NumberComponents[Inductor]);
        Lshunt.Type = Inductor;
        Lshunt.Orientation = vertical;
        Lshunt.parameter = 0;
        Lshunt.val.clear();
        Lshunt.val["L"] = Lshunt_LP.at(j);
        Lshunt.Coordinates.clear();
        Lshunt.Coordinates.push_back(posx);
        Lshunt.Coordinates.push_back(30);
        Components.append(Lshunt);
        //**********************************************************************************

        //************************** Series capacitor **********************************
        Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
        Cshunt.Type = Capacitor;
        Cshunt.Orientation = vertical;
        Cshunt.parameter = 0;
        Cshunt.val.clear();
        Cshunt.val["C"] = Cshunt_LP.at(j);
        Cshunt.Coordinates.clear();
        Cshunt.Coordinates.push_back(posx);
        Cshunt.Coordinates.push_back(90);
        Components.append(Cshunt);
        //******************************************************************************

        //GND
        Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
        Ground.Type = GND;
        Ground.Orientation = vertical;
        Ground.parameter = 0;
        Ground.Coordinates.clear();
        Ground.Coordinates.push_back(posx);
        Ground.Coordinates.push_back(130);
        Ground.val.clear();
        Components.append(Ground);

        //Wiring
        //***** Shunt inductance to node *****
        WI.OriginID = Lshunt.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = NI.ID;
        WI.PortDestination = 1;
        Wires.append(WI);
        //***** Shunt capacitor to GND *****
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = Ground.ID;
        WI.PortDestination = 1;
        Wires.append(WI);
        //***** Series inductor to node *****
        WI.OriginID = Lseries.ID;
        WI.PortOrigin = (sign != 1);
        WI.DestinationID = NI.ID;
        WI.PortDestination = 1;
        Wires.append(WI);
        //***** Shunt capacitor to shunt inductance *****
        WI.OriginID = Cshunt.ID;
        WI.PortOrigin = 1;
        WI.DestinationID = Lshunt.ID;
        WI.PortDestination = 0;
        Wires.append(WI);

        //Wire elements from previous sections
        if (j % 2 == 0)
        {
            for (int i = 0; i < ConnectionAuxR.size(); i++)
            {
                WI.OriginID = Lseries.ID;
                WI.PortOrigin = 1;
                WI.DestinationID = ConnectionAuxR.at(i);
                WI.PortDestination = 1;
                Wires.append(WI);
            }
            ConnectionAuxR.clear();
            ConnectionAuxR.append(NI.ID);
        }
        else
        {
            for (int i = 0; i < ConnectionAuxL.size(); i++)
            {
                WI.OriginID = Lseries.ID;
                WI.PortOrigin = 0;
                WI.DestinationID = ConnectionAuxL.at(i);
                WI.PortDestination = 1;
                Wires.append(WI);
            }
            ConnectionAuxL.clear();
            ConnectionAuxL.append(NI.ID);
        }

     /*   //Wiring output port
        if ((Specification.EllipticType == "Type A") && (j == IT-1))
        {
            WI.OriginID = TermSpar2.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = NI.ID;
            WI.PortDestination = 1;
            Wires.append(WI);
        }

        if ((Specification.EllipticType != "Type A") && (j == IT))
        {
            WI.OriginID = Cshunt_odd.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = NI.ID;
            WI.PortDestination = 1;
            Wires.append(WI);
        }

        //Wiring input port, N>2
        if (((Specification.EllipticType == "Type A") &&(j == IT) || ((Specification.EllipticType != "Type A") && (j == IT-1))))
        {
            WI.OriginID = TermSpar1.ID;
            WI.PortOrigin = 0;
            WI.DestinationID = NI.ID;
            WI.PortDestination = 1;
            Wires.append(WI);
        }*/


        if (j % 2 == 0)
        {//Right
           QucsNetlist+=QString("L:L%1 _NR%2 _NR%3 L=\"%4 H\"\n").arg(Li).arg(NRi+2).arg(NRi).arg(Lseries_LP.at(j));
           QucsNetlist+=QString("L:L%1 _NR%2 _NR%3 L=\"%4 H\"\n").arg(Li+1).arg(NRi+1).arg(NRi+2).arg(Lshunt_LP.at(j));
           QucsNetlist+=QString("C:C%1 _NR%2 gnd C=\"%3 F\"\n").arg(Ci).arg(NRi+1).arg(Cshunt_LP.at(j));
           NRi+=2;
        }
        else
        {//Left
            QucsNetlist+=QString("L:L%1 _NL%2 _NL%3 L=\"%4 H\"\n").arg(Li).arg(NLi+2).arg(NLi).arg(Lseries_LP.at(j));
            QucsNetlist+=QString("L:L%1 _NL%2 _NL%3 L=\"%4 H\"\n").arg(Li+1).arg(NLi+1).arg(NLi+2).arg(Lshunt_LP.at(j));
            QucsNetlist+=QString("C:C%1 _NL%2 gnd C=\"%3 F\"\n").arg(Ci).arg(NLi+1).arg(Cshunt_LP.at(j));
            NLi+=2;
        }
        Ci++;
        Li +=2;
        (j % 2 == 0) ? index_end-=Kcell : index_beginning+=Kcell;
    }
    (Specification.order > 1) ? posx=index_end : posx=posT2-1.5*Kcell;
    int index = 1;
    //Central section
    //******************* Central series inductor **************************
    Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
    Lseries.Type = Inductor;
    Lseries.Orientation = horizontal;
    Lseries.parameter = 0;
    Lseries.val.clear();
    if (Specification.order%2==0) index = 0;
    Lseries.val["L"] = Lseries_LP.at(N-index);
    Lseries.Coordinates.clear();
    Lseries.Coordinates.push_back(posx+0.5*Kcell);
    Lseries.Coordinates.push_back(0);
    Components.append(Lseries);
    QucsNetlist+=QString("L:L%1 _N1 _NR%2 L=\"%3 H\"\n").arg(Li).arg(NRi).arg(Lseries_LP.at(N-index));


    //***** Series inductor to previous right section *****
    WI.OriginID = Lseries.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = NI.ID;
    WI.PortDestination = 0;
    Wires.append(WI);


 /*   //Wire central section to the next section on the right
    for (int i = 0; i < ConnectionAuxR.size(); i++)
    {
        WI.OriginID = NI.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = ConnectionAuxR.at(i);
        WI.PortDestination = 0;
        Wires.append(WI);
    }

    if ((Specification.EllipticType != "Type A") && (Specification.order <= 3))
    {
        WI.OriginID = Cshunt_odd.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = NI.ID;
        WI.PortDestination = 1;
        Wires.append(WI);
    }

    //***** Shunt capacitor to node *****
    WI.OriginID = NI.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = Cshunt.ID;;
    WI.PortDestination = 1;
    Wires.append(WI);

    //***** Shunt capacitor to GND *****
    WI.OriginID = Ground.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = Cshunt.ID;;
    WI.PortDestination = 0  ;
    Wires.append(WI);

    if (Specification.order == 1)
    {//Wire the output port
        WI.OriginID = TermSpar2.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = NI.ID;
        WI.PortDestination = 1;
        Wires.append(WI);
    }
*/

    //Node
    NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    NI.Coordinates.clear();
    NI.Coordinates.push_back(posx);
    NI.Coordinates.push_back(0);
    Nodes.append(NI);

    //***** Series inductor to previous right section *****
    WI.OriginID = Lseries.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = NI.ID;
    WI.PortDestination = 0;
    Wires.append(WI);

   //********************************************************************************
    //*************************** First series inductor *********************************
    Lseries.ID=QString("L%1").arg(++NumberComponents[Inductor]);
    Lseries.Type = Inductor;
    Lseries.Orientation = horizontal;
    Lseries.parameter = 0;
    Lseries.val.clear();
    if (Specification.order%2==0) index = 1;
    else index = 0;
    Lseries.val["L"] = Lseries_LP.at(N-index);
    Lseries.Coordinates.clear();
    Lseries.Coordinates.push_back(posx-Kcell*0.5);
    Lseries.Coordinates.push_back(0);
    Components.append(Lseries);
    QucsNetlist+=QString("L:L%1 _NL%2 _N1 L=\"%3 H\"\n").arg(Li+1).arg(NLi).arg(Lseries_LP.at(N-index));
    //**********************************************************************************




    //************************** Shunt inductor **********************************
    Lshunt.ID=QString("L%1").arg(++NumberComponents[Inductor]);
    Lshunt.Type = Inductor;
    Lshunt.Orientation = vertical;
    Lshunt.parameter = 0;
    Lshunt.val.clear();
    Lshunt.val["L"] = Lshunt_LP.at(N-1);
    Lshunt.Coordinates.clear();
    Lshunt.Coordinates.push_back(posx);
    Lshunt.Coordinates.push_back(30);
    Components.append(Lshunt);
    QucsNetlist+=QString("L:L%1 _N1 _N2 L=\"%2 H\"\n").arg(Li+2).arg(Lshunt_LP.at(N-1));
    //******************************************************************************

    //******************* Shunt capacitor **************************
    Cshunt.ID=QString("C%1").arg(++NumberComponents[Capacitor]);
    Cshunt.Type = Capacitor;
    Cshunt.Orientation = vertical;
    Cshunt.parameter = 0;
    Cshunt.val.clear();
    Cshunt.val["C"] = Cshunt_LP.at(N-1);
    Cshunt.Coordinates.clear();
    Cshunt.Coordinates.push_back(posx);
    Cshunt.Coordinates.push_back(90);
    Components.append(Cshunt);
    QucsNetlist += QString("C:C%1 _N2 gnd C=\"%3 F\"\n").arg(Ci).arg(Cshunt_LP.at(N-1));
    //GND
    Ground.ID=QString("GND%1").arg(++NumberComponents[GND]);
    Ground.Type = GND;
    Ground.Orientation = vertical;
    Ground.parameter = 0;
    Ground.Coordinates.clear();
    Ground.Coordinates.push_back(posx);
    Ground.Coordinates.push_back(130);
    Ground.val.clear();
    Components.append(Ground);

    //Wiring

    //***** Series inductor to node *****
    WI.OriginID = NI.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = Lseries.ID;;
    WI.PortDestination = 1;
    Wires.append(WI);
    //***** Shunt inductor to node *****
    WI.OriginID = Lshunt.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = NI.ID;
    WI.PortDestination = 1;
    Wires.append(WI);

   /* //Node
    NI.ID = QString("N%1").arg(++NumberComponents[ConnectionNodes]);
    NI.Coordinates.clear();
    NI.Coordinates.push_back(posx-0.5*Kcell);
    NI.Coordinates.push_back(0);
    Nodes.append(NI);*/

    //***** Shunt indutor to shunt capacitor *****
    WI.OriginID = Lshunt.ID;
    WI.PortOrigin = 0;
    WI.DestinationID = Cshunt.ID;;
    WI.PortDestination = 1;
    Wires.append(WI);

    //***** Shunt capacitor to GND *****
    WI.OriginID = Ground.ID;
    WI.PortOrigin = 1;
    WI.DestinationID = Cshunt.ID;;
    WI.PortDestination = 0  ;
    Wires.append(WI);

  /*  //Wire previous section (on the left)
    for (int i = 0; i < ConnectionAuxL.size(); i++)
    {
        WI.OriginID = NI.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = ConnectionAuxL.at(i);
        WI.PortDestination = 1;
        Wires.append(WI);
    }

    if (Specification.order <= 2)
    {//Wire the input port
        WI.OriginID = TermSpar1.ID;
        WI.PortOrigin = 0;
        WI.DestinationID = NI.ID;
        WI.PortDestination = 1;
        Wires.append(WI);
    }*/
}
