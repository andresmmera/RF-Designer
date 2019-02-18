#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>
#include <QStringList>
#include <complex>
#include <deque>
#include <vector>

enum ComponentType {
  Capacitor,
  Inductor,
  Term,
  GND,
  ConnectionNodes,
  Resistor,
  TransmissionLine,
  OpenStub,
  ShortStub,
  CoupledLines,
  Coupler
};

enum ResponseType {
  Butterworth,
  Chebyshev,
  Legendre,
  Elliptic,
  Blichinkoff,
  Bessel,
  LinearPhaseEqError,
  Gegenbauer
};
enum FilterClass { Lowpass, Highpass, Bandpass, Bandstop };
enum Coupling {
  CapacitativeCoupledShuntResonators,
  InductiveCoupledSeriesResonators
};
enum SemiLumpedImplementation { ONLY_INDUCTORS, INDUCTORS_AND_SHUNT_CAPS };

static const double SPEED_OF_LIGHT =
    299792458.0; // REMOVE THIS WHEN THIS TOOL BECOMES INTEGRATED IN QUCS

struct PrototypeTableProperties {
  QString ID;
  std::vector<int> N;
  std::vector<double> Ripple;
  std::vector<double> RL;
};

struct FilterSpecifications {
  FilterClass FilterType;      // Lowpass, Highpass, Bandpass, Bandstop
  ResponseType FilterResponse; // Butterworth, Chebyshev, Cauer, etc...
  QString Implementation;
  Coupling DC_Coupling; // Only for bandpass direct coupled filters
  bool isCLC;           // CLC or LCL implementation
  unsigned int order;   // Filter order
  double Ripple;        // Ripple (Chebyshev and Cauer)
  double fc; // Cutoff frequency (lowpass and highpass filters) or center
             // frequency (bandpass and bandstop)
  double bw; // Filter bandwidth
  double ZS; // Source impedance
  double ZL; // Load impedance
  double as; // Stopband attenuation
  QString EllipticType; // Type of elliptic filter
  bool UseZverevTables;
  double minZ; // Minimum synthesizable impedance (only for stepped impedance
               // filters)
  double maxZ; // Maximum synthesizable impedance (only for stepped impedance
               // filters)
  SemiLumpedImplementation SemiLumpedISettings;
  double ImpedanceRatio;
};

struct NetworkInfo {
  std::vector<std::complex<double>> ZS;
  std::vector<std::complex<double>> ZL;
  QStringList topology;
  QList<struct ComponentInfo>
      Ladder; // Contains the ID of the components and their properties
};

struct PowerCombinerParams {
  QString Type; // Wilkinson, branchlines, Bagley, etc.
  int Noutputs; // Number of output branches
  int Nstages;  // Number of combiner stages (broadband Wilkinson)
  std::deque<double> OutputRatio; // Splitting ratio
  QString Implementation;         // LC, microstrip, ideal TL
  double alpha;                   // Attenuation constant of the ideal TL
  QString units;                  // mm, mil, um
  double freq;                    // Center freq
  double Z0;                      // Reference impedance
};

struct AttenuatorDesignParameters {
  QString Topology;   // Attenuator topology
  double Zin;         // Input impedance
  double Zout;        // Output impedance
  double Attenuation; // Attenuation in dB
  double Frequency;   // Central frequency of tuned attenuators
  double Pin;         // Input power in W
  bool Lumped_TL;     // Use the lumped equivalent of a QW transmission line
};

struct MatchingNetworkDesignParameters {
  QString Topology; // Attenuator topology
  double Zin;       // Input impedance
  double Zout;      // Output impedance
};

struct PdissAtt {
  double R1, R2, R3, R4;
};

#endif // STRUCTURES_H
