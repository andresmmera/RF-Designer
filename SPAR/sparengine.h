/* 
 * sparengine.h - S parameter engine class definition
 *
 * copyright (C) 2016 Andres Martinez-Mera <andresmartinezmera@gmail.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *
 */


#ifndef SPARENGINE_H
#define SPARENGINE_H

#include <string.h>
#include <set>
#include "MathOperations.h"
#include "Filtering/Network.h"

using namespace std;



class SparEngine
{
    static const double c0 = 299792458;//Speed of light (m/s)
public:
    SparEngine();
    Mat getSparams(QList<ComponentInfo>, complex<double>, complex<double>, double, QStringList);
    Mat getABCDmatrix(QList<ComponentInfo>, double, QStringList);
    void setNetwork(NetworkInfo);
    void setSimulationSettings(SP_Analysis);
    QMap<QString, vector<complex<double> > > getData();
    void run();
    vector<complex<double> > getSij(int, int);
    vector<double> getFreq();
private:
    NetworkInfo NI;
    SP_Analysis sim_settings;
    vector<complex<double> > S11, S22, S21, S12;
};

#endif // SPARENGINE_H
