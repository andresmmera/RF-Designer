/***************************************************************************
                                main.cpp
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
#include "UI/QucsRFDesignerWindow.h"
#include <QtGui>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QucsRFDesignerWindow mainWindow;
  mainWindow.setWindowIcon(QIcon(":/bitmaps/qucsRFdesigner.svg"));
  mainWindow.showMaximized();
  return app.exec();
}
