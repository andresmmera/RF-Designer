/***************************************************************************
                                QucsRFDesignerWindow.cpp
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
#include "QucsRFDesignerWindow.h"

QucsRFDesignerWindow::QucsRFDesignerWindow() {
  //************** Dock setup ******************
  dock_Schematic = new QDockWidget("Schematic");
  dock_Setup = new QDockWidget("");
  dock_DisplayWindow1 = new QDockWidget("Plot window");
  dock_DisplayWindow2 = new QDockWidget("Plot window");

  dock_Schematic->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_Setup->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_DisplayWindow1->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_DisplayWindow2->setAllowedAreas(Qt::AllDockWidgetAreas);
  //******************************* End of the dock setup

  //******************* Setup panel ******************************
  TabWidget = new QTabWidget(); // Tab widget
  //************* Filter Design tab ************************

  Filter_Tool = new FilterDesignTool();
  MatchingNetworkDesign_Tool = new MatchingNetworkDesignTool();
  PowerCombining_Tool = new PowerCombiningTool();
  AttenuatorDesign_Tool = new AttenuatorDesignTool();

  TabWidget->addTab(Filter_Tool, "Filter design");
  TabWidget->addTab(MatchingNetworkDesign_Tool, "Matching");
  TabWidget->addTab(PowerCombining_Tool, "Power Combining");
  TabWidget->addTab(AttenuatorDesign_Tool, "Attenuator Design");
  TabWidget->setMinimumSize(200, 150);

  //*********************************** End of the setup panel

  //******************* Plot window *******************
  DisplayWindow.append(new QCustomPlot(dock_DisplayWindow1));
  DisplayWindow.append(new QCustomPlot(dock_DisplayWindow2));

  for (int i = 0; i < DisplayWindow.size(); i++) {
    DisplayWindow[i]->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                                      QCP::iSelectAxes | QCP::iSelectLegend |
                                      QCP::iSelectPlottables);
    DisplayWindow[i]->setInteraction(QCP::iMultiSelect, true);
    DisplayWindow[i]->axisRect()->setupFullAxesBox();
    DisplayWindow[i]->plotLayout()->insertRow(0);
    DisplayWindow[i]->plotLayout()->addElement(
        0, 0, new QCPTextElement(DisplayWindow[i]));
    DisplayWindow[i]->legend->setVisible(true);
    QFont legendFont; // = font();
    legendFont.setPointSize(10);
    DisplayWindow[i]->legend->setFont(legendFont);
    DisplayWindow[i]->legend->setSelectedFont(legendFont);
    DisplayWindow[i]->legend->setSelectableParts(
        QCPLegend::spItems); // legend box shall not be selectable, only legend
                             // items

    connect(DisplayWindow[i], SIGNAL(mouseWheel(QWheelEvent *)), this,
            SLOT(simulate()));
    connect(DisplayWindow[i], SIGNAL(mouseRelease(QMouseEvent *)), this,
            SLOT(simulate()));
  }

  //*********************************** End of the plot window region

  SchematicWidget = new GraphWidget(dock_Schematic); // Schematic window
  SchematicWidget->setMinimumSize(200, 150);
  connect(SchematicWidget,
          SIGNAL(SendComponentSelectionToMainFunction(ComponentInfo)), this,
          SLOT(ComponentSelected(ComponentInfo)));

  //************ Set docks and add them to the main window *************
  dock_Setup->setWidget(TabWidget);
  dock_DisplayWindow1->setWidget(DisplayWindow[0]);
  dock_DisplayWindow2->setWidget(DisplayWindow[1]);
  dock_Schematic->setWidget(SchematicWidget);
  ;

  addDockWidget(Qt::LeftDockWidgetArea, dock_Setup);
  addDockWidget(Qt::RightDockWidgetArea, dock_DisplayWindow1);
  addDockWidget(Qt::LeftDockWidgetArea, dock_Schematic);
  splitDockWidget(dock_Setup, dock_Schematic, Qt::Vertical);
  //************************************* End adding docks to the main window

  createActions();
  createMenus();

  //************ Set default SPAR analsis parameters *****************
  SPAR_Settings.fstart = .5e9;  // 500MHz
  SPAR_Settings.fstop = 1.5e9;  // 2GHz
  SPAR_Settings.n_points = 500; // points

  DisplayWindow[0]->xAxis->setRange(SPAR_Settings.fstart / 1e6,
                                    SPAR_Settings.fstop / 1e6);
  DisplayWindow[0]->yAxis->setRange(-50, 0);
  QSharedPointer<QCPAxisTickerFixed> fixedTickerY(new QCPAxisTickerFixed);
  fixedTickerY->setTickStep(5);
  DisplayWindow[0]->yAxis->setTicker(fixedTickerY);

  connect(TabWidget, SIGNAL(currentChanged(int)), this, SLOT(SwitchTabs(int)));
  connect(Filter_Tool, SIGNAL(simulateNetwork(SchematicContent)), this,
          SLOT(ReceiveNetworkFromDesignTools(SchematicContent)));
  connect(PowerCombining_Tool, SIGNAL(simulateNetwork(SchematicContent)), this,
          SLOT(ReceiveNetworkFromDesignTools(SchematicContent)));
  connect(MatchingNetworkDesign_Tool, SIGNAL(simulateNetwork(SchematicContent)),
          this, SLOT(ReceiveNetworkFromDesignTools(SchematicContent)));
  connect(AttenuatorDesign_Tool, SIGNAL(simulateNetwork(SchematicContent)),
          this, SLOT(ReceiveNetworkFromDesignTools(SchematicContent)));

  Filter_Tool->design();
}

QucsRFDesignerWindow::~QucsRFDesignerWindow() {
  delete PreferencesAction;
  delete RFToolBar;
  delete SchematicWidget;
  for (int i = 0; i < DisplayWindow.size(); i++)
    delete DisplayWindow[i];
}

// Launches the preferences window
void QucsRFDesignerWindow::PreferencesWindow() {
  PreferencesDialog *PD = new PreferencesDialog();
  PD->show();
  connect(PD, SIGNAL(sendSettings(ToolSettings)), this,
          SLOT(ReceiveSettings(ToolSettings)));
}

void QucsRFDesignerWindow::ReceiveSettings(ToolSettings TS) {
  Tool_Settings = TS;
  UpdateWindows();
}

// This functions creates the connect() pairs to set up the actions
void QucsRFDesignerWindow::createActions() {
  PreferencesAction = new QAction("Settings", this);
  PreferencesAction->setStatusTip("Set up preferences");
  connect(PreferencesAction, SIGNAL(triggered()), this,
          SLOT(PreferencesWindow()));
}

void QucsRFDesignerWindow::createMenus() {
  RFToolBar = addToolBar(tr("RFtools"));
  PreferencesAction->setIcon(QIcon(":/bitmaps/Settings.png"));
  RFToolBar->addAction(PreferencesAction);
}

// This function updates the content of the schematic window and the display
void QucsRFDesignerWindow::UpdateWindows() {}

void QucsRFDesignerWindow::updateGraph(
    int DisplayID, vector<double> freq_,
    QMap<QString, vector<complex<double>>> data) {
  DisplayWindow[DisplayID]->clearGraphs();

  double k = 1e6;
  freq_ = freq_ / k; // Convert to MHz

  QVector<double> freq =
      QVector<double>::fromStdVector(freq_); // Get frequency axis
  QMapIterator<QString, QPen> MapIT(SchContent.getDisplayGraphs());

  while (MapIT.hasNext()) {
    MapIT.next();
    std::vector<double> aux = 20 * log(abs(data[MapIT.key()]));
    QVector<double> trace = QVector<double>::fromStdVector(aux);
    DisplayWindow[DisplayID]->addGraph();
    QString title = MapIT.key();
    DisplayWindow[DisplayID]->graph()->setName(title);
    DisplayWindow[DisplayID]->graph()->setPen(MapIT.value());
    DisplayWindow[DisplayID]->graph()->setData(freq, trace);
  }

  DisplayWindow[DisplayID]->replot();
}

void QucsRFDesignerWindow::updateGraph(int DisplayID, vector<double> Pin_,
                                       QMap<QString, vector<double>> data,
                                       QMap<QString, QPen> TraceProperties,
                                       QString xlabel, QString ylabel) {
  DisplayWindow[DisplayID]->clearGraphs();
  DisplayWindow[DisplayID]->clearItems();

  QVector<double> Pin =
      QVector<double>::fromStdVector(Pin_); // Get frequency axis
  QMapIterator<QString, vector<double>> MapIT(data);

  while (MapIT.hasNext()) {
    MapIT.next();
    QVector<double> trace = QVector<double>::fromStdVector(data[MapIT.key()]);
    DisplayWindow[DisplayID]->addGraph();
    QString title = MapIT.key();
    title.append(QString(" (dBm)"));
    DisplayWindow[DisplayID]->graph()->setName(title);
    DisplayWindow[DisplayID]->graph()->setPen(TraceProperties[MapIT.key()]);
    DisplayWindow[DisplayID]->graph()->setData(Pin, trace);
  }

  // X axis step
  QSharedPointer<QCPAxisTickerFixed> fixedTickerX(new QCPAxisTickerFixed);
  fixedTickerX->setTickStep(5);
  DisplayWindow[DisplayID]->xAxis->setTicker(fixedTickerX);
  DisplayWindow[DisplayID]->xAxis->setLabel(xlabel);
  // Y axis step
  QSharedPointer<QCPAxisTickerFixed> fixedTickerY(new QCPAxisTickerFixed);
  fixedTickerY->setTickStep(5);
  DisplayWindow[DisplayID]->yAxis->setTicker(fixedTickerY);
  DisplayWindow[DisplayID]->yAxis->setLabel(ylabel);
  DisplayWindow[DisplayID]->replot();
}

void QucsRFDesignerWindow::plotPoints(int DisplayID,
                                      QMap<QString, QPointF> InterceptPoints,
                                      QString style) {
  QMapIterator<QString, QPointF> MapIT(InterceptPoints);
  QVector<QCPItemText *> Label;
  int i = 0;
  QVector<QCPItemCurve *> Arrows;

  if (style == "dot") {
    while (MapIT.hasNext()) {
      MapIT.next();
      DisplayWindow[DisplayID]->addGraph();
      QVector<double> x(1);
      QVector<double> y(1);
      x[0] = InterceptPoints[MapIT.key()].x();
      y[0] = InterceptPoints[MapIT.key()].y();
      DisplayWindow[DisplayID]->graph()->setData(x, y);
      DisplayWindow[DisplayID]->graph()->setPen(QColor(0, 0, 0));
      DisplayWindow[DisplayID]->graph()->setScatterStyle(
          QCPScatterStyle(QCPScatterStyle::ssCircle, 10));
      DisplayWindow[DisplayID]->graph()->setName(MapIT.key());

      // Set label
      Label.append(new QCPItemText(DisplayWindow[DisplayID]));
      Label[i]->setText(MapIT.key());
      Label[i]->setFont(QFont(font().family(), 10));
      Label[i]->position->setCoords(
          x[0] + 1, y[0] - 1); // lower right corner of axis rect
      i++;
    }
  }

  if (style == "Arrow") {
    int i = 0;
    DisplayWindow[DisplayID]->clearItems();
    DisplayWindow[DisplayID]->addGraph();
    while (MapIT.hasNext()) {
      MapIT.next();

      double x, y;
      x = InterceptPoints[MapIT.key()].x();
      y = InterceptPoints[MapIT.key()].y();

      Arrows.append(new QCPItemCurve(DisplayWindow[DisplayID]));
      Arrows[i]->startDir->setParentAnchor(Arrows[i]->start);
      Arrows[i]->startDir->setCoords(0,
                                     1); // direction 30 pixels to the left of
                                         // parent anchor (tracerArrow->start)
      Arrows[i]->end->setCoords(x, y);
      Arrows[i]->start->setCoords(x, DisplayWindow[1]->yAxis->range().lower);
      Arrows[i]->endDir->setParentAnchor(Arrows[i]->end);
      Arrows[i]->endDir->setCoords(0, 1);
      Arrows[i]->setHead(QCPLineEnding::esSpikeArrow);
      DisplayWindow[DisplayID]->graph()->setName(MapIT.key());
      i++;
    }
  }
  DisplayWindow[DisplayID]->replot();
}

// This function loads the Qucs dataset and converts it into a QMap structure
QMap<QString, vector<complex<double>>>
QucsRFDesignerWindow::loadQucsDataSet(QString dataset_path) {
  QFile file(dataset_path);
  QMap<QString, vector<complex<double>>> data;
  QString variable;
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::information(0, "error", file.errorString());
  }

  QTextStream in(&file);

  while (!in.atEnd()) {
    QString line = in.readLine();
    QStringList fields = line.split(" ");
    if ((!fields.at(0).compare("<indep")) ||
        (!fields.at(0).compare(
            "<dep"))) { // Read new variable and put it into the map
      vector<complex<double>> tmpComplex;
      variable = fields.at(1); // Variable name
      line = in.readLine();
      while (
          (line.compare("</indep>")) &&
          (line.compare("</dep>"))) { // Read data until it finds the ending tag
        line.replace("+j", ",");
        line.replace("-j", ",-");
        std::istringstream String2Complex(
            '(' + line.toStdString() +
            ')'); // Convert to std::string to complex
        std::complex<double> c;
        String2Complex >> c;
        tmpComplex.push_back(c);
        line = in.readLine();
      }
      data[variable] = tmpComplex;
    }
  }

  file.close();
  return data;
}

void QucsRFDesignerWindow::ReceiveNetworkFromDesignTools(SchematicContent SI) {
  SchContent = SI;
  simulate();
}

void QucsRFDesignerWindow::simulate() {
  if (!SchContent.getDescription().contains("NOT LADDER"))
    SimulateLadderSPAR();
  else
    SimulateSPAR();
}

// S-parameter simulation using the built-in ladder SPAR simulator. This is used
// only for SPAR simulations relying on the input/output port (complex)
// impedances
void QucsRFDesignerWindow::SimulateLadderSPAR() {
  QCPRange xAxisRange = DisplayWindow[0]->xAxis->range();
  DisplayWindow[0]->yAxis->setLabel("dB");
  DisplayWindow[0]->xAxis->setLabel("MHz");
  if (xAxisRange.upper <
      SPAR_Settings.fstart *
          1e-6) { // Well, the previous plot wasn't a SPAR simulation
    DisplayWindow[0]->xAxis->setRange(SPAR_Settings.fstart * 1e-6,
                                      SPAR_Settings.fstop * 1e-6);
    xAxisRange = DisplayWindow[0]->xAxis->range();
  }
  SparEngine SPARSim;
  SP_Analysis SPARSettings;
  SPARSettings.fstart = xAxisRange.lower * 1e6;
  SPARSettings.fstop = xAxisRange.upper * 1e6;
  SPARSettings.n_points = SPAR_Settings.n_points;
  SPARSim.setSimulationSettings(SPARSettings);

  // Convert SchematicContent object data into NetworkInfo for SPAR simulation
  NetworkInfo NWI;
  std::vector<complex<double>> ZS(1), ZL(1);
  ZS[0] = SchContent.getZin(); // Str2Complex(SchContent.getZinString());  //
                               // Port 1 impedance
  ZL[0] = SchContent.getZout(); // Str2Complex(SchContent.getZoutString()); //
                                // Port 2 impedance
  NWI.ZS = ZS;
  NWI.ZL = ZL;
  NWI.Ladder = SchContent.getComponents();
  SPARSim.setNetwork(NWI);
  SPARSim.run();

  // Display the result
  DisplayWindow[0]->yAxis->setLabel("dB");
  DisplayWindow[0]->xAxis->setLabel("MHz");
  if (xAxisRange.upper <
      SPAR_Settings.fstart *
          1e-6) { // Well, the previous plot wasn't a SPAR simulation
    DisplayWindow[0]->xAxis->setRangeLower(SPAR_Settings.fstart);
    DisplayWindow[0]->xAxis->setRangeUpper(SPAR_Settings.fstop);
    xAxisRange = DisplayWindow[0]->xAxis->range();
  }

  SchematicWidget->clear(); // Remove the components in the scene
  SchematicWidget->setSchematic(SchContent);

  QMap<QString, vector<complex<double>>> data = SPARSim.getData();
  updateGraph(0, SPARSim.getFreq(), data);
}

// S-parameter simulation using qucsator
void QucsRFDesignerWindow::SimulateSPAR() {
  QCPRange xAxisRange = DisplayWindow[0]->xAxis->range();
  DisplayWindow[0]->yAxis->setLabel("dB");
  DisplayWindow[0]->xAxis->setLabel("MHz");
  if (xAxisRange.upper <
      SPAR_Settings.fstart *
          1e-6) { // Well, the previous plot wasn't a SPAR simulation
    DisplayWindow[0]->xAxis->setRange(SPAR_Settings.fstart * 1e-6,
                                      SPAR_Settings.fstop * 1e-6);
    xAxisRange = DisplayWindow[0]->xAxis->range();
  }

  SPAR_Settings.fstart = xAxisRange.lower * 1e6;
  SPAR_Settings.fstop = xAxisRange.upper * 1e6;

  if (SPAR_Settings.fstart < 0)
    SPAR_Settings.fstart = 0;
  if (SPAR_Settings.fstop < 0)
    return;

  qDebug() << "SPAN: " << xAxisRange.lower * 1e6 << "  "
           << xAxisRange.upper * 1e6;

  SchematicWidget->clear(); // Remove the components in the scene

  netlist = SchContent.getQucsNetlist();
  netlist +=
      QString(
          ".SP:SP1 Type=\"lin\" Start=\"%1 Hz\" Stop=\"%2 Hz\" Points=\"%3\"\n")
          .arg(SPAR_Settings.fstart)
          .arg(SPAR_Settings.fstop)
          .arg(SPAR_Settings.n_points);
  QFile file("netlist");
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream stream(&file);
    stream << netlist << endl;
  }
  file.close();
  system("qucsator -i netlist -o data.dat");

  // Update schematic window
  SchematicWidget->setSchematic(SchContent);

  // Load info from Qucs dataset and update graph
  QMap<QString, vector<complex<double>>> data = loadQucsDataSet("data.dat");
  // Update graph
  vector<complex<double>> freq = data["frequency"];
  updateGraph(0, real(freq), data);
}

void QucsRFDesignerWindow::SwitchTabs(int tabindex) {
  switch (tabindex) {
  case 0: // Filtering
    dock_DisplayWindow2->hide();
    Filter_Tool->design();
    break;
  case 1: // Matching
    dock_DisplayWindow2->hide();
    MatchingNetworkDesign_Tool->design();
    break;
  case 2: // Power combining
    dock_DisplayWindow2->hide();
    PowerCombining_Tool->design();
    break;
  case 3: // Attenuator design
    dock_DisplayWindow2->hide();
    AttenuatorDesign_Tool->design();
    break;
  }
}

// Whenever a component is selected in the schematic widget, it emits a signal
// that is firstly catched by the GraphWidget and then is retransmitted to the
// main class. This function is the one that handles such signals so it must
// take into account the current tool.
void QucsRFDesignerWindow::ComponentSelected(ComponentInfo CI) {}
