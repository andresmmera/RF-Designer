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
#ifndef MATCHINGNETWORKDESIGNTOOL_H
#define MATCHINGNETWORKDESIGNTOOL_H

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>

#include "Schematic/Network.h"
#include "Schematic/SchematicContent.h"
#include "UI/SPAR/S1P_InputWidget.h"
#include "UI/SPAR/S2P_InputWidget.h"

#include "MatchingNetwork/Lsection.h"

class MatchingNetworkDesignTool : public QWidget {
  Q_OBJECT
public:
  MatchingNetworkDesignTool();
  ~MatchingNetworkDesignTool();
  void design();

private slots:
  void UpdateDesignParameters();
  void on_TopoCombo_currentIndexChanged(int);
  void launchSPAR();
  void launchS1P();
  void SwitchSingle_TwoPort_MatchingMode();
  void FreqDependentLoad();
  void BroadbandHandler();

private:
  QLabel *Topology_Label, *Zin_Label, *Ohm_Zin_Label, *Zout_Label,
      *Ohm_Zout_Label, *FreqStart_Label, *FreqEnd_Label, *Zin_J, *Zout_J;
  QDoubleSpinBox *ZinRSpinBox, *ZoutRSpinBox, *ZinISpinBox, *ZoutISpinBox,
      *FreqStart_Spinbox, *FreqEnd_Spinbox;
  QComboBox *Topology_Combo, *FreqStart_Scale_Combo, *FreqEnd_Scale_Combo;
  QCheckBox *TwoPort_Matching_Checkbox, *FreqDep_Checkbox;
  QRadioButton *Solution1_RB, *Solution2_RB;
  QPushButton *EditSPAR, *EditS1P;
  QRadioButton *BroadbandRadioButton, *SingleFrequencyRadiobutton;
  QGroupBox *FreqModeSelectionGroupbox;
  double getScaleFreq(int);

  SchematicContent SchContent; // Schematic representation
  S1P_InputWidget *S1PW;
  S2P_InputWidget *S2PW;

  S1P_DATA DATA_S1P;
  S2P_DATA DATA_S2P;

signals:
  void simulateNetwork(SchematicContent);

private slots:
  void ReceiveS1PData(S1P_DATA);
  void ReceiveS2PData(S2P_DATA);
};

#endif // MATCHINGNETWORKDESIGNTOOL_H
