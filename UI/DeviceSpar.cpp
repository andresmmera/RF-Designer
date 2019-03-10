/***************************************************************************
                                DeviceSpar.cpp
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
#include "DeviceSpar.h"

DeviceSPAR::DeviceSPAR() {
  QGridLayout *SPARDataLayout = new QGridLayout();

  // Data entry mode
  QWidget *S2PWidget = new QWidget();
  QHBoxLayout *S2PLayout = new QHBoxLayout();
  AddS2PRadioButton = new QRadioButton("Add S2P");
  connect(AddS2PRadioButton, SIGNAL(clicked(bool)), this,
          SLOT(UpdateS2PDataEntry()));
  AddSingleFreqRadioButton = new QRadioButton("Enter single frequency");
  AddSingleFreqRadioButton->setChecked(true);
  connect(AddSingleFreqRadioButton, SIGNAL(clicked(bool)), this,
          SLOT(UpdateS2PDataEntry()));
  S2PLayout->addWidget(AddS2PRadioButton);
  S2PLayout->addWidget(AddSingleFreqRadioButton);
  S2PWidget->setLayout(S2PLayout);
  SPARDataLayout->addWidget(S2PWidget, 0, 0, 1, 3);

  // Data entry widget
  QGridLayout *SPAR_GridLayout = new QGridLayout();
  // Frequency
  QStringList FreqScale;
  FreqScale << "GHz"
            << "MHz"
            << "kHz"
            << "Hz";

  freqLabel = new QLabel("Freq");
  freqSpinBox = new QDoubleSpinBox();
  freqSpinBox->setMinimum(1);
  freqSpinBox->setMaximum(1e6);
  freqSpinBox->setDecimals(0);
  freqSpinBox->setValue(1000);
  freqScaleCombo = new QComboBox();
  freqScaleCombo->addItems(FreqScale);
  freqScaleCombo->setCurrentIndex(1);
  SPAR_GridLayout->addWidget(freqLabel, 0, 0);
  SPAR_GridLayout->addWidget(freqSpinBox, 0, 1);
  SPAR_GridLayout->addWidget(freqScaleCombo, 0, 2);

  // Add point
  AddPoint = new QPushButton("Add sample");
  SPAR_GridLayout->addWidget(AddPoint, 1, 0);
  connect(AddPoint, SIGNAL(clicked(bool)), this, SLOT(addSingleFreqData()));

  // S-matrix label
  S_Matrix_Label = new QLabel("S-matrix");
  S_Matrix_Label->setStyleSheet("font-weight: bold; color: black");
  S_Matrix_Label->setAlignment(Qt::AlignCenter);
  SPAR_GridLayout->addWidget(S_Matrix_Label, 2, 0, 1, 3);

  // S-matrix input
  SPAR = new QGroupBox("S-parameter input");
  SPAR->setToolTip(QString(
      "Use '/' or ';' as separator between magnitude and phase. "
      "Use \"deg\" or 'º' for degrees and \"r\" or \"rad\" for radians"));
  S2PInputTable = new QTableWidget();
  S2PInputTable->setRowCount(2);
  S2PInputTable->setColumnCount(2);
  S2PInputTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  S2PInputTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  S2PInputTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // Enter some sample data
  S2PInputTable->setItem(0, 0, new QTableWidgetItem("0.1/30"));
  S2PInputTable->setItem(0, 1, new QTableWidgetItem("20/155"));
  S2PInputTable->setItem(1, 0, new QTableWidgetItem("0.05/10"));
  S2PInputTable->setItem(1, 1, new QTableWidgetItem("0.1/55"));

  SPAR_GridLayout->addWidget(S2PInputTable, 3, 0, 1, 3);
  SPAR->setLayout(SPAR_GridLayout);

  SPARDataLayout->addWidget(SPAR, 1, 0, 1, 3);

  // S2P file chooser
  S2PFileButton = new QPushButton("Browse S2P");
  SPARDataLayout->addWidget(S2PFileButton, 2, 0);
  connect(S2PFileButton, SIGNAL(clicked(bool)), this, SLOT(LoadS2PFile()));
  S2PFileButton->hide(); // By default this is hidden

  // Data
  QGroupBox *DataWidget = new QGroupBox("Data display");
  QGridLayout *DataLayout = new QGridLayout();

  // Display real-imaginary or MA data
  QWidget *RI_or_MA = new QWidget();
  RI_Radiobutton = new QRadioButton("R+jX");
  MA_RadioButton = new QRadioButton("Mag | Phase");
  RI_Radiobutton->setChecked(true);
  QGridLayout *RI_MA_GridLayout = new QGridLayout();
  RI_MA_GridLayout->addWidget(RI_Radiobutton, 0, 0);
  RI_MA_GridLayout->addWidget(MA_RadioButton, 0, 1);
  RI_or_MA->setLayout(RI_MA_GridLayout);
  DataLayout->addWidget(RI_or_MA, 1, 0, 1, 2);

  S2PTable = new QTableWidget();
  S2PTable->setRowCount(0);
  S2PTable->setColumnCount(5);
  QStringList Labels;
  Labels << "Freq"
         << "S11"
         << "S12"
         << "S21"
         << "S22";
  S2PTable->setHorizontalHeaderLabels(Labels);
  S2PTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  S2PTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  DataWidget->setLayout(DataLayout);
  DataLayout->addWidget(S2PTable, 2, 0, 1, 2);
  DeletePoint = new QPushButton("Remove sample");
  ClearAll = new QPushButton("Clear all");
  connect(ClearAll, SIGNAL(clicked(bool)), this, SLOT(ClearTable()));
  DataLayout->addWidget(DeletePoint, 3, 0);
  DataLayout->addWidget(ClearAll, 3, 1);

  SPARDataLayout->addWidget(DataWidget, 3, 0, 1, 3);
  this->setLayout(SPARDataLayout);
  this->setWindowTitle("S-parameter data entry tool");
}

DeviceSPAR::~DeviceSPAR() {
  delete AddS2PRadioButton;
  delete AddSingleFreqRadioButton;
  delete freqLabel;
  delete freqScaleCombo;
  delete freqSpinBox;
  delete RI_Radiobutton;
  delete S2PTable;
  delete S2PInputTable;
  delete DeletePoint;
  delete ClearAll;
  delete SPAR;
}

void DeviceSPAR::UpdateS2PDataEntry() {
  if (AddS2PRadioButton->isChecked()) {
    S2PFileButton->show();
    SPAR->hide();
  } else {
    S2PFileButton->hide();
    SPAR->show();
  }
}

void DeviceSPAR::LoadS2PFile() {
  QString filename = QFileDialog::getOpenFileName(
      this, tr("Open S2P File"), "/home", tr("Touchstone S2P (*.s2p)"));
}

// Triggered when the user presses the "add single point" button
void DeviceSPAR::addSingleFreqData() {

  // Read frequency from the interface
  QString fscale = freqScaleCombo->currentText();
  double scale;
  if (fscale == "GHz")
    scale = 1e9;
  else if (fscale == "MHz")
    scale = 1e6;
  else if (fscale == "kHz")
    scale = 1e3;
  else
    scale = 1;
  double freq = freqSpinBox->value() * scale;

  // Read the S matrix
  // The data may be entered as R+jX or MA. The magnitude may be entered in
  // natural units (default) or dB. On the other hand, the angle may be
  // expressed in degrees (default) or radians

  std::complex<double> S11 =
      ReadS2PFromUserInput(S2PInputTable->item(0, 0)->text());
  std::complex<double> S12 =
      ReadS2PFromUserInput(S2PInputTable->item(0, 1)->text());
  std::complex<double> S21 =
      ReadS2PFromUserInput(S2PInputTable->item(1, 0)->text());
  std::complex<double> S22 =
      ReadS2PFromUserInput(S2PInputTable->item(1, 1)->text());

  // Add the previous data to the internal data storage structures
  DATA.Freq.push_back(freq);
  DATA.S11.push_back(S11);
  DATA.S12.push_back(S12);
  DATA.S21.push_back(S21);
  DATA.S22.push_back(S22);

  DATA = Sort(DATA);

  // Now, put the sorted data in the table
  PutDataInTable();

  // Increase the value in the freq spinbox so as to minimize the probability of
  // the user entering duplicated data
  freqSpinBox->setValue(freqSpinBox->value() + 1);
}

// This function writes the content of the class variable 'DATA' in the main
// QTableWidget
void DeviceSPAR::PutDataInTable() {
  S2PTable->clearContents();
  for (int r = 0; r < DATA.Freq.size(); r++) {
    if (r >= S2PTable->rowCount()) {
      // Add a new row to the table
      S2PTable->insertRow(S2PTable->rowCount());
    }

    // Add data to the row
    S2PTable->setItem(r, 0,
                      new QTableWidgetItem(QString("%1").arg(num2str(
                          DATA.Freq.at(r), 3, Frequency)))); // Frequency
    S2PTable->setItem(
        r, 1,
        new QTableWidgetItem(QString("%1+j%2")
                                 .arg(DATA.S11.at(r).real())
                                 .arg(DATA.S11.at(r).imag()))); // S11
    S2PTable->setItem(
        r, 2,
        new QTableWidgetItem(QString("%1+j%2")
                                 .arg(DATA.S21.at(r).real())
                                 .arg(DATA.S21.at(r).imag()))); // S11
    S2PTable->setItem(
        r, 3,
        new QTableWidgetItem(QString("%1+j%2")
                                 .arg(DATA.S12.at(r).real())
                                 .arg(DATA.S12.at(r).imag()))); // S11
    S2PTable->setItem(
        r, 4,
        new QTableWidgetItem(QString("%1+j%2")
                                 .arg(DATA.S22.at(r).real())
                                 .arg(DATA.S22.at(r).imag()))); // S11
  }
}

// This function converts a QString complex data into std::complex
std::complex<double> DeviceSPAR::ReadS2PFromUserInput(QString data) {
  QString R, I, M, A;
  double R_, I_, M_, A_;
  int sign = 1;
  int index;
  QStringList separator; // Define a range of characters that may separate
                         // magnitude from angle
  separator << "/"
            << "_"
            << "'"
            << "#"
            << "<"
            << "|";

  // Remove spaces
  data.trimmed();

  // Remove the product operator (*) in case the user entered that between j and
  // X
  data.remove('*');

  // Identify if the input is RI or MA
  if (data.contains("j") || data.contains("i")) {
    // The input is RI
    index = data.indexOf("j");
    if (index == -1)
      index = data.indexOf("i");

    if (data.at(index - 1) == '-')
      sign = -1;

    // Separate the real from the imaginary part
    R = data.mid(0, index - 1);
    I = data.mid(index + 1);

    R_ = R.toDouble();
    I_ = I.toDouble();

    return std::complex<double>(R_, sign * I_);
  } else {
    // The input is MA
    // Find the separator
    int index = -1;
    for (int i = 0; i < separator.length(); i++) {
      if (data.contains(separator.at(i))) {
        index = data.indexOf(separator.at(i));
        break;
      }
    }
    // Separate magnitude from phase
    M = data.mid(0, index);
    A = data.mid(index + 1);

    // Now, detect if the magnitude is in natural units or dB
    if (M.contains("dB", Qt::CaseInsensitive)) { // dB is case sensitive, but
                                                 // the user may enter "db"
      M.mid(0, M.length() - 2);
      M_ = pow(10, 0.05 * M.toDouble()); // Convert dB to natural units
    } else {
      M_ = M.toDouble(); // The magnitude is already expressed in natural units
    }

    // Is the angle expressed in degrees or radians?
    if (A.contains("r")) { // The angle is in rad
      // The angle is in radians
      if (A.contains("r")) {
        index = A.indexOf("r");
        A.mid(0, index); // Remove r, rad or rd
      }
      A_ = A.toDouble();
    } else { // degrees
      if (A.contains("deg")) {
        index = A.indexOf("d");
        A.mid(0, index);              // Remove deg, d or dg
      } else if (A.contains("deg")) { // It contains º
        index = A.indexOf("º");
        A.mid(0, index); // Remove º
      }
      A_ = A.toDouble();
      // Convert deg to rad
      A_ *= M_PI / 180;
    }
    return std::complex<double>(M_ * cos(A_), M_ * sin(A_));
  }
}

void DeviceSPAR::ClearTable() { S2PTable->clear(); }
