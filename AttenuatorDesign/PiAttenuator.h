/***************************************************************************
                                PiAttenuator.h
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
#ifndef PIATTENUATOR_H
#define PIATTENUATOR_H
#include "Schematic/Network.h"
#include "Schematic/SchematicContent.h"
#include "Schematic/component.h"

class PiAttenuator : public Network {
public:
  PiAttenuator();
  virtual ~PiAttenuator();
  PiAttenuator(AttenuatorDesignParameters);
  void synthesize();
  struct PdissAtt Pdiss;

private:
  struct AttenuatorDesignParameters Specs;
};
#endif // PIATTENUATOR_H
