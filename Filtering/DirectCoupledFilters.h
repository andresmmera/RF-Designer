/***************************************************************************
                                DirectCoupledFilters.h
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
#ifndef DIRECTCOUPLEDFILTERS_H
#define DIRECTCOUPLEDFILTERS_H
#include "Filtering/LowpassPrototypeCoeffs.h"
#include "Filtering/Network.h"
#include "Schematic/component.h"
#include "general.h"
#include <QPen>

class DirectCoupledFilters : public Network {
public:
  DirectCoupledFilters();
  virtual ~DirectCoupledFilters();
  DirectCoupledFilters(FilterSpecifications);
  void synthesize();
  SchematicContent getSchematic() { return Schematic; }

private:
  struct FilterSpecifications Specification;
  SchematicContent Schematic;

  std::deque<double> gi; // Lowpass prototype

  void Synthesize_Capacitative_Coupled_Shunt_Resonators();
  void Synthesize_Inductive_Coupled_Series_Resonators();
};

#endif // DIRECTCOUPLED_H
