#ifndef GENERAL_H
#define GENERAL_H
#include "Filtering/Network.h"
#include "Schematic/component.h"
#include <QString>
#include <cmath>
enum Units { Capacitance, Inductance, Length, Resistance };

QString RoundVariablePrecision(double);
QString num2str(double, Units);
QString num2str(double);
std::complex<double> Str2Complex(QString);
QString ConvertLengthFromM(QString, double);

#endif // GENERAL_H
