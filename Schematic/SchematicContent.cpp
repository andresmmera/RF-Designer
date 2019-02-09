#include "SchematicContent.h"

SchematicContent::SchematicContent() {
  Comps.clear();
  Nodes.clear();
  Wires.clear();
  displayGraphs.clear();
  // Initialize list of components
  NumberComponents[Capacitor] = 0;
  NumberComponents[Inductor] = 0;
  NumberComponents[Term] = 0;
  NumberComponents[GND] = 0;
  NumberComponents[ConnectionNodes] = 0;
  NumberComponents[Resistor] = 0;
  NumberComponents[TransmissionLine] = 0;
  NumberComponents[CoupledLines] = 0;
  NumberComponents[Coupler] = 0;
  NumberComponents[OpenStub] = 0;
  NumberComponents[ShortStub] = 0;
}

// This function returns a structure for doing a simple (internal) ladder
// simulation
NetworkInfo SchematicContent::getLadder() {
  NetworkInfo NWI;
  std::vector<std::complex<double>> ZS(1), ZL(1);
  ZS[0] = Str2Complex(Comps[0].val["Z"]);                // Port 1 impedance
  ZL[0] = Str2Complex(Comps[Comps.size() - 1].val["Z"]); // Port 2 impedance
  NWI.ZS = ZS;
  NWI.ZL = ZL;
  NWI.Ladder = Comps;
  return NWI;
}

QString SchematicContent::getQucsNetlist() {
  // Build Qucs netlist
  QString QucsNetlist;
  QString codestr;
  for (int i = 0; i < Comps.length(); i++) {
    codestr = Comps[i].getQucs();
    if (!codestr.isEmpty())
      QucsNetlist += codestr;
  }
  return QucsNetlist;
}
void SchematicContent::appendComponent(struct ComponentInfo C) {
  Comps.append(C);
}
void SchematicContent::appendWire(struct WireInfo W) { Wires.append(W); }
void SchematicContent::appendWire(QString O, int ON, QString D, int DN) {
  WireInfo WI(O, ON, D, DN);
  Wires.append(WI);
}
void SchematicContent::appendWire(QString O, int ON, QString D, int DN,
                                  QColor c) {
  WireInfo WI(O, ON, D, DN);
  WI.WireColor = c;
  Wires.append(WI);
}

void SchematicContent::appendNode(struct NodeInfo N) { Nodes.append(N); }

QList<struct ComponentInfo> SchematicContent::getComponents() { return Comps; }
QList<struct WireInfo> SchematicContent::getWires() { return Wires; }
QList<struct NodeInfo> SchematicContent::getNodes() { return Nodes; }

void SchematicContent::setDescription(QString d) { Description = d; }
QString SchematicContent::getDescription() { return Description; }

void SchematicContent::appendGraph(QString title, QPen pen) {
  displayGraphs[title] = pen;
}
void SchematicContent::clearGraphs() { displayGraphs.clear(); }

QMap<QString, QPen> SchematicContent::getDisplayGraphs() {
  return displayGraphs;
}

double SchematicContent::getZin() { return Comps[0].val["Z"].toDouble(); }
double SchematicContent::getZout() {
  return Comps[Comps.size() - 1].val["Z"].toDouble();
}

QString SchematicContent::getZinString() { return Comps[0].val["Z"]; }
QString SchematicContent::getZoutString() {
  return Comps[Comps.size() - 1].val["Z"];
}

struct SP_Analysis SchematicContent::getSPAR_Sweep() {
  return SPAR_Settings;
}

void SchematicContent::setSPAR_Sweep(SP_Analysis SPAR) { SPAR_Settings = SPAR; }
