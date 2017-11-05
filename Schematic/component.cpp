
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "Schematic/component.h"

#include <qdebug.h>

//! [0]
Component::Component(GraphWidget *graphWidget, ComponentType comp, ComponentOrientation Or_, std::map<QString,QString> val, QString ID_)
    : graph(graphWidget)
{
    ID = ID_;
    CompType = comp;
    Orientation = Or_;
    Value = val;
    setFlag(ItemIsMovable, false);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
}

Component::Component(GraphWidget *graphWidget, ComponentInfo CI): graph(graphWidget)
{
    ID=CI.ID;
    CompType = CI.Type;
    Orientation = CI.Orientation;
    Value = CI.val;
    setPos(CI.Coordinates.at(0),CI.Coordinates.at(1));//Coordinates in the schematic window
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
}

void Component::addWire(Wire *Wire)
{
    WireList << Wire;
    Wire->adjust();
}

QList<Wire *> Component::Wires() const
{
    return WireList;
}

// This function returns the bounding box of the component. That is the region where the component can be painted
QRectF Component::boundingRect() const
{
    QRect R;
    switch(CompType)
    {
    case TransmissionLine:
        (Orientation == horizontal) ? R = QRect(-25, -10, 50, 60) : R = QRect(-15, -20, 60, 50);
        break;
    case Resistor:
        (Orientation == horizontal) ? R = QRect(-25, -10, 50, 40) : R = QRect(-15, -20, 60, 50);
        break;
    case Capacitor:
    case Inductor:
        (Orientation == horizontal) ? R = QRect(-25, -10, 50, 40) : R = QRect(-15, -20, 50, 50);
        break;
    case GND:
        R = QRect(-2*7, -2*7, 2*15, 2*10);
        break;
    case Term:
        (Orientation == horizontal) ? R = QRect(0, -10, 40, 40) : R = QRect(-40, -10, 40, 40);
        break;
    }
    return R;
}

// This function gives the area where the component can be selected
QPainterPath Component::shape() const
{
    QPainterPath path;
    switch(CompType)
    {
    case Capacitor:
        path.addRect(-2*7, -2*7, 2*15, 2*15);
        break;
    case TransmissionLine:
    case Resistor:
        (Orientation == horizontal) ? path.addRect(-2*9, -2*5, 2*20, 2*10) : path.addRect(-2*5, -2*9, 2*15, 2*20);
        break;
    case Inductor:
        (Orientation == horizontal) ? path.addRect(-2*9, -2*5, 2*20, 2*10) : path.addRect(-2*5, -2*9, 2*10, 2*20);
        break;
    case GND:
        path.addRect(-2*7, -2*7, 2*15, 2*10);
        break;
    case Term:
        (Orientation == horizontal) ? path.addRect(0, -2*4, 2*5, 2*8) : path.addRect(-2*5, -2*4, 2*5, 2*8);
        break;

    }

    return path;
}

