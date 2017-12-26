#include "SmithChartTool.h"

SmithChartTool::SmithChartTool()
{
    //Initialize list of components
    NumberComponents[Capacitor] = 0;
    NumberComponents[Inductor] = 0;
    NumberComponents[Term] = 0;
    NumberComponents[GND] = 0;
    NumberComponents[ConnectionNodes] = 0;

    //****************************** BUILD THE UI **************************
    QGridLayout *SmithChartToolLayout =  new QGridLayout();
    FC_Spinbox = new QDoubleSpinBox();
    FC_Spinbox->setValue(1000);
    freq_scale_Combobox = new QComboBox();
    freq_scale_Combobox->addItem("GHz");
    freq_scale_Combobox->addItem("MHz");
    freq_scale_Combobox->addItem("kHz");
    freq_scale_Combobox->addItem("Hz");
    freq_scale_Combobox->setCurrentIndex(1);//MHz

    SmithChartToolLayout->addWidget(new QLabel("Frequency"), 0, 0);
    SmithChartToolLayout->addWidget(FC_Spinbox, 0, 1);
    SmithChartToolLayout->addWidget(freq_scale_Combobox, 0, 2);

    SourceImpedanceReal = new QDoubleSpinBox();
    SourceImpedanceReal->setValue(50);
    SourceImpedanceImag = new QDoubleSpinBox();

    LoadImpedanceReal = new QDoubleSpinBox();
    LoadImpedanceReal->setValue(10);
    LoadImpedanceImag = new QDoubleSpinBox();

    //Source
    SmithChartToolLayout->addWidget(new QLabel("Z0"), 1, 0);
    SmithChartToolLayout->addWidget(SourceImpedanceReal, 1, 1);
    SmithChartToolLayout->addWidget(new QLabel("+j"), 1, 2);
    SmithChartToolLayout->addWidget(SourceImpedanceImag, 1, 3);
    SmithChartToolLayout->addWidget(new QLabel("Ohm"), 1, 4);

    //Load
    SmithChartToolLayout->addWidget(new QLabel("ZL"), 2, 0);
    SmithChartToolLayout->addWidget(LoadImpedanceReal, 2, 1);
    SmithChartToolLayout->addWidget(new QLabel("+j"), 2, 2);
    SmithChartToolLayout->addWidget(LoadImpedanceImag, 2, 3);
    SmithChartToolLayout->addWidget(new QLabel("Ohm"), 2, 4);

    QHBoxLayout * buttonpanel = new QHBoxLayout();
    QGroupBox *ButtonsGroupBox = new QGroupBox("Parts");

    CsButton = new QPushButton();
    CsButton->setIcon(QIcon(":/bitmaps/SeriesCapacitor.png"));
    CsButton->setIconSize(QSize(65,65));
    CsButton->setMinimumSize(50,50);

    CpButton = new QPushButton();
    CpButton->setIcon(QIcon(":/bitmaps/ShuntCapacitor.png"));
    CpButton->setIconSize(QSize(65,65));
    CpButton->setMinimumSize(50,50);

    LsButton = new QPushButton();
    LsButton->setIcon(QIcon(":/bitmaps/SeriesInductor.png"));
    LsButton->setIconSize(QSize(65,65));
    LsButton->setMinimumSize(50,50);

    LpButton = new QPushButton();
    LpButton->setIcon(QIcon(":/bitmaps/ShuntInductor.png"));
    LpButton->setIconSize(QSize(65,65));
    LpButton->setMinimumSize(50,50);

    TLButton = new QPushButton();
    TLButton->setIcon(QIcon(":/bitmaps/TransmissionLine.png"));
    TLButton->setIconSize(QSize(65,65));
    TLButton->setMinimumSize(50,50);

    OCStubButton = new QPushButton();
    OCStubButton->setIcon(QIcon(":/bitmaps/OpenCircuitStub.png"));
    OCStubButton->setIconSize(QSize(65,65));
    OCStubButton->setMinimumSize(50,50);

    SCStubButton = new QPushButton();
    SCStubButton->setIcon(QIcon(":/bitmaps/ShortCircuitStub.png"));
    SCStubButton->setIconSize(QSize(65,65));
    SCStubButton->setMinimumSize(50,50);


    RemovePartButton = new QPushButton();
    RemovePartButton->setIcon(QIcon(":/bitmaps/RemoveIcon.png"));
    RemovePartButton->setIconSize(QSize(65,65));
    RemovePartButton->setMinimumSize(50,50);


    buttonpanel->addWidget(CsButton);
    buttonpanel->addWidget(CpButton);
    buttonpanel->addWidget(LsButton);
    buttonpanel->addWidget(LpButton);
    buttonpanel->addWidget(TLButton);
    buttonpanel->addWidget(OCStubButton);
    buttonpanel->addWidget(SCStubButton);
    buttonpanel->addWidget(RemovePartButton);

    ButtonsGroupBox->setLayout(buttonpanel);

    SmithChartToolLayout->addWidget(ButtonsGroupBox, 3, 0,1,3);

    ListOfComponents = new QListWidget();
    ListOfProperties = new QListWidget();
    PropertyValue = new QDoubleSpinBox();
    PropertyUnits = new QComboBox();

    SmithChartToolLayout->addWidget(ListOfComponents, 4, 0);
    SmithChartToolLayout->addWidget(ListOfProperties, 4, 1);
    SmithChartToolLayout->addWidget(PropertyValue, 4, 2);
    SmithChartToolLayout->addWidget(PropertyUnits, 4, 3);

    this->setLayout(SmithChartToolLayout);

    connect(CsButton, SIGNAL(clicked(bool)), this, SLOT(addSeriesCapacitor()));
    connect(CpButton, SIGNAL(clicked(bool)), this, SLOT(addShuntCapacitor()));
    connect(LsButton, SIGNAL(clicked(bool)), this, SLOT(addSeriesInductor()));
    connect(LpButton, SIGNAL(clicked(bool)), this, SLOT(addShuntInductor()));
    connect(TLButton, SIGNAL(clicked(bool)), this, SLOT(addTransmissionLine()));
    connect(OCStubButton, SIGNAL(clicked(bool)), this, SLOT(addOpenCircuitStub()));
    connect(SCStubButton, SIGNAL(clicked(bool)), this, SLOT(addShortCircuitStub()));
    connect(RemovePartButton, SIGNAL(clicked(bool)), this, SLOT(RemovePart()));

    connect(ListOfComponents, SIGNAL(clicked(QModelIndex)), this, SLOT(UpdateComponentPropertiesList()));
    connect(ListOfComponents, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(UpdateComponentPropertiesList()));
}

