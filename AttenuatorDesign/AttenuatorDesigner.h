/***************************************************************************
                                AttenuatorDesigner.h
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
#ifndef ATTENUATORDESIGNER_H
#define ATTENUATORDESIGNER_H
#include "Filtering/Network.h"
#include "general.h"
#include "math.h"

struct PdissAtt {
  double R1, R2, R3, R4;
};

class AttenuatorDesigner {
public:
  AttenuatorDesigner(AttenuatorDesignParameters);
  QList<ComponentInfo> getComponents();
  QList<WireInfo> getWires();
  QList<NodeInfo> getNodes();
  QMap<QString, QPen> displaygraphs;
  void synthesize();
  QString getQucsNetlist() { return QucsNetlist; };
  struct PdissAtt Pdiss; // Power dissipated by the resistors

private:
  AttenuatorDesignParameters Specs;
  QList<ComponentInfo> Components;
  QList<WireInfo> Wires;
  QList<NodeInfo> Nodes;

  QString QucsNetlist;
  QMap<ComponentType, int>
      NumberComponents; // List for assigning IDs to the filter components

  // Attenuator design functions
  void PiAttenuator();
  void TeeAttenuator();
  void BridgedTeeAttenuator();
  void ReflectionAttenuator();
};

#endif // ATTENUATORDESIGNER_H
