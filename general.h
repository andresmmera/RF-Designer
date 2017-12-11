#ifndef GENERAL_H
#define GENERAL_H
#include <QString>
#include <cmath>
#include "Filtering/Network.h"
#include "Schematic/component.h"
enum Units{Capacitance, Inductance, Length, Resistance};


struct InterceptPointsData
{
    double IP3;
    double IP2;
    double Gain;
    double Pout;
    double IM2;
    double IM3;
    double fc;
    double delta;
};

QString RoundVariablePrecision(double);
QString num2str(double, Units);


#endif // GENERAL_H
