/***************************************************************************
                                EdgeCoupled.h
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
#ifndef EDGECOUPLED_H
#define EDGECOUPLED_H
#include "Filtering/Network.h"
#include "general.h"
#include <QPen>

class EdgeCoupled {
public:
  EdgeCoupled();
  QMap<QString, QPen> displaygraphs;
};

#endif // EDGECOUPLED_H
