/***************************************************************************
                                QW_SeriesAttenuator.cpp
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
#ifndef QW_SERIESATTENUATOR_H
#define QW_SERIESATTENUATOR_H
#include "Schematic/Network.h"
#include "Schematic/SchematicContent.h"
#include "Schematic/component.h"

class QW_SeriesAttenuator : public Network {
public:
  QW_SeriesAttenuator();
  virtual ~QW_SeriesAttenuator();
  QW_SeriesAttenuator(AttenuatorDesignParameters);
  void synthesize();
  struct PdissAtt Pdiss;

private:
  struct AttenuatorDesignParameters Specs;
};
#endif // QW_SERIESATTENUATOR_H
