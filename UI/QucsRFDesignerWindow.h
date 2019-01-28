/***************************************************************************
                                QucsRFDesignerWindow.h
                                ----------
    copyright            :  QUCS team
    author                :  2019 Andres Martinez-Mera
    email                  :  andresmmera@protonmail.com
 ***************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QucsRFDesignerWindow_H
#define QucsRFDesignerWindow_H
#include <QColor>
#include <QComboBox>
#include <QDockWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>

#include "SPAR/sparengine.h"
#include "Schematic/component.h"
#include "Schematic/graphwidget.h"
#include "UI/QucsRFDesignerWindow.h"
#include "UI/preferencesdialog.h"
#include "UI/smithchart.h"
#include "qcustomplot.h"

// Tools
#include "UI/AttenuatorDesignTool.h"
#include "UI/FilterDesignTool.h"
#include "UI/PowerCombiningTool.h"

// Math operations
#include "SPAR/MathOperations.h"

struct ToolSettings {
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

class QucsRFDesignerWindow : public QMainWindow {
  Q_OBJECT
public:
  QucsRFDesignerWindow();
  ~QucsRFDesignerWindow();
  QTabWidget *TabWidget;

private:
  void clear();

private slots: // Functions to launch the actions
  void PreferencesWindow();
  void ReceiveSettings(ToolSettings);
  QMap<QString, vector<complex<double>>> loadQucsDataSet(QString);
  void ShowSmithChart();
  void ReceiveNetworkFromDesignTools(
      struct SchematicInfo); // Simulates the network after changing the design
                             // goals
  void simulate(); // Rerun simulation when the user does some action over the
                   // display
  void
  SwitchTabs(int); // Whenever the tool tab is changed, this slot forces a new
                   // design without the need of modify a design parameter
  void ComponentSelected(ComponentInfo);

  // Simulations
  void SimulateSPAR();
  void SimulateLadderSPAR();
  void PlotImpedanceTransformations();

private: // Actions
  void createActions();
  void createMenus();
  void UpdateWindows();
  QToolBar *RFToolBar;

  QAction *PreferencesAction, *SmithAction;
  bool SmithWindowVisible;
  QString netlist;
  // ************************* Widgets *******************************
  GraphWidget *SchematicWidget;
  QVector<QCustomPlot *> DisplayWindow;
  FilterDesignTool *Filter_Tool;           // Widget for filter design
  PowerCombiningTool *PowerCombining_Tool; // Widget for power combiner design
  AttenuatorDesignTool *AttenuatorDesign_Tool;

  SmithChart *Smith_plot;

  // ************************** Docks ********************************
  QDockWidget *dock_Schematic, *dock_Setup, *dock_DisplayWindow1,
      *dock_DisplayWindow2, *dock_Smith;

  // ************************ SIMULATION SETTINGS ********************
  SP_Analysis SPAR_Settings;
  NetworkInfo NWI; // Synthesized network
  SchematicInfo SchInfo;

  // ************************ TOOL SETTINGS *************************
  ToolSettings Tool_Settings;

  // ************************ UPDATE GRAPH ***************************
  void updateGraph(int, vector<double>, QMap<QString, vector<complex<double>>>);
  void updateGraph(int, vector<double>, QMap<QString, vector<double>>,
                   QMap<QString, QPen>, QString, QString);
  void plotPoints(int, QMap<QString, QPointF>, QString);
};

#endif // QucsRFDesignerWindow_H
