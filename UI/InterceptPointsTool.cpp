#include "InterceptPointsTool.h"

InterceptPointsTool::InterceptPointsTool()
{
    QGridLayout *InterceptPointsLayout =  new QGridLayout();
    QGridLayout *TwoTonesTestLayout =  new QGridLayout();


    QGroupBox * TwoTonesTestGB = new QGroupBox("Two-tones test");

    CalculateInterceptPointsCheckbox = new QCheckBox("Calculate intercept points");
    CalculateInterceptPointsCheckbox->setChecked(true);
    InterceptPointsLayout->addWidget(CalculateInterceptPointsCheckbox, 0, 0);

    CenterFreqSpinbox = new QDoubleSpinBox();
    CenterFreqSpinbox->setRange(0, 10000);
    CenterFreqSpinbox->setValue(500);
    TwoTonesTestLayout->addWidget(new QLabel("Central frequency"),0, 0);
    TwoTonesTestLayout->addWidget(CenterFreqSpinbox, 0, 1);
    TwoTonesTestLayout->addWidget(new QLabel("MHz"), 0, 2);

    deltaSpinBox = new QDoubleSpinBox();
    deltaSpinBox->setRange(1, 10000);
    deltaSpinBox->setValue(100);
    TwoTonesTestLayout->addWidget(new QLabel("BW between carriers"),1, 0);
    TwoTonesTestLayout->addWidget(deltaSpinBox, 1, 1);
    TwoTonesTestLayout->addWidget(new QLabel("kHz"), 1, 2);


    FundamentalPowerSpinbox = new QDoubleSpinBox();
    FundamentalPowerSpinbox->setValue(10);
    TwoTonesTestLayout->addWidget(new QLabel("Fundamental power"), 2, 0);
    TwoTonesTestLayout->addWidget(FundamentalPowerSpinbox, 2, 1);
    TwoTonesTestLayout->addWidget(new QLabel("dBm"), 2, 2);

    IM3PowerSpinbox = new QDoubleSpinBox();
    IM3PowerSpinbox->setRange(-100, 100);
    IM3PowerSpinbox->setValue(-25);
    TwoTonesTestLayout->addWidget(new QLabel("IM3 power"),3, 0);
    TwoTonesTestLayout->addWidget(IM3PowerSpinbox, 3, 1);
    TwoTonesTestLayout->addWidget(new QLabel("dBm"), 3, 2);

    IM2PowerSpinbox = new QDoubleSpinBox();
    IM2PowerSpinbox->setRange(-100, 100);
    IM2PowerSpinbox->setValue(-20);
    TwoTonesTestLayout->addWidget(new QLabel("IM2 power"), 4, 0);
    TwoTonesTestLayout->addWidget(IM2PowerSpinbox, 4, 1);
    TwoTonesTestLayout->addWidget(new QLabel("dBm"), 4, 2);

    GainSpinbox = new QDoubleSpinBox();
    GainSpinbox->setRange(-100, 100);
    GainSpinbox->setValue(10);
    TwoTonesTestLayout->addWidget(new QLabel("Gain"), 5, 0);
    TwoTonesTestLayout->addWidget(GainSpinbox, 5, 1);
    TwoTonesTestLayout->addWidget(new QLabel("dB"), 5, 2);

    TwoTonesTestGB->setLayout(TwoTonesTestLayout);


    QGroupBox * IP3GB = new QGroupBox("IP3");
    QGridLayout *IP3GB_Layout = new QGridLayout();
    OIP3Spinbox = new QDoubleSpinBox();
    OIP3Spinbox->setMinimum(-100);
    OIP3Spinbox->setValue(30);
    IIP3LineEdit = new QLineEdit();
    //OIP3
    IP3GB_Layout->addWidget(new QLabel("OIP3"), 0, 0);
    IP3GB_Layout->addWidget(OIP3Spinbox, 0, 1);
    IP3GB_Layout->addWidget(new QLabel("dBm"), 0, 2);
    //IIP3
    IP3GB_Layout->addWidget(new QLabel("IIP3"), 1, 0);
    IP3GB_Layout->addWidget(IIP3LineEdit, 1, 1);
    IP3GB_Layout->addWidget(new QLabel("dBm"), 1, 2);
    IP3GB->setLayout(IP3GB_Layout);

    QGroupBox * IP2GB = new QGroupBox("IP2");
    QGridLayout *IP2GB_Layout = new QGridLayout();
    OIP2Spinbox = new QDoubleSpinBox();
    OIP2Spinbox->setMinimum(-100);
    OIP2Spinbox->setValue(40);
    IIP2LineEdit = new QLineEdit();
    //OIP2
    IP2GB_Layout->addWidget(new QLabel("OIP2"), 0, 0);
    IP2GB_Layout->addWidget(OIP2Spinbox, 0, 1);
    IP2GB_Layout->addWidget(new QLabel("dBm"), 0, 2);
    //IIP2
    IP2GB_Layout->addWidget(new QLabel("IIP2"), 1, 0);
    IP2GB_Layout->addWidget(IIP2LineEdit, 1, 1);
    IP2GB_Layout->addWidget(new QLabel("dBm"), 1, 2);
    IP2GB->setLayout(IP2GB_Layout);

    InterceptPointsLayout->addWidget(TwoTonesTestGB, 1, 0, 1, 2);
    InterceptPointsLayout->addWidget(IP3GB, 2, 0);
    InterceptPointsLayout->addWidget(IP2GB, 2, 1);

    //Set background color role
    QPalette pal = FundamentalPowerSpinbox->palette();
    pal.setColor(FundamentalPowerSpinbox->backgroundRole(), Qt::yellow);
    FundamentalPowerSpinbox->setPalette(pal);
    IM3PowerSpinbox->setPalette(pal);
    IM2PowerSpinbox->setPalette(pal);


    connect(CalculateInterceptPointsCheckbox, SIGNAL(clicked(bool)), this, SLOT(EnableCalcIP()));
    connect(FundamentalPowerSpinbox, SIGNAL(valueChanged(double)), this, SLOT(CalculateInterceptPoints()));
    connect(GainSpinbox, SIGNAL(valueChanged(double)), this, SLOT(SendSimulationData()));
    connect(IM3PowerSpinbox, SIGNAL(valueChanged(double)), this, SLOT(CalculateInterceptPoints()));
    connect(IM2PowerSpinbox, SIGNAL(valueChanged(double)), this, SLOT(CalculateInterceptPoints()));

    connect(OIP3Spinbox, SIGNAL(valueChanged(double)), this, SLOT(UpdateInterceptPoints()));
    connect(OIP2Spinbox, SIGNAL(valueChanged(double)), this, SLOT(UpdateInterceptPoints()));

    connect(CenterFreqSpinbox, SIGNAL(valueChanged(double)), this, SLOT(CalculateInterceptPoints()));
    connect(deltaSpinBox, SIGNAL(valueChanged(double)), this, SLOT(CalculateInterceptPoints()));

    this->setLayout(InterceptPointsLayout);
}


