#include "QucsRFDesignerWindow.h"

QucsRFDesignerWindow::QucsRFDesignerWindow()
{
    //************** Dock setup ******************
    dock_Schematic =  new QDockWidget("Schematic");
    dock_Setup =  new QDockWidget("");
    dock_DisplayWindow1 =  new QDockWidget("Plot window");
    dock_DisplayWindow2 = new QDockWidget("Plot window");

    dock_Schematic->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock_Setup->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock_DisplayWindow1->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock_DisplayWindow2->setAllowedAreas(Qt::AllDockWidgetAreas);
    //******************************* End of the dock setup


    //******************* Setup panel ******************************
    TabWidget = new QTabWidget();//Tab widget
    //************* Filter Design tab ************************

    Filter_Tool = new FilterDesignTool();
    //*********** Impedance Matching tab *********************
    QWidget *MatchingWidget = new QWidget();
    PowerCombining_Tool = new PowerCombiningTool();
    IP_Tool = new InterceptPointsTool();

    TabWidget->addTab(Filter_Tool, "Filter design");
    TabWidget->addTab(MatchingWidget, "Matching");
    TabWidget->addTab(PowerCombining_Tool, "Power Combining");
    TabWidget->addTab(IP_Tool, "Intercept Points");
    TabWidget->setMinimumSize(300, 200);
    //*********************************** End of the setup panel

    //******************* Plot window *******************
    DisplayWindow.append(new QCustomPlot(dock_DisplayWindow1));
    DisplayWindow.append(new QCustomPlot(dock_DisplayWindow2));

    for (int i = 0; i < DisplayWindow.size(); i++)
    {
      DisplayWindow[i]->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                QCP::iSelectLegend | QCP::iSelectPlottables);
      DisplayWindow[i]->setInteraction(QCP::iMultiSelect, true);
      DisplayWindow[i]->axisRect()->setupFullAxesBox();
      DisplayWindow[i]->plotLayout()->insertRow(0);
      DisplayWindow[i]->plotLayout()->addElement(0, 0, new QCPTextElement(DisplayWindow[i]));
      DisplayWindow[i]->legend->setVisible(true);
      QFont legendFont;// = font();
      legendFont.setPointSize(10);
      DisplayWindow[i]->legend->setFont(legendFont);
      DisplayWindow[i]->legend->setSelectedFont(legendFont);
      DisplayWindow[i]->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

      connect(DisplayWindow[i], SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(simulate()));
      connect(DisplayWindow[i], SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(simulate()));
    }

    //*********************************** End of the plot window region

    SchematicWidget = new GraphWidget(dock_Schematic);//Schematic window

    Smith_plot = new SmithChart();
    Smith_plot->showLine(true);


    //************ Set docks and add them to the main window *************
    dock_Setup->setWidget(TabWidget);
    dock_DisplayWindow1->setWidget(DisplayWindow[0]);
    dock_DisplayWindow2->setWidget(DisplayWindow[1]);
    dock_Schematic->setWidget(SchematicWidget);

    addDockWidget(Qt::LeftDockWidgetArea, dock_Setup);
    addDockWidget(Qt::RightDockWidgetArea, dock_DisplayWindow1);
    addDockWidget(Qt::LeftDockWidgetArea, dock_Schematic);
    splitDockWidget(dock_Setup, dock_Schematic, Qt::Vertical);
    //************************************* End adding docks to the main window

    createActions();
    createMenus();

    //************ Set default SPAR analsis parameters *****************
    SPAR_Settings.fstart = .5e9;//500MHz
    SPAR_Settings.fstop = 1.5e9;//2GHz
    SPAR_Settings.n_points = 500;//points

    //***********  Set default tool settings ****************************
    Tool_Settings.ShowTraces.push_back(true);//Show S21
    Tool_Settings.ShowTraces.push_back(true);//Show S11
    Tool_Settings.ShowTraces.push_back(false);//Do not show S22 by default

    Tool_Settings.TraceColor.push_back(Qt::red);//S21 -> red
    Tool_Settings.TraceColor.push_back(Qt::blue);//S11 -> blue
    Tool_Settings.TraceColor.push_back(Qt::darkGreen);//S22 -> green

    Tool_Settings.fstart = 5e8;
    Tool_Settings.fstop = 2e9;
    Tool_Settings.xstep = 1e8;
    Tool_Settings.ymin = -50;
    Tool_Settings.ymax = 0;
    Tool_Settings.ystep = 5;
    Tool_Settings.FixedAxes = true;

    DisplayWindow[0]->xAxis->setRange(SPAR_Settings.fstart/1e6, SPAR_Settings.fstop/1e6);
    DisplayWindow[0]->yAxis->setRange(Tool_Settings.ymin, Tool_Settings.ymax);

    connect(TabWidget, SIGNAL(currentChanged(int)), this, SLOT(SwitchTabs(int)));
    connect(Filter_Tool, SIGNAL(simulateNetwork(struct SchematicInfo)), this, SLOT(ReceiveNetworkFromDesignTools(struct SchematicInfo)));
    connect(PowerCombining_Tool, SIGNAL(simulateNetwork(struct SchematicInfo)), this, SLOT(ReceiveNetworkFromDesignTools(struct SchematicInfo)));
    connect(IP_Tool, SIGNAL(simulateDiagram(InterceptPointsData)), this, SLOT(receiveInterceptDiagramData(InterceptPointsData)));

    Filter_Tool->design();
}





