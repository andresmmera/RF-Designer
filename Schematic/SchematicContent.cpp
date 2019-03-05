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
  NumberWires = 0;
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

void SchematicContent::assignNetToWiresConnectedToNode(QString NodeID,
                                                       QString net) {
  for (int i = 0; i < Wires.length(); i++) {
    if (!Wires[i].getNet().isEmpty())
      continue;
    if ((Wires[i].OriginID == NodeID) || (Wires[i].DestinationID == NodeID))
      Wires[i].setNet(net);
  }
}

QString SchematicContent::getQucsNetlist() {
  // Build Qucs netlist
  /* QString QucsNetlist;
   QString codestr;
   for (int i = 0; i < Comps.length(); i++) {
     codestr = Comps[i].getQucs();
     if (!codestr.isEmpty())
       QucsNetlist += codestr;
   }
   return QucsNetlist;*/

  // Firstly, find those wires connected to ground
  // Check if the wire is connected to ground
  for (int i = 0; i < Wires.length(); i++) {
    if ((Wires[i].OriginID.contains("GND")) ||
        (Wires[i].DestinationID.contains("GND")))
      Wires[i].setNet("gnd");
  }

  // We need to find those wires which connect two nodes and
  // assign them the same net
  for (int i = 0; i < Wires.length(); i++) {
    if ((Wires[i].OriginID.at(0) == 'N') &&
        (Wires[i].DestinationID.at(0) == 'N')) {
      // Then both nodes must share the same net. In other words, every wire
      // connected to these nodes must share the same net
      QString net_name;
      if (Wires[i].getNet().isEmpty())
        net_name = Wires[i].OriginID;
      else
        net_name = Wires[i].getNet();
      // Propagate this net to the wires connected to the above nodes
      assignNetToWiresConnectedToNode(Wires[i].OriginID, net_name);
      assignNetToWiresConnectedToNode(Wires[i].DestinationID, net_name);
    }
  }

  for (int i = 0; i < Wires.length(); i++) {

    if (!Wires[i].getNet().isEmpty())
      continue; // The net was already set

    // A wire has two endings. If one of them is connected to a node, then the
    // wire ID takes the name of that node
    QString net_name;
    if (Wires[i].OriginID.at(0) == 'N') {
      net_name = Wires[i].OriginID;
      Wires[i].setNet(net_name);
      assignNetToWiresConnectedToNode(Wires[i].OriginID, net_name);
    }
    if (Wires[i].DestinationID.at(0) == 'N') {
      net_name = Wires[i].DestinationID;
      Wires[i].setNet(net_name);
      assignNetToWiresConnectedToNode(Wires[i].DestinationID, net_name);
    }
  }

  // Finally, we hace to look for those wires not connected to a node. Let's set
  // the net = Wire ID
  for (int i = 0; i < Wires.length(); i++) {
    if (!Wires[i].getNet().isEmpty())
      continue; // The net was already set

    Wires[i].setNet(Wires[i].getID());
  }

  QString codestr;
  QVector<QString> connections;
  int connection_counter;
  int open_counter = 0; // Counts the number of open nodes;
  // Now, iterate through the components list. The component name and its
  // connections are recorded in a line in the netlist
  for (int i = 0; i < Comps.length(); i++) {
    if (Comps[i].Type == GND)
      continue;
    codestr += Comps[i].getQucsCode(); // Get component code

    // Set nodes according to the information updated in the above loop. We need
    // to loop again the Wires list update the connections
    connections.clear(); // Clear the data of a previous component
    connection_counter = 0;
    connections.resize(Comps[i].getNumberOfPorts());

    // Check if the component is a term. In that case, this a 2-port device, but
    // it one of its ports is intrinsically connected to ground
    if (Comps[i].Type == Term)
      connections[1] = QString("gnd");
    if (Comps[i].Type == ShortStub)
      connections[0] = QString("gnd");
    if (Comps[i].Type == OpenStub)
      connections[0] = QString("NOPEN%1").arg(++open_counter);

    int num_ports = Comps[i].getNumberOfPorts();
    for (int k = 0; k < Wires.length(); k++) {
      if (connection_counter == num_ports)
        break;
      if (Wires[k].DestinationID == Comps[i].ID) {
        connections[Wires[k].PortDestination] = Wires[k].getNet();
        connection_counter++;
      }
      if (Wires[k].OriginID == Comps[i].ID) {
        connections[Wires[k].PortOrigin] = Wires[k].getNet();
        connection_counter++;
      }
    }
    // In case not every component port is connected to a wire, we must assign
    // the other ports are opened.
    if (connection_counter < num_ports) {
      for (int i = 0; i < num_ports; i++) {
        if (connections[i].isEmpty())
          connections[i] = QString("NOPEN%1").arg(++open_counter);
      }
    }

    // Now put the content of the connections vector into the netlist
    for (int k = 0; k < connections.size(); k++) {
      codestr += QString(" %1").arg(connections[k]);
    }

    // Set properties
    codestr += " ";
    codestr += Comps[i].getQucsProperties();
  }
  return codestr;
}
void SchematicContent::appendComponent(struct ComponentInfo C) {
  Comps.append(C);
}
void SchematicContent::appendWire(WireInfo WI) {
  // Set wire ID
  QString ID = QString("W%1").arg(++NumberWires);
  WI.setID(ID);
  Wires.append(WI);
}
void SchematicContent::appendWire(QString O, int ON, QString D, int DN) {
  WireInfo WI(O, ON, D, DN);
  // Set wire ID
  QString ID = QString("W%1").arg(++NumberWires);
  WI.setID(ID);
  Wires.append(WI);
}
void SchematicContent::appendWire(QString O, int ON, QString D, int DN,
                                  QColor c) {
  WireInfo WI(O, ON, D, DN);
  WI.WireColor = c;
  // Set wire ID
  QString ID = QString("W%1").arg(++NumberWires);
  WI.setID(ID);
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

std::complex<double> SchematicContent::getZin() {
  return Comps[0].getValZ("Z");
}
std::complex<double> SchematicContent::getZout() {
  return Comps[Comps.size() - 1].getValZ("Z");
}

QString SchematicContent::getZinString() { return Comps[0].val["Z"]; }
QString SchematicContent::getZoutString() {
  return Comps[Comps.size() - 1].val["Z"];
}

struct SP_Analysis SchematicContent::getSPAR_Sweep() {
  return SPAR_Settings;
}

void SchematicContent::setSPAR_Sweep(SP_Analysis SPAR) { SPAR_Settings = SPAR; }
