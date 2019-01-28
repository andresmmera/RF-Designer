/***************************************************************************
                                AttenuatorDesignTool.cpp
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
#ifndef ATTENUATORDESIGNTOOL_H
#define ATTENUATORDESIGNTOOL_H

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QWidget>

#include "AttenuatorDesign/AttenuatorDesigner.h"
#include "Filtering/Network.h"

class AttenuatorDesignTool : public QWidget {
  Q_OBJECT
public:
  AttenuatorDesignTool();
  ~AttenuatorDesignTool();
  void design();

private slots:
  void UpdateDesignParameters();
  void on_TopoCombo_currentIndexChanged(int);

private:
  QLabel *Topology_Label, *Attenuation_Label, *dBLabelAtt, *Zin_Label,
      *Ohm_Zin_Label, *Zout_Label, *Ohm_Zout_Label, *Pin_Label, *Pdiss_R1_Label,
      *Pdiss_R2_Label, *Pdiss_R3_Label, *freqLabel;
  QDoubleSpinBox *AttenuationSpinBox, *ZinSpinBox, *ZoutSpinBox, *Pin_SpinBox,
      *freqSpinBox;
  QComboBox *Topology_Combo, *Pin_units_Combo, *R1_Pdiss_Units_Combo,
      *R2_Pdiss_Units_Combo, *R3_Pdiss_Units_Combo, *FreqScaleCombo;
  QLineEdit *Pdiss_R1_Lineedit, *Pdiss_R2_Lineedit, *Pdiss_R3_Lineedit;
  QCheckBox *LumpedImplementationCheckbox;

  double getFreq();
  double getPowerW();
  QString netlist;
  SchematicInfo SchInfo; // Schematic representation

signals:
  void simulateNetwork(struct SchematicInfo);
};

#endif // ATTENUATORDESIGNTOOL_H