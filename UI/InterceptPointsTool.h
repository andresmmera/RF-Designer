#ifndef INTERCEPTPOINTS_H
#define INTERCEPTPOINTS_H
#include <QLabel>
#include <QSpinBox>
#include <QGridLayout>
#include "general.h"
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>

class InterceptPointsTool : public QWidget
{
     Q_OBJECT
public:
    InterceptPointsTool();
private:

    QDoubleSpinBox *FundamentalPowerSpinbox, *GainSpinbox, *IM3PowerSpinbox, *IM2PowerSpinbox;
    QDoubleSpinBox *CenterFreqSpinbox, *deltaSpinBox;
    QDoubleSpinBox *OIP3Spinbox, *OIP2Spinbox;
    QLineEdit *IIP3LineEdit, *IIP2LineEdit;
    QCheckBox *CalculateInterceptPointsCheckbox;
    struct InterceptPointsData IPdata;

public slots:
    void CalculateInterceptPoints();//It is public to allow the main tool to
    //create the intercept diagram without changing the input data

private slots:
    void EnableCalcIP();
    void UpdateInterceptPoints();
    void SendSimulationData();

signals:
    void simulateDiagram(struct InterceptPointsData);

};

#endif // INTERCEPTPOINTS_H
