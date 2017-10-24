#include "QucsRFDesignerWindow.h"

QucsRFDesignerWindow::QucsRFDesignerWindow()
{
    //Check if the mySQL driver for Qt is installed
    if (!QSqlDatabase::drivers().contains("QMYSQL"))
        QMessageBox::critical(this, "Database not loaded", "You need to install the QMYSQL driver to load the Zverev tables");

    // Establish connection with the database
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("ZverevTables");
    db.setUserName("QucsRFdesigner");
    db.setPassword("");
    DBservice = db.open();

    if (DBservice)
    {
        //Identify data available in the tables to fill the user input widgets
        QSqlQuery query;
        QStringList Columns;
        QStringList TablesID = db.tables();
        QMap<QString, struct PrototypeTableProperties> TablesProperties;
        for (int i = 0; i < TablesID.length(); i++)
        {
            //Get the columns
            query.exec(QString("SHOW COLUMNS FROM ZverevTables.%1;").arg(TablesID.at(i)));
            query.first();
            Columns.clear();
            do {
                Columns.append(query.value(0).toString());
            }while(query.next());

            //Get order available on each table
            for (int j = 0; j < Columns.size(); j++)
            {
                query.exec(QString("SELECT N FROM ZverevTables.%2;").arg(Columns.at(j)).arg(TablesID.at(i)));
                query.first();
                do {
                    int aux = query.value(0).toInt();
                    if (std::find(TablesProperties[TablesID.at(i)].N.begin(), TablesProperties[TablesID.at(i)].N.end(), aux) == TablesProperties[TablesID.at(i)].N.end())
                        TablesProperties[TablesID.at(i)].N.push_back(aux);
                }while(query.next());
            }
        }
    }
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

    QWidget *FilterDesignWidget = SetupFilterDesignGUI();
    //*********** Impedance Matching tab *********************
    QWidget *MatchingWidget = new QWidget();
    // So far, it is empty...

    TabWidget->addTab(FilterDesignWidget, "Filter design");
    TabWidget->addTab(MatchingWidget, "Matching");
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
    connect(SchematicWidget, SIGNAL(SendTuningSignalToMainWindow(ComponentInfo)), this, SLOT(addTuningPanel(ComponentInfo)));

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

    //Connection functions for updating the network requirements and simulate on the fly
    connect(FilterImplementationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateDesignParameters()));
    connect(CLCRadioButton, SIGNAL(toggled(bool)), this, SLOT(ChangeRL_CLC_LCL_mode()));
    //connect(LCLRadioButton, SIGNAL(toggled(bool)), this, SLOT(ChangeRL_CLC_LCL_mode()));
    connect(FilterResponseTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ResposeComboChanged()));
    connect(EllipticType, SIGNAL(currentIndexChanged(int)), this, SLOT(EllipticTypeChanged()));
    connect(FilterClassCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateDesignParameters()));
    connect(OrderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(UpdateDesignParameters()));
    connect(FCSpinbox, SIGNAL(valueChanged(double)), this, SLOT(UpdateDesignParameters()));
    connect(FC_ScaleCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateDesignParameters()));
    connect(BWSpinbox, SIGNAL(valueChanged(double)), this, SLOT(UpdateDesignParameters()));
    connect(BW_ScaleCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateDesignParameters()));
    connect(SourceImpedanceLineEdit, SIGNAL(textChanged(QString)), this, SLOT(UpdateDesignParameters()));
    connect(RippleSpinbox, SIGNAL(valueChanged(double)), this, SLOT(UpdateDesignParameters()));
    connect(StopbandAttSpinbox, SIGNAL(valueChanged(double)), this, SLOT(UpdateDesignParameters()));
    connect(EllipticType, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateDesignParameters()));
    connect(UseZverevTablesCheckBox, SIGNAL(clicked(bool)), this, SLOT(SwitchZverevTablesMode(bool)));
    connect(OrderCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateRL_and_Ripple()));
    connect(RippleCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateLoad_Impedance(int)));
    connect(RLCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateRipple(int)));

    PlotWidget->xAxis->setRange(SPAR_Settings.fstart/1e6, SPAR_Settings.fstop/1e6);
    PlotWidget->yAxis->setRange(Tool_Settings.ymin, Tool_Settings.ymax);

    //Update specifications
    UpdateDesignParameters();
}





