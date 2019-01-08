
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "Schematic/component.h"

#include <qdebug.h>

Component::Component(GraphWidget *graphWidget, ComponentType comp, double Rot_,
                     std::map<QString, QString> val, QString ID_)
    : graph(graphWidget) {
  ID = ID_;
  CompType = comp;
  Rotation = Rot_;
  Value = val;
  setFlag(ItemIsMovable, false);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
}

Component::Component(GraphWidget *graphWidget, ComponentInfo CI)
    : graph(graphWidget) {
  ID = CI.ID;
  CompType = CI.Type;
  Rotation = CI.Rotation;
  Value = CI.val;
  setPos(CI.Coordinates.at(0),
         CI.Coordinates.at(1)); // Coordinates in the schematic window
  setFlag(ItemIsMovable);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
}

void Component::addWire(Wire *Wire) {
  WireList << Wire;
  Wire->adjust();
}

QList<Wire *> Component::Wires() const { return WireList; }

// This function returns the bounding box of the component. That is the region
// where the component can be painted
QRectF Component::boundingRect() const {
  QRect R;
  switch (CompType) {
  case OpenStub:
  case ShortStub:
  case TransmissionLine:
    R = QRect(-40, -40, 80, 80);
    break;
  case Resistor:
    R = QRect(-30, -30, 60, 60);
    break;
  case Capacitor:
  case Inductor:
    R = QRect(-40, -40, 80, 80);
    break;
  case GND:
    R = QRect(-2 * 7, -2 * 7, 2 * 15, 2 * 10);
    break;
  case Term:
    R = QRect(-25, -25, 50, 50);
    break;
  default:
    break;
  }
  return R;
}

// This function gives the area where the component can be selected
QPainterPath Component::shape() const {
  QPainterPath path;
  switch (CompType) {
  case Capacitor:
    path.addRect(-15, -15, 30, 30);
    break;
  case OpenStub:
  case ShortStub:
  case TransmissionLine:
  case Resistor:
    path.addRect(-30, -30, 60, 60);
    break;
  case Inductor:
    path.addRect(-20, -20, 40, 40);
    break;
  case GND:
    path.addRect(-2 * 7, -2 * 7, 2 * 15, 2 * 10);
    break;
  case Term:
    path.addRect(-15, -15, 30, 30);
    break;
  default:
    break;
  }

  return path;
}

void Component::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                      QWidget *) {
  painter->setPen(QPen(Qt::darkBlue, 1));
  painter->setFont(QFont("Arial", 6, QFont::Bold));
  switch (CompType) {
  case Capacitor:
    paintCapacitor(painter);
    break;
  case Inductor:
    paintInductor(painter);
    break;
  case TransmissionLine:
    paintTransmissionLine(painter);
    break;
  case OpenStub:
    paintOpenStub(painter);
    break;
  case ShortStub:
    paintShortStub(painter);
    break;
  case Resistor:
    paintResistor(painter);
    break;
  case GND:
    paintGND(painter);
    break;
  case Term:
    paintTerm(painter);
    break;
  default:
    break;
  }

  /*
  painter->setPen(QPen(Qt::red, 1));
  painter->drawPoint(QPoint(0, 0));

  // Debug code: Shows the bounding box of the component. This is the
  // region where the selection works painter->setPen(QPen(Qt::red, 1));
  painter->drawPath(
      this->shape()); // Component box-> This is the area where the
  // component can be selected painter->setPen(QPen(Qt::green, 1));
  painter->drawRect(
      this->boundingRect()); // Component bounding box->This is the
                             // area where the component can be painted
                             */
}

QVariant Component::itemChange(GraphicsItemChange change,
                               const QVariant &value) {
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

void Component::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mousePressEvent(event);
}

void Component::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mouseReleaseEvent(event);
}

void Component::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  update();
  struct ComponentInfo CI;
  CI.ID = this->ID;
  CI.Rotation = this->Rotation;
  CI.Type = this->CompType;
  CI.val = this->Value;
  emit DoubleClicked(CI);
  QGraphicsItem::mouseDoubleClickEvent(event);
}

