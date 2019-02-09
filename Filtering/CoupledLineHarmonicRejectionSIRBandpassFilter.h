/***************************************************************************
                                CoupledLineHarmonicRejectionSIRBandpassFilter.h
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
#ifndef CoupledLineHarmonicRejectionSIRBandpassFilter_H
#define CoupledLineHarmonicRejectionSIRBandpassFilter_H

#include "Filtering/LowpassPrototypeCoeffs.h"
#include "Schematic/Network.h"
#include "Schematic/component.h"
#include "general.h"
#include <QPen>

class CoupledLineHarmonicRejectionSIRBandpassFilter : public Network {
public:
  CoupledLineHarmonicRejectionSIRBandpassFilter();
  virtual ~CoupledLineHarmonicRejectionSIRBandpassFilter();
  CoupledLineHarmonicRejectionSIRBandpassFilter(FilterSpecifications);
  void synthesize();

private:
  struct FilterSpecifications Specification;
};

#endif // CoupledLineHarmonicRejectionSIRBandpassFilter_H
