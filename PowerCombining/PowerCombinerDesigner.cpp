#include "PowerCombinerDesigner.h"

PowerCombinerDesigner::PowerCombinerDesigner(PowerCombinerParams SPC)
{
   Specs = SPC;
}

QList<ComponentInfo> PowerCombinerDesigner::getComponents()
{
    return Components;
}


QList<WireInfo> PowerCombinerDesigner::getWires()
{
    return Wires;
}

QList<NodeInfo> PowerCombinerDesigner::getNodes()
{
    return Nodes;
}


void PowerCombinerDesigner::synthesize()
{
    if (Specs.Type == "Wilkinson")             Wilkinson();
    if (Specs.Type == "Multistage Wilkinson")  MultistageWilkinson();
    /*if (Specs.Type == "Tee")                   Tee();
    if (Specs.Type == "Branchline")            Branchline();
    if (Specs.Type == "Double box branchline") DoubleBoxBranchline();
    if (Specs.Type == "Travelling Wave")       TravellingWave();
    if (Specs.Type == "Tree")                  Tree();*/
}




TwoWayWilkinsonParams PowerCombinerDesigner::CalculateWilkinson()
{
    double K = Specs.OutputRatio;//Conversion to natural units
    TwoWayWilkinsonParams WilkinsonParams;
    // Wilkinson divider design equations
    double K2 =K*K;
    WilkinsonParams.Z3 = Specs.Z0*sqrt((K2+1)/(K*K*K));
    WilkinsonParams.Z2 = K2*WilkinsonParams.Z3;
    WilkinsonParams.R=Specs.Z0*((K2+1)/K);
    WilkinsonParams.R2 = Specs.Z0*K;
    WilkinsonParams.R3 = Specs.Z0/K;
    return WilkinsonParams;
}




//This function creates a string for the transmission line length and automatically changes the unit length if the value lies outside [1,999.99]
QString PowerCombinerDesigner::ConvertLengthFromM(double len)
{
  int index;

  if (Specs.units == "mm")   index = 0;
  if (Specs.units == "mil")  index = 1;
  if (Specs.units == "um")   index = 2;
  if (Specs.units == "nm")   index = 3;
  if (Specs.units == "inch") index = 4;
  if (Specs.units == "ft")   index = 5;
  if (Specs.units == "m")    index = 6;

  double conv;

  do{
  conv=len;
  switch (index)
  {
    case 1: //mils
          conv *= 39370.1;
          if (conv > 999.99)
          {
            index = 4;//inches
            break;
          }
          if(conv < 1)
          {
            index = 2;//microns
            break;
          }
          return QString("%1 mil").arg(RoundVariablePrecision(conv));
    case 2: //microns
          conv *= 1e6;
          if (conv > 999.99)
          {
            index = 0;//milimeters
            break;
          }
          if(conv < 1)
          {
            index = 3;//nanometers
            break;
          }
          return QString("%1 um").arg(RoundVariablePrecision(conv));
    case 3: //nanometers
          conv *= 1e9;
          if (conv > 999.99)
          {
            index = 2;//microns
            break;
          }
          return QString("%1 nm").arg(RoundVariablePrecision(conv));
    case 4: //inch
          conv *= 39.3701;
          if (conv > 999.99)
          {
            index = 5;//feets
            break;
          }
          if(conv < 1)
          {
            index = 1;//mils
            break;
          }
          return QString("%1 in").arg(RoundVariablePrecision(conv));
    case 5: //ft
          conv *= 3.280841666667;
          if (conv > 999.99)
          {
            index = 6;//meters
            break;
          }
          if(conv < 1)
          {
            index = 4;//inches
            break;
          }
          return QString("%1 ft").arg(RoundVariablePrecision(conv));
    case 6: //m
          if(conv < 1)
          {
            index = 0;//mm
            break;
          }
          return QString("%1").arg(RoundVariablePrecision(len));
    default: //milimeters
          conv *=1e3;
          if (conv > 999.99)
          {
            index = 6;//meters
            break;
          }
          if(conv < 1)
          {
            index = 2;//microns
            break;
          }
          return QString("%1 mm").arg(RoundVariablePrecision(conv));
  }
  }while(true);
  return QString("");
}
