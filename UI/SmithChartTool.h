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
    void SelectComponent(ComponentInfo);
private:
    QList<ComponentInfo> Components;
    QList<WireInfo> Wires;
    QList<NodeInfo> Nodes;

    QString QucsNetlist;
    QMap<ComponentType, int> NumberComponents;//List for assigning IDs to the filter components

    QDoubleSpinBox *FC_Spinbox, *SourceImpedanceReal, *SourceImpedanceImag, *LoadImpedanceReal, *LoadImpedanceImag, *Z0_Spinbox;
    QComboBox *freq_scale_Combobox;
    QPushButton *CsButton, *CpButton, *LsButton, *LpButton, *TLButton,
                *SCStubButton, *OCStubButton, *RemovePartButton;

    QListWidget *ListOfComponents, *ListOfProperties;
    QDoubleSpinBox *PropertyValue;
    QComboBox *PropertyUnits;

    QString netlist;
    SchematicInfo SchInfo;//Schematic representation

    QStringList CapacitanceUnits, InductanceUnits, ResistanceUnits, LengthUnits;
    void simulate();
    void CalcImpedanceTrace();
    double getFrequency();
    QPointF getZS();
    double Z0;//Chart reference impedance

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
    void UpdatePropertyValue();
    void UpdateValueUnits(QString);
};

#endif // SMITHCHARTTOOL_H
