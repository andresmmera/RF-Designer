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
#include "General/io.h"
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

using namespace std;

class DeviceSPAR : public QWidget {
  Q_OBJECT
public:
  DeviceSPAR();
  ~DeviceSPAR();

private:
  QPushButton *S2P_File;
  QRadioButton *AddS2PRadioButton, *AddSingleFreqRadioButton, *RI_Radiobutton,
      *MA_RadioButton, *DegRadiobutton, *RadRadiobutton, *MdBA_RadioButton;
  QLabel *freqLabel, *S_Matrix_Label, *fstartMatchingS2P_Label,
      *fendMatchingS2P_Label;
  QDoubleSpinBox *freqSpinBox, *fstartSpinbox, *fendSpinbox;
  QComboBox *freqScaleCombo, *fendScaleCombo, *fstartScaleCombo;
  QTableWidget *S2PTable, *S2PInputTable;
  QPushButton *AddPoint, *DeletePoint, *ClearAll, *S2PFileButton;
  QGroupBox *SPAR, *RadDegGroupbox, *S2P_Groupbox;

  struct S2P_DATA
      DATA; // Only for data transfer to main widget. It uses std::vector.
            // However, std::deque is needed for handling data dynamically...
  std::deque<std::complex<double>> S11, S12, S21, S22;
  std::deque<double> Freq;
  void SortData();

  std::complex<double> ReadS2PFromUserInput(QString);
  QString rect2polar(std::complex<double>);
  int loadS2Pdata(QString);
  double getS2PfreqScale(string line);
  string RemoveBlankSpaces(string line);

private slots:
  void UpdateS2PDataEntry();
  void PutDataInTable();
  void LoadS2PFile();
  void addSingleFreqData();
  void ClearTable();
};

#endif // DEVICESPAR_H
