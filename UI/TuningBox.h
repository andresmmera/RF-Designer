#ifndef TUNINGBOX_H
#define TUNINGBOX_H

#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QDockWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>
#include "Schematic/component.h"

/*
   This class inherits QDockWidget so as to create a dockable tuning box. This way the tuning box can be placed
   anywhere in the main window
*/

class TuningBox : public QDockWidget
{
    Q_OBJECT
public:
    TuningBox(ComponentInfo TI);
    QString getID();
private:
    QSlider *ValueSlider;
    QDoubleSpinBox *MinSpinBox, *MaxSpinBox, *CurrentValueSpinbox;
    QComboBox *MinValScaleCombo, *MaxValScaleCombo, *CurrentValScaleCombo;
    ComponentInfo TuningInfo;
    QPushButton *IncreaseValueButton, *DecreaseValueButton;
    QStringList createItems(ComponentInfo);
    double getScale(int);
public slots:
    void SliderChanged(int);
    void VisibilityHandler(bool visiblity);
    void IncreaseValue();
    void DecreaseValue();

signals:
    void TuningChanged(ComponentInfo);
    void ClosingEvent(QString);
};

#endif // tuningbox_
