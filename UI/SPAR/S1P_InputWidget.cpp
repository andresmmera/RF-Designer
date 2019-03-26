/***************************************************************************
                                S1P_InputWidget.cpp
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
#include "S1P_InputWidget.h"

S1P_InputWidget::S1P_InputWidget() {

  QGridLayout *SPARDataLayout = new QGridLayout();
  // Data entry mode
  QWidget *S1PWidget = new QWidget();
  QHBoxLayout *S1PLayout = new QHBoxLayout();
  AddS1PRadioButton = new QRadioButton("Add S1P");
  connect(AddS1PRadioButton, SIGNAL(clicked(bool)), this,
          SLOT(UpdateS1PDataEntry()));
  AddSingleFreqRadioButton = new QRadioButton("Enter single frequency");
  AddSingleFreqRadioButton->setChecked(true);
  connect(AddSingleFreqRadioButton, SIGNAL(clicked(bool)), this,
          SLOT(UpdateS1PDataEntry()));
  S1PLayout->addWidget(AddS1PRadioButton);
  S1PLayout->addWidget(AddSingleFreqRadioButton);
  S1PWidget->setLayout(S1PLayout);
  SPARDataLayout->addWidget(S1PWidget, 0, 0, 1, 3);

  // Data entry widget
  QGridLayout *SPAR_GridLayout = new QGridLayout();

  // Impedance or S-parameter data chooser
  InputData_GroupBox = new QGroupBox("Data entry format");
  QHBoxLayout *DataChooserInputLayout = new QHBoxLayout();
  Z_EntryData_RadioButton = new QRadioButton("Impedance data (Z)");
  connect(Z_EntryData_RadioButton, SIGNAL(clicked(bool)), this,
          SLOT(ChangeSZMode()));
  S_EntryData_RadioButton = new QRadioButton("S-parameter data");
  S_EntryData_RadioButton->setChecked(true);
  connect(S_EntryData_RadioButton, SIGNAL(clicked(bool)), this,
          SLOT(ChangeSZMode()));

  DataChooserInputLayout->addWidget(Z_EntryData_RadioButton);
  DataChooserInputLayout->addWidget(S_EntryData_RadioButton);
  InputData_GroupBox->setLayout(DataChooserInputLayout);
  SPAR_GridLayout->addWidget(InputData_GroupBox, 0, 0, 1, 4);

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
  SPAR_GridLayout->addWidget(freqLabel, 1, 0);
  SPAR_GridLayout->addWidget(freqSpinBox, 1, 1);
  SPAR_GridLayout->addWidget(freqScaleCombo, 1, 2);

  // Reference impedance
  Z0Label = new QLabel("Z0");
  Z0_SpinBox = new QDoubleSpinBox();
  Z0_SpinBox->setMinimum(1);
  Z0_SpinBox->setMaximum(1e3);
  Z0_SpinBox->setDecimals(0);
  Z0_SpinBox->setValue(50);
  Z0_Ohm_Label = new QLabel(QChar(0xa9, 0x03));
  SPAR_GridLayout->addWidget(Z0Label, 2, 0);
  SPAR_GridLayout->addWidget(Z0_SpinBox, 2, 1);
  SPAR_GridLayout->addWidget(Z0_Ohm_Label, 2, 2);

  // Add point
  AddPoint = new QPushButton("Add sample");
  SPAR_GridLayout->addWidget(AddPoint, 3, 0);
  connect(AddPoint, SIGNAL(clicked(bool)), this, SLOT(addSingleFreqData()));

  // S-matrix label
  S_Matrix_Label = new QLabel("S-matrix");
  S_Matrix_Label->setStyleSheet("font-weight: bold; color: black");
  S_Matrix_Label->setAlignment(Qt::AlignCenter);
  SPAR_GridLayout->addWidget(S_Matrix_Label, 4, 0, 1, 3);

  // S-matrix input
  SPAR = new QGroupBox("Impedance data entry");

  SPAR->setToolTip(QString(
      "Use '/' or ';' as separator between magnitude and phase. "
      "Use \"deg\" or 'º' for degrees and \"r\" or \"rad\" for radians"));
  S1PInputTable = new QTableWidget();
  S1PInputTable->setRowCount(1);
  S1PInputTable->setColumnCount(1);
  S1PInputTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  S1PInputTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  S1PInputTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // Enter some sample data
  S1PInputTable->setItem(0, 0, new QTableWidgetItem("0.1/30"));

  SPAR_GridLayout->addWidget(S1PInputTable, 5, 0, 1, 3);
  SPAR->setLayout(SPAR_GridLayout);

  SPARDataLayout->addWidget(SPAR, 1, 0, 1, 3);

  // S1P file chooser
  S1P_Groupbox = new QGroupBox("S1P file");
  QGridLayout *S1P_File_Layout = new QGridLayout();
  S1PFileButton = new QPushButton("Browse S1P");
  connect(S1PFileButton, SIGNAL(clicked(bool)), this, SLOT(LoadS1PFile()));
  S1P_File_Layout->addWidget(S1PFileButton, 0, 0);
  S1P_Groupbox->hide(); // By default this is hidden
  S1P_Groupbox->setLayout(S1P_File_Layout);

  SPARDataLayout->addWidget(S1P_Groupbox, 2, 0, 1, 3);

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

  S1PTable = new QTableWidget();
  S1PTable->setRowCount(0);
  S1PTable->setColumnCount(3);
  QStringList Labels;
  Labels << "Freq"
         << "S11"
         << "Z11";
  S1PTable->setHorizontalHeaderLabels(Labels);
  S1PTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  S1PTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  DataWidget->setLayout(DataLayout);
  DataLayout->addWidget(S1PTable, 2, 0, 1, 4);
  DataLayout->addWidget(new QLabel(QString("Z0 = 50%1").arg(QChar(0xa9, 0x03))),
                        3, 0);
  DeletePoint = new QPushButton("Remove sample");
  ClearAll = new QPushButton("Clear all");
  connect(ClearAll, SIGNAL(clicked(bool)), this, SLOT(ClearTable()));

  ReadyButton = new QPushButton("Update data");
  connect(ReadyButton, SIGNAL(clicked(bool)), this, SLOT(ReadyButtonHandle()));
  DataLayout->addWidget(DeletePoint, 4, 0);
  DataLayout->addWidget(ClearAll, 4, 1);
  DataLayout->addWidget(ReadyButton, 4, 2);

  SPARDataLayout->addWidget(DataWidget, 3, 0, 1, 4);

  this->setLayout(SPARDataLayout);
}

S1P_InputWidget::~S1P_InputWidget() {
  delete AddS1PRadioButton;
  delete AddSingleFreqRadioButton;
  delete freqLabel;
  delete freqScaleCombo;
  delete freqSpinBox;
  delete RI_Radiobutton;
  delete S1PTable;
  delete S1PInputTable;
  delete DeletePoint;
  delete ClearAll;
  delete SPAR;
  delete RadDegGroupbox;
  delete S1P_Groupbox;
  delete InputData_GroupBox;
  delete S_EntryData_RadioButton;
  delete Z_EntryData_RadioButton;

  delete S_Matrix_Label;
}

void S1P_InputWidget::UpdateS1PDataEntry() {
  if (AddS1PRadioButton->isChecked()) {
    S1P_Groupbox->show();
    SPAR->hide();
  } else {
    S1P_Groupbox->hide();
    SPAR->show();
  }
}

void S1P_InputWidget::LoadS1PFile() {
  QString filename = QFileDialog::getOpenFileName(
      this, tr("Open S1P File"), "/home", tr("Touchstone S1P (*.S1P)"));
  if (filename.isEmpty())
    return;

  IO *readS1P = new IO();
  readS1P->loadS1Pdata(filename.toStdString(), LOAD);
  S1P_DATA data = readS1P->getS1P(LOAD);

  // Clear previous data
  Freq.clear();
  S11.clear();
  Z11.clear();
  ClearTable();

  for (unsigned int i = 0; i < data.Freq.size(); i++) {
    Freq.push_back(data.Freq[i]);
    Z11.push_back(data.Z11[i]);
    // The data given by the IO object is an impedance. It is needed to
    // calculate the reflection coefficient
    std::complex<double> Z0(50, 0);
    S11.push_back((Z0 - data.Z11[i]) / (Z0 + data.Z11[i]));
  }

  delete readS1P; // No longer needed
  PutDataInTable();
}

double S1P_InputWidget::getFreqScale(QString fscale) {
  double scale;
  if (fscale == "GHz")
    scale = 1e9;
  else if (fscale == "MHz")
    scale = 1e6;
  else if (fscale == "kHz")
    scale = 1e3;
  else
    scale = 1;
  return scale;
}

// Triggered when the user presses the "add single point" button
void S1P_InputWidget::addSingleFreqData() {

  // Read frequency from the interface
  QString fscale = freqScaleCombo->currentText();
  double freq = freqSpinBox->value() * getFreqScale(fscale);

  // Read the S11 or Z11 data
  // The data may be entered as R+jX or MA. The magnitude may be entered in
  // natural units (default) or dB. On the other hand, the angle may be
  // expressed in degrees (default) or radians

  std::complex<double> X =
      ReadS1PFromUserInput(S1PInputTable->item(0, 0)->text());
  std::complex<double> Z0 = std::complex<double>(50, 0);

  // Is the input an s-parameter or an impedance?
  if (Z_EntryData_RadioButton->isChecked()) {
    Z11.push_back(X);
    S11.push_back((X - Z0) / (X + Z0));
  } else {
    // The data entered is the reflection coefficient
    S11.push_back(X);
    Z11.push_back(Z0 * (complex<double>(1, 0) + X) /
                  (complex<double>(1, 0) - X));
  }

  // Add the previous data to the internal data storage structures
  Freq.push_back(freq);
  SortData();

  // Now, put the sorted data in the table
  PutDataInTable();

  // Increase the value in the freq spinbox so as to minimize the probability of
  // the user entering duplicated data
  freqSpinBox->setValue(freqSpinBox->value() + 1);
}

// This function writes the content of the class variable 'DATA' in the main
// QTableWidget
void S1P_InputWidget::PutDataInTable() {

  // Check the MA-RI settings
  if (MA_RadioButton->isChecked()) {
    // Enable deg/rag controls
    RadDegGroupbox->setEnabled(true);
  } else {
    // Disable deg/rag controls
    RadDegGroupbox->setEnabled(false);
  }

  S1PTable->clearContents();
  for (int r = 0; r < Freq.size(); r++) {
    if (r >= S1PTable->rowCount()) {
      // Add a new row to the table
      S1PTable->insertRow(S1PTable->rowCount());
    }

    // Add data to the row
    S1PTable->setItem(r, 0,
                      new QTableWidgetItem(QString("%1").arg(
                          num2str(Freq.at(r), 3, Frequency)))); // Frequency
    S1PTable->setItem(
        r, 1,
        new QTableWidgetItem(QString("%1").arg(rect2polar(S11.at(r))))); // S11
    S1PTable->setItem(
        r, 2,
        new QTableWidgetItem(QString("%1").arg(rect2polar(Z11.at(r))))); // Z11
  }
}

// This function converts a QString complex data into std::complex
std::complex<double> S1P_InputWidget::ReadS1PFromUserInput(QString data) {
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

void S1P_InputWidget::ClearTable() { S1PTable->clear(); }

QString S1P_InputWidget::rect2polar(std::complex<double> Z) {
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
void S1P_InputWidget::SortData() {

  if (Freq.size() == 1)
    return;
  std::deque<double> freq_sorted;
  std::deque<std::complex<double>> S11_sorted, Z11_sorted;
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
  Z11_sorted.resize(freq_sorted.size());

  // Now apply the same order in the other vectors
  for (unsigned int i = 0; i < sort_vector.size(); i++) {
    S11_sorted[i] = S11[sort_vector[i]];
    Z11_sorted[i] = Z11[sort_vector[i]];
  }

  // Replace the sorted vectors in the SPAR structure
  Freq.clear();
  S11.clear();
  Z11.clear();

  for (unsigned int i = 0; i < S11_sorted.size(); i++) {
    Freq.push_back(freq_sorted[i]);
    S11.push_back(S11_sorted[i]);
    Z11.push_back(Z11_sorted[i]);
  }
  return;
}

// This function is triggered when the user changes the data input mode and
// modifies the visibility of the UI elements
void S1P_InputWidget::ChangeSZMode() {
  if (Z_EntryData_RadioButton->isChecked()) {
    // Z mode
    S_Matrix_Label->setText(QString("Impedance [%1]").arg(QChar(0xa9, 0x03)));
    Z0Label->hide();
    Z0_Ohm_Label->hide();
    Z0_SpinBox->hide();
  } else {
    // S mode
    S_Matrix_Label->setText("Reflection coefficient");
    Z0Label->show();
    Z0_Ohm_Label->show();
    Z0_SpinBox->show();
  }
}

void S1P_InputWidget::ReadyButtonHandle() {
  S1P_DATA data;
  data.Freq.resize(Freq.size());
  data.Z11.resize(Freq.size());
  for (int i = 0; i < Freq.size(); i++) {
    data.Freq[i] = Freq[i];
    data.Z11[i] = Z11[i];
  }
  emit sendData(data);
}