// This function returns the location of the port specified by port_number. It
// is used by a wire object so as to know how to connect symbols
QPoint Component::getPortLocation(int port_number) {
  QPoint P;
  switch (CompType) {
  case OpenStub:
  case ShortStub:
  case TransmissionLine:
  case Resistor:
  case Inductor:
    switch (port_number) {
    case 1:
      P = QPoint(0, -25);
      break;
    case 0:
    default:
      P = QPoint(0, 25);
    }
    break;
  case Capacitor:
    switch (port_number) {
    case 1:
      P = QPoint(0, -20);
      break;
    case 0:
    default:
      P = QPoint(0, 20);
    }
    break;
  case GND:
    P = QPoint(0, -11);
    break;
  case Term:
    P = QPoint(0, 0);
    break;
  default:
    break;
  }
  RotatePoint(P);
  return P;
}

// This function rotates the port position P about the object centroid (0,0)
// according to the component rotation
void Component::RotatePoint(QPoint &P) {
  double r = (M_PI / 180) * Rotation;
  double x_rotated = P.x() * cos(r) - P.y() * sin(r);
  double y_rotated = P.x() * sin(r) + P.y() * cos(r);
  P.setX(x_rotated);
  P.setY(y_rotated);
}

// This function rotates the port position P about the object centroid (0,0)
// according to the angle in the function arguments
void Component::RotatePoint(QPoint &P, double angle) {
  double r = (M_PI / 180) * angle;
  double x_rotated = P.x() * cos(r) - P.y() * sin(r);
  double y_rotated = P.x() * sin(r) + P.y() * cos(r);
  P.setX(x_rotated);
  P.setY(y_rotated);
}

QString Component::getID() { return ID; }

void Component::setRotation(double R) { Rotation = R; }

void Component::setParameters(std::map<QString, QString> val) { Value = val; }

std::map<QString, QString> Component::getParameters() { return Value; }

void Component::setComponentType(ComponentType CT) { CompType = CT; }

ComponentType Component::getComponentType() { return CompType; }

// Given the property name, this function returns its value in coplex format
double ComponentInfo::getVal(QString Property) {
  QString val_ = this->val[Property];
  QString suffix;
  val_.remove(" "); // Remove blank spaces (if exists)
  double scale = 1;
  int index = 1;
  // Find the suffix
  // Examine each character until finding the first letter, then determine the
  // scale factor
  for (int i = 0; i < val_.length(); i++) {
    if (val_.at(i).isLetter()) {
      index = i;
      suffix = val_.at(i);
      break;
    }
  }

  if (suffix == "f")
    scale = 1e-15;
  else {
    if (suffix == "p")
      scale = 1e-12;
    else {
      if (suffix == "n")
        scale = 1e-9;
      else {
        if (suffix == "u")
          scale = 1e-6;
        else {
          if (suffix == "m")
            scale = 1e-3;
          else {
            if (suffix == "K")
              scale = 1e3;
            else {
              if (suffix == "M")
                scale = 1e6;
              else {
                if (suffix == "G")
                  scale = 1e9;
                else {
                  if (suffix == "T")
                    scale = 1e12;
                }
              }
            }
          }
        }
      }
    }
  }

  // Remove the suffix from the string and convert the property to numerical
  // format
  QString val = val_.left(index);

  // Now, find out if the number is complex or real
  /*if (index = val.indexOf("j"))
  {//Need to separate the real from the imaginary part
      double sign = 1;
      if (val[index-1] == '-') sign = -1;
      double realpart = val.left(index-1).toDouble();//Notice  we have to take
  into account the sign double imagpart = val.right(index).toDouble(); return
  std::complex<double>(realpart, sign*imagpart);
  }
  else
  {*/
  return val.toDouble() * scale;
  //}
}

// This function draws the capacitor with the rotation specified by the Rotation
// field.
void Component::paintCapacitor(QPainter *painter) {

  if (Rotation != 0) {
    painter->rotate(Rotation);
  }

  // The following lines draw the capacitor in a vertical position
  // Lines from ports to parallel plates
  painter->drawLine(QPoint(0, 20), QPoint(0, 4));
  painter->drawLine(QPoint(0, -20), QPoint(0, -4));

  // Parallel plates
  painter->drawLine(QPoint(-10, -4), QPoint(10, -4));
  painter->drawLine(QPoint(-10, 4), QPoint(10, 4));
  painter->setPen(QPen(Qt::black, 1));

  if (Rotation != 0) { // The rotation is undone to draw the text
    painter->rotate(-Rotation);
  }

  QPoint OriginText(2, 5);
  if (Rotation != 0)
    OriginText.setX(-10), OriginText.setY(10);

  painter->drawText(QRect(OriginText, QPoint(100, 100)),
                    QString("%1").arg(this->ID));
  painter->drawText(QRect(OriginText + QPoint(0, 10), QPoint(100, 100)),
                    QString("%1").arg(Value["C"]));
}

