/***************************************************************************
                                QuarterWaveFilters.h
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

#ifndef QUARTERWAVEFILTERS_H
#define QUARTERWAVEFILTERS_H

#include "Filtering/LowpassPrototypeCoeffs.h"
#include "Filtering/Network.h"
#include "Schematic/component.h"
#include "general.h"
#include <QPen>

class QuarterWaveFilters : public Network {
public:
  QuarterWaveFilters();
  virtual ~QuarterWaveFilters();
  QuarterWaveFilters(FilterSpecifications);
  void synthesize();
  SchematicContent getSchematic() { return Schematic; }

private:
  struct FilterSpecifications Specification;
  SchematicContent Schematic;
};

#endif // QUARTERWAVEFILTERS_H
