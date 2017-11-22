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
#include <QValidator>

#include "Filtering/Network.h"
#include "PowerCombining/PowerCombinerDesigner.h"

class BagleyValidator : public QValidator
{
    Q_OBJECT
public:
    BagleyValidator(QObject *parent = 0) : QValidator(parent){};
    virtual State validate ( QString & input, int & pos ) const
    {
        if (input.isEmpty())
            return Acceptable;

        bool b;
        int val = input.toInt(&b);

        if ((b == true) && (val % 2 != 0))
        {
            return Acceptable;
        }
        return Invalid;
    }
};



class PowerCombiningTool : public QWidget
{
    Q_OBJECT
public:
    PowerCombiningTool();
    ~PowerCombiningTool();
    void design();

private slots:
    void UpdateDesignParameters();
    void on_TopoCombo_currentIndexChanged(int);

private:
    QLabel *OhmLabel, *K1LabeldB, *NStagesLabel, *K1Label, *FreqLabel, *RefImp, *TopoLabel;
    QLabel *AlphaLabel, *AlphadBLabel, *UnitsLabel;
    QDoubleSpinBox *RefImpSpinbox, *FreqSpinbox, *AlphaSpinbox;
    QDoubleSpinBox *K1Spinbox,*K2Spinbox,*K3Spinbox;
    QSpinBox *NStagesSpinbox;
    QComboBox *BranchesCombo;
    QComboBox *TopoCombo, *FreqScaleCombo, *UnitsCombo, *ImplementationCombobox;
    QGroupBox *ImplementationgroupBox;
    QRadioButton *IdealTLradioButton, *MicrostripradioButton, *LumpedElementsradioButton;

    double getScaleFreq();
    QString netlist;
    SchematicInfo SchInfo;//Schematic representation

    //Input validation
    QValidator * Bagley_Validator;

signals:
    void simulateNetwork(struct SchematicInfo);
};


#endif // POWERCOMBININGTOOL_H
