#ifndef QucsRFDesignerWindow_H
#define QucsRFDesignerWindow_H
#include <QMenuBar>
#include <QMainWindow>
#include <QGridLayout>
#include <QTabWidget>
#include <QDockWidget>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QColor>
#include <QToolBar>
#include <QToolButton>

#include "qcustomplot.h"
#include "Schematic/graphwidget.h"
#include "Schematic/component.h"
#include "UI/QucsRFDesignerWindow.h"
#include "UI/preferencesdialog.h"
#include "Filtering/CanonicalFilter.h"
#include "Filtering/EllipticFilter.h"
#include "Filtering/DirectCoupledFilters.h"
#include "Filtering/Network.h"
#include "SPAR/sparengine.h"
#include "UI/smithchart.h"

#include <QtSql>


struct ToolSettings
{
    QString PathToQucsator;
    double fstart;
    double fstop;
    double Npoints;
    std::vector<bool> ShowTraces;
    std::vector<QColor> TraceColor;
    double xstep;
    double ymin;
    double ymax;
    double ystep;
    bool FixedAxes;
};

class QucsRFDesignerWindow : public QMainWindow
{
    Q_OBJECT
public:
    QucsRFDesignerWindow();
    ~QucsRFDesignerWindow();
    QTabWidget *TabWidget;
private:
    void clear();
    double getScale(QString);

private slots://Functions to launch the actions
    void PreferencesWindow();
    void UpdateDesignParameters();
    void ReceiveSettings(ToolSettings);
    QMap<QString, vector<complex<double> > > loadQucsDataSet(QString);
    void UpdateSparSweep();
    void ResposeComboChanged();
    void EllipticTypeChanged();
    //Zverev mode slots
    void SwitchZverevTablesMode(bool);
    void UpdateRipple(int);//Update the load resistance and the ripple for a given filter order according to the Zverev tables
    void ChangeRL_CLC_LCL_mode();
    void UpdateLoad_Impedance(int);
    void UpdateRL_and_Ripple();
    void ShowSmithChart();

private://Actions
    void createActions();
    void createMenus();
    void UpdateWindows();
    QStringList setItemsResponseTypeCombo();
    QToolBar *RFToolBar;

    QAction *PreferencesAction, *SmithAction;
    bool SmithWindowVisible;
    QString netlist;
    // ************************* Widgets *******************************
    GraphWidget *SchematicWidget;
    QCustomPlot *PlotWidget;

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

    struct FilterSpecifications Filter_SP;//This struct contains the filter specifications given by the user

    SmithChart * Smith_plot;

    // *********************** ZVEREV DATABASE ***************************
    QSqlDatabase db;
    bool DBservice;

    // ************************ SIMULATION SETTINGS ********************
    SP_Analysis SPAR_Settings;
    NetworkInfo NWI;//Synthesized network

    // ************************ TOOL SETTINGS *************************
    ToolSettings Tool_Settings;

    // ************************ UPDATE GRAPH ***************************
    void updateGraph(vector<double>, vector<complex<double> > , vector<complex<double> > , vector<complex<double> > );


};

#endif // QucsRFDesignerWindow_H