//This function builds the schematic and emits the corresponding signal to simulate the circuit in the main class
void SmithChartTool::simulate()
{
    ComponentInfo C, L, Ground, TL;
    WireInfo WI;
    NodeInfo NI;

    Nodes.clear();
    Wires.clear();

    int posx = 0, delta=50;
    std::vector<double> position(2);

    //Add (temporarily) term1
    ComponentInfo Term1;
    Term1.setParams("Term1", Term, vertical, 0, 0, "N0", "gnd");
    Term1.val["Z"] = num2str(SourceImpedanceReal->value(), Resistance);
    Components.push_front(Term1);
    QString last_node =  Term1.ID;//Auxiiar variable to wire components between loop cycles

    posx+=delta;

    //This loop iterates the whole components list and builds the ladder
    for (int i = 1; i < Components.length(); i++)
    {
        //At this point we only need to know if the orientation is series or shunt to set
        // the component location since the component type has already been defined at the pushbutton handlers
        if (Components[i].Orientation == horizontal)
        {//Series
            position[0] = posx;
            position[1] = 0;
            WI.setParams(last_node, 1, Components[i].ID, 0);
            Wires.append(WI);
            last_node = Components[i].ID;
        }
        else
        {//Shunt

            if (Components[i].ID.contains("GND", Qt::CaseInsensitive))
            {//It's ground, so the ypos must be lower and substract the increment in the x axis
                posx-=delta;
                position[0] = posx;
                position[1] = 75;
                // GND always appear related to a shunt element
                WI.setParams(Components[i-1].ID, 0, Components[i].ID, 0);
                Wires.append(WI);
            }
            else
            {//Shunt component
                //We need to add a node and wire the current and the previous element to that
                NI.setParams(QString("N%1").arg(++NumberComponents[ConnectionNodes]), posx, 0);
                Nodes.append(NI);

                position[0] = posx;
                position[1] = 25;

                WI.setParams(Components[i].ID, 1, NI.ID, 0);
                Wires.append(WI);

                WI.setParams(last_node, 1, NI.ID, 0);
                Wires.append(WI);

                last_node = Components[i].ID;
            }
        }
        Components[i].Coordinates = position;
        posx+=delta;

    }

    //Add (temporarily) term2
    ComponentInfo Term2;
    Term2.setParams("Term2", Term, horizontal, posx, 0, "N0", "gnd");
    Term2.val["Z"] = num2str(LoadImpedanceReal->value(), Resistance);
    Components.push_back(Term2);
    WI.setParams(last_node, 1, Term2.ID, 0);
    Wires.append(WI);

    QMap <QString, QPen> displaygraphs;
    displaygraphs[QString("S[2,1]")] = QPen(Qt::red, 1, Qt::SolidLine);
    displaygraphs[QString("S[1,1]")] = QPen(Qt::blue, 1, Qt::SolidLine);


    SchInfo.Comps = Components;
    SchInfo.Nodes = Nodes;
    SchInfo.Wires = Wires;
    SchInfo.displayGraphs = displaygraphs;

   // SchInfo.displayGraphs = DCF->displaygraphs;*/

    //Remove terms from the components list
    Components.pop_back();
    Components.pop_front();

    emit simulateNetwork(SchInfo);

}

