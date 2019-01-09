/***************************************************************************
                                preferencesdialog.cpp
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
#include "preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) : QWidget(parent) {
  QGridLayout *mainLayout = new QGridLayout(this);
  //************** Simulation settings ****************
  QGroupBox *SimSettingsGroupBox = new QGroupBox("Simulation settings");
  QGridLayout *SimSettingsLayout = new QGridLayout(this);
  fstartLineedit = new QLineEdit("500");
  fstartScaleCombo = new QComboBox();
  fstartScaleCombo->addItem("GHz");
  fstartScaleCombo->addItem("MHz");
  fstartScaleCombo->addItem("kHz");
  fstartScaleCombo->addItem("Hz");
  fstartScaleCombo->setCurrentIndex(1);

  fstopLineedit = new QLineEdit("2000");
  fstopScaleCombo = new QComboBox();
  fstopScaleCombo->addItem("GHz");
  fstopScaleCombo->addItem("MHz");
  fstopScaleCombo->addItem("kHz");
  fstopScaleCombo->addItem("Hz");
  fstopScaleCombo->setCurrentIndex(1);

  NpointsLineedit = new QLineEdit("500");
  SimSettingsLayout->addWidget(new QLabel("fstart"), 0, 0);
  SimSettingsLayout->addWidget(fstartLineedit, 0, 1);
  SimSettingsLayout->addWidget(fstartScaleCombo, 0, 2);

  SimSettingsLayout->addWidget(new QLabel("fstop"), 1, 0);
  SimSettingsLayout->addWidget(fstopLineedit, 1, 1);
  SimSettingsLayout->addWidget(fstopScaleCombo, 1, 2);

  SimSettingsLayout->addWidget(new QLabel("Npoints"), 2, 0);
  SimSettingsLayout->addWidget(NpointsLineedit, 2, 1);

  SimSettingsGroupBox->setLayout(SimSettingsLayout);
  mainLayout->addWidget(SimSettingsGroupBox, 0, 0);

  // ********************* Trace settings **********************
  QGroupBox *TraceSettingsGroupBox = new QGroupBox("Traces");
  QGridLayout *TraceSettingsLayout = new QGridLayout(this);
  S21Checkbox = new QCheckBox("S21 (dB)");
  S11Checkbox = new QCheckBox("S11 (dB)");
  S22Checkbox = new QCheckBox("S22 (dB)");
  S21Checkbox->setChecked(true);
  S11Checkbox->setChecked(true);

  S21ColorButton = new QPushButton();
  S11ColorButton = new QPushButton();
  S22ColorButton = new QPushButton();

  S21ColorButton->setStyleSheet("background-color: red");
  S11ColorButton->setStyleSheet("background-color: blue");
  S22ColorButton->setStyleSheet("background-color: darkGreen");

  // Add checkboxes
  TraceSettingsLayout->addWidget(S21Checkbox, 0, 0);
  TraceSettingsLayout->addWidget(S11Checkbox, 1, 0);
  TraceSettingsLayout->addWidget(S22Checkbox, 2, 0);

  // Add color buttons
  TraceSettingsLayout->addWidget(S21ColorButton, 0, 1);
  TraceSettingsLayout->addWidget(S11ColorButton, 1, 1);
  TraceSettingsLayout->addWidget(S22ColorButton, 2, 1);

  // Connect buttons to event handler
  connect(S21ColorButton, SIGNAL(clicked(bool)), this, SLOT(S21ColorHandler()));
  connect(S11ColorButton, SIGNAL(clicked(bool)), this, SLOT(S11ColorHandler()));
  connect(S22ColorButton, SIGNAL(clicked(bool)), this, SLOT(S22ColorHandler()));

  // Add widget to the panel
  TraceSettingsGroupBox->setLayout(TraceSettingsLayout);
  mainLayout->addWidget(TraceSettingsGroupBox, 1, 0);

  // Graph scale
  QGroupBox *GraphSettingsGroupBox = new QGroupBox("Graph");
  QGridLayout *GraphSettingsLayout = new QGridLayout(this);

  GraphYminLineedit = new QLineEdit("-50");
  GraphYmaxLineedit = new QLineEdit("0");
  GraphYstepLineedit = new QLineEdit("5");

  FixedScaleCheckbox = new QCheckBox("Fix scale");
  FixedScaleCheckbox->setChecked(true);
  GraphFstepLineedit = new QLineEdit("100");
  XscaleStepCombobox = new QComboBox();
  XscaleStepCombobox->addItem("GHz");
  XscaleStepCombobox->addItem("MHz");
  XscaleStepCombobox->addItem("kHz");
  XscaleStepCombobox->addItem("Hz");
  XscaleStepCombobox->setCurrentIndex(1);

  GraphSettingsLayout->addWidget(FixedScaleCheckbox, 0, 0);

  GraphSettingsLayout->addWidget(new QLabel("Y min"), 1, 0);
  GraphSettingsLayout->addWidget(GraphYminLineedit, 1, 1);
  GraphSettingsLayout->addWidget(new QLabel("dB"), 1, 2);

  GraphSettingsLayout->addWidget(new QLabel("Y max"), 2, 0);
  GraphSettingsLayout->addWidget(GraphYmaxLineedit, 2, 1);
  GraphSettingsLayout->addWidget(new QLabel("dB"), 2, 2);

  GraphSettingsLayout->addWidget(new QLabel("Y step"), 3, 0);
  GraphSettingsLayout->addWidget(GraphYstepLineedit, 3, 1);
  GraphSettingsLayout->addWidget(new QLabel("dB"), 3, 2);

  GraphSettingsLayout->addWidget(new QLabel("X step"), 4, 0);
  GraphSettingsLayout->addWidget(GraphFstepLineedit, 4, 1);
  GraphSettingsLayout->addWidget(XscaleStepCombobox, 4, 2);

  GraphSettingsGroupBox->setLayout(GraphSettingsLayout);
  mainLayout->addWidget(GraphSettingsGroupBox, 2, 0);

  //************** Path to qucsator ***********
  QPushButton *QucsactorbrowseButton = new QPushButton("Browse...", this);
  connect(QucsactorbrowseButton, SIGNAL(clicked()), this,
          SLOT(Qucsactorbrowse()));
  mainLayout->addWidget(new QLabel("Path to qucsator:"), 3, 0);
  mainLayout->addWidget(QucsactorbrowseButton, 3, 1);
  //***************************** end path to qucsator

  //************** Save Button ************
  QPushButton *SaveButton = new QPushButton("Save", this);
  mainLayout->addWidget(SaveButton, 4, 0);
  connect(SaveButton, SIGNAL(clicked()), this, SLOT(Save()));

  TracesColor.push_back(Qt::red);
  TracesColor.push_back(Qt::blue);
  TracesColor.push_back(Qt::darkGreen);

  setWindowTitle("Preferences");
  setLayout(mainLayout);
  setMinimumSize(QSize(100, 100));
}

// Select the path to qucsator
void PreferencesDialog::Qucsactorbrowse() {
  QFileDialog::getOpenFileName(this, tr("Path to qucsator"), "~",
                               tr("Binary Files"));
}

void PreferencesDialog::Save() {
  ToolSettings TS;
  TS.fstart = fstartLineedit->text().toDouble() *
              getScale(fstartScaleCombo->currentText());
  TS.fstop = fstopLineedit->text().toDouble() *
             getScale(fstopScaleCombo->currentText());
  TS.Npoints = NpointsLineedit->text().toDouble();
  TS.TraceColor = TracesColor;
  TS.ystep = GraphYstepLineedit->text().toDouble();
  TS.ymin = GraphYminLineedit->text().toDouble();
  TS.ymax = GraphYmaxLineedit->text().toDouble();
  TS.xstep = GraphFstepLineedit->text().toDouble() *
             getScale(XscaleStepCombobox->currentText());
  TS.FixedAxes = FixedScaleCheckbox->isChecked();

  // Selected traces
  std::vector<bool> ST;
  ST.push_back(S21Checkbox->isChecked());
  ST.push_back(S11Checkbox->isChecked());
  ST.push_back(S22Checkbox->isChecked());
  TS.ShowTraces = ST;

  emit sendSettings(TS);
}

double PreferencesDialog::getScale(QString scale) {
  if (!scale.compare("GHz"))
    return 1e9;
  if (!scale.compare("MHz"))
    return 1e6;
  if (!scale.compare("kHz"))
    return 1e3;
  else
    return 1;
}

void PreferencesDialog::S21ColorHandler() {
  TracesColor.at(0) = QColorDialog::getColor(Qt::red, this);
  S21ColorButton->setStyleSheet(
      QString("background-color: %1").arg(TracesColor.at(0).name()));
}

void PreferencesDialog::S11ColorHandler() {
  TracesColor.at(1) = QColorDialog::getColor(Qt::blue, this);
  S11ColorButton->setStyleSheet(
      QString("background-color: %1").arg(TracesColor.at(1).name()));
}

void PreferencesDialog::S22ColorHandler() {
  TracesColor.at(2) = QColorDialog::getColor(Qt::darkGreen, this);
  S22ColorButton->setStyleSheet(
      QString("background-color: %1").arg(TracesColor.at(2).name()));
}
