/***************************************************************************
                                QW_ShuntAttenuator.cpp
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
#ifndef QW_SHUNTATTENUATOR_H
#define QW_SHUNTATTENUATOR_H
#include "Schematic/Network.h"
#include "Schematic/SchematicContent.h"
#include "Schematic/component.h"

class QW_ShuntAttenuator : public Network {
public:
  QW_ShuntAttenuator();
  virtual ~QW_ShuntAttenuator();
  QW_ShuntAttenuator(FilterSpecifications);
  void synthesize();
  struct PdissAtt Pdiss;

private:
  struct AttenuatorDesignParameters Specification;
};
#endif // QW_SHUNTATTENUATOR_H