void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(QPen(Qt::darkBlue, 1));
    painter->setFont(QFont("Arial", 6, QFont::Bold));
    switch(CompType)
    {
    case Capacitor:
        if (Orientation == vertical)
        {
            painter->drawLine(QPoint(0, 25), QPoint(0,2*2));
            painter->drawLine(QPoint(0, -25), QPoint(0,-2*2));
            painter->drawLine(QPoint(-2*5, -2*2), QPoint(2*5,-2*2));
            painter->drawLine(QPoint(-2*5, 2*2), QPoint(2*5,2*2));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QRect(2,5,100,100), QString("%1").arg(this->ID));
            painter->drawText(QRect(2,15,100,100), QString("%1").arg(Value["C"]));
        }
        else
        {
            painter->drawLine(QPoint(2*15, 0), QPoint(2*2,0));
            painter->drawLine(QPoint(-2*15, 0), QPoint(-2*2,0));
            painter->drawLine(QPoint(-2*2, -2*5), QPoint(-2*2,2*5));
            painter->drawLine(QPoint(2*2, -2*5), QPoint(2*2,2*5));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QRect(-10,10,100,100),  QString("%1").arg(this->ID));
            painter->drawText(QRect(-10,20,100,100),  QString("%1").arg(Value["C"]));
        }
        break;
    case Inductor:
        if (Orientation == vertical)
        {
            painter->drawLine(QPoint(0, -2*15), QPoint(0, -2*7));
            painter->drawArc(QRect(-2*2, -2*7, 2*5.0, 2*5.0), -90*16, 180*16);
            painter->drawArc(QRect(-2*2, -2*2, 2*5.0, 2*5.0), -90*16, 180*16);
            painter->drawArc(QRect(-2*2, 2*3, 2*5.0, 2*5.0), -90*16, 180*16);
            painter->drawLine(QPoint(0, 16), QPoint(0, 40));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QRect(7,5,100,100), QString("%1").arg(this->ID));
            painter->drawText(QRect(7,15,100,100), QString("%1").arg(Value["L"]));
        }
        else
        {
            painter->drawLine(QPoint(-2*15, 0), QPoint(-2*7, 0));
            painter->drawArc(QRect(-2*7, -2*3, 2*5.0, 2*5.0), 0*16, 180*16);
            painter->drawArc(QRect(-2*2, -2*3, 2*5.0, 2*5.0), 0*16, 180*16);
            painter->drawArc(QRect(2*3, -2*3, 2*5.0, 2*5.0), 0*16, 180*16);
            painter->drawLine(QPoint(2*8, 0), QPoint(2*20, 0));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QRect(-10,0,100,100),  QString("%1").arg(this->ID));
            painter->drawText(QRect(-10,10,100,100),  QString("%1").arg(Value["L"]));

        }
        break;
    case TransmissionLine:

        if (Orientation == vertical)
        {
            int w = 15;
            painter->drawLine(QPoint(0, -30), QPoint(0, -14));
            painter->drawLine(QPoint(-0.5*w, -14), QPoint(0.5*w, -14));
            painter->drawLine(QPoint(-0.5*w, -14), QPoint(0.5*w, -14));
            painter->drawLine(QPoint(-0.5*w, -14), QPoint(-0.5*w, 16));
            painter->drawLine(QPoint(0.5*w, -14), QPoint(0.5*w, 16));
            painter->drawLine(QPoint(-0.5*w, 16), QPoint(0.5*w, 16));
            painter->drawLine(QPoint(0, 16), QPoint(0, 40));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QRect(7,5,100,100), QString("%1").arg(this->ID));
            painter->drawText(QRect(7,15,100,100), QString("%1").arg(Value["Z"].replace("Ohm", QChar(0xa9, 0x03))));
            painter->drawText(QRect(7,15,100,100), QString("%1").arg(Value["L"]));
        }
        else
        {
            int w = 15;
            painter->drawLine(QPoint(-30, 0), QPoint(-14, 0));
            painter->drawLine(QPoint(-14, -0.5*w), QPoint(-14, 0.5*w));
            painter->drawLine(QPoint(16, -0.5*w), QPoint(16, 0.5*w));
            painter->drawLine(QPoint(-14, 0.5*w), QPoint(16, 0.5*w));
            painter->drawLine(QPoint(-14, -0.5*w), QPoint(16, -0.5*w));
            painter->drawLine(QPoint(16, 0), QPoint(40, 0));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QRect(-15,7,100,100), QString("%1").arg(this->ID));
            painter->drawText(QRect(-15,13,100,100), QString("%1").arg(Value["Z"].replace("Ohm", QChar(0xa9, 0x03))));
            painter->drawText(QRect(-15,20,100,100), QString("%1").arg(Value["L"]));
        }
        break;
    case Resistor:
        if (Orientation == vertical)
        {
            int w = 5;
            painter->drawLine(QPoint(0, -30), QPoint(0, -14));

            painter->drawLine(QPoint(0, -14), QPoint(w, -14+2.5));
            painter->drawLine(QPoint(w, -14+2.5), QPoint(-w, -14+7.5));

            painter->drawLine(QPoint(-w, -14+7.5), QPoint(w, -14+12.5));
            painter->drawLine(QPoint(w, -14+12.5), QPoint(-w, -14+17.5));

            painter->drawLine(QPoint(-w, -14+17.5), QPoint(w, -14+22.5));
            painter->drawLine(QPoint(w, -14+22.5), QPoint(-w, -14+27.5));

            painter->drawLine(QPoint(-w, -14+27.5), QPoint(0, 16));

            painter->drawLine(QPoint(0, 16), QPoint(0, 40));
            painter->setPen(QPen(Qt::black, 1));
            painter->drawText(QRect(7,5,100,100), QString("%1").arg(this->ID));
            painter->drawText(QRect(7,15,100,100), QString("%1").arg(Value["R"].replace("Ohm", QChar(0xa9, 0x03))));
        }
        else
        {
            //NOT IMPLEMENTED YET
        }
        break;
    case GND:
            painter->drawLine(QPoint(0, -2*5), QPoint(0,0));
            painter->drawLine(QPoint(-2*5, 0), QPoint(2*5,0));
        break;
    case Term:
        if (Orientation == vertical)
        {
           QPainterPath path;
           path.moveTo (-2*5, -2*3);
           path.lineTo (0, 0);
           path.lineTo (-2*5, 2*3);
           path.lineTo (-2*5, -2*3);
           painter->setPen (Qt :: NoPen);
           painter->fillPath (path, QBrush (QColor ("red")));
           painter->setPen(QPen(Qt::black, 1));
           QString str = QString("%1%2").arg(75).arg(QChar(0xa9, 0x03));
           painter->drawText(QRect(-30,0,100,100), QString("%1").arg(this->ID));
           painter->drawText(QRect(-30,10,100,100), QString("%1").arg(Value["Z"].replace("Ohm", QChar(0xa9, 0x03))));
        }
        else
        {
            QPainterPath path;
            path.moveTo (10, -6);
            path.lineTo (0, 0);
            path.lineTo (10, 6);
            path.lineTo (10, -6);
            painter->setPen (Qt :: NoPen);
            painter->fillPath (path, QBrush (QColor ("red")));
            painter->setPen(QPen(Qt::black, 1));
            QString str = QString("%1%2").arg(75).arg(QChar(0xa9, 0x03));
            painter->drawText(QRect(15,0,100,100), QString("%1").arg(this->ID));
            painter->drawText(QRect(15,10,100,100), QString("%1").arg(Value["Z"].replace("Ohm", QChar(0xa9, 0x03))));
        }
        break;

    }

