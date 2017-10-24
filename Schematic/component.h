#ifndef COMPONENT_H
#define COMPONENT_H

#include <QGraphicsItem>
#include <QList>
#include <cmath>
#include <QGraphicsView>

#include "Schematic/symbol.h"
#include "Schematic/wire.h"
#include "Schematic/graphwidget.h"
#include "Filtering/Network.h"

class Wire;

class Component : public Symbol
{
Q_OBJECT
public:
    Component(GraphWidget *graphWidget, ComponentType, ComponentOrientation, std::map<QString, double>, QString ID);
    Component(GraphWidget *graphWidget, struct ComponentInfo);
    void addWire(Wire *Wire);
    QList<Wire *> Wires() const;

    enum { Type = UserType + 1 };
    int type() const { return Type; }


    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPoint getPortLocation(int);
    QString getID();
    void setOrientation(ComponentOrientation);
    void setParameters(std::map<QString,double>);
    std::map<QString, double> getParameters();
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
    ComponentOrientation Orientation;
    std::map<QString, double> Value;
    QString ID;
    QString num2str(double);
    QString RoundVariablePrecision(double);
signals:
    void DoubleClicked(struct ComponentInfo);



};
//! [0]

#endif
