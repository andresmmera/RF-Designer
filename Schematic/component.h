/***************************************************************************
                                component.h
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
#ifndef COMPONENT_H
#define COMPONENT_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QList>
#include <cmath>

#include "Filtering/Network.h"
#include "Schematic/graphwidget.h"
#include "Schematic/symbol.h"
#include "Schematic/wire.h"
#include "general.h"

class Wire;

enum ComponentType {
  Capacitor,
  Inductor,
  Term,
  GND,
  ConnectionNodes,
  Resistor,
  TransmissionLine,
  OpenStub,
  ShortStub,
  CoupledLines
};

class ComponentInfo {
public:
  ComponentInfo() : Coordinates(2){};
  ComponentInfo(QString ID_, ComponentType Type_, double rot_, double x,
                double y,              // Coordinates
                QString N1, QString N2 // Nodes
                )
      : ID(ID_), Type(Type_), Rotation(rot_), Net1(N1), Net2(N2),
        Coordinates(2) {
    Coordinates[0] = x;
    Coordinates[1] = y;
    Connections.append(N1);
    Connections.append(N2);
  };

  QString ID;
  QStringList Connections;
  ComponentType Type;
  double Rotation;
  QString Net1, Net2; // ID of the nodes where the component is connected
  std::vector<double> Coordinates;

  void setParams(QString ID_, ComponentType Type_, double Rotation_, double x,
                 double y,              // Coordinates
                 QString N1, QString N2 // Nodes
  ) {
    ID = ID_;
    Type = Type_;
    Rotation = Rotation_;
    Coordinates[0] = x;
    Coordinates[1] = y;
    Connections.append(N1);
    Connections.append(N2);
  };

  void setParams(QString ID_, ComponentType Type_, double Rotation_, double x,
                 double y,         // Coordinates
                 QStringList Nodes // Nodes
  ) {
    ID = ID_;
    Type = Type_;
    Rotation = Rotation_;
    Coordinates[0] = x;
    Coordinates[1] = y;
    Connections = Nodes;
  };

  QString getQucs() {
    QString code;
    switch (Type) {
    case GND:
      return QString(""); // Grounds are interpreted as a component in the sense
                          // they have a graphical representation,
      // but they have no meaning in terms of the Qucs netlist
    case Capacitor:
      code = "C";
      break;
    case Inductor:
      code = "L";
      break;
    case CoupledLines:
      code = "CTLIN";
      break;
    case OpenStub:
    case ShortStub:
    case TransmissionLine:
      code = "TLIN";
      break;
    case Term:
      code = "Pac";
      val["Num"] = QString(ID).remove("T");
      val["f"] = "1 GHz";
      break;
    case Resistor:
      code = "R";
      break;
    default:
      break;
    }
    code += QString(":%1").arg(ID);
    for (int i = 0; i < Connections.size(); i++)
      code += QString(" %1").arg(Connections[i]);
    std::map<QString, QString>::iterator it = val.begin();
    QString prop; // Temporal variable to translate the internal property names
                  // to Qucs property names
    while (it != val.end()) {
      switch (Type) {
      case OpenStub:
      case ShortStub:
      case TransmissionLine:
      case CoupledLines:
        prop = it->first;
        if (prop == "Length")
          prop = "L";
        if (prop == "Z0")
          prop = "Z";
        code += QString(" %1=\"%2\"").arg(prop).arg(it->second);
        break;

      case Term:
        prop = it->first;
        if (prop == "Z0")
          prop = "Z";
        code += QString(" %1=\"%2\"").arg(prop).arg(it->second);
        break;
      default:
        code += QString(" %1=\"%2\"").arg(it->first).arg(it->second);
      }
      it++;
    }
    code += "\n";
    return code;
  };

  unsigned int parameter;
  std::map<QString, QString> val; // freq, L1.L, C1.C,...

  double getVal(QString);
};

class WireInfo {
public:
  WireInfo(){};
  WireInfo(QString O, int OP, QString D, int DP)
      : OriginID(O), PortOrigin(OP), DestinationID(D), PortDestination(DP){};
  void setParams(QString O, int OP, QString D, int DP) {
    OriginID = O, DestinationID = D;
    PortOrigin = OP, PortDestination = DP;
    WireColor = Qt::black;
  };
  void setParams(QString O, int OP, QString D, int DP, QColor Color) {
    OriginID = O, DestinationID = D;
    PortOrigin = OP, PortDestination = DP;
    WireColor = Color;
  };
  QString OriginID;
  int PortOrigin;
  QString DestinationID;
  int PortDestination;
  QColor WireColor;
};

class NodeInfo {
public:
  NodeInfo() : Coordinates(2){};
  NodeInfo(QString ID_, double x, double y) : ID(ID_), Coordinates(2) {
    Coordinates[0] = x;
    Coordinates[1] = y;
  };
  void setParams(QString ID_, double x, double y) {
    ID = ID_;
    Coordinates[0] = x;
    Coordinates[1] = y;
  };
  QString ID;
  std::vector<double> Coordinates;
};

class Component : public Symbol {
  Q_OBJECT
public:
  Component(GraphWidget *graphWidget, ComponentType, double,
            std::map<QString, QString>, QString ID);
  Component(GraphWidget *graphWidget, struct ComponentInfo);
  void addWire(Wire *Wire);
  QList<Wire *> Wires() const;

  enum { Type = UserType + 1 };
  int type() const { return Type; }

  QRectF boundingRect() const;
  QPainterPath shape() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget);
  QPoint getPortLocation(int);
  QString getID();
  void setRotation(double);
  void setParameters(std::map<QString, QString>);
  std::map<QString, QString> getParameters();
  void setComponentType(ComponentType);
  ComponentType getComponentType();

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);

  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
  QList<Wire *> WireList;
  QPointF newPos;
  GraphWidget *graph;
  ComponentType CompType;
  double Rotation;
  std::map<QString, QString> Value;
  QString ID;

  void RotatePoint(QPoint &);
  void RotatePoint(QPoint &, double);

  // Component symbol code
  void paintCapacitor(QPainter *);
  void paintInductor(QPainter *);
  void paintTransmissionLine(QPainter *);
  void paintResistor(QPainter *);
  void paintTerm(QPainter *);
  void paintGND(QPainter *);
  void paintOpenStub(QPainter *);
  void paintShortStub(QPainter *);
  void paintCoupledLines(QPainter *);

signals:
  void DoubleClicked(struct ComponentInfo);
};
//! [0]

#endif
