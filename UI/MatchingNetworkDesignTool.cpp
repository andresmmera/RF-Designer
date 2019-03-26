/***************************************************************************
                                MatchingNetworkDesignTool.cpp
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
#include "MatchingNetworkDesignTool.h"
#include "Schematic/component.h"

MatchingNetworkDesignTool::MatchingNetworkDesignTool() {
  S1PW = new S1P_InputWidget();
  S2PW = new S2P_InputWidget();

  // Set some sample sample data for broadband matching
  DATA_S1P.Freq.resize(200);
  DATA_S1P.Z11.resize(200);
  for (int i = 0; i < 200; i++) {
    DATA_S1P.Freq[i] = (900 + i) * 1e6;
    DATA_S1P.Z11[i] = std::complex<double>(25, 0);
  }

  QGridLayout *MatchingNetworkDesignLayout = new QGridLayout();

  // Freq. dependent checkbox
  FreqDep_Checkbox = new QCheckBox("Freq. dependent load");
  MatchingNetworkDesignLayout->addWidget(FreqDep_Checkbox, 0, 0);
  connect(FreqDep_Checkbox, SIGNAL(clicked(bool)), this,
          SLOT(FreqDependentLoad()));

  // Two-port checkbox
  TwoPort_Matching_Checkbox = new QCheckBox("Two-port matching");
  MatchingNetworkDesignLayout->addWidget(TwoPort_Matching_Checkbox, 0, 1);
  TwoPort_Matching_Checkbox->setChecked(false);
  connect(TwoPort_Matching_Checkbox, SIGNAL(clicked(bool)), this,
          SLOT(SwitchSingle_TwoPort_MatchingMode()));

  // Groupbox for the frequency settings
  FreqModeSelectionGroupbox = new QGroupBox("Matching band settings");
  BroadbandRadioButton = new QRadioButton("Broadband matching");
  SingleFrequencyRadiobutton = new QRadioButton("Single freq.");
  SingleFrequencyRadiobutton->setChecked(true);
  QHBoxLayout *FreqSettingsLayout = new QHBoxLayout();
  FreqSettingsLayout->addWidget(SingleFrequencyRadiobutton);
  FreqSettingsLayout->addWidget(BroadbandRadioButton);
  FreqModeSelectionGroupbox->setLayout(FreqSettingsLayout);
  MatchingNetworkDesignLayout->addWidget(FreqModeSelectionGroupbox, 0, 2);
  connect(BroadbandRadioButton, SIGNAL(clicked(bool)), this,
          SLOT(BroadbandHandler()));
  connect(SingleFrequencyRadiobutton, SIGNAL(clicked(bool)), this,
          SLOT(BroadbandHandler()));

  // Topology
  Topology_Label = new QLabel("Topology");
  Topology_Combo = new QComboBox();
  QStringList matching_methods;
  matching_methods.append(tr("L-section"));
  matching_methods.append(tr("Single stub"));
  matching_methods.append(tr("Double stub"));
  matching_methods.append(QString("%1 %2/4")
                              .arg(tr("Multistage "))
                              .arg(QString(QChar(0xBB, 0x03))));
  matching_methods.append(tr("Cascaded L-sections"));
  matching_methods.append(QString("%1/4 line").arg(QChar(0xBB, 0x03)));
  matching_methods.append(QString("%1/8 + %1/4 line").arg(QChar(0xBB, 0x03)));
  matching_methods.append(QString("%1-type").arg(QChar(0xC0, 0x03)));
  matching_methods.append(tr("Tee-Type"));
  matching_methods.append(tr("Tapped C transformer"));
  matching_methods.append(tr("Tapped L transformer"));
  matching_methods.append(tr("Double tapped resonator"));
  matching_methods.append(tr("Single tuned transformer"));
  matching_methods.append(tr("Parallel double-tuned transformer"));
  matching_methods.append(tr("Series double-tuned transformer"));
  Topology_Combo->addItems(matching_methods);
  MatchingNetworkDesignLayout->addWidget(Topology_Label, 1, 0);
  MatchingNetworkDesignLayout->addWidget(Topology_Combo, 1, 1);

  // Solution number. In some matching techniques there are two possible
  // solutions
  QWidget *SolutionWidget = new QWidget();
  QHBoxLayout *SolutionLayout = new QHBoxLayout();
  Solution1_RB = new QRadioButton("Solution 1");
  Solution2_RB = new QRadioButton("Solution 2");
  Solution1_RB->setChecked(true);
  SolutionLayout->addWidget(Solution1_RB);
  SolutionLayout->addWidget(Solution2_RB);
  SolutionWidget->setLayout(SolutionLayout);
  MatchingNetworkDesignLayout->addWidget(SolutionWidget, 1, 2);

  // Input impedance
  Zin_Label = new QLabel("Zin");
  ZinRSpinBox = new QDoubleSpinBox();
  ZinRSpinBox->setMinimum(0.5);
  ZinRSpinBox->setMaximum(10000);
  ZinRSpinBox->setSingleStep(0.5);
  ZinRSpinBox->setValue(50);
  Zin_J = new QLabel("+j");
  ZinISpinBox = new QDoubleSpinBox();
  ZinISpinBox->setMinimum(-10000);
  ZinISpinBox->setMaximum(10.00);
  ZinISpinBox->setSingleStep(0.5);
  ZinISpinBox->setValue(0);
  Ohm_Zin_Label = new QLabel(QChar(0xa9, 0x03));
  MatchingNetworkDesignLayout->addWidget(Zin_Label, 2, 0);
  MatchingNetworkDesignLayout->addWidget(ZinRSpinBox, 2, 1);
  MatchingNetworkDesignLayout->addWidget(Zin_J, 2, 2);
  MatchingNetworkDesignLayout->addWidget(ZinISpinBox, 2, 3);
  MatchingNetworkDesignLayout->addWidget(Ohm_Zin_Label, 2, 4);

  // Edit S-parameters button
  EditSPAR = new QPushButton("Edit device S-parameters");
  EditSPAR->hide();
  MatchingNetworkDesignLayout->addWidget(EditSPAR, 3, 0, 1, 4);

  // Output impedance
  Zout_Label = new QLabel("Zout");
  ZoutRSpinBox = new QDoubleSpinBox();
  ZoutRSpinBox->setMinimum(0.5);
  ZoutRSpinBox->setMaximum(10000);
  ZoutRSpinBox->setSingleStep(0.5);
  ZoutRSpinBox->setValue(75);
  Zout_J = new QLabel("+j");
  ZoutISpinBox = new QDoubleSpinBox();
  ZoutISpinBox->setMinimum(-10000);
  ZoutISpinBox->setMaximum(10000);
  ZoutISpinBox->setSingleStep(0.5);
  ZoutISpinBox->setValue(0);
  Ohm_Zout_Label = new QLabel(QChar(0xa9, 0x03));
  EditS1P = new QPushButton("S1P");
  EditS1P->hide();
  connect(EditS1P, SIGNAL(clicked(bool)), this, SLOT(launchS1P()));
  MatchingNetworkDesignLayout->addWidget(Zout_Label, 4, 0);
  MatchingNetworkDesignLayout->addWidget(ZoutRSpinBox, 4, 1);
  MatchingNetworkDesignLayout->addWidget(Zout_J, 4, 2);
  MatchingNetworkDesignLayout->addWidget(ZoutISpinBox, 4, 3);
  MatchingNetworkDesignLayout->addWidget(Ohm_Zout_Label, 4, 4);
  MatchingNetworkDesignLayout->addWidget(EditS1P, 4, 1);

  // Frequency range. Start
  FreqStart_Label = new QLabel("freq");
  FreqStart_Spinbox = new QDoubleSpinBox();
  FreqStart_Spinbox->setMinimum(1);
  FreqStart_Spinbox->setMaximum(1e6);
  FreqStart_Spinbox->setDecimals(0);
  FreqStart_Spinbox->setValue(1000);
  FreqStart_Spinbox->setSingleStep(1); // Step fixed to 1 Hz/kHz/MHz/GHz
  QStringList FreqScale;
  FreqScale << "GHz"
            << "MHz"
            << "kHz"
            << "Hz";
  FreqStart_Scale_Combo = new QComboBox();
  FreqStart_Scale_Combo->addItems(FreqScale);
  FreqStart_Scale_Combo->setCurrentIndex(1);
  MatchingNetworkDesignLayout->addWidget(FreqStart_Label, 5, 0);
  MatchingNetworkDesignLayout->addWidget(FreqStart_Spinbox, 5, 1);
  MatchingNetworkDesignLayout->addWidget(FreqStart_Scale_Combo, 5, 2);

  // Frequency range. End
  FreqEnd_Label = new QLabel("Fstop");
  FreqEnd_Spinbox = new QDoubleSpinBox();
  FreqEnd_Spinbox->setMinimum(1);
  FreqEnd_Spinbox->setMaximum(1e6);
  FreqEnd_Spinbox->setDecimals(0);
  FreqEnd_Spinbox->setValue(1200);
  FreqEnd_Spinbox->setSingleStep(1); // Step fixed to 1 Hz/kHz/MHz/GHz
  FreqEnd_Scale_Combo = new QComboBox();
  FreqEnd_Scale_Combo->addItems(FreqScale);
  MatchingNetworkDesignLayout->addWidget(FreqEnd_Label, 6, 0);
  MatchingNetworkDesignLayout->addWidget(FreqEnd_Spinbox, 6, 1);
  MatchingNetworkDesignLayout->addWidget(FreqEnd_Scale_Combo, 6, 2);

  // Since it is more common to design narrowband matching networks than
  // broadband, the end-freq widgets are hidden
  FreqEnd_Label->hide();
  FreqEnd_Spinbox->hide();
  FreqEnd_Scale_Combo->hide();

  connect(Topology_Combo, SIGNAL(currentIndexChanged(int)), this,
          SLOT(on_TopoCombo_currentIndexChanged(int)));
  connect(ZinRSpinBox, SIGNAL(valueChanged(double)), this,
          SLOT(UpdateDesignParameters()));
  connect(ZinISpinBox, SIGNAL(valueChanged(double)), this,
          SLOT(UpdateDesignParameters()));
  connect(EditSPAR, SIGNAL(pressed()), this, SLOT(launchSPAR()));
  connect(ZoutRSpinBox, SIGNAL(valueChanged(double)), this,
          SLOT(UpdateDesignParameters()));
  connect(ZoutISpinBox, SIGNAL(valueChanged(double)), this,
          SLOT(UpdateDesignParameters()));
  connect(FreqStart_Spinbox, SIGNAL(valueChanged(double)), this,
          SLOT(UpdateDesignParameters()));
  connect(FreqEnd_Spinbox, SIGNAL(valueChanged(double)), this,
          SLOT(UpdateDesignParameters()));
  connect(FreqStart_Scale_Combo, SIGNAL(currentIndexChanged(int)), this,
          SLOT(UpdateDesignParameters()));
  connect(FreqEnd_Scale_Combo, SIGNAL(currentIndexChanged(int)), this,
          SLOT(UpdateDesignParameters()));
  connect(Solution1_RB, SIGNAL(clicked(bool)), this,
          SLOT(UpdateDesignParameters()));
  connect(Solution2_RB, SIGNAL(clicked(bool)), this,
          SLOT(UpdateDesignParameters()));

  // Connect data entry tools to main form
  connect(S1PW, SIGNAL(sendData(S1P_DATA)), this,
          SLOT(ReceiveS1PData(S1P_DATA)));
  connect(S2PW, SIGNAL(sendData(S2P_DATA)), this,
          SLOT(ReceiveS2PData(S2P_DATA)));

  this->setLayout(MatchingNetworkDesignLayout);

  on_TopoCombo_currentIndexChanged(0);
}

MatchingNetworkDesignTool::~MatchingNetworkDesignTool() {
  delete Topology_Label;
  delete Topology_Combo;
  delete Solution1_RB;
  delete Solution2_RB;
  delete Zin_Label;
  delete ZinRSpinBox;
  delete Zin_J;
  delete ZinISpinBox;
  delete Ohm_Zin_Label;
  delete Ohm_Zout_Label;
  delete ZoutRSpinBox;
  delete Zout_J;
  delete ZoutISpinBox;
  delete Zout_Label;
  delete FreqStart_Label;
  delete FreqStart_Scale_Combo;
  delete FreqStart_Spinbox;
  delete FreqEnd_Label;
  delete FreqEnd_Scale_Combo;
  delete FreqEnd_Spinbox;
  delete EditS1P;
  delete S1PW;
  delete S2PW;
  delete BroadbandRadioButton;
  delete FreqDep_Checkbox;
  delete SingleFrequencyRadiobutton;
  delete FreqModeSelectionGroupbox;
}

void MatchingNetworkDesignTool::UpdateDesignParameters() {
  MatchingNetworkDesignParameters Specs;

  // Get the frequency band where matching is needed
  Specs.freqStart = FreqStart_Spinbox->value() *
                    getScaleFreq(FreqStart_Scale_Combo->currentIndex());
  Specs.freqEnd = FreqEnd_Spinbox->value() *
                  getScaleFreq(FreqEnd_Scale_Combo->currentIndex());

  if (Specs.freqStart > Specs.freqEnd) {
    // Well, this is wrong
    return;
  }

  // Update port impedances
  Specs.Zin = std::complex<double>(ZinRSpinBox->value(), ZinISpinBox->value());
  if (BroadbandRadioButton->isChecked()) {
    // Broadband mode

    // Resize vectors
    Specs.Zout.resize(DATA_S1P.Z11.size());
    Specs.ZoutF.resize(DATA_S1P.Z11.size());
    Specs.Zout = DATA_S1P.Z11;
    Specs.ZoutF = DATA_S1P.Freq;

    Specs.BroadbandMode = true;
  } else {
    // Narrowband mode
    Specs.Zout.resize(1);
    Specs.Zout[0] =
        std::complex<double>(ZoutRSpinBox->value(), ZoutISpinBox->value());
    Specs.BroadbandMode = false;
  }

  Specs.Topology = Topology_Combo->currentText();
  if (Solution1_RB->isChecked())
    Specs.Solution = 1;
  else
    Specs.Solution = 2;

  // Design the matching network
  if (Specs.Topology == "L-section") {
    Lsection *L = new Lsection(Specs);
    L->synthesize();
    SchContent = L->Schematic;
    delete L;
  }
  SchContent.getInputReflectionCoefficient = true;
  // EMIT SIGNAL TO SIMULATE
  emit simulateNetwork(SchContent);
}

// This function is triggered by a change in the current selected combo item. It
// determines the visibility of the UI components
void MatchingNetworkDesignTool::on_TopoCombo_currentIndexChanged(int index) {
  switch (index) {
  case 0: // Lsection
    Solution1_RB->show();
    Solution2_RB->show();
    ZinISpinBox->setEnabled(false); // The L-section method does not support
                                    // complex source impedance data
  default:
    break;
  }
  UpdateDesignParameters();
}

// The purpose of this function is to trigger a design from the main application
void MatchingNetworkDesignTool::design() { UpdateDesignParameters(); }

// This function scales the frequency according to 'FreqScaleCombo' combobox
double MatchingNetworkDesignTool::getScaleFreq(int index) {
  double exp = 1;
  switch (index) {
  case 0:
    exp = 9;
    break;
  case 1:
    exp = 6;
    break;
  case 2:
    exp = 3;
    break;
  case 3:
    exp = 1;
    break;
  }
  return pow(10, exp);
}

void MatchingNetworkDesignTool::launchSPAR() { S2PW->show(); }

// This function is triggered when the user activates or deactivates the
// two-port matching option
void MatchingNetworkDesignTool::SwitchSingle_TwoPort_MatchingMode() {
  if (TwoPort_Matching_Checkbox->isChecked()) {
    // Make visible two-port matching widgets and hide the single-port stuff
    EditSPAR->show();
  } else {
    // Make visible single-port matching input widgets and hide the two-port
    // stuff
    EditSPAR->hide();
  }
}

// This function is triggered when the user activates the frequency-dependent
// load option
void MatchingNetworkDesignTool::FreqDependentLoad() {
  if (FreqDep_Checkbox->isChecked()) {
    EditS1P->show();
    ZoutISpinBox->hide();
    ZoutRSpinBox->hide();
    Zout_J->hide();
    Ohm_Zout_Label->hide();
    // Set broadband mode on
    BroadbandRadioButton->blockSignals(true);
    SingleFrequencyRadiobutton->blockSignals(true);
    BroadbandRadioButton->setChecked(true);
    SingleFrequencyRadiobutton->setChecked(false);
    BroadbandRadioButton->blockSignals(false);
    SingleFrequencyRadiobutton->blockSignals(false);
    BroadbandHandler(); // Set UI in broadband mode
  } else {
    EditS1P->hide();
    ZoutISpinBox->show();
    ZoutRSpinBox->show();
    Zout_J->show();
    Ohm_Zout_Label->show();
  }
}

// This function is called when the user selects the broadband matching option.
// For broadband matching, it is needed to hide the frequency selection widget
void MatchingNetworkDesignTool::BroadbandHandler() {
  if (BroadbandRadioButton->isChecked()) {
    // Broadband matching
    FreqStart_Label->setText("Fstart");
    FreqEnd_Label->show();
    FreqEnd_Spinbox->show();
    FreqEnd_Scale_Combo->show();
  } else {
    // Narrowband matching
    FreqStart_Label->setText("freq");
    FreqEnd_Label->hide();
    FreqEnd_Spinbox->hide();
    FreqEnd_Scale_Combo->hide();
  }
}

void MatchingNetworkDesignTool::launchS1P() { S1PW->show(); }

void MatchingNetworkDesignTool::ReceiveS1PData(S1P_DATA D) {
  DATA_S1P = D;
  if (DATA_S1P.Freq.size() > 1) {
    // Enable broadband mode
    FreqDep_Checkbox->blockSignals(true);
    FreqDep_Checkbox->setEnabled(true);
    FreqDep_Checkbox->blockSignals(false);
  }
}

void MatchingNetworkDesignTool::ReceiveS2PData(S2P_DATA D) { DATA_S2P = D; }
