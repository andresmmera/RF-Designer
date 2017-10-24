#include <QtGui>
#include "UI/QucsRFDesignerWindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QucsRFDesignerWindow mainWindow;
    mainWindow.setWindowIcon(QIcon(":/bitmaps/qucsRFdesigner.svg"));
    mainWindow.showMaximized();
    return app.exec();
}



