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
    /*if (Specs.Type == "Multistage Wilkinson")  MultistageWilkinson();
    if (Specs.Type == "Tee")                   Tee();
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
