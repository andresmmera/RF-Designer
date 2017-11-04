#include "QucsRFDesignerWindow.h"

QucsRFDesignerWindow::QucsRFDesignerWindow()
{
    //************** Dock setup ******************
    QDockWidget *dock_Schematic =  new QDockWidget("Schematic");
    QDockWidget *dock_Setup =  new QDockWidget("");
    QDockWidget *dock_PlotWindow =  new QDockWidget("Plot window");

    dock_Schematic->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock_Setup->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock_PlotWindow->setAllowedAreas(Qt::AllDockWidgetAreas);
    //******************************* End of the dock setup


    //******************* Setup panel ******************************
    TabWidget = new QTabWidget();//Tab widget
    //************* Filter Design tab ************************

    Filter_Tool = new FilterDesignTool();
    //*********** Impedance Matching tab *********************
    QWidget *MatchingWidget = new QWidget();
    PowerCombining_Tool = new PowerCombiningTool();


    TabWidget->addTab(Filter_Tool, "Filter design");
    TabWidget->addTab(MatchingWidget, "Matching");
    TabWidget->addTab(PowerCombining_Tool, "Power Combining");
    TabWidget->setMinimumSize(300, 200);
    //*********************************** End of the setup panel

    //******************* Plot window *******************
    PlotWidget = new QCustomPlot(dock_PlotWindow);
    PlotWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                QCP::iSelectLegend | QCP::iSelectPlottables);
    PlotWidget->setInteraction(QCP::iMultiSelect, true);
    PlotWidget->axisRect()->setupFullAxesBox();

    PlotWidget->plotLayout()->insertRow(0);
    QCPTextElement *title = new QCPTextElement(PlotWidget);
    PlotWidget->plotLayout()->addElement(0, 0, title);

    PlotWidget->legend->setVisible(true);
    QFont legendFont;// = font();
    legendFont.setPointSize(10);
    PlotWidget->legend->setFont(legendFont);
    PlotWidget->legend->setSelectedFont(legendFont);
    PlotWidget->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
    PlotWidget->setMinimumSize(400, 200);
    PlotWidget->yAxis->setLabel("dB");
    PlotWidget->xAxis->setLabel("MHz");
    connect(PlotWidget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(UpdateSparSweep()));
    connect(PlotWidget, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(UpdateSparSweep()));
    //*********************************** End of the plot window region

    SchematicWidget = new GraphWidget(dock_Schematic);//Schematic window

    Smith_plot = new SmithChart();
    // Smith_plot->setMinimumSize(200, 400);
    Smith_plot->showLine(true);


    //************ Set docks and add them to the main window *************
    dock_Setup->setWidget(TabWidget);
    dock_PlotWindow->setWidget(PlotWidget);
    dock_Schematic->setWidget(SchematicWidget);

    addDockWidget(Qt::LeftDockWidgetArea, dock_Setup);
    addDockWidget(Qt::RightDockWidgetArea, dock_PlotWindow);
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

    PlotWidget->xAxis->setRange(SPAR_Settings.fstart/1e6, SPAR_Settings.fstop/1e6);
    PlotWidget->yAxis->setRange(Tool_Settings.ymin, Tool_Settings.ymax);

    connect(Filter_Tool, SIGNAL(simulateNetwork(struct SchematicInfo)), this, SLOT(ReceiveNetworkFromDesignTools(struct SchematicInfo)));
    connect(PowerCombining_Tool, SIGNAL(simulateNetwork(struct SchematicInfo)), this, SLOT(ReceiveNetworkFromDesignTools(struct SchematicInfo)));

    connect(PlotWidget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(simulate()));
    connect(PlotWidget, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(simulate()));

    Filter_Tool->design();
}





QucsRFDesignerWindow::~QucsRFDesignerWindow()
{
    delete PreferencesAction;
    delete RFToolBar;
    delete SchematicWidget;
    delete PlotWidget;

    /* delete dock_Schematic;
  delete dock_Setup;
  delete dock_PlotWindow;
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

void QucsRFDesignerWindow::updateGraph(vector<double> freq_, vector<complex<double> > S21_, vector<complex<double> > S11_, vector<complex<double> > S22_)
{
    PlotWidget->clearGraphs();

    double k = 1e6;
    freq_=freq_/k;//Convert to MHz

    QVector<double> freq = QVector<double>::fromStdVector(freq_);//Get frequency axis

    if (Tool_Settings.ShowTraces.at(0))//Show S21
    {
        std::vector<double> aux = 20*log(abs(S21_));
        QVector<double> S21 = QVector<double>::fromStdVector(aux);
        QColor S21Color = Tool_Settings.TraceColor.at(0);
        PlotWidget->addGraph();
        PlotWidget->graph()->setName(QString("S21 (dB)"));
        PlotWidget->graph()->setPen(QPen(S21Color));
        PlotWidget->graph()->setData(freq, S21);
    }


    if (Tool_Settings.ShowTraces.at(1))//Show S11
    {
        std::vector<double> aux = 20*log(abs(S11_));
        QVector<double> S11 = QVector<double>::fromStdVector(aux);
        QColor S11Color = Tool_Settings.TraceColor.at(1);
        PlotWidget->addGraph();
        PlotWidget->graph()->setName(QString("S11 (dB)"));
        PlotWidget->graph()->setPen(QPen(S11Color));
        PlotWidget->graph()->setData(freq, S11);
    }

    if (Tool_Settings.ShowTraces.at(2))//Show S22
    {
        std::vector<double> aux = 20*log(abs(S22_));
        QVector<double> S22 = QVector<double>::fromStdVector(aux);
        QColor S22Color = Tool_Settings.TraceColor.at(2);
        PlotWidget->addGraph();
        PlotWidget->graph()->setName(QString("S22 (dB)"));
        PlotWidget->graph()->setPen(QPen(S22Color));
        PlotWidget->graph()->setData(freq, S22);
    }

    //X axis step
    QSharedPointer<QCPAxisTickerFixed> fixedTickerX(new QCPAxisTickerFixed);
    fixedTickerX->setTickStep(Tool_Settings.xstep/k);
    PlotWidget->xAxis->setTicker(fixedTickerX);
    //Y axis step
    QSharedPointer<QCPAxisTickerFixed> fixedTickerY(new QCPAxisTickerFixed);
    fixedTickerY->setTickStep(Tool_Settings.ystep);
    PlotWidget->yAxis->setTicker(fixedTickerY);

    PlotWidget->replot();

    //Update Smith Chart
    std::complex<double> Z;
    Smith_plot->clear();
    for(unsigned int i = 0; i < S11_.size(); i++)
    {
        Z = ((std::complex<double>(1, 0)+S11_.at(i)))/(std::complex<double>(1, 0)-S11_.at(i));
        Smith_plot->setData(Z.real(),Z.imag());
    }


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


void QucsRFDesignerWindow::UpdateSparSweep()
{

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
    QCPRange xAxisRange = PlotWidget->xAxis->range();
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
    updateGraph(real(freq), data["S[2,1]"], data["S[1,1]"], data["S[2,2]"]);
}
