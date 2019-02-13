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

  //************** Path to qucsator ***********
  QPushButton *QucsactorbrowseButton = new QPushButton("Browse...", this);
  connect(QucsactorbrowseButton, SIGNAL(clicked()), this,
          SLOT(Qucsactorbrowse()));
  mainLayout->addWidget(new QLabel("Path to qucsator:"), 0, 0);
  mainLayout->addWidget(QucsactorbrowseButton, 0, 1);
  //***************************** end path to qucsator

  //************** Save Button ************
  QPushButton *SaveButton = new QPushButton("Save", this);
  mainLayout->addWidget(SaveButton, 1, 0);
  connect(SaveButton, SIGNAL(clicked()), this, SLOT(Save()));

  setWindowTitle("Preferences");
  setLayout(mainLayout);
}

// Select the path to qucsator
void PreferencesDialog::Qucsactorbrowse() {
  path_qucsator = QFileDialog::getOpenFileName(this, tr("Path to qucsator"),
                                               "~", tr("Binary Files"));
}

void PreferencesDialog::Save() {
  ToolSettings TS;
  TS.PathToQucsator = path_qucsator;
  emit sendSettings(TS);
}
