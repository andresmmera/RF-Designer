/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QPainter>

#include "symbol.h"
#include "wire.h"

#include <math.h>

static const double Pi = 3.14159265358979323846264338327950288419717;

Wire::Wire(Symbol *sourceNode, int ps, Symbol *destNode, int pd)
    : arrowSize(10) {
  setAcceptedMouseButtons(0);
  source = sourceNode;
  dest = destNode;
  port_num_source = ps;
  port_num_dest = pd;
  source->addWire(this);
  dest->addWire(this);
  WireColor = Qt::black;
  adjust();
}

Wire::Wire() : arrowSize(10) { setAcceptedMouseButtons(0); }

void Wire::setSource(Symbol *sourceNode, int port) {
  source = sourceNode;
  port_num_source = port;
}

void Wire::setDestination(Symbol *destNode, int port) {
  dest = destNode;
  port_num_dest = port;
}

void Wire::setColor(QColor Color) { WireColor = Color; }

void Wire::paintWire() {
  source->addWire(this);
  dest->addWire(this);
  adjust();
}

Symbol *Wire::sourceNode() const { return source; }

Symbol *Wire::destNode() const { return dest; }

void Wire::adjust() {
  if (!source || !dest)
    return;

  QPoint PortSource = source->getPortLocation(port_num_source);
  QPoint PortDest = dest->getPortLocation(port_num_dest);
  QLineF line(mapFromItem(source, PortSource.x(), PortSource.y()),
              mapFromItem(dest, PortDest.x(), PortDest.y()));

  prepareGeometryChange();

  sourcePoint = line.p1();
  destPoint = line.p2();
}

QRectF Wire::boundingRect() const {
  return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                    destPoint.y() - sourcePoint.y()))
      .normalized();
}

void Wire::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                 QWidget *) {
  if (!source || !dest)
    return;
  /*  painter->setPen(QPen(Qt::red, 1));
    painter->drawRect(this->boundingRect());*/

  QLineF lineV(sourcePoint, QPoint(sourcePoint.x(), destPoint.y()));
  QLineF lineH(QPoint(sourcePoint.x(), destPoint.y()), destPoint);

  // Draw the line itself
  painter->setPen(
      QPen(WireColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  if (sourcePoint.x() != destPoint.x())
    painter->drawLine(lineH);
  if (sourcePoint.y() != destPoint.y())
    painter->drawLine(lineV);
}
