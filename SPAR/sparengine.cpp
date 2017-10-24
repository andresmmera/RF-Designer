/*
 * sparengine.cpp - S parameter engine class implementation
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

#include "sparengine.h"

SparEngine::SparEngine()
{
}


// Returns the S matrix at a given frequency for the specified input and load impedances
Mat SparEngine::getSparams(QList<ComponentInfo> x, complex<double> zs, complex<double> zl, double f, QStringList topology)
{
    Mat ABCD = getABCDmatrix(x, f, topology);
    Mat S;
    //Convert ABCD to S parameters
    S(0,0) = (ABCD(0,0)*zl+ABCD(0,1)-ABCD(1,0)*conj(zs)*zl-ABCD(1,1)*conj(zs))/(ABCD(0,0)*zl+ABCD(0,1)+ABCD(1,0)*zs*zl+ABCD(1,1)*zs);
    S(0,1) = (2.*(ABCD(0,0)*ABCD(1,1)-ABCD(0,1)*ABCD(1,0))*sqrt(real(zs)*real(zl)))/(ABCD(0,0)*zl+ABCD(0,1)+ABCD(1,0)*zs*zl+ABCD(1,1)*zs);
    S(1,0) = (2.*sqrt(real(zs)*real(zl)))/(ABCD(0,0)*zl+ABCD(0,1)+ABCD(1,0)*zs*zl+ABCD(1,1)*zs);
    S(1,1) = (-ABCD(0,0)*conj(zl)+ABCD(0,1)-ABCD(1,0)*conj(zl)*zs+ABCD(1,1)*zs)/(ABCD(0,0)*zl+ABCD(0,1)+ABCD(1,0)*zs*zl+ABCD(1,1)*zs);

   return S;
}


// Returns the ABCD matrix at a given frequency
Mat SparEngine::getABCDmatrix(QList<ComponentInfo> x, double f, QStringList topology)
{
    int index;
    double w = 2*pi*f;
    double beta = w/c0;
    complex<double> gamma = complex<double>(0, beta);
    complex<double> Z, Y, num, den;

    QStringList options;//Available network elements
    options.append("LS");
    options.append("CS");
    options.append("LP");
    options.append("CP");
    options.append("TL");
    options.append("OS");
    options.append("SS");
    options.append("MSerR");
    options.append("MShuR");

    Mat ABCD, ABCD_t;

    ABCD.eye();

    for (int i = 0, k=0; i < topology.length(); i++, k++)
    {
        index = options.indexOf(topology.at(i));
        switch(index)
        {
        case 0: //Series inductance
                ABCD_t(0,0) = 1.;
                ABCD_t(0,1) = complex<double>(0,w*x.at(i).val.at(0));
                ABCD_t(1,0) = 0;
                ABCD_t(1,1) = 1.;
            break;
        case 1: //Series capacitor
                ABCD_t(0,0) = 1.;
                ABCD_t(0,1) = complex<double>(0,-1/(w*x.at(i).val.at(0)));
                ABCD_t(1,0) = 0;
                ABCD_t(1,1) = 1.;
            break;
        case 2: //Parallel inductance
                ABCD_t(0,0) = 1.;
                ABCD_t(0,1) = 0;
                ABCD_t(1,0) = complex<double>(0,-1./(w*x.at(i).val.at(0)));
                ABCD_t(1,1) = 1.;
            break;
        case 3://Parallel capacitor
                ABCD_t(0,0) = 1.;
                ABCD_t(0,1) = 0;
                ABCD_t(1,0) = complex<double>(0, w*x.at(i).val.at(0));
                ABCD_t(1,1) = 1.;
            break;
        case 4://Transmission line
                /*ABCD_t(0,0) = cosh(gamma*x.at(k+1));
                ABCD_t(0,1) = x.at(k)*sinh(gamma*x.at(k+1));
                ABCD_t(1,0) = sinh(gamma*x.at(k+1))/x.at(k);
                ABCD_t(1,1) = cosh(gamma*x.at(k+1));
            k++;//It involves two parameters, so we need to skip the next index
            break;*/
        case 5:
               /* ABCD_t(0,0) = 1.;
                ABCD_t(0,1) = 0;
                ABCD_t(1,0) = (tanh(gamma*x.at(k+1)))/x.at(k);
                ABCD_t(1,1) = cosh(gamma*x.at(k+1));
            k++;
            break;*/
        case 6:
               /* ABCD_t(0,0) = 1.;
                ABCD_t(0,1) = 0;
                ABCD_t(1,0) = 1./(x.at(k)*tanh(gamma*x.at(k+1)));
                ABCD_t(1,1) = 1;
            k++;*/
            break;
        default: ABCD.eye(); 
                 return ABCD;
        }

        ABCD = ABCD*ABCD_t;
    }
        return ABCD;
}


void SparEngine::setNetwork(NetworkInfo nwi)
{
    NI = nwi;
}


void SparEngine::setSimulationSettings(SP_Analysis spa)
{
    sim_settings = spa;
    sim_settings.freq = linspace(spa.fstart, spa.fstop, spa.n_points);
}

void SparEngine::run()
{
    if(NI.ZS.size() == 1)
    {
        NI.ZS = NI.ZS*ones(sim_settings.n_points);
    }
    if(NI.ZL.size() == 1)
    {
        NI.ZL = NI.ZL*ones(sim_settings.n_points);
    }

    for (unsigned int i = 0; i < sim_settings.freq.size(); i++)
    {
       Mat S = getSparams(NI.Ladder, NI.ZS.at(i), NI.ZL.at(i), sim_settings.freq.at(i), NI.topology);
       S11.push_back(S(0,0));
       S21.push_back(S(1,0));
       S22.push_back(S(1,1));
    }

}


vector<complex<double> > SparEngine::getSij(int i, int j)
{
    if ((i == 1) && ( j == 1 )) return S11;
    if ((i == 2) && ( j == 1 )) return S21;
    if ((i == 2) && ( j == 2 )) return S22;
    return vector<complex<double> > (0,0);
}

vector<double> SparEngine::getFreq()
{
    return sim_settings.freq;
}