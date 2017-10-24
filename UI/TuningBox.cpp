#include "TuningBox.h"
#include <qdebug.h>
TuningBox::TuningBox(ComponentInfo TI)
{
    TuningInfo = TI;
    QGridLayout *mainLayout = new QGridLayout(this);
    MaxSpinBox = new QDoubleSpinBox();
    MinSpinBox = new QDoubleSpinBox();
    CurrentValueSpinbox= new QDoubleSpinBox();

    MaxSpinBox->setDecimals(2);
    MaxSpinBox->setSingleStep(0.1);

    MinSpinBox->setDecimals(2);
    MinSpinBox->setSingleStep(0.1);

    CurrentValueSpinbox->setDecimals(2);
    CurrentValueSpinbox->setSingleStep(0.1);

    MaxValScaleCombo = new QComboBox();
    MinValScaleCombo = new QComboBox();
    CurrentValScaleCombo = new QComboBox();

    //Add items to the comboboxes
    MaxValScaleCombo->addItems(createItems(TI));
    MinValScaleCombo->addItems(createItems(TI));
    CurrentValScaleCombo->addItems(createItems(TI));


    //Default scale
    if (TI.Type==Capacitor)
    {
        MaxValScaleCombo->setCurrentIndex(1);
        MinValScaleCombo->setCurrentIndex(1);
        CurrentValScaleCombo->setCurrentIndex(1);
    }

    if (TI.Type==Inductor)
    {
        MaxValScaleCombo->setCurrentIndex(2);
        MinValScaleCombo->setCurrentIndex(2);
        CurrentValScaleCombo->setCurrentIndex(2);
    }

    if (TI.Type==Term)
    {
        MaxValScaleCombo->setCurrentIndex(5);
        MinValScaleCombo->setCurrentIndex(5);
        CurrentValScaleCombo->setCurrentIndex(5);
    }

    MaxSpinBox->setValue(1.5*TI.val[""]*getScale(MaxValScaleCombo->currentIndex()));
    MinSpinBox->setValue(0.5*TI.val[""]*getScale(MinValScaleCombo->currentIndex()));
    CurrentValueSpinbox->setValue(TI.val[""]*getScale(CurrentValScaleCombo->currentIndex()));

    IncreaseValueButton = new QPushButton("Up");
    DecreaseValueButton = new QPushButton("Down");

    ValueSlider = new QSlider();
    ValueSlider->setTickPosition(QSlider::TicksLeft);
    ValueSlider->setValue(50);
    mainLayout->addWidget(new QLabel(TI.ID),0,0,1,2,Qt::AlignCenter);
    mainLayout->addWidget(new QLabel("Value:"),1,0);
    mainLayout->addWidget(CurrentValueSpinbox,4,1,Qt::AlignCenter);
    mainLayout->addWidget(CurrentValScaleCombo, 4, 2);
    mainLayout->addWidget(new QLabel("Max:"),2,0);
    mainLayout->addWidget(MaxSpinBox,2,1);
    mainLayout->addWidget(MaxValScaleCombo, 2, 2);
    mainLayout->addWidget(IncreaseValueButton,3,0,1,2);
    mainLayout->addWidget(ValueSlider,4,0,Qt::AlignCenter);
    mainLayout->addWidget(DecreaseValueButton,5,0,1,2);
    mainLayout->addWidget(new QLabel("Min:"),6,0);
    mainLayout->addWidget(MinSpinBox,6,1);
    mainLayout->addWidget(MinValScaleCombo, 6, 2);

    this->setLayout(mainLayout);
    QWidget *TuningBoxWidget = new QWidget();
    TuningBoxWidget->setLayout(mainLayout);
    this->setWidget(TuningBoxWidget);
    setMinimumSize(QSize(80,50));
    connect(IncreaseValueButton, SIGNAL(clicked(bool)), this, SLOT(IncreaseValue()));
    connect(DecreaseValueButton, SIGNAL(clicked(bool)), this, SLOT(DecreaseValue()));
    connect(ValueSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderChanged(int)));
    connect(this,SIGNAL(visibilityChanged(bool)),this,SLOT(VisibilityHandler(bool)));//Connect the closing event
}

QString TuningBox::getID()
{
    return TuningInfo.ID;
}


void TuningBox::SliderChanged(int val)
{
    double min_value = MinSpinBox->value()/getScale(MinValScaleCombo->currentIndex());
    double max_value = MaxSpinBox->value()/getScale(MaxValScaleCombo->currentIndex());
    double step = 1e-2*(max_value-min_value);
    double current_value = min_value+val*step;
    TuningInfo.val[""] = current_value;
    CurrentValueSpinbox->setValue(current_value*getScale(CurrentValScaleCombo->currentIndex()));
    emit this->TuningChanged(TuningInfo);
}

//This slot captures the closing event
void TuningBox::VisibilityHandler(bool visible)
{
    if (visible == false) emit this->ClosingEvent(TuningInfo.ID);
}


void TuningBox::IncreaseValue()
{
    ValueSlider->setValue(ValueSlider->value()+1);
}



void TuningBox::DecreaseValue()
{
    ValueSlider->setValue(ValueSlider->value()-1);
}


QStringList TuningBox::createItems(ComponentInfo CI)
{
    QStringList scale;
    QString Unit;
    if (CI.Type == Capacitor) Unit = "F";
    if (CI.Type == Inductor) Unit = "H";
    if (CI.Type == Term) Unit = "Ohm";

    scale.append(QString("f%1").arg(Unit));//femto
    scale.append(QString("p%1").arg(Unit));//pico
    scale.append(QString("n%1").arg(Unit));//nano
    scale.append(QString("u%1").arg(Unit));//micro
    scale.append(QString("m%1").arg(Unit));//mili
    scale.append(QString("%1").arg(Unit));
    return scale;
}


double TuningBox::getScale(int index)
{
    switch (index)
    {
    case 0: return 1e15;
    case 1: return 1e12;
    case 2: return 1e9;
    case 3: return 1e6;
    case 4: return 1e3;
    case 5: return 1;
    }
}
