#ifndef POWERCOMBINERDESIGNER_H
#define POWERCOMBINERDESIGNER_H
#include "Filtering/Network.h"
#include "general.h"
#include <QPen>

struct TwoWayWilkinsonParams
{
    double Z2;//Branch 2 impedance
    double Z3;//Branch 3 impedance
    double R;//Isolation resistor
    double R2;//Branch 2 terminating impedance
    double R3;//Branch 3 terminating impedance
};


class PowerCombinerDesigner
{
public:
    PowerCombinerDesigner(PowerCombinerParams);
    QList<ComponentInfo> getComponents();
    QList<WireInfo> getWires();
    QList<NodeInfo> getNodes();
    QMap<QString, QPen> displaygraphs;
    void synthesize();
    QString getQucsNetlist(){return QucsNetlist;};

private:
    PowerCombinerParams Specs;
    QList<ComponentInfo> Components;
    QList<WireInfo> Wires;
    QList<NodeInfo> Nodes;

    QString QucsNetlist;
    QMap<ComponentType, int> NumberComponents;//List for assigning IDs to the filter components

    QString ConvertLengthFromM(double);

    //Power combiner design functions
    void Wilkinson();
    void MultistageWilkinson();
    TwoWayWilkinsonParams CalculateWilkinson();
    void TJunction();
    void Branchline();
    void DoubleBoxBranchline();
    void Bagley();
    void Gysel();
    void Lim_Eom();
    void Wilkinson3Way_ImprovedIsolation();
    void TravellingWave();
    void Tree();

    std::deque<double> ChebyshevTaper(double, double);
    std::deque<double> calcMultistageWilkinsonIsolators(std::deque<double>, double, std::complex<double>);

};

#endif // POWERCOMBINERDESIGNER_H