QucsRFDesignerWindow::~QucsRFDesignerWindow()
{
    delete PreferencesAction;
    delete RFToolBar;
    delete SchematicWidget;
    for (int i =0; i< DisplayWindow.size(); i++)
    delete DisplayWindow[i];

    /* delete dock_Schematic;
  delete dock_Setup;
  delete dock_DisplayWindow1;
  delete TabWidget;
  delete FilterDesignWidget;
  delete FilterDesignLayout;
  delete TeePiWidget;
  delete TeePiLayout;*/


}

//Launches the preferences window
void QucsRFDesignerWindow::PreferencesWindow()
{
    PreferencesDialog * PD = new PreferencesDialog();
    PD->show();
    connect(PD, SIGNAL(sendSettings(ToolSettings)), this, SLOT(ReceiveSettings(ToolSettings)));
}

void QucsRFDesignerWindow::ReceiveSettings(ToolSettings TS)
{
    SPAR_Settings.fstart = TS.fstart;
    SPAR_Settings.fstop = TS.fstop;
    SPAR_Settings.n_points = TS.Npoints;

    Tool_Settings = TS;
    UpdateWindows();
}

// This functions creates the connect() pairs to set up the actions
void QucsRFDesignerWindow::createActions()
{
    PreferencesAction = new QAction("Settings", this);
    PreferencesAction->setStatusTip("Set up preferences");
    connect(PreferencesAction, SIGNAL(triggered()), this, SLOT(PreferencesWindow()));

    SmithAction = new QAction("Smith Chart", this);
    SmithAction->setStatusTip("Show Smith Chart plot");
    connect(SmithAction, SIGNAL(changed()), this, SLOT(ShowSmithChart()));

}


void QucsRFDesignerWindow::createMenus()
{
    RFToolBar = addToolBar(tr("RFtools"));
    PreferencesAction->setIcon(QIcon(":/bitmaps/Settings.png"));
    RFToolBar->addAction(PreferencesAction);

    SmithAction->setIcon(QIcon(":/bitmaps/SmithChartIcon.png"));
    SmithAction->setCheckable(true);
    RFToolBar->addAction(SmithAction);
}


//This function updates the content of the schematic window and the display
void QucsRFDesignerWindow::UpdateWindows()
{


}

