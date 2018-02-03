#include "SmithChartTool.h"

SmithChartTool::SmithChartTool()
{
    //Initialize list of components
    NumberComponents[Capacitor] = 0;
    NumberComponents[Inductor] = 0;
    NumberComponents[Term] = 0;
    NumberComponents[GND] = 0;
    NumberComponents[ConnectionNodes] = 0;

    CapacitanceUnits.clear();
    CapacitanceUnits.append("fF");
    CapacitanceUnits.append("pF");
    CapacitanceUnits.append("nF");
    CapacitanceUnits.append("uF");

    InductanceUnits.clear();
    InductanceUnits.append("fH");
    InductanceUnits.append("pH");
    InductanceUnits.append("nH");
    InductanceUnits.append("uH");

    ResistanceUnits.clear();
    ResistanceUnits.append("Ohm");
    ResistanceUnits.append("kOhm");

    LengthUnits.clear();
    LengthUnits.append("nm");
    LengthUnits.append("um");
    LengthUnits.append("mm");
    LengthUnits.append("cm");
    LengthUnits.append("m");

    //****************************** BUILD THE UI **************************
    QGridLayout *SmithChartToolLayout =  new QGridLayout();
    FC_Spinbox = new QDoubleSpinBox();
    FC_Spinbox->setMaximum(10000);
    FC_Spinbox->setMinimum(1);
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
    SourceImpedanceReal->setMaximum(1000);
    SourceImpedanceReal->setMinimum(1);
    SourceImpedanceImag = new QDoubleSpinBox();
    SourceImpedanceImag->setMaximum(1000);
    SourceImpedanceImag->setMinimum(-1000);

    LoadImpedanceReal = new QDoubleSpinBox();
    LoadImpedanceReal->setValue(50);
    LoadImpedanceReal->setMaximum(1000);
    LoadImpedanceReal->setMinimum(1);
    LoadImpedanceImag = new QDoubleSpinBox();
    LoadImpedanceImag->setMaximum(1000);
    LoadImpedanceImag->setMinimum(-1000);

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

    Z0_Spinbox = new QDoubleSpinBox();
    Z0 = 50;
    Z0_Spinbox->setValue(Z0);
    Z0_Spinbox->setMinimum(1);
    Z0_Spinbox->setMaximum(1000);
    SmithChartToolLayout->addWidget(new QLabel("Z0"), 3, 0);
    SmithChartToolLayout->addWidget(Z0_Spinbox, 3, 1);
    SmithChartToolLayout->addWidget(new QLabel("Ohm"), 3, 2 );


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

    SmithChartToolLayout->addWidget(ButtonsGroupBox, 4, 0,1,3);

    ListOfComponents = new QListWidget();
    ListOfProperties = new QListWidget();
    PropertyValue = new QDoubleSpinBox();
    PropertyValue->setMinimum(0.1);
    PropertyValue->setMaximum(1000);
    PropertyValue->setSingleStep(0.1);
    PropertyUnits = new QComboBox();

    SmithChartToolLayout->addWidget(ListOfComponents, 5, 0);
    SmithChartToolLayout->addWidget(ListOfProperties, 5, 1);
    SmithChartToolLayout->addWidget(PropertyValue, 5, 2);
    SmithChartToolLayout->addWidget(PropertyUnits, 5, 3);

    this->setLayout(SmithChartToolLayout);

    connect(CsButton, SIGNAL(clicked(bool)), this, SLOT(addSeriesCapacitor()));
    connect(CpButton, SIGNAL(clicked(bool)), this, SLOT(addShuntCapacitor()));
    connect(LsButton, SIGNAL(clicked(bool)), this, SLOT(addSeriesInductor()));
    connect(LpButton, SIGNAL(clicked(bool)), this, SLOT(addShuntInductor()));
    connect(TLButton, SIGNAL(clicked(bool)), this, SLOT(addTransmissionLine()));
    connect(OCStubButton, SIGNAL(clicked(bool)), this, SLOT(addOpenCircuitStub()));
    connect(SCStubButton, SIGNAL(clicked(bool)), this, SLOT(addShortCircuitStub()));
    connect(RemovePartButton, SIGNAL(clicked(bool)), this, SLOT(RemovePart()));
    connect(SourceImpedanceReal, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(SourceImpedanceImag, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(LoadImpedanceReal, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(LoadImpedanceImag, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(FC_Spinbox, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(freq_scale_Combobox, SIGNAL(currentIndexChanged(QString)), this, SLOT(UpdatePropertyValue()));
    connect(SourceImpedanceReal, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(SourceImpedanceImag, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(LoadImpedanceReal, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(LoadImpedanceImag, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));

    connect(ListOfComponents, SIGNAL(clicked(QModelIndex)), this, SLOT(UpdateComponentPropertiesList()));
    connect(ListOfComponents, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(UpdateComponentPropertiesList()));
    connect(ListOfProperties, SIGNAL(currentTextChanged(QString)), this, SLOT(UpdateValueUnits(QString)));
    connect(PropertyValue, SIGNAL(valueChanged(double)), this, SLOT(UpdatePropertyValue()));
    connect(PropertyUnits, SIGNAL(currentIndexChanged(QString)), this, SLOT(UpdatePropertyValue()));
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

    CalcImpedanceTrace();//Get the impedance transformations

    emit simulateNetwork(SchInfo);

}

void SmithChartTool::UpdateValueUnits(QString property)
{
    //Update the combobox
    PropertyUnits->blockSignals(true);
    PropertyValue->blockSignals(true);
    PropertyUnits->clear();
    if (property == "C")
        PropertyUnits->addItems(CapacitanceUnits);
    else if (property == "L")
        PropertyUnits->addItems(InductanceUnits);
        else if (property == "Length")
            PropertyUnits->addItems(LengthUnits);
           else if (property == "Z0")
               PropertyUnits->addItems(ResistanceUnits);
                else return;

    //Update the value of the spinbox
    QString val = Components[ListOfComponents->currentRow()].val[ListOfProperties->currentItem()->text()];
    int index = -1;
    for (int i = 0; i < val.length(); i++)
    {
        if (val.at(i).isLetter())
        {
            index = i;
            break;
        }
    }
    PropertyValue->setValue(val.mid(0, index).toDouble());
    //Get the proper index
    index = PropertyUnits->findText(val.mid(index));
    PropertyUnits->setCurrentIndex(index);
    PropertyUnits->blockSignals(false);
    PropertyValue->blockSignals(false);
}

void SmithChartTool::UpdateComponentPropertiesList()
{
    ListOfComponents->blockSignals(true);
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
    ListOfComponents->blockSignals(false);
}

void SmithChartTool::addSeriesCapacitor()
{
    ComponentInfo Cseries(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, horizontal, 0, 0, "", "");
    Cseries.val["C"] = "1pF";
    ListOfComponents->addItem(Cseries.ID);
    Components.append(Cseries);
    ListOfComponents->setCurrentRow(ListOfComponents->count()-1);//Select the last element
    UpdateComponentPropertiesList();

    //Set the default value in the combos
    PropertyValue->setValue(1);
    PropertyUnits->setCurrentIndex(1);//pF

    simulate();
}

void SmithChartTool::addShuntCapacitor()
{
    ComponentInfo Cshunt(QString("C%1").arg(++NumberComponents[Capacitor]), Capacitor, vertical, 0, 0, "", "");
    Cshunt.val["C"] = "1pF";
    ListOfComponents->addItem(Cshunt.ID);
    Components.append(Cshunt);
    ComponentInfo Ground;
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 0, 0, "", "");
    Components.append(Ground);

    ListOfComponents->setCurrentRow(ListOfComponents->count()-1);//Select the last element
    UpdateComponentPropertiesList();

    //Set the default value in the combos
    PropertyValue->setValue(1);
    PropertyUnits->setCurrentIndex(1);//pF

    simulate();
}

void SmithChartTool::addSeriesInductor()
{
    ComponentInfo Lseries(QString("L%1").arg(++NumberComponents[Inductor]), Inductor, horizontal, 0, 0, "", "");
    Lseries.val["L"] = "10nH";
    ListOfComponents->addItem(Lseries.ID);
    Components.append(Lseries);

    ListOfComponents->setCurrentRow(ListOfComponents->count()-1);//Select the last element
    UpdateComponentPropertiesList();

    //Set the default value in the combos
    PropertyValue->setValue(10);
    PropertyUnits->setCurrentIndex(2);//nH

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

    ListOfComponents->setCurrentRow(ListOfComponents->count()-1);//Select the last element
    UpdateComponentPropertiesList();

    //Set the default value in the combos
    PropertyValue->setValue(10);
    PropertyUnits->setCurrentIndex(2);//nH

    simulate();
}

void SmithChartTool::addTransmissionLine()
{
    ComponentInfo TL(QString("TL%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, horizontal, 0, 0, "", "");
    TL.val["Z0"] = "50Ohm";
    TL.val["Length"] = "5mm";
    ListOfComponents->addItem(TL.ID);
    Components.append(TL);

    ListOfComponents->setCurrentRow(ListOfComponents->count()-1);//Select the last element
    UpdateComponentPropertiesList();

    //Set the default value in the combos
    PropertyValue->setValue(5);
    PropertyUnits->setCurrentIndex(2);//mm

    simulate();
}

void SmithChartTool::addOpenCircuitStub()
{
    ComponentInfo OStub(QString("TL%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, 0, "", "");
    OStub.val["Z0"] = "50Ohm";
    OStub.val["Length"] = "5mm";
    ListOfComponents->addItem(OStub.ID);
    Components.append(OStub);

    ListOfComponents->setCurrentRow(ListOfComponents->count()-1);//Select the last element
    UpdateComponentPropertiesList();

    //Set the default value in the combos
    PropertyValue->setValue(5);
    PropertyUnits->setCurrentIndex(2);//mm

    simulate();
}

void SmithChartTool::addShortCircuitStub()
{
    ComponentInfo SStub(QString("TL%1").arg(++NumberComponents[TransmissionLine]), TransmissionLine, vertical, 0, 0, "", "gnd");
    SStub.val["Z0"] = "50Ohm";
    SStub.val["Length"] = "5mm";
    ListOfComponents->addItem(SStub.ID);
    Components.append(SStub);
    ComponentInfo Ground;
    Ground.setParams(QString("GND%1").arg(++NumberComponents[GND]), GND, vertical, 0, 0, "", "");
    Components.append(Ground);

    ListOfComponents->setCurrentRow(ListOfComponents->count()-1);//Select the last element
    UpdateComponentPropertiesList();

    //Set the default value in the combos
    PropertyValue->setValue(5);
    PropertyUnits->setCurrentIndex(2);//mm

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


void SmithChartTool::SelectComponent(ComponentInfo CI)
{
  //Iterate the list and select the component
    for (int i = 0; i < ListOfComponents->count(); i++)
    {
        if (ListOfComponents[i].currentItem()->text() == CI.ID)
        {
            ListOfComponents->setCurrentRow(i);
            return;
        }
    }
}

//Updates the value of the selected property of the current component
void SmithChartTool::UpdatePropertyValue()
{
  double suffix = 1;
  if (PropertyUnits->currentText().contains("f")) suffix = 1e-15;
     else if (PropertyUnits->currentText().contains("p")) suffix = 1e-12;
          else if (PropertyUnits->currentText().contains("n")) suffix = 1e-9;
               else if (PropertyUnits->currentText().contains("u")) suffix = 1e-6;
                    else if (PropertyUnits->currentText().contains("m")) suffix = 1e-3;
                         else if (PropertyUnits->currentText().contains("k")) suffix = 1e3;

  //Find the component and update the property value
  int updateCompIndex;//Internal component list
  for (int i = 0; i < Components.length(); i++)
  {
      if (Components[i].ID == ListOfComponents->currentItem()->text())
      {
          updateCompIndex = i;
          break;
      }
  }

  Components[updateCompIndex].val[ListOfProperties->currentItem()->text()] = PropertyValue->text() + PropertyUnits->currentText();
  simulate();
}

//Get the frequency at which the matching network is designed
double SmithChartTool::getFrequency()
{
    double scale = 1;
    if (freq_scale_Combobox->currentText() == "GHz") scale = 1e9;
    else if (freq_scale_Combobox->currentText() == "MHz") scale = 1e6;
    else if (freq_scale_Combobox->currentText() == "kHz") scale = 1e3;

    return FC_Spinbox->value()*scale;
}

QPointF SmithChartTool::getZS()
{
   return QPointF(SourceImpedanceReal->value(), SourceImpedanceImag->value());
}

void SmithChartTool::CalcImpedanceTrace()
{
   QVector<QPointF> Ztrace;
   int npoints = 200;//Point per arc
   QPointF Zstart, Yaux, dZ, dY;
   QPointF Zaux;
   double freq = getFrequency();
   Z0 = Z0_Spinbox->value();
   Ztrace.clear();
   Ztrace.append(getZS()/Z0);
   for (int i = 0; i <  SchInfo.Comps.length(); i++)
   {
       if (SchInfo.Comps.at(i).Type == Capacitor)
       {
           if (SchInfo.Comps.at(i).Orientation == horizontal)
           {//Series
               double C_lowZ = 1/(2*M_PI*freq*1);//Calculate the capacity which presents a 1 Ohm impedance
               double dC = (C_lowZ - SchInfo.Comps[i].getVal("C"))/1e4;
               Zstart = Ztrace.last();//Start
               double Citer, Xiter;
               double Xdelta;
               double Cprev = C_lowZ;
               while (Citer != SchInfo.Comps[i].getVal("C"))
               {
                   Citer = Cprev - dC;//The trace is divided into npoints in order to get a continuous arc trace.
                   Xiter = 1/(2*M_PI*freq*Citer*Z0);
                   double Xlast = Ztrace.last().y();//Xc of the last point
                   Xdelta = std::abs(Xlast - Xiter);
                   while (Xdelta < 0.1)//Look for a significant value of Citer which produces a visual step in the Smith chart
                   {
                     Citer -= dC;
                     Xiter = 1/(2*M_PI*freq*Citer*Z0);
                     Xdelta = std::abs(Xlast- Xiter);
                   }
                   if (Citer < SchInfo.Comps[i].getVal("C"))//It reached the reactance of the selected capacitor
                   {
                       Citer = SchInfo.Comps[i].getVal("C");
                       dZ = QPointF(0, -1/(2*M_PI*freq*Citer*Z0));
                       Ztrace.append(Zstart + dZ);
                       break;
                   }
                   Cprev = Citer;
                   dZ = QPointF(0, -1/(2*M_PI*freq*Citer*Z0));
                   Ztrace.append(Zstart + dZ);
               }
           }
           else
           {//Shunt
               double C_lowY = 1e-3/(2*M_PI*freq);//Calculate the capacity which presents a 1k Ohm impedance
               double dC = (SchInfo.Comps[i].getVal("C")-C_lowY)/1e2;
               Zstart = Ztrace.last();//Start
               double Citer, Biter;
               double Bdelta;
               double Cprev = C_lowY;
               std::complex<double> Z, Yaux, dZ;
               Z = std::complex<double>(Zstart.x(), Zstart.y());
               Yaux = std::complex<double>(1,0)/Z;
               std::complex<double> Ystart = Yaux;

               while (Citer != SchInfo.Comps[i].getVal("C"))
               {
                   Citer = Cprev + dC;//The trace is divided into npoints in order to get a continuous arc trace.
                   Biter = (2*M_PI*freq*Citer*Z0);
                   Z = std::complex<double>(Ztrace.last().x(), Ztrace.last().y());
                   Yaux = std::complex<double>(1,0)/Z;
                   double Blast = Yaux.imag();//Bc at the last point
                   Bdelta = std::abs(Blast - Biter);

                   while (Bdelta < 0.1)//Look for a significant value of Citer which produces a visual step in the Smith chart
                   {
                     Citer += dC;
                     Biter = (2*M_PI*freq*Citer*Z0);
                     Bdelta = std::abs(Blast- Biter);
                   }
                   if (Citer > SchInfo.Comps[i].getVal("C"))//It reached the reactance of the selected capacitor
                   {
                       Citer = SchInfo.Comps[i].getVal("C");
                       Yaux = Ystart + std::complex<double>(0, (2*M_PI*freq*Citer*Z0));
                       Yaux = std::complex<double>(1,0)/Yaux;
                       Ztrace.append(QPointF(Yaux.real(), Yaux.imag()));
                       break;
                   }
                   Cprev = Citer;
                   Yaux = Ystart + std::complex<double>(0, (2*M_PI*freq*Citer*Z0));
                   Yaux = std::complex<double>(1,0)/Yaux;
                   Ztrace.append(QPointF(Yaux.real(), Yaux.imag()));
               }
           }

       }
       if (SchInfo.Comps.at(i).Type == Inductor)
       {
           if (SchInfo.Comps.at(i).Orientation == horizontal)
           {//Series
               double L_lowZ = 1/(2*M_PI*freq);//Calculate the capacity which presents a 0.1 Ohm impedance
               double dL = (SchInfo.Comps[i].getVal("L") - L_lowZ)/npoints;
               Zaux = Ztrace.last();
               for (int p = 0; p < npoints; p++)
               {
                   double Liter = L_lowZ + p*dL;//The trace is divided into npoints in order to get a continuous arc trace.
                   dZ = QPointF(0, (2*M_PI*freq*Liter)/Z0);
                   Ztrace.append(Zaux+dZ);
               }
           }
           else
           {//Shunt
               double L_lowY = 100e-9;//1/(2*M_PI*freq*10);//Calculate the capacity which presents a 1k Ohm impedance
               double dL = (L_lowY - SchInfo.Comps[i].getVal("L"))/npoints;

               std::complex<double> Z, Yaux;
               Zaux = Ztrace.last();
               Z = std::complex<double>(Zaux.x(), Zaux.y());
               Yaux = std::complex<double>(1,0)/Z;
               for (int p = 0; p < npoints; p++)
               {
               double Liter = L_lowY - p*dL;//The trace is divided into npoints in order to get a continuous arc trace.
               double dY = -Z0/(2*M_PI*freq*Liter);
               Z = std::complex<double>(1,0)/(Yaux + std::complex<double>(0, dY));
               Ztrace.append(QPointF(Z.real(), Z.imag()));
               }
           }

       }
   }

   SchInfo.ImpedanceTrace = Ztrace;
}
