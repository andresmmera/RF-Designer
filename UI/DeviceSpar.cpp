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
  S2P_Groupbox = new QGroupBox("S2P file");
  QGridLayout *S2P_File_Layout = new QGridLayout();
  S2PFileButton = new QPushButton("Browse S2P");
  connect(S2PFileButton, SIGNAL(clicked(bool)), this, SLOT(LoadS2PFile()));
  S2P_File_Layout->addWidget(S2PFileButton, 0, 0);
  fstartMatchingS2P_Label = new QLabel("Fstart matching");
  fstartSpinbox = new QDoubleSpinBox();
  fstartSpinbox->setMinimum(1);
  fstartSpinbox->setMaximum(1e6);
  fstartSpinbox->setDecimals(0);
  fstartSpinbox->setValue(470);
  fstartScaleCombo = new QComboBox();
  fstartScaleCombo->addItems(FreqScale);
  fstartScaleCombo->setCurrentIndex(1);
  S2P_File_Layout->addWidget(fstartMatchingS2P_Label, 1, 0);
  S2P_File_Layout->addWidget(fstartSpinbox, 1, 1);
  S2P_File_Layout->addWidget(fstartScaleCombo, 1, 2);

  fendMatchingS2P_Label = new QLabel("Fend matching");
  fendSpinbox = new QDoubleSpinBox();
  fendSpinbox->setMinimum(1);
  fendSpinbox->setMaximum(1e6);
  fendSpinbox->setDecimals(0);
  fendSpinbox->setValue(860);
  fendScaleCombo = new QComboBox();
  fendScaleCombo->addItems(FreqScale);
  fendScaleCombo->setCurrentIndex(1);
  S2P_File_Layout->addWidget(fendMatchingS2P_Label, 2, 0);
  S2P_File_Layout->addWidget(fendSpinbox, 2, 1);
  S2P_File_Layout->addWidget(fendScaleCombo, 2, 2);
  S2P_Groupbox->hide(); // By default this is hidden
  S2P_Groupbox->setLayout(S2P_File_Layout);

  SPARDataLayout->addWidget(S2P_Groupbox, 2, 0, 1, 3);

  // Data
  QGroupBox *DataWidget = new QGroupBox("Data display");
  QGridLayout *DataLayout = new QGridLayout();

  // Display real-imaginary or MA data
  QWidget *data_format_Widget = new QWidget();
  RI_Radiobutton = new QRadioButton("R+jX");
  MA_RadioButton = new QRadioButton("Mag | Phase");
  MdBA_RadioButton = new QRadioButton("Mag(dB) | Phase");
  RI_Radiobutton->setChecked(true);
  QGridLayout *RI_MA_GridLayout = new QGridLayout();
  RI_MA_GridLayout->addWidget(RI_Radiobutton, 0, 0);
  RI_MA_GridLayout->addWidget(MA_RadioButton, 0, 1);
  data_format_Widget->setLayout(RI_MA_GridLayout);
  DataLayout->addWidget(data_format_Widget, 1, 0, 1, 2);

  // Angle units
  RadDegGroupbox = new QGroupBox("Angle units");
  QGridLayout *angle_units_layout = new QGridLayout();
  DegRadiobutton = new QRadioButton("Degrees");
  RadRadiobutton = new QRadioButton("Radians");
  DegRadiobutton->setChecked(true);
  angle_units_layout->addWidget(DegRadiobutton, 0, 0);
  angle_units_layout->addWidget(RadRadiobutton, 0, 1);
  RadDegGroupbox->setLayout(angle_units_layout);
  DataLayout->addWidget(RadDegGroupbox, 1, 2, 1, 2);

  // Connect radiobuttons to handle function
  connect(RI_Radiobutton, SIGNAL(clicked(bool)), this, SLOT(PutDataInTable()));
  connect(MA_RadioButton, SIGNAL(clicked(bool)), this, SLOT(PutDataInTable()));
  connect(MdBA_RadioButton, SIGNAL(clicked(bool)), this,
          SLOT(PutDataInTable()));
  connect(DegRadiobutton, SIGNAL(clicked(bool)), this, SLOT(PutDataInTable()));
  connect(RadRadiobutton, SIGNAL(clicked(bool)), this, SLOT(PutDataInTable()));

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
  DataLayout->addWidget(S2PTable, 2, 0, 1, 4);
  DeletePoint = new QPushButton("Remove sample");
  ClearAll = new QPushButton("Clear all");
  connect(ClearAll, SIGNAL(clicked(bool)), this, SLOT(ClearTable()));
  DataLayout->addWidget(DeletePoint, 3, 0);
  DataLayout->addWidget(ClearAll, 3, 1);

  SPARDataLayout->addWidget(DataWidget, 3, 0, 1, 4);
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
  delete S_Matrix_Label;
  delete RadDegGroupbox;
  delete S2P_Groupbox;
  delete fstartMatchingS2P_Label;
  delete fendMatchingS2P_Label;
  delete fstartSpinbox;
  delete fendSpinbox;
  delete fstartScaleCombo;
  delete fendScaleCombo;
}

void DeviceSPAR::UpdateS2PDataEntry() {
  if (AddS2PRadioButton->isChecked()) {
    S2P_Groupbox->show();
    SPAR->hide();
  } else {
    S2P_Groupbox->hide();
    SPAR->show();
  }
}

