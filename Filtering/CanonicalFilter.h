/***************************************************************************
                                CanonicalFilter.h
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
#ifndef CANONICALFILTER_H
#define CANONICALFILTER_H

#include "Filtering/LowpassPrototypeCoeffs.h"
#include "Filtering/Network.h"
#include "Schematic/SchematicContent.h"
#include "Schematic/component.h"
#include "general.h"
#include <QPen>
class CanonicalFilter : public Network {
public:
  CanonicalFilter();
  virtual ~CanonicalFilter();
  CanonicalFilter(FilterSpecifications);
  void synthesize();
  void setSemilumpedMode(bool);
  SchematicContent getSchematic();

private:
  struct FilterSpecifications Specification;
  bool semilumped = false; // Activate semilumped implementation mode

  SchematicContent Schematic; // This object contains all the circuit data of
                              // the filter, i.e. components and nets and it
                              // returns data for the simulation

  std::deque<double> gi;

  //***********  Schematic synthesis ********************
  void SynthesizeLPF();           // Lowpass
  void SynthesizeSemilumpedLPF(); // Semilumped Lowpass implementation
  void SynthesizeHPF();           // Highpass
  void SynthesizeBPF();           // Bandpass
  void SynthesizeBSF();           // Bandstop
};

#endif // CANONICALFILTER_H