// This function draws the inductor with the rotation specified by the Rotation
// field.
void Component::paintInductor(QPainter *painter) {
  if (Rotation != 0) {
    painter->rotate(Rotation);
  }

  // The following lines draw the capacitor in a vertical position
  painter->drawLine(QPoint(0, -25), QPoint(0, -2 * 7));
  painter->drawArc(QRect(-2 * 2, -2 * 7, 2 * 5.0, 2 * 5.0), -90 * 16, 180 * 16);
  painter->drawArc(QRect(-2 * 2, -2 * 2, 2 * 5.0, 2 * 5.0), -90 * 16, 180 * 16);
  painter->drawArc(QRect(-2 * 2, 2 * 3, 2 * 5.0, 2 * 5.0), -90 * 16, 180 * 16);
  painter->drawLine(QPoint(0, 16), QPoint(0, 25));
  painter->setPen(QPen(Qt::black, 1));

  if (Rotation != 0) { // The rotation is undone to draw the text
    painter->rotate(-Rotation);
  }

  QPoint OriginText(10, -10);
  if (Rotation != 0)
    OriginText.setX(-10), OriginText.setY(5);

  painter->drawText(QRect(OriginText, QPoint(100, 100)),
                    QString("%1").arg(this->ID));
  painter->drawText(QRect(OriginText + QPoint(0, +10), QPoint(100, 100)),
                    QString("%1").arg(Value["L"]));
}

void Component::paintGND(QPainter *painter) {
  if (Rotation != 0) {
    painter->rotate(Rotation);
  }

  painter->drawLine(QPoint(0, -10), QPoint(0, 0));
  painter->drawLine(QPoint(-10, 0), QPoint(10, 0));

  if (Rotation != 0) { // The rotation is undone to draw the text
    painter->rotate(-Rotation);
  }
  painter->drawText(QRect(7, 5, 100, 100), QString("%1").arg(this->ID));
  painter->drawText(QRect(7, 15, 100, 100), QString("%1").arg(Value["L"]));
}

