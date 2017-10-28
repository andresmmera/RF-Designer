#ifndef FILTERDESIGNTOOL_H
#define FILTERDESIGNTOOL_H
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
#include <QtSql>
#include <QMessageBox>

#include "Filtering/CanonicalFilter.h"
#include "Filtering/EllipticFilter.h"
#include "Filtering/DirectCoupledFilters.h"
#include "Filtering/Network.h"


class FilterDesignTool : public QWidget
{
    Q_OBJECT
public:
    FilterDesignTool();
    ~FilterDesignTool();
    QString getQucsNetlist();
    SchematicInfo getSchematic();
    SP_Analysis SPAR_Settings;//The simulation settings vary depending on the design
    void design();

private slots:
    void UpdateDesignParameters();
    void ResposeComboChanged();
    void EllipticTypeChanged();
    //Zverev mode slots
    void SwitchZverevTablesMode(bool);
    void UpdateRipple(int);//Update the load resistance and the ripple for a given filter order according to the Zverev tables
    void ChangeRL_CLC_LCL_mode();
    void UpdateLoad_Impedance(int);
    void UpdateRL_and_Ripple();

private:
    // ************************** FILTER DESIGN ***************************
    QWidget * SetupFilterDesignGUI();
    QComboBox *FilterClassCombo, *FilterResponseTypeCombo, *FilterImplementationCombo, *FC_ScaleCombobox, *BW_ScaleCombobox;
    QComboBox *EllipticType, *DC_CouplingTypeCombo;
    QDoubleSpinBox *FCSpinbox, *BWSpinbox, *RippleSpinbox,*StopbandAttSpinbox;
    QSpinBox *OrderSpinBox;
    QComboBox *OrderCombobox, *RLCombobox, *PhaseErrorCombobox;//Zverev mode
    QComboBox *RippleCombobox;
    QLineEdit *SourceImpedanceLineEdit;
    QRadioButton *CLCRadioButton, *LCLRadioButton;
    QLabel *StopbandAttLabel, *StopbandAttdBLabel, *EllipticTypeLabel, *RippleLabel, *RippledBLabel, *DC_CouplingLabel;
    QLabel *RLlabel, *RLlabelOhm, *PhaseErrorLabel, *PhaseErrorLabelDeg;//Zverev mode
    QCheckBox *UseZverevTablesCheckBox;
    QStringList DefaultFilterResponses;
    struct FilterSpecifications Filter_SP;//User specifications
    QString netlist;
    SchematicInfo SchInfo;//Schematic representation

    // *********************** ZVEREV DATABASE ***************************
    QSqlDatabase db;
    bool DBservice;

    QStringList setItemsResponseTypeCombo();
    double getScale(QString);//Gets the scale from the comboboxes
    void synthesize();

signals:
    void simulateNetwork(struct SchematicInfo);


};

#endif // FILTERDESIGNTOOL_H