QucsRFDesignerWindow::~QucsRFDesignerWindow()
{
    delete PreferencesAction;
    delete RFToolBar;
    delete SchematicWidget;
    delete PlotWidget;
    delete FilterClassCombo;
    delete FilterResponseTypeCombo;
    delete FilterImplementationCombo;
    delete FC_ScaleCombobox;
    delete BW_ScaleCombobox;
    delete EllipticType;
    delete FCSpinbox;
    delete BWSpinbox;
    delete RippleSpinbox;
    delete StopbandAttSpinbox;
    delete OrderSpinBox;
    delete SourceImpedanceLineEdit;
    delete CLCRadioButton;
    delete LCLRadioButton;
    delete StopbandAttLabel;
    delete StopbandAttdBLabel;
    delete EllipticTypeLabel;

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

void QucsRFDesignerWindow::addTuningPanel(ComponentInfo PartInfo)
{
    if (PartInfo.Type==GND) return;//Ground components cannot be tuned!
    //Check if the component is already being tuned
    foreach (TuningBox * TB, TunableComponents) {
        if (!QString::compare(TB->getID(), PartInfo.ID, Qt::CaseSensitive))
        {
            TunableComponents.removeOne(TB);
            TB->close();
            delete TB;
            return;//If the parameter is already being tuned...
        }
    }

    TuningBox *TB = new TuningBox(PartInfo);
    TunableComponents.push_back(TB);
    connect(TB, SIGNAL(ClosingEvent(QString)), this, SLOT(CloseTuningBox(QString)));
    connect(TB, SIGNAL(TuningChanged(ComponentInfo)), this, SLOT(TuningChanged(ComponentInfo)));
    addDockWidget(Qt::BottomDockWidgetArea, TB);
}

//This function handles the event of moving the tuners
void QucsRFDesignerWindow::TuningChanged(ComponentInfo CI)
{
    qDebug() << "Tuning changed: " << CI.ID << " -> " << CI.val.at(0);
    SchematicWidget->ModifyComponent(CI);//Change the component value in the schematic

    int index1 = netlist.indexOf(CI.ID);
    int index2 = netlist.indexOf("\n", index1);
    QString oldLine = netlist.mid(index1, index2-index1);//Find component line

    QString prop;
    if (CI.Type==Inductor) prop = "L";
    if (CI.Type==Capacitor) prop = "C";
    if (CI.Type==Term) prop = "Z";

    //Find property
    int index3 = oldLine.indexOf(prop+"=")+3;
    int index4 = oldLine.indexOf("\"", index3);
    QString currentVal =  oldLine.mid(index3, index4-index3);
    QString aux = oldLine;
    aux.replace(currentVal, QString::number(CI.val.at(0)));
    netlist.replace(oldLine, aux);

    QFile file("netlist");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << netlist << endl;
    }
    file.close();
    system("qucsator -i netlist -o data.dat");

    // Load info from Qucs dataset and update graph
    QMap<QString, vector<complex<double> > > data = loadQucsDataSet("data.dat");
    //Update graph
    vector<complex<double> > freq=data["frequency"];
    updateGraph(real(freq), data["S[2,1]"], data["S[1,1]"], data["S[2,2]"]);

}


void QucsRFDesignerWindow::CloseTuningBox(QString TB_ID)
{
    //Remove the box from the tunable components list
    foreach (TuningBox * TB, TunableComponents) {
        if (!QString::compare(TB->getID(), TB_ID, Qt::CaseSensitive))
        {
            TunableComponents.removeOne(TB);
            TB->close();
            //  delete TB;
            return;//If the parameter is already being tuned...
        }
    }
}


