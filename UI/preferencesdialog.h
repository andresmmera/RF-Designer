#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QColorDialog>
#include "UI/QucsRFDesignerWindow.h"

class PreferencesDialog : public QWidget
{
    Q_OBJECT
public:
    PreferencesDialog(QWidget *parent = 0);
private slots:
    void Qucsactorbrowse();
    void Save();
    //Color handlers
    void S21ColorHandler();
    void S11ColorHandler();
    void S22ColorHandler();
private:
    //S parameter simulation settings
    QLineEdit *fstartLineedit, *fstopLineedit, *NpointsLineedit;
    QComboBox *fstartScaleCombo, *fstopScaleCombo;

    //Trace settings
    QCheckBox *S21Checkbox, *S11Checkbox, *S22Checkbox;
    QPushButton *S21ColorButton, *S11ColorButton, *S22ColorButton;
    std::vector<QColor> TracesColor;

    //Graph settings
    QCheckBox *FixedScaleCheckbox;
    QLineEdit *GraphYminLineedit, *GraphYmaxLineedit, *GraphYstepLineedit, *GraphFstepLineedit;
    QComboBox *XscaleStepCombobox;

    //Frequency scale
    double getScale(QString);
signals:
    void sendSettings(struct ToolSettings);
};

#endif // PREFERENCESDIALOG_
