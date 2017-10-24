#include <UI/QucsRFDesignerWindow.h>

QWidget * QucsRFDesignerWindow::SetupFilterDesignGUI()
{
    QWidget *FilterDesignWidget = new QWidget();
    QGridLayout *FilterDesignLayout =  new QGridLayout();

    //********** Filter Implementation ***********
    FilterImplementationCombo = new QComboBox();
    FilterImplementationCombo->addItem("LC Ladder");
    FilterImplementationCombo->addItem("LC Direct Coupled");
    FilterDesignLayout->addWidget(new QLabel("Implementation"),0,0);
    FilterDesignLayout->addWidget(FilterImplementationCombo,0,1);
    //******** Tee or Pi (LC ladder only) ********
    QWidget *TeePiWidget = new QWidget();
    QHBoxLayout *TeePiLayout = new QHBoxLayout();
    CLCRadioButton = new QRadioButton("CLC");
    LCLRadioButton = new QRadioButton("LCL");
    CLCRadioButton->setChecked(true);
    TeePiLayout->addWidget(CLCRadioButton);
    TeePiLayout->addWidget(LCLRadioButton);
    TeePiWidget->setLayout(TeePiLayout);
    FilterDesignLayout->addWidget(TeePiWidget, 0,2);
    //************ Response type **************
    FilterResponseTypeCombo = new QComboBox();
    DefaultFilterResponses.append("Chebyshev");
    DefaultFilterResponses.append("Butterworth");
    DefaultFilterResponses.append("Elliptic");
    FilterResponseTypeCombo->addItems(DefaultFilterResponses);
    UseZverevTablesCheckBox = new QCheckBox("Use Zverev tables");
    UseZverevTablesCheckBox->setChecked(false);
    UseZverevTablesCheckBox->setEnabled(DBservice);
    FilterDesignLayout->addWidget(new QLabel("Response"),1,0);
    FilterDesignLayout->addWidget(FilterResponseTypeCombo,1,1);
    FilterDesignLayout->addWidget(UseZverevTablesCheckBox,1,2);
    //********** Direct coupled filters - Coupling type *****
    DC_CouplingTypeCombo = new QComboBox();
    DC_CouplingTypeCombo->addItem("Capacitative");
    DC_CouplingTypeCombo->addItem("Inductive");
    DC_CouplingTypeCombo->addItem("Magnetic");
    DC_CouplingLabel = new QLabel("Coupling");
    FilterDesignLayout->addWidget(DC_CouplingLabel,2,0);
    FilterDesignLayout->addWidget(DC_CouplingTypeCombo,2,1);
    DC_CouplingTypeCombo->hide();
    DC_CouplingLabel->hide();
    //************ Filter class ****************
    FilterClassCombo = new QComboBox();
    FilterClassCombo->addItem("Lowpass");
    FilterClassCombo->addItem("Highpass");
    FilterClassCombo->addItem("Bandpass");
    FilterClassCombo->addItem("Bandstop");
    FilterDesignLayout->addWidget(new QLabel("Class"),3,0);
    FilterDesignLayout->addWidget(FilterClassCombo,3,1);
    //*************** Order *******************
    OrderSpinBox = new QSpinBox();
    OrderSpinBox->setValue(2);
    OrderSpinBox->setMinimum(1);
    OrderCombobox = new QComboBox();//Combobox option for Zverev mode. This is filled up in the corresponding slot
    OrderCombobox->hide();
    FilterDesignLayout->addWidget(new QLabel("Order"),4,0);
    FilterDesignLayout->addWidget(OrderSpinBox,4,1);
    FilterDesignLayout->addWidget(OrderCombobox,4,1);
    //******* Cutoff freq (Lowpass and Highpass) *********
    //****** Central freq (Bandpass and Bandstop) *********
    FCSpinbox = new QDoubleSpinBox();
    FCSpinbox->setMinimum(1);
    FCSpinbox->setMaximum(1e6);
    FCSpinbox->setDecimals(0);
    FCSpinbox->setValue(1000);
    FCSpinbox->setSingleStep(1);//Step fixed to 1 Hz/kHz/MHz/GHz
    FC_ScaleCombobox = new QComboBox();
    FC_ScaleCombobox->addItem("GHz");
    FC_ScaleCombobox->addItem("MHz");
    FC_ScaleCombobox->addItem("kHz");
    FC_ScaleCombobox->addItem("Hz");
    FC_ScaleCombobox->setCurrentIndex(1);
    FilterDesignLayout->addWidget(new QLabel("Cutoff freq"),5,0);
    FilterDesignLayout->addWidget(FCSpinbox,5,1);
    FilterDesignLayout->addWidget(FC_ScaleCombobox,5,2);
    //************* Bandwidth ***********
    BWSpinbox = new QDoubleSpinBox();
    BWSpinbox->setMinimum(1);
    BWSpinbox->setMaximum(1e6);
    BWSpinbox->setDecimals(0);
    BWSpinbox->setValue(1000);
    BWSpinbox->setSingleStep(1);//Step fixed to 1 Hz/kHz/MHz/GHz
    BW_ScaleCombobox = new QComboBox();
    BW_ScaleCombobox->addItem("GHz");
    BW_ScaleCombobox->addItem("MHz");
    BW_ScaleCombobox->addItem("kHz");
    BW_ScaleCombobox->addItem("Hz");
    BW_ScaleCombobox->setCurrentIndex(1);
    FilterDesignLayout->addWidget(new QLabel("Bandwidth"),6,0);
    FilterDesignLayout->addWidget(BWSpinbox,6,1);
    FilterDesignLayout->addWidget(BW_ScaleCombobox,6,2);
    //************** Ripple ****************
    RippleSpinbox = new QDoubleSpinBox();
    RippleSpinbox->setMinimum(0.01);
    RippleSpinbox->setMaximum(2);
    RippleSpinbox->setValue(0.01);
    RippleSpinbox->setSingleStep(0.01);//Step fixed to 0.01dB
    RippleCombobox = new QComboBox();
    RippleCombobox->hide();
    RippleLabel = new QLabel("Ripple");
    RippledBLabel = new QLabel("dB");
    FilterDesignLayout->addWidget(RippleLabel,7,0);
    FilterDesignLayout->addWidget(RippleSpinbox,7,1);
    FilterDesignLayout->addWidget(RippleCombobox,7,1);
    FilterDesignLayout->addWidget(RippledBLabel,7,2);
    //************** Stopband attenuation ****************
    StopbandAttSpinbox = new QDoubleSpinBox();
    StopbandAttSpinbox->setMinimum(5);
    StopbandAttSpinbox->setMaximum(150);
    StopbandAttSpinbox->setValue(30);
    StopbandAttSpinbox->setSingleStep(0.1);//Step fixed to 0.1dB
    StopbandAttLabel = new QLabel("Stopband att");
    StopbandAttdBLabel = new QLabel("dB");
    FilterDesignLayout->addWidget(StopbandAttLabel,8,0);
    FilterDesignLayout->addWidget(StopbandAttSpinbox,8,1);
    FilterDesignLayout->addWidget(StopbandAttdBLabel,8,2);
    StopbandAttSpinbox->setVisible(false);
    StopbandAttLabel->setVisible(false);
    StopbandAttdBLabel->setVisible(false);
    //************** Elliptic type ****************
    EllipticTypeLabel = new QLabel("Elliptic type");
    EllipticType = new QComboBox();
    EllipticType->addItem("Type A");
    EllipticType->addItem("Type B");
    EllipticType->addItem("Type C");
    EllipticType->addItem("Type S");
    EllipticTypeLabel->setVisible(false);
    EllipticType->setVisible(false);
    FilterDesignLayout->addWidget(EllipticTypeLabel,9,0);
    FilterDesignLayout->addWidget(EllipticType,9,1);
    //************* Load resistance (Zverev mode only) ***********
    RLCombobox = new QComboBox();
    RLlabel = new QLabel("Load");
    RLlabelOhm = new QLabel(QChar(0xa9, 0x03));
    RLCombobox->hide();
    RLlabel->hide();
    RLlabelOhm->hide();
    FilterDesignLayout->addWidget(RLlabel,9,0);
    FilterDesignLayout->addWidget(RLCombobox,9,1);
    FilterDesignLayout->addWidget(RLlabelOhm,9,2);
    //**************** Phase error (Zverev mode only) ***********
    PhaseErrorLabel = new QLabel("Phase error");
    PhaseErrorCombobox = new QComboBox();
    PhaseErrorLabelDeg = new QLabel("deg");
    PhaseErrorLabel->hide();
    PhaseErrorCombobox->hide();
    PhaseErrorLabelDeg->hide();
    FilterDesignLayout->addWidget(PhaseErrorLabel,10,0);
    FilterDesignLayout->addWidget(PhaseErrorCombobox,10,1);
    FilterDesignLayout->addWidget(PhaseErrorLabelDeg,10,2);
    //************ Source impedance **********
    SourceImpedanceLineEdit = new QLineEdit("50");
    FilterDesignLayout->addWidget(new QLabel("ZS"),11,0);
    FilterDesignLayout->addWidget(SourceImpedanceLineEdit,11,1);
    FilterDesignLayout->addWidget(new QLabel(QChar(0xa9, 0x03)),11,2);

    FilterDesignWidget->setLayout(FilterDesignLayout);
    return FilterDesignWidget;

}