void DeviceSPAR::LoadS2PFile() {
  QString filename = QFileDialog::getOpenFileName(
      this, tr("Open S2P File"), "/home", tr("Touchstone S2P (*.s2p)"));
  IO *readS2P = new IO();
  readS2P->loadS2Pdata(filename.toStdString());
  delete readS2P;
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

  std::complex<double> S11_ =
      ReadS2PFromUserInput(S2PInputTable->item(0, 0)->text());
  std::complex<double> S12_ =
      ReadS2PFromUserInput(S2PInputTable->item(0, 1)->text());
  std::complex<double> S21_ =
      ReadS2PFromUserInput(S2PInputTable->item(1, 0)->text());
  std::complex<double> S22_ =
      ReadS2PFromUserInput(S2PInputTable->item(1, 1)->text());

  // Add the previous data to the internal data storage structures
  Freq.push_back(freq);
  S11.push_back(S11_);
  S12.push_back(S12_);
  S21.push_back(S21_);
  S22.push_back(S22_);

  SortData();

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
  for (int r = 0; r < Freq.size(); r++) {
    if (r >= S2PTable->rowCount()) {
      // Add a new row to the table
      S2PTable->insertRow(S2PTable->rowCount());
    }

    // Add data to the row
    S2PTable->setItem(r, 0,
                      new QTableWidgetItem(QString("%1").arg(
                          num2str(Freq.at(r), 3, Frequency)))); // Frequency
    S2PTable->setItem(
        r, 1,
        new QTableWidgetItem(QString("%1").arg(rect2polar(S11.at(r))))); // S11
    S2PTable->setItem(
        r, 2,
        new QTableWidgetItem(QString("%1").arg(rect2polar(S12.at(r))))); // S12
    S2PTable->setItem(
        r, 3,
        new QTableWidgetItem(QString("%1").arg(rect2polar(S21.at(r))))); // S21
    S2PTable->setItem(
        r, 4,
        new QTableWidgetItem(QString("%1").arg(rect2polar(S22.at(r))))); // S22
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

QString DeviceSPAR::rect2polar(std::complex<double> Z) {
  // Check the selected data format
  bool MA_nu = MA_RadioButton->isChecked();
  bool MA_dB = MdBA_RadioButton->isChecked();
  bool deg = DegRadiobutton->isChecked();
  QString angle_units, mag_units = "";

  double mag, ang;

  // Polar
  if ((MA_nu) || (MA_dB)) {
    mag = sqrt(Z.real() * Z.real() + Z.imag() * Z.imag());

    // Angle
    ang = atan2(Z.imag(), Z.real());
    if (deg) { // Degrees
      ang *= 180 / M_PI;
      angle_units = "deg";
    } else {
      angle_units = QString("rad");
    }

    if (MA_dB) {
      mag = 10 * log10(mag);
      mag_units = "dB";
    }
    return QString("%1%2/%3%4")
        .arg(mag)
        .arg(mag_units)
        .arg(ang)
        .arg(angle_units);
  } else {
    // RI selected
    if (Z.imag() < 0) {
      return QString("%1-j%2").arg(Z.real()).arg(abs(Z.imag()));
    } else {
      return QString("%1+j%2").arg(Z.real()).arg(Z.imag());
    }
  }
}

// Sort by frequency an s-parameter data struct
void DeviceSPAR::SortData() {

  if (Freq.size() == 1)
    return;
  std::deque<double> freq_sorted;
  std::deque<std::complex<double>> S11_sorted, S12_sorted, S21_sorted,
      S22_sorted;
  std::deque<int> sort_vector;
  double min = 1e20, last_min = -1;
  int min_index;

  do {
    for (unsigned int i = 0; i < Freq.size(); i++) {
      if ((Freq.at(i) < min) && (Freq.at(i) >= last_min)) {
        min = Freq.at(i);
        min_index = i;
      }
    }

    if (min == last_min) {
      // Duplicated data
    } else {
      sort_vector.push_back(min_index);
      freq_sorted.push_back(min);
      last_min = min;
    }

    Freq.erase(Freq.begin() + min_index);
    min = 1e20;
  } while (Freq.size() > 0);

  // Resize vectors to the size of freq_sorted
  S11_sorted.resize(freq_sorted.size());
  S21_sorted.resize(freq_sorted.size());
  S12_sorted.resize(freq_sorted.size());
  S22_sorted.resize(freq_sorted.size());

  // Now apply the same order in the other vectors
  for (unsigned int i = 0; i < sort_vector.size(); i++) {
    S11_sorted[i] = S11[sort_vector[i]];
    S12_sorted[i] = S12[sort_vector[i]];
    S21_sorted[i] = S21[sort_vector[i]];
    S22_sorted[i] = S22[sort_vector[i]];
  }

  // Replace the sorted vectors in the SPAR structure
  Freq.clear();
  S11.clear();
  S21.clear();
  S12.clear();
  S22.clear();

  for (unsigned int i = 0; i < S11_sorted.size(); i++) {
    Freq.push_back(freq_sorted[i]);
    S11.push_back(S11_sorted[i]);
    S21.push_back(S21_sorted[i]);
    S12.push_back(S12_sorted[i]);
    S22.push_back(S22_sorted[i]);
  }
  return;
}