void SmithChartTool::UpdateComponentPropertiesList()
{
    ListOfProperties->clear();
    if (ListOfComponents->count()==0)return;//The last component was already removed
    QString ComponentName = ListOfComponents->currentItem()->text();
    if (ComponentName[0] == 'C')
    {
        ListOfProperties->addItem("C");
    }
    if (ComponentName[0] == 'L')
    {
        ListOfProperties->addItem("L");
    }
    if (ComponentName[0] == 'T')
    {
        ListOfProperties->addItem("Length");
        ListOfProperties->addItem("Z0");
    }

    //Update QSpinBox with the current value of the selected property
    ListOfProperties->setCurrentRow(0);

    QString compval = Components[ListOfComponents->currentRow()].val[ListOfProperties->currentItem()->text()];
    PropertyValue->setValue(5);

}

void SmithChartTool::addSeriesCapacitor()
{
    ComponentInfo Cseries(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, horizontal, 0, 0, "", "");
    Cseries.val["C"] = "1pF";
    ListOfComponents->addItem(Cseries.ID);
    Components.append(Cseries);
    simulate();
}

void SmithChartTool::addShuntCapacitor()
{
    ComponentInfo Cshunt(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical, 0, 0, "", "");
    ListOfComponents->addItem(Cshunt.ID);
    Cshunt.val["C"] = "1pF";
    Components.append(Cshunt);
    ComponentInfo Ground;
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 0, 0, "", "");
    Components.append(Ground);
    simulate();
}

void SmithChartTool::addSeriesInductor()
{
    ComponentInfo Lseries(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal, 0, 0, "", "");
    Lseries.val["L"] = "10nH";
    ListOfComponents->addItem(Lseries.ID);
    Components.append(Lseries);
    simulate();
}

void SmithChartTool::addShuntInductor()
{
    ComponentInfo Lshunt(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, vertical, 0, 0, "", "");
    Lshunt.val["L"] = "10nH";
    ListOfComponents->addItem(Lshunt.ID);
    Components.append(Lshunt);
    ComponentInfo Ground;
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 0, 0, "", "");
    Components.append(Ground);
    simulate();
}

void SmithChartTool::addTransmissionLine()
{
    ComponentInfo TL(QString("TL%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 0, 0, "", "");
    TL.val["Z"] = "50Ohm";
    TL.val["L"] = "10mm";
    ListOfComponents->addItem(TL.ID);
    Components.append(TL);
    simulate();
}

void SmithChartTool::addOpenCircuitStub()
{
    ComponentInfo OStub(QString("TL%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, 0, "", "");
    OStub.val["Z"] = "50Ohm";
    OStub.val["L"] = "10mm";
    ListOfComponents->addItem(OStub.ID);
    Components.append(OStub);
    ComponentInfo Ground;
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 0, 0, "", "");
    Components.append(Ground);
    simulate();
}

void SmithChartTool::addShortCircuitStub()
{
    ComponentInfo SStub(QString("TL%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, 0, "", "gnd");
    SStub.val["Z"] = "50Ohm";
    SStub.val["L"] = "10mm";
    ListOfComponents->addItem(SStub.ID);
    Components.append(SStub);
    ComponentInfo Ground;
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 0, 0, "", "");
    Components.append(Ground);
    simulate();
}

void SmithChartTool::RemovePart()
{
    int removeIndex = ListOfComponents->currentIndex().row();//UI list
    if ((removeIndex < 0) || (removeIndex >= ListOfComponents->count()))return;//Check that the component index exists


    QString ID = ListOfComponents->currentItem()->text();
    int removeIndexCompList;//Internal component list
    for (int i = 0; i < Components.length(); i++)
    {
        if (Components[i].ID == ID)
        {
            removeIndexCompList = i;
            break;
        }
    }

    ComponentType type = Components[removeIndexCompList].Type;
    ListOfComponents->blockSignals(true);
    qDeleteAll(ListOfComponents->selectedItems());//Remove from the UI list
    if (Components[removeIndexCompList].Orientation == vertical)
    {//It's a shunt element, so it is a gnd there...
      Components.removeAt(removeIndexCompList);
    }
    Components.removeAt(removeIndexCompList);//Remove from the internal component list (where Components[0] is the input term, hence +1)

    //Update the component counters
    switch (type)
    {
      case Capacitor:
        NumberComponents[Capacitor]--;
        break;
      case Inductor:
        NumberComponents[Inductor]--;
        break;
       case TransmissionLine:
        NumberComponents[TransmissionLine]--;
        break;
    }

    simulate();
    ListOfComponents->blockSignals(false);
    UpdateComponentPropertiesList();
}
