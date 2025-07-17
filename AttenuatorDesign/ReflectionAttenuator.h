/***************************************************************************
                                ReflectionAttenuator.cpp
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
#ifndef REFLECTIONATTENUATOR_H
#define REFLECTIONATTENUATOR_H
#include "Schematic/Network.h"
#include "Schematic/SchematicContent.h"
#include "Schematic/component.h"

class ReflectionAttenuator : public Network {
public:
  ReflectionAttenuator();
  virtual ~ReflectionAttenuator();
  ReflectionAttenuator(AttenuatorDesignParameters);
  void synthesize();
  struct PdissAtt Pdiss;

private:
  AttenuatorDesignParameters Specs;
};
#endif // REFLECTIONATTENUATOR_H
