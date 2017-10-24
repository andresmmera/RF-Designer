#ifndef DIRECTCOUPLEDFILTERS_H
#define DIRECTCOUPLEDFILTERS_H
#include "Filtering/Network.h"
#include "Filtering/LowpassPrototypeCoeffs.h"

class DirectCoupledFilters : public Network
{
public:
    DirectCoupledFilters();
    DirectCoupledFilters(FilterSpecifications);
    QList<ComponentInfo> getComponents();
    QList<WireInfo> getWires();
    QList<NodeInfo> getNodes();
    void synthesize();
    NetworkInfo getLadder();
    QString getQucsNetlist(){return QucsNetlist;};

private:
    struct FilterSpecifications Specification;
    QList<ComponentInfo> Components;
    QList<WireInfo> Wires;
    QList<NodeInfo> Nodes;

    QString QucsNetlist;

    QMap<ComponentType, int> NumberComponents;//List for assigning IDs to the filter components
    std::deque<double> gi;//Lowpass prototype
    std::vector<std::complex<double> > Poles;
    std::vector<std::complex<double> > Zeros;

    void Synthesize_Capacitative_Coupled_Shunt_Resonators();
    void Synthesize_Inductive_Coupled_Series_Resonators();
};


#endif // DIRECTCOUPLED_H