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

#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class PreferencesDialog : public QWidget {
  Q_OBJECT
public:
  PreferencesDialog(QWidget *parent = 0);
private slots:
  void Qucsactorbrowse();
  void Save();

private:
  QString path_qucsator;

signals:
  void sendSettings(struct ToolSettings);
};

#endif // PREFERENCESDIALOG_
