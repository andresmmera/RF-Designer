#ifndef NETWORK_H
#define NETWORK_H

#include <QList>
#include <QMap>
#include <QStringList>
#include <vector>
#include <deque>
#include <list>
#include <cmath>
#include <complex>
#include <QFile>

#include <QDebug>
#include <QtSql>

enum ComponentType {Capacitor, Inductor, Term, GND, ConnectionNodes, Resistor, TransmissionLine};
enum ComponentOrientation {vertical, horizontal};
enum ResponseType {Butterworth, Chebyshev, Legendre, Elliptic, Blichinkoff, Bessel, LinearPhaseEqError, Gegenbauer};
enum FilterClass {Lowpass, Highpass, Bandpass, Bandstop};
enum Coupling {CapacitativeCoupledShuntResonators, InductiveCoupledSeriesResonators};


struct PrototypeTableProperties
{
  QString ID;
  std::vector<int> N;
  std::vector<double> Ripple;
  std::vector<double> RL;
};


struct FilterSpecifications
{
   FilterClass FilterType;//Lowpass, Highpass, Bandpass, Bandstop
   ResponseType FilterResponse;//Butterworth, Chebyshev, Cauer, etc...
   QString Implementation;
   Coupling DC_Coupling;//Only for bandpass direct coupled filters
   bool isCLC;//CLC or LCL implementation
   unsigned int order;//Filter order
   double Ripple;//Ripple (Chebyshev and Cauer)
   double fc;//Cutoff frequency (lowpass and highpass filters) or center frequency (bandpass and bandstop)
   double bw;//Filter bandwidth
   double ZS;//Source impedance
   double ZL;//Load impedance
   double as;//Stopband attenuation
   QString EllipticType;//Type of elliptic filter
   bool UseZverevTables;
};



struct ComponentInfo
{
    QString ID;
    unsigned int parameter;
    std::map<QString,double> val;//freq, L1.L, C1.C,...
    QString Net1, Net2;//ID of the nodes where the component is connected
    ComponentOrientation Orientation;
    std::vector<double> Coordinates;
    ComponentType Type;
};

struct WireInfo
{
    QString OriginID;
    QString DestinationID;
    int PortOrigin;
    int PortDestination;
};

struct NodeInfo
{
    QString ID;
    std::vector<double> Coordinates;
};


struct NetworkInfo
{
    std::vector<std::complex<double> > ZS;
    std::vector<std::complex<double> > ZL;
    QStringList topology;
    QList<ComponentInfo> Ladder;//Contains the ID of the components and their properties
};

struct SP_Analysis
{
    unsigned int n_points;
    double fstart;
    double fstop;
    std::vector<double > freq;
};

struct SchematicInfo
{
    struct SP_Analysis SPAR_Settings;
    QString netlist;
    QList<ComponentInfo> Comps;
    QList<WireInfo> Wires;
    QList<NodeInfo> Nodes;
    QMap <QString, QPen>displayGraphs;
};

struct PowerCombinerParams
{
    QString Type;//Wilkinson, branchlines, Bagley, etc.
    int Noutputs;//Number of output branches
    int Nstages;//Number of combiner stages (broadband Wilkinson)
    double OutputRatio;
    QString Implementation;//LC, microstrip, ideal TL
    double alpha;//Attenuation constant of the ideal TL
    QString units;//mm, mil, um
    double freq;//Center freq
    double Z0;//Reference impedance
};

class Network
{
public:
    virtual QList<ComponentInfo> getComponents()=0;
    virtual QList<WireInfo> getWires()=0;
    virtual QList<NodeInfo> getNodes()=0;
    virtual void synthesize()=0;
};

#endif // NETWORK_H