void QucsRFDesignerWindow::updateGraph(int DisplayID, vector<double> freq_, QMap<QString, vector<complex<double> > > data)
{
    DisplayWindow[DisplayID]->clearGraphs();

    double k = 1e6;
    freq_=freq_/k;//Convert to MHz

    QVector<double> freq = QVector<double>::fromStdVector(freq_);//Get frequency axis
    QMapIterator<QString, QPen> MapIT(SchInfo.displayGraphs);

    while (MapIT.hasNext())
    {
        MapIT.next();
        std::vector<double> aux = 20*log(abs(data[MapIT.key()]));
        QVector<double> trace = QVector<double>::fromStdVector(aux);
        DisplayWindow[DisplayID]->addGraph();
        QString title = MapIT.key();
        DisplayWindow[DisplayID]->graph()->setName(title);
        DisplayWindow[DisplayID]->graph()->setPen(MapIT.value());
        DisplayWindow[DisplayID]->graph()->setData(freq, trace);
    }

    //X axis step
    QSharedPointer<QCPAxisTickerFixed> fixedTickerX(new QCPAxisTickerFixed);
    fixedTickerX->setTickStep(Tool_Settings.xstep/k);
    DisplayWindow[DisplayID]->xAxis->setTicker(fixedTickerX);
    //Y axis step
    QSharedPointer<QCPAxisTickerFixed> fixedTickerY(new QCPAxisTickerFixed);
    fixedTickerY->setTickStep(Tool_Settings.ystep);
    DisplayWindow[DisplayID]->yAxis->setTicker(fixedTickerY);

    DisplayWindow[DisplayID]->replot();

    //Update Smith Chart
  /*  std::complex<double> Z;
    Smith_plot->clear();
    for(unsigned int i = 0; i < S11_.size(); i++)
    {
        Z = ((std::complex<double>(1, 0)+S11_.at(i)))/(std::complex<double>(1, 0)-S11_.at(i));
        Smith_plot->setData(Z.real(),Z.imag());
    }*/


}


void QucsRFDesignerWindow::updateGraph(int DisplayID, vector<double> Pin_, QMap<QString, vector<double> > data, QMap<QString, QPen>TraceProperties,
                                       QString xlabel, QString ylabel)
{
    DisplayWindow[DisplayID]->clearGraphs();
    DisplayWindow[DisplayID]->clearItems();

    QVector<double> Pin = QVector<double>::fromStdVector(Pin_);//Get frequency axis
    QMapIterator<QString, vector<double> > MapIT(data);

    while (MapIT.hasNext())
    {
        MapIT.next();
        QVector<double> trace = QVector<double>::fromStdVector(data[MapIT.key()]);
        DisplayWindow[DisplayID]->addGraph();
        QString title = MapIT.key();
        title.append(QString(" (dBm)"));
        DisplayWindow[DisplayID]->graph()->setName(title);
        DisplayWindow[DisplayID]->graph()->setPen(TraceProperties[MapIT.key()]);
        DisplayWindow[DisplayID]->graph()->setData(Pin, trace);
    }

    //X axis step
    QSharedPointer<QCPAxisTickerFixed> fixedTickerX(new QCPAxisTickerFixed);
    fixedTickerX->setTickStep(5);
    DisplayWindow[DisplayID]->xAxis->setTicker(fixedTickerX);
    DisplayWindow[DisplayID]->xAxis->setLabel(xlabel);
    //Y axis step
    QSharedPointer<QCPAxisTickerFixed> fixedTickerY(new QCPAxisTickerFixed);
    fixedTickerY->setTickStep(5);
    DisplayWindow[DisplayID]->yAxis->setTicker(fixedTickerY);
    DisplayWindow[DisplayID]->yAxis->setLabel(ylabel);
    DisplayWindow[DisplayID]->replot();

}