void InterceptPointsTool::EnableCalcIP()
{
    if (CalculateInterceptPointsCheckbox->isChecked())
    {
        FundamentalPowerSpinbox->setEnabled(true);
        IM3PowerSpinbox->setEnabled(true);
        IM2PowerSpinbox->setEnabled(true);
    }
    else
    {
        FundamentalPowerSpinbox->setEnabled(false);
        IM3PowerSpinbox->setEnabled(false);
        IM2PowerSpinbox->setEnabled(false);
    }

}

//This function calculates the IP3 and IP2 from the two tone test and pass them to the intercept diagram simulator
void InterceptPointsTool::CalculateInterceptPoints()
{
    IPdata.IP3 = FundamentalPowerSpinbox->value() + .5*(FundamentalPowerSpinbox->value() - IM3PowerSpinbox->value());
    IPdata.IP2 = 2*FundamentalPowerSpinbox->value() - IM2PowerSpinbox->value();

    IPdata.Pout = FundamentalPowerSpinbox->value();
    IPdata.IM2 = IM2PowerSpinbox->value();
    IPdata.IM3 = IM3PowerSpinbox->value();

    //These spinboxes are updated, so if the background was yellow (indicating outdated data), now it will be cleared
    FundamentalPowerSpinbox->setAutoFillBackground(false);
    IM2PowerSpinbox->setAutoFillBackground(false);
    IM3PowerSpinbox->setAutoFillBackground(false);

    //Update IP3/IP2 boxes
    OIP3Spinbox->blockSignals(true);
    OIP2Spinbox->blockSignals(true);
    OIP3Spinbox->setValue(IPdata.IP3);
    OIP2Spinbox->setValue(IPdata.IP2);
    //Update input intercept points
    IIP3LineEdit->setText(QString("%1").arg(IPdata.IP3-IPdata.Gain));
    IIP2LineEdit->setText(QString("%1").arg(IPdata.IP2-IPdata.Gain));
    OIP3Spinbox->blockSignals(false);
    OIP2Spinbox->blockSignals(false);

    SendSimulationData();
}


void InterceptPointsTool::UpdateInterceptPoints()
{
    IPdata.IP3 = OIP3Spinbox->value();
    IPdata.IP2 = OIP2Spinbox->value();
    //Update input intercept points
    IIP3LineEdit->setText(QString("%1").arg(IPdata.IP3-IPdata.Gain));
    IIP2LineEdit->setText(QString("%1").arg(IPdata.IP2-IPdata.Gain));

    //Now the Pout, IM2 and IM3 spinboxes data is outdated. In order to present valid data in the spectrum display
    //we need to calculate some valid Pout, IM2, IM3 data
    IPdata.Pout = IPdata.IP3-15;//Let's choose some Pout near P1dB (P1dB is typically 10-15dB below IP3 depending on the device technology)
    double Pin = IPdata.Pout - IPdata.Gain;
    IPdata.IM3 = 3*Pin - 2*IPdata.IP3 + 3*IPdata.Gain;
    IPdata.IM2 = 2*Pin -   IPdata.IP2 + 2*IPdata.Gain;

    //The data presented by the following spinboxes is now outdated, so their background will be set to yellow
    FundamentalPowerSpinbox->setAutoFillBackground(true);
    IM2PowerSpinbox->setAutoFillBackground(true);
    IM3PowerSpinbox->setAutoFillBackground(true);

    SendSimulationData();
}


void InterceptPointsTool::SendSimulationData()
{
    IPdata.Gain = GainSpinbox->value();
    IPdata.fc = CenterFreqSpinbox->value()*1e6;
    IPdata.delta = deltaSpinBox->value()*1e3;
    emit simulateDiagram(IPdata);
}
