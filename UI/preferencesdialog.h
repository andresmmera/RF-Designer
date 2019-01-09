/***************************************************************************
                                preferencesdialog.h
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
#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "UI/QucsRFDesignerWindow.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class PreferencesDialog : public QWidget {
  Q_OBJECT
public:
  PreferencesDialog(QWidget *parent = 0);
private slots:
  void Qucsactorbrowse();
  void Save();
  // Color handlers
  void S21ColorHandler();
  void S11ColorHandler();
  void S22ColorHandler();

private:
  // S parameter simulation settings
  QLineEdit *fstartLineedit, *fstopLineedit, *NpointsLineedit;
  QComboBox *fstartScaleCombo, *fstopScaleCombo;

  // Trace settings
  QCheckBox *S21Checkbox, *S11Checkbox, *S22Checkbox;
  QPushButton *S21ColorButton, *S11ColorButton, *S22ColorButton;
  std::vector<QColor> TracesColor;

  // Graph settings
  QCheckBox *FixedScaleCheckbox;
  QLineEdit *GraphYminLineedit, *GraphYmaxLineedit, *GraphYstepLineedit,
      *GraphFstepLineedit;
  QComboBox *XscaleStepCombobox;

  // Frequency scale
  double getScale(QString);
signals:
  void sendSettings(struct ToolSettings);
};

#endif // PREFERENCESDIALOG_
