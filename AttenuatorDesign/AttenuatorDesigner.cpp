/***************************************************************************
                                AttenuatorDesigner.cpp
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
#include "AttenuatorDesigner.h"

AttenuatorDesigner::AttenuatorDesigner(AttenuatorDesignParameters SPC) {
  Specs = SPC;
}

QList<ComponentInfo> AttenuatorDesigner::getComponents() { return Components; }

QList<WireInfo> AttenuatorDesigner::getWires() { return Wires; }

QList<NodeInfo> AttenuatorDesigner::getNodes() { return Nodes; }

void AttenuatorDesigner::synthesize() {
  if (Specs.Topology == "Pi")
    PiAttenuator();

  // Build Qucs netlist
  QucsNetlist.clear();
  QString codestr;
  for (int i = 0; i < Components.length(); i++) {
    codestr = Components[i].getQucs();
    if (!codestr.isEmpty())
      QucsNetlist += codestr;
  }
}