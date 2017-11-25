#ifndef GENERAL_H
#define GENERAL_H
#include <QString>
#include <cmath>
#include "Filtering/Network.h"
#include "Schematic/component.h"
enum Units{Capacitance, Inductance, Length, Resistance};

QString RoundVariablePrecision(double);
QString num2str(double, Units);


#endif // GENERAL_H