//This function updates the content of the schematic window and the display
void QucsRFDesignerWindow::UpdateWindows()
{
    SchematicWidget->clear();//Remove the components in the scene
    QList<ComponentInfo> Comps;
    QList<WireInfo> Wires;
    QList<NodeInfo> Nodes;
    EllipticFilter *EF;
    CanonicalFilter *CF;
    DirectCoupledFilters *DCF;
    netlist.clear();

    //Recalculate network
    if (FilterImplementationCombo->currentText() == "LC Ladder")
    {
        if (FilterResponseTypeCombo->currentText() == "Elliptic")
        {
            EF = new EllipticFilter(Filter_SP);
            EF->synthesize();
            netlist = EF->getQucsNetlist();
            Comps = EF->getComponents();
            Wires = EF->getWires();
            Nodes = EF->getNodes();
            delete EF;
        }
        else
        {
            CF = new CanonicalFilter(Filter_SP);
            CF->synthesize();
            netlist = CF->getQucsNetlist();
            Comps = CF->getComponents();
            Wires = CF->getWires();
            Nodes = CF->getNodes();
            delete CF;
        }
     }
    if (FilterImplementationCombo->currentText() == "LC Direct Coupled")
    {
           DCF = new DirectCoupledFilters(Filter_SP);
           DCF->synthesize();
           netlist = DCF->getQucsNetlist();
           Comps = DCF->getComponents();
           Wires = DCF->getWires();
           Nodes = DCF->getNodes();
           delete DCF;
    }
    //Write Qucs netlist into file
    netlist += QString(".SP:SP1 Type=\"lin\" Start=\"%1 Hz\" Stop=\"%2 Hz\" Points=\"%3\"\n").arg(SPAR_Settings.fstart).arg(SPAR_Settings.fstop).arg(SPAR_Settings.n_points);
    QFile file("netlist");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << netlist << endl;
    }
    file.close();
    system("qucsator -i netlist -o data.dat");

    //Update schematic window
    SchematicWidget->setComponents(Comps);
    SchematicWidget->setNodes(Nodes);
    SchematicWidget->setWires(Wires);

    // Load info from Qucs dataset and update graph
    QMap<QString, vector<complex<double> > > data = loadQucsDataSet("data.dat");
    //Update graph
    vector<complex<double> > freq=data["frequency"];
    updateGraph(real(freq), data["S[2,1]"], data["S[1,1]"], data["S[2,2]"]);

}

void QucsRFDesignerWindow::ResposeComboChanged()
{
    bool ActivateCauer = !FilterResponseTypeCombo->currentText().compare("Elliptic");
    StopbandAttSpinbox->setVisible(ActivateCauer);
    StopbandAttLabel->setVisible(ActivateCauer);
    StopbandAttdBLabel->setVisible(ActivateCauer);

    EllipticType->setVisible(ActivateCauer);
    EllipticTypeLabel->setVisible(ActivateCauer);
    OrderSpinBox->setMinimum(1);
    if (ActivateCauer)
    {
        CLCRadioButton->setText("Min L");
        LCLRadioButton->setText("Min C");
    }
    else
    {
        CLCRadioButton->setText("CLC");
        LCLRadioButton->setText("LCL");
    }

    QString table = FilterResponseTypeCombo->currentText();

    if (UseZverevTablesCheckBox->isChecked())
    {
        //When the response type is changed, the values of RL and ripple must be updated according to the content of the Zverev tables
        QStringList data;
        QSqlQuery query;

        //Fill order data
        table = FilterResponseTypeCombo->currentText().trimmed();//Name of the table without whitespaces
        query.exec(QString("SELECT N FROM ZverevTables.%1;").arg(table));
        query.first();
        do {
            QString aux = query.value(0).toString();
            if (!data.contains(aux)) data.append(aux);
        }while(query.next());
        OrderCombobox->blockSignals(true);
        OrderCombobox->clear();
        OrderCombobox->addItems(data);
        QString aux = OrderCombobox->currentText();
        OrderCombobox->setCurrentIndex(0);//Set the default selection
        // ... and try to find the same order in the recently updated data.
        for (int i = 0; i < OrderCombobox->count(); i++)
        {
            if (OrderCombobox->itemText(i) == aux)
            {
                OrderCombobox->setCurrentIndex(i);
                break;
            }
        }
        OrderCombobox->blockSignals(false);
        UpdateRipple(0);
        UpdateLoad_Impedance(1);
    }


    if ((table != "Chebyshev") &&(table != "LinearPhase")&&(table != "Gegenbauer"))
    {
        (UseZverevTablesCheckBox->isChecked()) ? RippleCombobox->hide() : RippleSpinbox->hide();
        RippleLabel->hide();
        RippledBLabel->hide();
    }
    else
    {
        if (table == "Gegenbauer") RippleLabel->setText("Alpha");
        if (table == "Chebyshev") RippleLabel->setText("Ripple");
        if (table == "LinearPhase") RippleLabel->setText("Phase ripple");
        (UseZverevTablesCheckBox->isChecked()) ? RippleCombobox->show() : RippleSpinbox->show();
        RippleLabel->show();
        RippledBLabel->show();
    }

    UpdateDesignParameters();
    this->adjustSize();
}

