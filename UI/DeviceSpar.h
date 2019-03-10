/***************************************************************************
                                DeviceSpar.h
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
#ifndef DEVICESPAR_H
#define DEVICESPAR_H
#include "general.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QWidget>
#include <complex>

class DeviceSPAR : public QWidget {
  Q_OBJECT
public:
  DeviceSPAR();
  ~DeviceSPAR();

private:
  QPushButton *S2P_File;
  QRadioButton *AddS2PRadioButton, *AddSingleFreqRadioButton, *RI_Radiobutton,
      *MA_RadioButton;
  QLabel *freqLabel, *S_Matrix_Label;
  QDoubleSpinBox *freqSpinBox;
  QComboBox *freqScaleCombo;
  QTableWidget *S2PTable, *S2PInputTable;
  QPushButton *AddPoint, *DeletePoint, *ClearAll, *S2PFileButton;
  QGroupBox *SPAR;
  struct S2P_DATA DATA;

  std::complex<double> ReadS2PFromUserInput(QString);
  void PutDataInTable();

private slots:
  void UpdateS2PDataEntry();
  void LoadS2PFile();
  void addSingleFreqData();
  void ClearTable();
};

#endif // DEVICESPAR_H