/*    //Debug code: Shows the bounding box of the component. This is the region where the selection works
   painter->setPen(QPen(Qt::red, 1));
   painter->drawPath(this->shape());//Component box-> This is the area where the component can be selected
   painter->setPen(QPen(Qt::green, 1));
   painter->drawRect(this->boundingRect());//Component bounding box->This is the area where the component can be painted
*/

}

QVariant Component::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (Wire *Wire, WireList)
            Wire->adjust();
        graph->itemMoved();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Component::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void Component::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

void Component::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    struct ComponentInfo CI;
    CI.ID=this->ID;
    CI.Orientation = this->Orientation;
    CI.Type=this->CompType;
    CI.val=this->Value;
    CI.parameter=0;//By default it tunes the 1st tunable parameter of the component. Later I'll see how to handle more than one params
    emit DoubleClicked(CI);
    QGraphicsItem::mouseDoubleClickEvent(event);
}


//This function returns the location of the port specified by port_number. It is used by a wire object so as to know how to connect symbols
QPoint Component::getPortLocation(int port_number)
{
    QPoint P;
    switch (CompType)
    {
       case TransmissionLine:
       case Resistor:
       case Inductor:
       case Capacitor:
        switch (port_number)
        {
        case 1: (Orientation == vertical)? P = QPoint(0, -28) : P = QPoint(25, -8);
            break;
        default:(Orientation == vertical)? P = QPoint(0, 18) : P = QPoint(-25, -8);
        }
        break;
    case GND:
        P = QPoint(0, -18);
        break;
    case Term:
        P = QPoint(0, -8);
    }

    return P;
}


QString Component::getID()
{
    return ID;
}

void Component::setOrientation(ComponentOrientation CO)
{
    Orientation = CO;
}

void Component::setParameters(std::map<QString, QString> val)
{
    Value = val;
}

std::map<QString, QString> Component::getParameters()
{
    return Value;
}

void Component::setComponentType(ComponentType CT)
{
    CompType = CT;
}

ComponentType Component::getComponentType()
{
    return CompType;
}

