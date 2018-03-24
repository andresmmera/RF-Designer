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
    void SynthesizeEllipticFilter();
    void InsertEllipticSection(int &, int &, QStringList &, int, bool, bool);

    void Insert_LowpassMinL_Section(int &, int &, QStringList &, int, bool, bool);
    void Insert_HighpassMinC_Section(int &, int &, QStringList &, int, bool, bool);

    void Insert_LowpassMinC_Section(int &, int &, QStringList &, int, bool, bool);
    void Insert_HighpassMinL_Section(int &, int &, QStringList &, int, bool, bool);

    void Insert_Bandpass_1_Section(int &, int &, QStringList &, int, bool, bool);

};
#endif