void QucsRFDesignerWindow::plotPoints(int DisplayID, QMap<QString, QPointF> InterceptPoints, QString style)
{
    QMapIterator<QString, QPointF > MapIT(InterceptPoints);
    QVector<QCPItemText *> Label;
    int i = 0;
    QVector<QCPItemCurve *> Arrows;

    if (style == "dot")
    {
        while (MapIT.hasNext())
        {
           MapIT.next();
           DisplayWindow[DisplayID]->addGraph();
           QVector<double> x(1);
           QVector<double> y(1);
           x[0] = InterceptPoints[MapIT.key()].x();
           y[0] = InterceptPoints[MapIT.key()].y();
           DisplayWindow[DisplayID]->graph()->setData(x, y);
           DisplayWindow[DisplayID]->graph()->setPen(QColor(0, 0, 0));
           DisplayWindow[DisplayID]->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 10));
           DisplayWindow[DisplayID]->graph()->setName(MapIT.key());

           //Set label
           Label.append(new QCPItemText(DisplayWindow[DisplayID]));
           Label[i]->setText(MapIT.key());
           Label[i]->setFont(QFont(font().family(), 10));
           Label[i]->position->setCoords(x[0]+1, y[0]-1); // lower right corner of axis rect
           i++;
        }
    }

    if (style == "Arrow")
    {
        int i=0;
        DisplayWindow[DisplayID]->clearItems();
        DisplayWindow[DisplayID]->addGraph();
        while (MapIT.hasNext())
        {
           MapIT.next();

           double x, y;
           x = InterceptPoints[MapIT.key()].x();
           y = InterceptPoints[MapIT.key()].y();

           Arrows.append(new QCPItemCurve(DisplayWindow[DisplayID]));
           Arrows[i]->startDir->setParentAnchor(Arrows[i]->start);
           Arrows[i]->startDir->setCoords(0, 1); // direction 30 pixels to the left of parent anchor (tracerArrow->start)
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


//This function loads the Qucs dataset and converts it into a QMap structure
QMap<QString, vector<complex<double> > > QucsRFDesignerWindow::loadQucsDataSet(QString dataset_path)
{
    QFile file(dataset_path);
    QMap<QString, vector<complex<double> > > data;
    QString variable;
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        if ((!fields.at(0).compare("<indep")) || (!fields.at(0).compare("<dep")))
        {//Read new variable and put it into the map
            vector<complex<double> > tmpComplex;
            variable = fields.at(1);//Variable name
            line = in.readLine();
            while ((line.compare("</indep>"))&&(line.compare("</dep>")))
            {//Read data until it finds the ending tag
                line.replace("+j", ",");
                line.replace("-j", ",-");
                std::istringstream String2Complex('(' + line.toStdString() + ')');//Convert to std::string to complex
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



void QucsRFDesignerWindow::ShowSmithChart()
{
    if (SmithAction->isChecked())
    {
        Smith_plot->show();
    }
    else
    {
        Smith_plot->hide();
    }
}



void QucsRFDesignerWindow::ReceiveNetworkFromDesignTools(struct SchematicInfo SI)
{
    SchInfo = SI;
    simulate();
}

void QucsRFDesignerWindow::simulate()
{
    switch(TabWidget->currentIndex())
    {
        case 3://Intercept point simulation
          SimulateInterceptDiagram();
          break;
        default:
          SimulateSPAR();
    }
}


void QucsRFDesignerWindow::SimulateSPAR()
{
    QCPRange xAxisRange = DisplayWindow[0]->xAxis->range();
    DisplayWindow[0]->yAxis->setLabel("dB");
    DisplayWindow[0]->xAxis->setLabel("MHz");
    if (xAxisRange.upper < SPAR_Settings.fstart*1e-6)
    {//Well, the previous plot wasn't a SPAR simulation
      DisplayWindow[0]->xAxis->setRangeLower(SPAR_Settings.fstart);
      DisplayWindow[0]->xAxis->setRangeUpper(SPAR_Settings.fstop);
      xAxisRange = DisplayWindow[0]->xAxis->range();
    }

    SPAR_Settings.fstart =xAxisRange.lower*1e6;
    SPAR_Settings.fstop =xAxisRange.upper*1e6;

    if (SPAR_Settings.fstart < 0) SPAR_Settings.fstart = 0;
    if (SPAR_Settings.fstop < 0) return;

    qDebug() << "SPAN: " << xAxisRange.lower*1e6 << "  " << xAxisRange.upper*1e6;

    SchematicWidget->clear();//Remove the components in the scene

    netlist = SchInfo.netlist;
    netlist += QString(".SP:SP1 Type=\"lin\" Start=\"%1 Hz\" Stop=\"%2 Hz\" Points=\"%3\"\n").arg(SPAR_Settings.fstart).arg(SPAR_Settings.fstop).arg(SPAR_Settings.n_points);
    QFile file("netlist");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << netlist << endl;
    }
    file.close();
    system("qucsator -i netlist -o data.dat");

    //Update schematic window
    SchematicWidget->setSchematic(SchInfo);

    // Load info from Qucs dataset and update graph
    QMap<QString, vector<complex<double> > > data = loadQucsDataSet("data.dat");
    //Update graph
    vector<complex<double> > freq=data["frequency"];
    updateGraph(0, real(freq), data);
}

void QucsRFDesignerWindow::receiveInterceptDiagramData(InterceptPointsData IP_info)
{
    IP_data = IP_info;
    simulate();
}

void QucsRFDesignerWindow::SimulateInterceptDiagram()
{
    QCPRange PinRange = DisplayWindow[0]->xAxis->range();
    QCPRange PoutRange = DisplayWindow[0]->yAxis->range();
    QMap<QString, QPen> TraceProperties;//Trace properties

    //Legend position
    DisplayWindow[0]->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);

    if ((PinRange.upper > 100))
    {
        DisplayWindow[0]->xAxis->setRange(IP_data.IP3-IP_data.Gain-40, IP_data.IP3-IP_data.Gain+20);
        PinRange = DisplayWindow[0]->xAxis->range();
    }

    if ((PoutRange.upper < IP_data.IP3) || (PoutRange.lower > IP_data.IP3))
    {
        DisplayWindow[0]->yAxis->setRange(IP_data.IP3-40, IP_data.IP3+20);
    }
    int npoints = PinRange.upper - PinRange.lower;//1dB/step
    vector<double> Pin(npoints), Pout(npoints), IM3(npoints), IM2(npoints);

    //Input power
    Pin[0] = PinRange.lower;
    for (int i = 1; i < npoints; i++) Pin[i] = Pin[i-1] + 1;

    //Fundamental
    Pout = IP_data.Gain+Pin;
    QMap<QString, vector<double> > data;
    data["Fundamental"] = Pout;
    TraceProperties["Fundamental"] = QPen(Qt::blue, 3);


    //IM3
    IM3 = - 2*IP_data.IP3 + 3*IP_data.Gain + 3*Pin;
    data["IM3"] = IM3;
    TraceProperties["IM3"] = QPen (Qt::red, 3);

    //IM2
    IM2 = - IP_data.IP2 + 2*IP_data.Gain+ 2*Pin;
    data["IM2"] = IM2;
    TraceProperties["IM2"] = QPen (Qt::green, 3);

    QString ylabel("Pout (dBm)");
    QString xlabel("Pin (dBm)");

    updateGraph(0, Pin, data, TraceProperties, xlabel, ylabel);

    //Plot IP3 and IP2
    QMap<QString, QPointF > InterceptPoints;
    InterceptPoints["IP3"] = QPointF(IP_data.IP3 - IP_data.Gain, IP_data.IP3);
    InterceptPoints["IP2"] = QPointF(IP_data.IP2 - IP_data.Gain, IP_data.IP2);
    plotPoints(0, InterceptPoints, "dot");


    //Display two-tone test spectrum (Diagram2)
    QMap<QString, QPointF > SpectrumTones;
    double f1 = (IP_data.fc - IP_data.delta)*1e-6;
    double f2 = (IP_data.fc + IP_data.delta)*1e-6;
    SpectrumTones["f1"] = QPointF(f1, IP_data.Pout);
    SpectrumTones["f2"] = QPointF(f2, IP_data.Pout);

    SpectrumTones["2f1-f2"] = QPointF(2*f1-f2, IP_data.IM3);
    SpectrumTones["2f2-f1"] = QPointF(2*f2-f1, IP_data.IM3);

    SpectrumTones["f1+f2"] = QPointF(f1+f2, IP_data.IM2);
    SpectrumTones["|f1-f2|"] = QPointF(abs(f1-f2), IP_data.IM2);

    QCPRange freq = DisplayWindow[1]->xAxis->range();
    if ((freq.upper < IP_data.fc*1e-6) || (freq.lower > 2*IP_data.fc*1e-6))
    {
      DisplayWindow[1]->xAxis->setRange(f1-2*(f2-f1), f2+2*(f2-f1));
      DisplayWindow[1]->yAxis->setRange(IP_data.IM3-10, IP_data.Pout+5);
    }
    DisplayWindow[1]->legend->setVisible(false);

    //Set the scale to 5dB/div
    QSharedPointer<QCPAxisTickerFixed> fixedTickerY(new QCPAxisTickerFixed);
    fixedTickerY->setTickStep(5);
    DisplayWindow[1]->yAxis->setTicker(fixedTickerY);
    DisplayWindow[1]->yAxis->setLabel(ylabel);
    DisplayWindow[1]->xAxis->setLabel("freq (MHz)");

    plotPoints(1, SpectrumTones, "Arrow");
}

void QucsRFDesignerWindow::SwitchTabs(int tabindex)
{
   switch(tabindex)
   {
     case 0://Filtering
       Filter_Tool->design();
       break;
     case 1://Matching
       break;
     case 2://Power combining
       PowerCombining_Tool->design();
       break;
     case 3://Intercept points tool
        addDockWidget(Qt::RightDockWidgetArea, dock_DisplayWindow2);//Add new diagram to display the spectrum of the two-tone test
        IP_Tool->CalculateInterceptPoints();
      break;
   }
}