void Component::paintTransmissionLine(QPainter *painter) {

  if (Rotation != 0) {
    painter->rotate(Rotation);
  }

  int w = 15;
  painter->drawLine(QPoint(0, -25), QPoint(0, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(0.5 * w, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(0.5 * w, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(-0.5 * w, 16));
  painter->drawLine(QPoint(0.5 * w, -14), QPoint(0.5 * w, 16));
  painter->drawLine(QPoint(-0.5 * w, 16), QPoint(0.5 * w, 16));
  painter->drawLine(QPoint(0, 16), QPoint(0, 25));
  painter->setPen(QPen(Qt::black, 1));

  if (Rotation != 0) { // The rotation is undone to draw the text
    painter->rotate(-Rotation);
  }

  QPoint OriginText(10, -10);
  if (Rotation != 0)
    OriginText.setX(-10), OriginText.setY(10);

  painter->drawText(QRect(OriginText, QPoint(100, 100)),
                    QString("%1").arg(this->ID));
  painter->drawText(
      QRect(OriginText + QPoint(0, 10), QPoint(100, 100)),
      QString("%1").arg(Value["Z0"].replace("Ohm", QChar(0xa9, 0x03))));
  painter->drawText(QRect(OriginText + QPoint(0, 20), QPoint(100, 100)),
                    QString("%1").arg(Value["Length"]));
}

void Component::paintResistor(QPainter *painter) {

  if (Rotation != 0) {
    painter->rotate(Rotation);
  }

  int w = 5;
  painter->drawLine(QPoint(0, -25), QPoint(0, -14));
  painter->drawLine(QPoint(0, -14), QPoint(w, -14 + 2.5));
  painter->drawLine(QPoint(w, -14 + 2.5), QPoint(-w, -14 + 7.5));
  painter->drawLine(QPoint(-w, -14 + 7.5), QPoint(w, -14 + 12.5));
  painter->drawLine(QPoint(w, -14 + 12.5), QPoint(-w, -14 + 17.5));
  painter->drawLine(QPoint(-w, -14 + 17.5), QPoint(w, -14 + 22.5));
  painter->drawLine(QPoint(w, -14 + 22.5), QPoint(-w, -14 + 27.5));
  painter->drawLine(QPoint(-w, -14 + 27.5), QPoint(0, 16));
  painter->drawLine(QPoint(0, 16), QPoint(0, 25));

  if (Rotation != 0) {
    painter->rotate(-Rotation);
  }

  QPoint OriginText(10, -10);
  if (Rotation != 0)
    OriginText.setX(-10), OriginText.setY(10);

  painter->setPen(QPen(Qt::black, 1));
  painter->drawText(QRect(OriginText, QPoint(100, 100)),
                    QString("%1").arg(this->ID));
  painter->drawText(
      QRect(OriginText + QPoint(0, 10), QPoint(100, 100)),
      QString("%1").arg(Value["R"].replace("Ohm", QChar(0xa9, 0x03))));
}

void Component::paintTerm(QPainter *painter) {
  if (Rotation != 0) {
    painter->rotate(Rotation);
  }

  QPainterPath path;
  path.moveTo(10, -6);
  path.lineTo(0, 0);
  path.lineTo(10, 6);
  path.lineTo(10, -6);
  painter->setPen(Qt ::NoPen);
  painter->fillPath(path, QBrush(QColor("red")));
  painter->setPen(QPen(Qt::black, 1));

  if (Rotation != 0) { // The rotation is undone to draw the text
    painter->rotate(-Rotation);
  }

  QString str = QString("%1%2").arg(75).arg(QChar(0xa9, 0x03));
  painter->drawText(QRect(-5, 0, 100, 100), QString("%1").arg(this->ID));
  painter->drawText(
      QRect(-5, 10, 100, 100),
      QString("%1").arg(Value["Z"].replace("Ohm", QChar(0xa9, 0x03))));
}

void Component::paintOpenStub(QPainter *painter) {
  if (Rotation != 0) {
    painter->rotate(Rotation);
  }
  int w = 15;
  painter->drawLine(QPoint(0, -30), QPoint(0, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(0.5 * w, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(0.5 * w, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(-0.5 * w, 16));
  painter->drawLine(QPoint(0.5 * w, -14), QPoint(0.5 * w, 16));
  painter->drawLine(QPoint(-0.5 * w, 16), QPoint(0.5 * w, 16));
  painter->setPen(QPen(Qt::black, 1));

  if (Rotation != 0) { // The rotation is undone to draw the text
    painter->rotate(-Rotation);
  }

  QPoint OriginText(10, -10);
  if (Rotation != 0)
    OriginText.setX(-10), OriginText.setY(10);

  painter->drawText(QRect(OriginText, QPoint(100, 100)),
                    QString("%1").arg(this->ID));
  painter->drawText(
      QRect(OriginText + QPoint(0, 10), QPoint(100, 100)),
      QString("%1").arg(Value["Z0"].replace("Ohm", QChar(0xa9, 0x03))));
  painter->drawText(QRect(OriginText + QPoint(0, 20), QPoint(100, 100)),
                    QString("%1").arg(Value["Length"]));
}

void Component::paintShortStub(QPainter *painter) {

  if (Rotation != 0) {
    painter->rotate(Rotation);
  }

  int w = 15;
  painter->drawLine(QPoint(0, -25), QPoint(0, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(0.5 * w, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(0.5 * w, -14));
  painter->drawLine(QPoint(-0.5 * w, -14), QPoint(-0.5 * w, 16));
  painter->drawLine(QPoint(0.5 * w, -14), QPoint(0.5 * w, 16));
  painter->drawLine(QPoint(-0.5 * w, 16), QPoint(0.5 * w, 16));
  painter->drawLine(QPoint(0, 16), QPoint(0, 25));
  painter->drawLine(QPoint(-0.5 * w, 25), QPoint(0.5 * w, 25));
  painter->setPen(QPen(Qt::black, 1));

  if (Rotation != 0) { // The rotation is undone to draw the text
    painter->rotate(-Rotation);
  }

  QPoint OriginText(10, -10);
  if (Rotation != 0)
    OriginText.setX(-10), OriginText.setY(10);

  painter->drawText(QRect(OriginText, QPoint(100, 100)),
                    QString("%1").arg(this->ID));
  painter->drawText(
      QRect(OriginText + QPoint(0, 10), QPoint(100, 100)),
      QString("%1").arg(Value["Z0"].replace("Ohm", QChar(0xa9, 0x03))));
  painter->drawText(QRect(OriginText + QPoint(0, 20), QPoint(100, 100)),
                    QString("%1").arg(Value["Length"]));
}
