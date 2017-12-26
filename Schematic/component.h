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
#include "general.h"

class Wire;

enum ComponentType {Capacitor, Inductor, Term, GND, ConnectionNodes, Resistor, TransmissionLine};
enum ComponentOrientation {vertical, horizontal};

class ComponentInfo
{
public:
    ComponentInfo() : Coordinates(2){};
    ComponentInfo(QString ID_, ComponentType Type_,
                  ComponentOrientation Orientation_,
                  double x, double y, //Coordinates
                  QString N1, QString N2 //Nodes
                  )  :
        ID(ID_), Type(Type_), Orientation(Orientation_),
        Net1(N1), Net2(N2),
        Coordinates(2){
        Coordinates[0] = x;
        Coordinates[1] = y;
    };
    void setParams(QString ID_, ComponentType Type_,
                   ComponentOrientation Orientation_,
                   double x, double y,    //Coordinates
                   QString N1, QString N2 //Nodes
                   )
    {
        ID = ID_; Type = Type_; Orientation = Orientation_;
        Coordinates[0] = x; Coordinates[1] = y;
        Net1 = N1; Net2 = N2;
    };
    QString getQucs(){
        QString code;
        switch (Type)
        {
        case GND:
            return QString("");//Grounds are interpreted as a component in the sense they have a graphical representation,
            //but they have no meaning in terms of the Qucs netlist
        case Capacitor:
            code = "C";
            break;
        case Inductor:
            code = "L";
            break;
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
        }
        code += QString(":%1 %2 %3").arg(ID).arg(Net1).arg(Net2);
        std::map<QString, QString>::iterator it = val.begin();
        while (it != val.end())
        {
            code += QString(" %1=\"%2\"").arg(it->first).arg(it->second);
            it++;
        }
        code += "\n";
        return code;
    };
    QString ID;
    unsigned int parameter;
    std::map<QString,QString> val;//freq, L1.L, C1.C,...
    QString Net1, Net2;//ID of the nodes where the component is connected
    ComponentOrientation Orientation;
    std::vector<double> Coordinates;
    ComponentType Type;
    double getVal(QString);
};

class WireInfo
{
public:
    WireInfo(){};
    WireInfo(QString O, int OP, QString D, int DP) :
        OriginID(O), PortOrigin(OP),
        DestinationID(D), PortDestination(DP){};
    void setParams(QString O, int OP, QString D, int DP)
    {
        OriginID = O, DestinationID = D;
        PortOrigin = OP, PortDestination = DP;
    };
    QString OriginID;
    QString DestinationID;
    int PortOrigin;
    int PortDestination;
};

class NodeInfo
{
public:
    NodeInfo(): Coordinates(2){};
    NodeInfo(QString ID_, double x, double y) :
        ID(ID_), Coordinates(2){
        Coordinates[0] = x;
        Coordinates[1] = y;
    };
    void setParams(QString ID_, double x, double y)
    {
        ID = ID_;
        Coordinates[0] = x; Coordinates[1] = y;
    };
    QString ID;
    std::vector<double> Coordinates;
};


class Component : public Symbol
{
    Q_OBJECT
public:
    Component(GraphWidget *graphWidget, ComponentType, ComponentOrientation, std::map<QString, QString>, QString ID);
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
    void setParameters(std::map<QString,QString>);
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
    ComponentOrientation Orientation;
    std::map<QString, QString> Value;
    QString ID;
signals:
    void DoubleClicked(struct ComponentInfo);



};
//! [0]

#endif
