#ifndef ELLIPTICFILTER_H
#define ELLIPTICFILTER_H
#include "Filtering/Network.h"
#include <QPen>
#include "general.h"

class EllipticFilter : public Network
{
public:
    EllipticFilter();
    EllipticFilter(FilterSpecifications);
    ~EllipticFilter();
    QList<ComponentInfo> getComponents();
    QList<WireInfo> getWires();
    QList<NodeInfo> getNodes();
    void synthesize();
    NetworkInfo getLadder();
    QString getQucsNetlist(){return QucsNetlist;}
    QMap<QString, QPen> displaygraphs;

private:
    struct FilterSpecifications Specification;
    QList<ComponentInfo> Components;
    QList<WireInfo> Wires;
    QList<NodeInfo> Nodes;

    QString QucsNetlist;

    QMap<ComponentType, int> NumberComponents;//List for assigning IDs to the filter components
    std::vector<double> *Cseries_LP, *Lseries_LP, *Cshunt_LP;//Elliptic network parameters
    double RL;//Load resistance
    std::vector<std::complex<double> > Poles;
    std::vector<std::complex<double> > Zeros;

    //Lowpass prototype
    void EllipticTypeS();
    void EllipticTypesABC();
    double Sn(double, double);
    std::complex<double> Sn(double, std::complex<double>);

    //***********  Schematic synthesis ********************
    void SynthesizeLPF();//Lowpass
    void SynthesizeLPF_TypeS_MinL();
    void SynthesizeLPF_TypeS_MinC();
    void SynthesizeLPF_TypesABC_MinL();
    void SynthesizeLPF_TypesABC_MinC();

    void SynthesizeHPF();//Highpass
    void SynthesizeBPF();//Bandpass
    void SynthesizeBSF();//Bandstop
};
#endif