// This function catches the events related to the changes in the filter specificatios
void QucsRFDesignerWindow::UpdateDesignParameters()
{
    Filter_SP.Implementation = FilterImplementationCombo->currentText();
    //************************** Set filter response *********************************
    if (!FilterResponseTypeCombo->currentText().compare("Chebyshev")) Filter_SP.FilterResponse = Chebyshev;
    if (!FilterResponseTypeCombo->currentText().compare("Butterworth")) Filter_SP.FilterResponse = Butterworth;
    if (!FilterResponseTypeCombo->currentText().compare("Legendre")) Filter_SP.FilterResponse = Legendre;
    if (!FilterResponseTypeCombo->currentText().compare("Elliptic"))Filter_SP.FilterResponse = Elliptic;
    if (!FilterResponseTypeCombo->currentText().compare("Bessel"))Filter_SP.FilterResponse = Bessel;
    if (!FilterResponseTypeCombo->currentText().compare("Gegenbauer"))Filter_SP.FilterResponse = Gegenbauer;
    if (!FilterResponseTypeCombo->currentText().compare("LinearPhase"))Filter_SP.FilterResponse = LinearPhaseEqError;

    //**************************** Set filter type **************************************
    if (FilterImplementationCombo->currentText() == "LC Ladder")
    {
       if (!FilterClassCombo->currentText().compare("Lowpass")) Filter_SP.FilterType = Lowpass;
       if (!FilterClassCombo->currentText().compare("Highpass")) Filter_SP.FilterType = Highpass;
       if (!FilterClassCombo->currentText().compare("Bandpass")) Filter_SP.FilterType = Bandpass;
       if (!FilterClassCombo->currentText().compare("Bandstop")) Filter_SP.FilterType = Bandstop;
    }
    if (FilterImplementationCombo->currentText() == "LC Direct Coupled")
    {
        Filter_SP.FilterType = Bandpass;
        FilterClassCombo->setCurrentIndex(2);
    }
    //**************************** Set coupling ********************************************
    if (!DC_CouplingTypeCombo->currentText().compare("Capacitative")) Filter_SP.DC_Coupling = Capacitative;
    if (!DC_CouplingTypeCombo->currentText().compare("Inductive")) Filter_SP.DC_Coupling = Inductive;
    if (!DC_CouplingTypeCombo->currentText().compare("Magnetic")) Filter_SP.DC_Coupling = Magnetic;

    //Update user input
    if ((!FilterClassCombo->currentText().compare("Lowpass")) || (!FilterClassCombo->currentText().compare("Highpass")))
    {
        BWSpinbox->setEnabled(false);
        BW_ScaleCombobox->setEnabled(false);
    }
    else
    {
        BWSpinbox->setEnabled(true);
        BW_ScaleCombobox->setEnabled(true);
        if (Filter_SP.bw >= Filter_SP.fc)
        {
        BWSpinbox->blockSignals(true);
        BWSpinbox->setValue(0.1*FCSpinbox->value());//10% BW
        BW_ScaleCombobox->setCurrentIndex(FC_ScaleCombobox->currentIndex());
        BWSpinbox->blockSignals(false);
        }
    }

    if ((Filter_SP.FilterType == Lowpass) || (Filter_SP.FilterType == Highpass))
    {
        SPAR_Settings.fstart = Filter_SP.fc*0.5;
        SPAR_Settings.fstop = Filter_SP.fc*1.5;
        SPAR_Settings.n_points = 200;
    }
    else
    {
        SPAR_Settings.fstart = (Filter_SP.fc-Filter_SP.bw/2)*0.5;
        SPAR_Settings.fstop = (Filter_SP.fc-Filter_SP.bw/2)*1.5;
        SPAR_Settings.n_points = 200;
    }

    if (Filter_SP.Implementation == "LC Direct Coupled")
    {
        DC_CouplingTypeCombo->show();
        DC_CouplingLabel->show();
        FilterClassCombo->blockSignals(true);
        FilterResponseTypeCombo->blockSignals(true);
        FilterClassCombo->setCurrentIndex(2);
        FilterClassCombo->setEnabled(false);
        FilterResponseTypeCombo->clear();
        QStringList DC_responses = DefaultFilterResponses;
        DC_responses.removeAt(DC_responses.indexOf("Elliptic"));
        DC_responses.removeAt(DC_responses.indexOf("Cauer"));
        FilterResponseTypeCombo->addItems(DC_responses);
        FilterClassCombo->blockSignals(false);
        FilterResponseTypeCombo->blockSignals(false);
    }
    else
    {
        DC_CouplingTypeCombo->hide();
        DC_CouplingLabel->hide();
        FilterClassCombo->setEnabled(true);
        FilterResponseTypeCombo->blockSignals(true);
        QString CurrentResponse = FilterResponseTypeCombo->currentText();
        FilterResponseTypeCombo->clear();
        FilterResponseTypeCombo->addItems(DefaultFilterResponses);
        for (int i = 0; i < DefaultFilterResponses.length(); i++)
        {
            if (CurrentResponse == DefaultFilterResponses.at(i))
            {
              FilterResponseTypeCombo->setCurrentIndex(i);
              break;
            }
        }
        FilterResponseTypeCombo->blockSignals(false);
    }

    //Update parameters
    Filter_SP.bw = BWSpinbox->value()*getScale(BW_ScaleCombobox->currentText());
    Filter_SP.fc = FCSpinbox->value()*getScale(FC_ScaleCombobox->currentText());
    Filter_SP.isCLC = CLCRadioButton->isChecked();
    Filter_SP.ZS = SourceImpedanceLineEdit->text().toDouble();
    Filter_SP.UseZverevTables = UseZverevTablesCheckBox->isChecked();
    Filter_SP.Implementation = FilterImplementationCombo->currentText();

    if (!UseZverevTablesCheckBox->isChecked())
    {
        Filter_SP.order = OrderSpinBox->value();
        Filter_SP.Ripple = RippleSpinbox->value();
        Filter_SP.ZL = Filter_SP.ZS;
        Filter_SP.as = StopbandAttSpinbox->value();
        Filter_SP.EllipticType = EllipticType->currentText();
    }
    else
    {//The data comes from comboboxes rather than spinboxes
        Filter_SP.order = OrderCombobox->currentText().toInt();
        Filter_SP.Ripple = RippleCombobox->currentText().toDouble();
        bool ok;
        Filter_SP.ZL = RLCombobox->currentText().toDouble(&ok);
        if (!ok) Filter_SP.ZL = 1e8*Filter_SP.ZS;//It is a single ended filter
    }



    //Update windows
    UpdateSparSweep();
}

