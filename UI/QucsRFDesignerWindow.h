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
#include "SPAR/sparengine.h"
#include "UI/smithchart.h"

//Tools
#include "UI/FilterDesignTool.h"
#include "UI/PowerCombiningTool.h"
#include "UI/InterceptPointsTool.h"

//Math operations
#include "SPAR/MathOperations.h"

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

private slots://Functions to launch the actions
    void PreferencesWindow();
    void ReceiveSettings(ToolSettings);
    QMap<QString, vector<complex<double> > > loadQucsDataSet(QString);
    void ShowSmithChart();
    void ReceiveNetworkFromDesignTools(struct SchematicInfo);//Simulates the network after changing the design goals
    void receiveInterceptDiagramData(struct InterceptPointsData);
    void simulate();//Rerun simulation when the user does some action over the display
    void SimulateSPAR();
    void SimulateInterceptDiagram();
    void SwitchTabs(int);//Whenever the tool tab is changed, this slot forces a new design without the need of modify a design parameter

private://Actions
    void createActions();
    void createMenus();
    void UpdateWindows();
    QToolBar *RFToolBar;

    QAction *PreferencesAction, *SmithAction;
    bool SmithWindowVisible;
    QString netlist;
    // ************************* Widgets *******************************
    GraphWidget *SchematicWidget;
    QVector<QCustomPlot*> DisplayWindow;
    FilterDesignTool *Filter_Tool;//Widget for filter design
    PowerCombiningTool *PowerCombining_Tool;//Widget for power combiner design
    InterceptPointsTool *IP_Tool;

    SmithChart * Smith_plot;

    // ************************** Docks ********************************
    QDockWidget *dock_Schematic, *dock_Setup, *dock_DisplayWindow1, *dock_DisplayWindow2;

    // ************************ SIMULATION SETTINGS ********************
    SP_Analysis SPAR_Settings;
    NetworkInfo NWI;//Synthesized network
    SchematicInfo SchInfo;
    InterceptPointsData IP_data;

    // ************************ TOOL SETTINGS *************************
    ToolSettings Tool_Settings;

    // ************************ UPDATE GRAPH ***************************
    void updateGraph(int, vector<double>, QMap<QString, vector<complex<double> > >);
    void updateGraph(int, vector<double>, QMap<QString, vector<double> >, QMap<QString, QPen>, QString, QString );
    void plotPoints(int, QMap<QString, QPointF>, QString);


};

#endif // QucsRFDesignerWindow_H
