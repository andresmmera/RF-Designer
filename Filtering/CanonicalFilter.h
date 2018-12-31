#ifndef CANONICALFILTER_H
#define CANONICALFILTER_H

#include "Filtering/LowpassPrototypeCoeffs.h"
#include "Filtering/Network.h"
#include "Schematic/component.h"
#include "general.h"
#include <QPen>
class CanonicalFilter : public Network {
public:
  CanonicalFilter();
  virtual ~CanonicalFilter();
  CanonicalFilter(FilterSpecifications);
  QList<ComponentInfo> getComponents();
  QList<WireInfo> getWires();
  QList<NodeInfo> getNodes();
  void synthesize();
  NetworkInfo getLadder();
  QString getQucsNetlist() { return QucsNetlist; }
  QMap<QString, QPen> displaygraphs;

private:
  struct FilterSpecifications Specification;
  QList<ComponentInfo> Components;
  QList<WireInfo> Wires;
  QList<NodeInfo> Nodes;

  QString QucsNetlist;

  QMap<ComponentType, int>
      NumberComponents; // List for assigning IDs to the filter components
  std::vector<std::complex<double>> Poles;
  std::vector<std::complex<double>> Zeros;
  std::deque<double> gi;

  //***********  Schematic synthesis ********************
  void SynthesizeLPF(); // Lowpass
  void SynthesizeHPF(); // Highpass
  void SynthesizeBPF(); // Bandpass
  void SynthesizeBSF(); // Bandstop
};

#endif // CANONICALFILTER_H