// This function returns the scale of the argument
double QucsRFDesignerWindow::getScale(QString scale)
{
    if (!scale.compare("GHz")) return 1e9;
    if (!scale.compare("MHz")) return 1e6;
    if (!scale.compare("kHz")) return 1e3;
    if (!scale.compare("Hz")) return 1;
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
    QCPRange xAxisRange = PlotWidget->xAxis->range();
    (xAxisRange.lower < 0) ? SPAR_Settings.fstart = 0: SPAR_Settings.fstart =xAxisRange.lower*1e6;
    (xAxisRange.upper < 0) ? SPAR_Settings.fstop = 0: SPAR_Settings.fstop =xAxisRange.upper*1e6;
    qDebug() << "SPAN: " << xAxisRange.lower*1e6 << "  " << xAxisRange.upper*1e6;

    UpdateWindows();
}


void QucsRFDesignerWindow::EllipticTypeChanged()
{
    if (EllipticType->currentText() != "Type S")
    {
        OrderSpinBox->setMinimum(2);
    }
    else
    {
        OrderSpinBox->setMinimum(1);
    }
}


void QucsRFDesignerWindow::SwitchZverevTablesMode(bool ZverevMode)
{
    if (FilterResponseTypeCombo->currentText() == "Elliptic") return;//Zverev mode is only available for canonical responses
    if (ZverevMode)
    {
        QStringList data;
        QSqlQuery query;
        //Fill the response type combobox
        data.clear();
        query.exec(QString("SHOW TABLES FROM ZverevTables;"));
        query.first();
        do {
            data.append(query.value(0).toString());
        }while(query.next());
        QString aux = FilterResponseTypeCombo->currentText();
        FilterResponseTypeCombo->blockSignals(true);//Avoid calling slots while setting up this widget
        FilterResponseTypeCombo->clear();
        FilterResponseTypeCombo->addItems(data);
        //Find the index of the current response type and select it for the Zverev mode
        for (int i = 0; i < FilterResponseTypeCombo->count();i++)
        {
            if (FilterResponseTypeCombo->itemText(i) == aux)
            {
                FilterResponseTypeCombo->setCurrentIndex(i);
                break;
            }
        }
        ResposeComboChanged();//Fill the user input widgets with the parameters of the Zverev tables

        OrderSpinBox->hide();
        OrderCombobox->show();
        RLlabel->show();
        RLlabelOhm->show();
        RLCombobox->show();
        RippleCombobox->show();
        RippleSpinbox->hide();
        FilterResponseTypeCombo->blockSignals(false);
    }
    else
    {//Conventional mode. Restore the default filter responses
        OrderSpinBox->show();
        OrderCombobox->hide();
        OrderCombobox->clear();
        RLlabel->hide();
        RLlabelOhm->hide();
        RLCombobox->hide();
        RippleCombobox->hide();
        RippleSpinbox->show();

        QString aux = FilterResponseTypeCombo->currentText();//Last response selected in the Zverev mode
        FilterResponseTypeCombo->clear();
        FilterResponseTypeCombo->addItems(DefaultFilterResponses);//Fill the combo with the default option
        for (int i = 0; i < FilterResponseTypeCombo->count();i++)
        {//Find the index of the current response type and select it for the conventional mode
            if (FilterResponseTypeCombo->itemText(i) == aux)
            {
                FilterResponseTypeCombo->setCurrentIndex(i);
                break;
            }
        }
        //Update order and ripple
        OrderSpinBox->setValue(OrderCombobox->currentText().toInt());
        RippleSpinbox->setValue(RippleCombobox->currentText().toDouble());
    }
    ResposeComboChanged();
    UpdateDesignParameters();
}

