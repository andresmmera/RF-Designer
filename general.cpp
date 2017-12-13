#include "general.h"


//Rounds a double number using the minimum number of decimal places
QString RoundVariablePrecision(double val)
{
  int precision = 0;//By default, it takes 2 decimal places
  int sign = 1;
  if (val < 0) sign = -1;
  val = abs(val);
  while (val*pow(10, precision) < 100) precision++;//Adds another decimal place if the conversion is less than 0.1, 0.01, etc
  return QString::number(sign*val, 'F', precision);// Round to 'precision' decimals.
}


QString num2str(double Num, Units CompType)
{
  char c = 0;
  double cal = std::abs(Num);
  if(cal > 1e-20) {
    cal = std::log10(cal) / 3.0;
    if(cal < -0.2)  cal -= 0.98;
    int Expo = int(cal);

    if(Expo >= -5) if(Expo <= 4)
      switch(Expo) {
        case -5: c = 'f'; break;
        case -4: c = 'p'; break;
        case -3: c = 'n'; break;
        case -2: c = 'u'; break;
        case -1: c = 'm'; break;
        case  1: c = 'k'; break;
        case  2: c = 'M'; break;
        case  3: c = 'G'; break;
        case  4: c = 'T'; break;
      }

    if(c)  Num /= pow(10.0, double(3*Expo));
  }

  QString Str = RoundVariablePrecision(Num);
  if(c)  Str += c;
  QString unit;
  switch (CompType)
  {
     case Capacitance:
                 unit = QString("F");
                    break;
     case Inductance:
                 unit = QString("H");
                 break;
     case Resistance:
                 unit = QString("Ohm");
                 break;
  }
  Str += unit;
  return Str;
}
