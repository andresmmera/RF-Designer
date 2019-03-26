/***************************************************************************
                                S1P_InputWidget.h
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
#ifndef S1P_InputWidget_H
#define S1P_InputWidget_H
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

class S1P_InputWidget : public QWidget {
  Q_OBJECT
public:
  S1P_InputWidget();
  ~S1P_InputWidget();

private:
  QPushButton *S1P_File;
  QRadioButton *AddS1PRadioButton, *AddSingleFreqRadioButton, *RI_Radiobutton,
      *MA_RadioButton, *DegRadiobutton, *RadRadiobutton, *MdBA_RadioButton,
      *Z_EntryData_RadioButton, *S_EntryData_RadioButton;
  QLabel *freqLabel, *S_Matrix_Label, *Z0Label, *Z0_Ohm_Label;
  QDoubleSpinBox *freqSpinBox, *Z0_SpinBox;
  QComboBox *freqScaleCombo;
  QTableWidget *S1PTable, *S1PInputTable;
  QPushButton *AddPoint, *DeletePoint, *ClearAll, *S1PFileButton, *ReadyButton;
  QGroupBox *SPAR, *RadDegGroupbox, *S1P_Groupbox, *InputData_GroupBox;
  QTabWidget *SelectionTab;

  struct S1P_DATA
      DATA; // Only for data transfer to main widget. It uses std::vector.
            // However, std::deque is needed for handling data dynamically...
  std::deque<std::complex<double>> S11;
  std::deque<std::complex<double>> Z11;
  std::deque<double> Freq;
  void SortData();

  std::complex<double> ReadS1PFromUserInput(QString);
  QString rect2polar(std::complex<double>);
  int loadS1Pdata(QString);
  double getS1PfreqScale(string line);
  string RemoveBlankSpaces(string line);
  double getFreqScale(QString);

private slots:
  void UpdateS1PDataEntry();
  void PutDataInTable();
  void LoadS1PFile();
  void addSingleFreqData();
  void ClearTable();
  void ChangeSZMode();
  void ReadyButtonHandle();

signals:
  void sendData(S1P_DATA);
};

#endif // S1P_InputWidget_H
