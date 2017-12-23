#ifndef SMITHCHARTTOOL_H
#define SMITHCHARTTOOL_H
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QListWidget>


#include "Filtering/Network.h"

class SmithChartTool : public QWidget
{
    Q_OBJECT
public:
    SmithChartTool();
    QString getQucsNetlist();
    SchematicInfo getSchematic();
private:
    QList<ComponentInfo> Components;
    QList<WireInfo> Wires;
    QList<NodeInfo> Nodes;

    QString QucsNetlist;
    QMap<ComponentType, int> NumberComponents;//List for assigning IDs to the filter components

    QDoubleSpinBox *FC_Spinbox, *SourceImpedanceReal, *SourceImpedanceImag, *LoadImpedanceReal, *LoadImpedanceImag;
    QComboBox *freq_scale_Combobox;
    QPushButton *CsButton, *CpButton, *LsButton, *LpButton, *TLButton,
                *SCStubButton, *OCStubButton, *RemovePartButton;

    QListWidget *ListOfComponents, *ListOfProperties;
    QDoubleSpinBox *PropertyValue;
    QComboBox *PropertyUnits;

    QString netlist;
    SchematicInfo SchInfo;//Schematic representation

    void simulate();

signals:
    void simulateNetwork(struct SchematicInfo);

private slots:
    void addSeriesCapacitor();
    void addShuntCapacitor();
    void addSeriesInductor();
    void addShuntInductor();
    void addTransmissionLine();
    void addOpenCircuitStub();
    void addShortCircuitStub();
    void RemovePart();
    void UpdateComponentPropertiesList();
};

#endif // SMITHCHARTTOOL_H