void QucsRFDesignerWindow::ChangeRL_CLC_LCL_mode()
{
    RLCombobox->blockSignals(true);
    RLCombobox->setInsertPolicy(QComboBox::InsertAfterCurrent);
    if (UseZverevTablesCheckBox->isChecked())
    {
        bool mode = CLCRadioButton->isChecked();
        double ZS = SourceImpedanceLineEdit->text().toDouble();
        for (int i = 0; i < RLCombobox->count(); i++)
        {
            double aux = RLCombobox->itemText(i).toDouble();

            if (mode)
            {//CLC mode selected, then previously it was LCL
                aux /= ZS;
                aux = 1/aux;
                aux *= ZS;
            }
            else
            {//LCL mode selected, then previously it was CLC
                aux/= ZS;//Now aux = gi or aux = 1/gi
                aux = ZS/aux;
            }
            RLCombobox->insertItem(i, QString("%1").arg(aux));
            RLCombobox->removeItem(i+1);
        }
    }
    for (int i = 0; i < RLCombobox->count(); i++) qDebug() << QString("%1").arg(RLCombobox->itemText(i).toDouble());
    qDebug() << "Current index: " << RLCombobox->currentIndex() << " = " << RLCombobox->currentText().toDouble();
    RLCombobox->blockSignals(false);
    UpdateDesignParameters();
    return;
}


