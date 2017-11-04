#ifndef POWERCOMBININGTOOL_H
#define POWERCOMBININGTOOL_H
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtSql>
#include <QMessageBox>

#include "Filtering/Network.h"
#include "PowerCombining/PowerCombinerDesigner.h"

class PowerCombiningTool : public QWidget
{
    Q_OBJECT
public:
    PowerCombiningTool();
    ~PowerCombiningTool();

private slots:
    void UpdateDesignParameters();
    void on_TopoCombo_currentIndexChanged(int);

private:
    QLabel *OhmLabel, *K1LabeldB, *NStagesLabel, *K1Label, *FreqLabel, *RefImp, *TopoLabel;
    QLabel *AlphaLabel, *AlphadBLabel, *UnitsLabel;
    QDoubleSpinBox *RefImpSpinbox, *FreqSpinbox, *K1Spinbox, *AlphaSpinbox;
    QSpinBox *NStagesSpinbox;
    QComboBox *BranchesCombo;
    QComboBox *TopoCombo, *FreqScaleCombo, *UnitsCombo, *ImplementationCombobox;
    QGroupBox *ImplementationgroupBox;
    QRadioButton *IdealTLradioButton, *MicrostripradioButton, *LumpedElementsradioButton;

    double getScaleFreq();
    QString netlist;
    SchematicInfo SchInfo;//Schematic representation

signals:
    void simulateNetwork(struct SchematicInfo);
};

#endif // POWERCOMBININGTOOL_H
