/***************************************************************************
                                LowpassPrototypeCoeffs.h
                                ----------
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
#ifndef LOWPASSPROTOTYPECOEFFS_H
#define LOWPASSPROTOTYPECOEFFS_H
#include "Schematic/Network.h"
#include "Schematic/structures.h"
#include <QtSql>
#include <deque>

class LowpassPrototypeCoeffs {
public:
  LowpassPrototypeCoeffs(FilterSpecifications);
  std::deque<double> getCoefficients();
  void setDatabase(QSqlDatabase);

private:
  QSqlDatabase Zverev_db;
  FilterSpecifications Specification;

  std::deque<double> calcButterworth_gi();
  std::deque<double> calcChebyshev_gi();
};

#endif // LOWPASSPROTOTYPECOEFFS_H
