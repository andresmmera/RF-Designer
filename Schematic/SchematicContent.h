/***************************************************************************
                                SchematicContent.h
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

#ifndef SCHEMATICCONTENT_H
#define SCHEMATICCONTENT_H
#include "Filtering/Network.h"
#include "Schematic/component.h"

class SchematicContent {
public:
  SchematicContent();
  struct NetworkInfo
  getLadder(); // Returns a ladder code for using the internal
               // simulator. It's faster than using Qucs, but it is
               // restricted to ladder networks
  QString getQucsNetlist();

public:
  // Setter getter functions

  // Components, wires and nodes
  void appendComponent(struct ComponentInfo);
  void appendWire(struct WireInfo);
  void appendWire(QString, int, QString, int);
  void appendWire(QString, int, QString, int, QColor);
  void appendNode(struct NodeInfo);

  double getZin();
  double getZout();
  QString getZinString();
  QString getZoutString();

  QList<struct ComponentInfo> getComponents();
  QList<struct WireInfo> getWires();
  QList<struct NodeInfo> getNodes();

  // Description
  void setDescription(QString);
  QString getDescription();

  // Display graphs
  void appendGraph(QString, QPen);
  void clearGraphs();
  void setDisplayGraphs(QMap<QString, QPen>);
  QMap<QString, QPen> getDisplayGraphs();
  QMap<ComponentType, int>
      NumberComponents; // List for assigning IDs to the filter components

  // Frequency sweep
  struct SP_Analysis getSPAR_Sweep();
  void setSPAR_Sweep(struct SP_Analysis);

private:
  struct SP_Analysis SPAR_Settings;
  QList<struct ComponentInfo> Comps;
  QList<struct WireInfo> Wires;
  QList<struct NodeInfo> Nodes;
  QMap<QString, QPen> displayGraphs;
  QString Description;
  QString netlist;
};

#endif // SCHEMATICCONTENT_H
