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
  QGridLayout *MatchingNetworkDesignLayout = new QGridLayout();
  // Topology
  Topology_Label = new QLabel("Topology");
  Topology_Combo = new QComboBox();
  Topology_Combo->addItem("Pi");
  MatchingNetworkDesignLayout->addWidget(Topology_Label, 0, 0);
  MatchingNetworkDesignLayout->addWidget(Topology_Combo, 0, 1);

  // Input impedance
  Zin_Label = new QLabel("Zin");
  ZinSpinBox = new QDoubleSpinBox();
  ZinSpinBox->setMinimum(0.5);
  ZinSpinBox->setMaximum(1000);
  ZinSpinBox->setSingleStep(0.5);
  ZinSpinBox->setValue(50);
  Ohm_Zin_Label = new QLabel(QChar(0xa9, 0x03));
  MatchingNetworkDesignLayout->addWidget(Zin_Label, 1, 0);
  MatchingNetworkDesignLayout->addWidget(ZinSpinBox, 1, 1);
  MatchingNetworkDesignLayout->addWidget(Ohm_Zin_Label, 1, 2);

  // Output impedance
  Zout_Label = new QLabel("Zout");
  ZoutSpinBox = new QDoubleSpinBox();
  ZoutSpinBox->setMinimum(0.5);
  ZoutSpinBox->setMaximum(1000);
  ZoutSpinBox->setSingleStep(0.5);
  ZoutSpinBox->setValue(50);
  Ohm_Zout_Label = new QLabel(QChar(0xa9, 0x03));
  MatchingNetworkDesignLayout->addWidget(Zout_Label, 2, 0);
  MatchingNetworkDesignLayout->addWidget(ZoutSpinBox, 2, 1);
  MatchingNetworkDesignLayout->addWidget(Ohm_Zout_Label, 2, 2);

  connect(Topology_Combo, SIGNAL(currentIndexChanged(int)), this,
          SLOT(on_TopoCombo_currentIndexChanged(int)));
  connect(ZinSpinBox, SIGNAL(valueChanged(double)), this,
          SLOT(UpdateDesignParameters()));
  connect(ZoutSpinBox, SIGNAL(valueChanged(double)), this,
          SLOT(UpdateDesignParameters()));

  this->setLayout(MatchingNetworkDesignLayout);

  on_TopoCombo_currentIndexChanged(0);
}

MatchingNetworkDesignTool::~MatchingNetworkDesignTool() {
  delete Topology_Label;
  delete Topology_Combo;
  delete Zin_Label;
  delete ZinSpinBox;
  delete Ohm_Zin_Label;
  delete Ohm_Zout_Label;
  delete ZoutSpinBox;
  delete Zout_Label;
}

void MatchingNetworkDesignTool::UpdateDesignParameters() {
  MatchingNetworkDesignParameters Specs;
  Specs.Zin = ZinSpinBox->value();
  Specs.Zout = ZoutSpinBox->value();
  Specs.Topology = Topology_Combo->currentText();

  if (Specs.Topology == "L-section") {
    /*   Lsection *L = new Lsection(Specs);
       L->synthesize();
       SchContent = L->Schematic;
       delete L;*/
  }

  // EMIT SIGNAL TO SIMULATE
  emit simulateNetwork(SchContent);
}

// This function is triggered by a change in the current selected combo item
void MatchingNetworkDesignTool::on_TopoCombo_currentIndexChanged(int index) {
  switch (index) {
  case 0: // Lsection

  default:
    break;
  }
  UpdateDesignParameters();
}

// The purpose of this function is to trigger a design from the main application
void MatchingNetworkDesignTool::design() { UpdateDesignParameters(); }

// This function scales the frequency according to 'FreqScaleCombo' combobox
double MatchingNetworkDesignTool::getFreq() {
  double exp = 1;
  switch (FreqScaleCombo->currentIndex()) {
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
  return freqSpinBox->value() * pow(10, exp);
}