void QucsRFDesignerWindow::UpdateRipple(int refresh = 0)
{
    if (OrderCombobox->currentText().isEmpty()) return;
    double Ripple_CurrentVal = RippleCombobox->currentText().toDouble();
    double min_dist_err = 1e6;
    int index = 0, selected_index = 0;
    QStringList data;
    QSqlQuery query;
    QString table = FilterResponseTypeCombo->currentText().trimmed();//Name of the table without whitespaces

    data.clear();
    QString query_str;


    if ((FilterResponseTypeCombo->currentText() == "LinearPhase") || (FilterResponseTypeCombo->currentText() == "Chebyshev")|| (FilterResponseTypeCombo->currentText() == "Gegenbauer"))
    {
        //Fill ripple combobox
        data.clear();
        query_str = QString("SELECT CASE WHEN N = '%1' THEN Ripple ELSE -1 END as Ripple FROM ZverevTables.%2;").arg(OrderCombobox->currentText()).arg(table);
        query.exec(query_str);
        query.first();
        do {
            if (query.value(0).toDouble() == -1) continue;
            double aux = query.value(0).toDouble();
            if (!data.contains(QString::number(aux)))
            {
                data.append(QString::number(aux));
                if (std::abs(aux-Ripple_CurrentVal) < min_dist_err)
                {
                    min_dist_err = std::abs(aux-Ripple_CurrentVal);
                    selected_index = index;
                }
                index++;
            }

        }while(query.next());
        RippleCombobox->blockSignals(true);
        RippleCombobox->clear();
        RippleCombobox->addItems(data);
        RippleCombobox->setCurrentIndex(selected_index);
        RippleCombobox->blockSignals(false);
    }

    if (refresh>-1)UpdateDesignParameters();
}


void QucsRFDesignerWindow::UpdateLoad_Impedance(int refresh = 0)
{
    QStringList data;
    QSqlQuery query;
    QString table = FilterResponseTypeCombo->currentText().trimmed();//Name of the table without whitespaces
    double ZL_CurrentVal = RLCombobox->currentText().toDouble(), min_dist_err = 1e6;
    int index=0, selected_index=0;
    data.clear();
    QString query_str;

    if ((FilterResponseTypeCombo->currentText() == "LinearPhase") || (FilterResponseTypeCombo->currentText() == "Chebyshev")|| (FilterResponseTypeCombo->currentText() == "Gegenbauer"))
    {
        query_str = QString("SELECT CASE WHEN N = '%1' AND Ripple = '%3' THEN RL ELSE -1 END as RL FROM ZverevTables.%2;").arg(OrderCombobox->currentText()).arg(table).arg(RippleCombobox->currentText());
    }
    else
    {
        query_str = QString("SELECT CASE WHEN N = '%1' THEN RL ELSE -1 END as RL FROM ZverevTables.%2;").arg(OrderCombobox->currentText()).arg(table);
    }

    data.clear();
    query.exec(query_str);
    query.first();
    do {
        if (query.value(0).toDouble() == -1) continue;
        double aux = SourceImpedanceLineEdit->text().toDouble();
        (CLCRadioButton->isChecked()) ? aux *= query.value(0).toDouble(): aux/=query.value(0).toDouble();
        if (aux < 1e6)
        {
            if (!data.contains(QString::number(aux)))
            {
                data.append(QString::number(aux));
                if (std::abs(aux-ZL_CurrentVal) < min_dist_err)
                {
                    min_dist_err = std::abs(aux-ZL_CurrentVal);
                    selected_index = index;
                }
                index++;
            }
        }
        else
        {
            if (CLCRadioButton->isChecked())
            {
                if (!data.contains("inf")) data.append("inf");
                if (RLCombobox->currentText() == "inf") selected_index = index;
            }
            else
            {
                if (!data.contains("0")) data.append("0");
                if (RLCombobox->currentText() == "0") selected_index = index;

            }

        }
    }while(query.next());
    RLCombobox->blockSignals(true);
    RLCombobox->clear();
    RLCombobox->addItems(data);
    RLCombobox->setCurrentIndex(selected_index);
    RLCombobox->blockSignals(false);
    if (refresh>-1) UpdateDesignParameters();
}


void QucsRFDesignerWindow::UpdateRL_and_Ripple()
{
    UpdateRipple(-1);
    UpdateLoad_Impedance(1);
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
