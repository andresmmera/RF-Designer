/***************************************************************************
                                general.cpp
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
#include "general.h"

// Sort by frequency an s-parameter data struct
struct S2P_DATA Sort(struct S2P_DATA data) {

  if (data.Freq.size() == 1)
    return data;
  std::deque<double> freq_sorted;
  std::deque<std::complex<double>> S11_sorted, S12_sorted, S21_sorted,
      S22_sorted;
  std::deque<int> sort_vector;
  double min = 1e20, last_min = -1;
  int min_index;

  do {
    for (unsigned int i = 0; i < data.Freq.size(); i++) {
      if ((data.Freq.at(i) < min) && (data.Freq.at(i) >= last_min)) {
        min = data.Freq.at(i);
        min_index = i;
      }
    }

    if (min == last_min) {
      // Duplicated data
    } else {
      sort_vector.push_back(min_index);
      freq_sorted.push_back(min);
      last_min = min;
    }

    data.Freq.erase(data.Freq.begin() + min_index);
    min = 1e20;
  } while (data.Freq.size() > 0);

  // Resize vectors to the size of freq_sorted
  S11_sorted.resize(freq_sorted.size());
  S21_sorted.resize(freq_sorted.size());
  S12_sorted.resize(freq_sorted.size());
  S22_sorted.resize(freq_sorted.size());

  // Now apply the same order in the other vectors
  for (int i = 0; i < sort_vector.size(); i++) {
    S11_sorted[i] = data.S11[sort_vector[i]];
    S12_sorted[i] = data.S12[sort_vector[i]];
    S21_sorted[i] = data.S21[sort_vector[i]];
    S22_sorted[i] = data.S22[sort_vector[i]];
  }

  // Replace the sorted vectors in the SPAR structure
  data.Freq.clear();
  data.S11.clear();
  data.S21.clear();
  data.S12.clear();
  data.S22.clear();

  data.Freq = freq_sorted;
  data.S11 = S11_sorted;
  data.S12 = S12_sorted;
  data.S21 = S21_sorted;
  data.S22 = S22_sorted;

  return data;
}

// Rounds a double number using the minimum number of decimal places
QString RoundVariablePrecision(double val) {
  return RoundVariablePrecision(val, 0);
}
QString RoundVariablePrecision(double val, int precision) {
  int sign = 1;
  if (val < 0)
    sign = -1;
  val = std::abs(val);
  while (val * pow(10, precision) < 100)
    precision++; // Adds another decimal place if the conversion is less than
                 // 0.1, 0.01, etc
  return QString::number(sign * val, 'F',
                         precision); // Round to 'precision' decimals.
}

QString num2str(std::complex<double> Z, Units CompType) {
  QString Real = num2str(Z.real());

  if (abs(Z.imag()) < 1e-6)
    return num2str(Z.real(), CompType);
  else
    Real = num2str(Z.real());

  if (Z.imag() < 0)
    return QString("%1 -j %2").arg(Real).arg(num2str(abs(Z.imag()), CompType));
  else
    return QString("%1 +j %2").arg(Real).arg(num2str(Z.imag(), CompType));
}

QString num2str(double Num, Units CompType) {
  return num2str(Num, 0, CompType);
}

QString num2str(double Num, int precision, Units CompType) {
  QString Str = num2str(Num, precision);
  QString unit;
  switch (CompType) {
  case Capacitance:
    unit = QString("F");
    break;
  case Inductance:
    unit = QString("H");
    break;
  case Resistance:
    unit = QString("Ohm");
    break;
  case Degrees:
    unit = QString("º");
    break;
  case Frequency:
    unit = QString("Hz");
    break;
  default:
    break;
  }
  Str += unit;
  return Str;
}

QString num2str(double Num) { return num2str(Num, 0); }
QString num2str(double Num, int precision) {
  char c = 0;
  double cal = std::abs(Num);
  if (cal > 1e-20) {
    cal = std::log10(cal) / 3.0;
    if (cal < -0.2)
      cal -= 0.98;
    int Expo = int(cal);

    if (Expo >= -5)
      if (Expo <= 4)
        switch (Expo) {
        case -5:
          c = 'f';
          break;
        case -4:
          c = 'p';
          break;
        case -3:
          c = 'n';
          break;
        case -2:
          c = 'u';
          break;
        case -1:
          c = 'm';
          break;
        case 1:
          c = 'k';
          break;
        case 2:
          c = 'M';
          break;
        case 3:
          c = 'G';
          break;
        case 4:
          c = 'T';
          break;
        }

    if (c)
      Num /= pow(10.0, double(3 * Expo));
  }

  QString Str = RoundVariablePrecision(Num, precision);
  if (c)
    Str += c;
  return Str;
}

std::complex<double> Str2Complex(QString num) {
  // Remove whitespaces
  for (int i = 0; i < num.length(); i++)
    if (num.at(i).isSpace())
      num.remove(i, 1);

  // Remove the suffix
  if (num.indexOf("Ohm") != -1)
    num.remove(num.indexOf("Ohm"), 3);
  int index = num.indexOf("j"); // Indicates where the j is.
  if (index == -1) {            // Actually, num is a real number
    return std::complex<double>(num.toDouble(), 0);
  }

  // Need to separate the real from the imaginary part
  double sign = 1;
  if (num[index - 1] == '-')
    sign = -1;
  double realpart =
      num.left(index - 1)
          .toDouble(); // Notice  we have to take into account the sign
  double imagpart = num.right(index - 1).toDouble();
  return std::complex<double>(realpart, sign * imagpart);
}

// This function creates a string for the transmission line length and
// automatically changes the unit length if the value lies outside [1,999.99]
QString ConvertLengthFromM(QString units, double len) {
  int index;

  if (units == "mm")
    index = 0;
  if (units == "mil")
    index = 1;
  if (units == "um")
    index = 2;
  if (units == "nm")
    index = 3;
  if (units == "inch")
    index = 4;
  if (units == "ft")
    index = 5;
  if (units == "m")
    index = 6;

  double conv;

  do {
    conv = len;
    switch (index) {
    case 1: // mils
      conv *= 39370.1;
      if (conv > 999.99) {
        index = 4; // inches
        break;
      }
      if (conv < 1) {
        index = 2; // microns
        break;
      }
      return QString("%1 mil").arg(RoundVariablePrecision(conv));
    case 2: // microns
      conv *= 1e6;
      if (conv > 999.99) {
        index = 0; // milimeters
        break;
      }
      if (conv < 1) {
        index = 3; // nanometers
        break;
      }
      return QString("%1 um").arg(RoundVariablePrecision(conv));
    case 3: // nanometers
      conv *= 1e9;
      if (conv > 999.99) {
        index = 2; // microns
        break;
      }
      return QString("%1 nm").arg(RoundVariablePrecision(conv));
    case 4: // inch
      conv *= 39.3701;
      if (conv > 999.99) {
        index = 5; // feets
        break;
      }
      if (conv < 1) {
        index = 1; // mils
        break;
      }
      return QString("%1 in").arg(RoundVariablePrecision(conv));
    case 5: // ft
      conv *= 3.280841666667;
      if (conv > 999.99) {
        index = 6; // meters
        break;
      }
      if (conv < 1) {
        index = 4; // inches
        break;
      }
      return QString("%1 ft").arg(RoundVariablePrecision(conv));
    case 6: // m
      if (conv < 1) {
        index = 0; // mm
        break;
      }
      return QString("%1").arg(RoundVariablePrecision(len));
    default: // milimeters
      conv *= 1e3;
      if (conv > 999.99) {
        index = 6; // meters
        break;
      }
      if (conv < 1) {
        index = 2; // microns
        break;
      }
      return QString("%1 mm").arg(RoundVariablePrecision(conv));
    }
  } while (true);
  return QString("");
}
