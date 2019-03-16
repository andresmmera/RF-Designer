/***************************************************************************
                                io.cpp
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

#include "io.h"

IO::IO() {
  Nsamples = 30;
  fmatching_min = -1;
  fmatching_max = -1;
}

IO::~IO() {}

// Converts std::string to double
double string_to_double(const std::string &s) {
  std::istringstream i(s);
  double x;
  if (!(i >> x))
    return 0;
  return x;
}

// Extends std::tolower(int c) capability to std::string arguments
string tolower(string str) {
  char c;
  for (unsigned int i = 0; i < str.length(); i++) {
    c = str.at(i);
    str.at(i) = tolower(c);
  }
  return str;
}

// Removes consecutive blank spaces in a std::string
string RemoveBlankSpaces(string line) {
  // Remove consecutive repeated blank spaces and space at the beginning
  // Sometimes, the fields may be separated by \t...
  for (unsigned int i = 0; i < line.length(); i++) {
    if (i == 0) // Remove first space
    {
      if ((!line.substr(0, 1).compare(" ")) ||
          (!line.substr(0, 1).compare("\t"))) {
        line.erase(0, 1);
        i--;
      }
      continue;
    }
    if (((!line.substr(i - 1, 1).compare(" ")) ||
         (!line.substr(i - 1, 1).compare("\t"))) &&
        ((!line.substr(i, 1).compare(" ")) ||
         (!line.substr(i, 1).compare("\t")))) {
      line.erase(i, 1);
      i--;
    }
  }
  return line;
}

// Loads impedance data from a s1p file
int IO::loadS1Pdata(std::string filepath, terminal Port) {
  std::deque<std::complex<double>> S11;
  std::ifstream s2pfile(filepath.c_str()); // Tries to open the data file.
  if (!s2pfile.is_open()) // The data file cannot be opened => error
  {
    return -1;
  }

  std::string line;
  double freq_scale = 1;
  double Zref = 50;

  std::getline(s2pfile, line);
  while (line.compare(0, 1, "#")) // Looking for # field
  {
    std::getline(s2pfile, line);
  }

  line = tolower(line);
  freq_scale = getS2PfreqScale(line);

  // Get the impedance at which the S params were measured

  int Rindex = line.find_last_of("r");
  Rindex = line.find_first_not_of(" ", Rindex);
  Zref = atof(line.substr(Rindex + 1).c_str());

  bool is_indB = (line.find("db") != string::npos);
  bool RI = (line.find("ma") == string::npos);
  bool isS_par = (line.find(" s ") != string::npos);
  bool isZ_par = (line.find(" z ") != string::npos);

  while (getline(s2pfile, line)) { // Looking for the start of the raw data

    line = RemoveBlankSpaces(line);

    if ((!line.compare(0, 1, "!")) || (line.length() == 1))
      continue;
    else
      break;
  }

  // DATA beginning.
  // At this point, the number of frequency samples is not known, so it's better
  // to push data into queues and then arrange it into armadillo structures
  std::queue<double> frequency, S11M, S11A;
  unsigned int qsize = 0;

  do {
    line = RemoveBlankSpaces(line);
    if (line.empty() || (line.length() == 1))
      break;
    if (line.at(0) == '!')
      break; // Comment

    // Frequency
    int index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    frequency.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);

    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S11M.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);

    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S11A.push(string_to_double(line.substr(0, index)));

    qsize++;
  } while (std::getline(s2pfile, line));
  s2pfile.close();
  vector<double> freq(qsize);
  vector<complex<double>> S(qsize);
  vector<complex<double>> Z(qsize);

  double phi, S11m, S11a;
  for (unsigned int i = 0; i < qsize; i++) {
    freq[i] = freq_scale * frequency.front();
    frequency.pop();

    S11m = S11M.front();
    S11a = S11A.front();
    S11M.pop();
    S11A.pop();
    if (is_indB)
      S11m = pow(10, .05 * S11m);
    phi = (M_PI / 180) * S11a;

    if (RI) {
      S[i] = complex<double>(S11m, S11a);
    } else {
      S[i] = complex<double>(S11m, 0) * complex<double>(cos(phi), sin(phi));
    }

    if (isZ_par) { // The data file contains impedance data
      Z[i] = S[i];
    }
    if (isS_par) { // The data file contains s1p data
      Z[i] = Zref * ((1. + S[i]) / (1. - S[i])); // Z
    }
  }
  vector<string> candidates;
  if (Port == SOURCE) // Set source port properties
  {
    fS = freq;
    ZS = Z;
  }

  if (Port == LOAD) // Set load port properties
  {
    fL = freq;
    ZL = Z;
  }
  return 0;
}

// Reads a s2p Touchstone file
int IO::loadS2Pdata(std::string filepath) {
  std::ifstream s2pfile(filepath.c_str()); // Tries to open the data file.
  if (!s2pfile.is_open()) // The data file cannot be opened => error
  {
    return -1;
  }
  std::string line;
  double freq_scale = 1;
  // double Zref = 50;

  std::getline(s2pfile, line);
  while (line.compare(0, 1, "#")) // Looking for # field
  {
    std::getline(s2pfile, line);
  }

  line = tolower(line);
  freq_scale = getS2PfreqScale(line);

  // Get the impedance at which the S params were measured

  int Rindex = line.find_last_of("r");
  Rindex = line.find_first_not_of(" ", Rindex);
  DeviceS2P.Z0 = atof(line.substr(Rindex + 1).c_str());

  bool is_indB = (line.find("db") != string::npos);
  bool RI = (line.find("ri") != string::npos);

  while (getline(s2pfile, line)) { // Looking for the start of the raw data

    line = RemoveBlankSpaces(line);

    if ((!line.compare(0, 1, "!")) || (line.length() == 1))
      continue;
    else
      break;
  }

  // DATA beginning.
  // At this point, the number of frequency samples is not known, so it's better
  // to push data into queues and then arrange it into armadillo structures
  std::queue<double> frequency, S11M, S11A, S21M, S21A, S12M, S12A, S22M, S22A;
  std::deque<std::complex<double>> S11, S21, S12, S22;
  unsigned int qsize = 0;

  do {
    line = RemoveBlankSpaces(line);
    if (line.empty() || (line.length() == 1))
      break;
    if (line.at(0) == '!')
      break; // Comment

    // Frequency
    int index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    frequency.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);

    // Read S11
    //-----------------------------------------------
    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S11M.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);

    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S11A.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);
    //-----------------------------------------------

    // Read S21
    //-----------------------------------------------
    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S21M.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);

    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S21A.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);
    //-----------------------------------------------

    // Read S12
    //-----------------------------------------------
    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S12M.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);

    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S12A.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);
    //-----------------------------------------------

    // Read S22
    //-----------------------------------------------
    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S22M.push(string_to_double(line.substr(0, index)));
    line.erase(0, index + 1);

    index = line.find_first_of(" ");
    if (index == -1)
      index = line.find_first_of("\t");
    S22A.push(string_to_double(line.substr(0, index)));
    //-----------------------------------------------

    qsize++;
  } while (std::getline(s2pfile, line));
  s2pfile.close();
  vector<double> freq(qsize);

  double S11m, S11a, S21m, S21a, S12m, S12a, S22m, S22a;
  for (unsigned int i = 0; i < qsize; i++) {
    freq[i] = freq_scale * frequency.front();
    frequency.pop();

    S11m = S11M.front();
    S11a = S11A.front();
    S11M.pop();
    S11A.pop();

    S21m = S21M.front();
    S21a = S21A.front();
    S21M.pop();
    S21A.pop();

    S12m = S12M.front();
    S12a = S12A.front();
    S12M.pop();
    S12A.pop();

    S22m = S22M.front();
    S22a = S22A.front();
    S22M.pop();
    S22A.pop();

    if (is_indB)
      S11m = pow(10, .05 * S11m);
    if (is_indB)
      S21m = pow(10, .05 * S21m);
    if (is_indB)
      S12m = pow(10, .05 * S12m);
    if (is_indB)
      S22m = pow(10, .05 * S22m);

    if (RI) {
      S11.push_back(complex<double>(S11m, S11a));
      S21.push_back(complex<double>(S21m, S21a));
      S12.push_back(complex<double>(S12m, S12a));
      S22.push_back(complex<double>(S22m, S22a));
    } else {
      S11a = (M_PI / 180) * S11a;
      S21a = (M_PI / 180) * S21a;
      S12a = (M_PI / 180) * S12a;
      S22a = (M_PI / 180) * S22a;

      S11.push_back(complex<double>(S11m, 0) *
                    complex<double>(cos(S11a), sin(S11a)));
      S21.push_back(complex<double>(S21m, 0) *
                    complex<double>(cos(S21a), sin(S21a)));
      S12.push_back(complex<double>(S12m, 0) *
                    complex<double>(cos(S12a), sin(S12a)));
      S22.push_back(complex<double>(S22m, 0) *
                    complex<double>(cos(S22a), sin(S22a)));
    }
  }
  fAMP = freq;

  // Resize vectors
  DeviceS2P.S11.resize(S11.size());
  DeviceS2P.S12.resize(S11.size());
  DeviceS2P.S21.resize(S11.size());
  DeviceS2P.S22.resize(S11.size());

  // Transfer the data to S2P_DATA
  for (int i = 0; i < S21.size(); i++) {
    DeviceS2P.S11[i] = S11[i];
    DeviceS2P.S12[i] = S21[i];
    DeviceS2P.S21[i] = S12[i];
    DeviceS2P.S22[i] = S22[i];
  }
  return 0;
}

// Load and source impedances may be sampled at different frequencies. It is
// essential to resample them using the same frequency basis. This requires
// interpolation of complex data. It would be desirable to use spline or cubic
// interpolation, but it seems that they are not implemented in Armadillo
int IO::ResampleImpedances() {
  if (ZS.size() == 0)
    return 0; // Not set
  if (ZL.size() == 0)
    return 0; // Not set
  if ((Two_Port_Matching) && (fAMP.size() == 0))
    return 0; // Not set

  if (fmatching_min == -1)
    return 0;

  freq = linspace(fmatching_min, fmatching_max,
                  Nsamples); // Frequency vector employed for matching

  // Source impedance
  if (ZS.size() == 1) {
    complex<double> zs_temp = ZS[0];
    ZS.resize(freq.size());
    ZS = ones(ZS);
    ZS = Product(ZS, zs_temp);
  } else {
    // Extract impedance lying on the specified frequency band defined by the
    // user in the UI
    unsigned int i1s = closestIndex(fS, fmatching_min);
    unsigned int i2s = closestIndex(fS, fmatching_max);
    vector<complex<double>> zs =
        SubVector(ZS, i1s, i2s); // Useful impedance data
    vector<double> fs = SubVector(
        fS, i1s, i2s); // Frequency points where the impedance zs is sampled
    vector<complex<double>> ZS_ =
        interp(fs, zs, freq); // Interpolation using input data
    ZS.resize(ZS_.size());    // Resize ZS array
    ZS = ZS_;                 // Copy data
  }

  // Load impedance
  if (ZL.size() == 1) {
    complex<double> zl_temp = ZL[0];
    ZL.resize(freq.size());
    ZL = ones(ZL);
    ZL = Product(ZL, zl_temp);
  } else {
    // Extract impedance lying on the specified frequency band defined by the
    // user in the UI
    unsigned int i1l = closestIndex(fL, fmatching_min);
    unsigned int i2l = closestIndex(fL, fmatching_max);
    vector<complex<double>> zl =
        SubVector(ZL, i1l, i2l); // Useful impedance data
    vector<double> fl = SubVector(
        fL, i1l, i2l); // Frequency points where the impedance zs is sampled
    vector<complex<double>> ZL_ =
        interp(fl, zl, freq); // Interpolation using input data
    ZL.resize(ZL_.size());    // Resize ZS array
    ZL = ZL_;                 // Copy data
  }

  if (DeviceS2P.S11.empty())
    return 0; // One port matching

  // Amplifier SPAR data
  if (DeviceS2P.S11.size() == 1) // Constant SPAR vs frequency. Corner case, but
                                 // some user may input data like this
  {
    complex<double> S11_temp = DeviceS2P.S11[0];
    complex<double> S21_temp = DeviceS2P.S21[0];
    complex<double> S12_temp = DeviceS2P.S12[0];
    complex<double> S22_temp = DeviceS2P.S22[0];

    DeviceS2P.S11.resize(freq.size());
    DeviceS2P.S21.resize(freq.size());
    DeviceS2P.S12.resize(freq.size());
    DeviceS2P.S22.resize(freq.size());

    DeviceS2P.S11 = ones(DeviceS2P.S11);
    DeviceS2P.S21 = ones(DeviceS2P.S21);
    DeviceS2P.S12 = ones(DeviceS2P.S12);
    DeviceS2P.S22 = ones(DeviceS2P.S22);

    DeviceS2P.S11 = Product(DeviceS2P.S11, S11_temp);
    DeviceS2P.S21 = Product(DeviceS2P.S21, S21_temp);
    DeviceS2P.S12 = Product(DeviceS2P.S12, S12_temp);
    DeviceS2P.S22 = Product(DeviceS2P.S22, S22_temp);
  } else {
    // Extract impedance lying on the specified frequency band defined by the
    // user in the UI
    unsigned int i1amp = closestIndex(fAMP, fmatching_min);
    unsigned int i2amp = closestIndex(fAMP, fmatching_max);

    vector<complex<double>> s11 = SubVector(DeviceS2P.S11, i1amp, i2amp);
    vector<complex<double>> s21 = SubVector(DeviceS2P.S21, i1amp, i2amp);
    vector<complex<double>> s12 = SubVector(DeviceS2P.S12, i1amp, i2amp);
    vector<complex<double>> s22 = SubVector(DeviceS2P.S22, i1amp, i2amp);
    vector<double> fa =
        SubVector(fAMP, i1amp,
                  i2amp); // Frequency points where the impedance zs is sampled

    vector<complex<double>> S11_ =
        interp(fa, s11, freq); // Interpolation using input data
    vector<complex<double>> S21_ = interp(fa, s21, freq);
    vector<complex<double>> S12_ = interp(fa, s12, freq);
    vector<complex<double>> S22_ = interp(fa, s22, freq);

    DeviceS2P.S11.resize(S11_.size());
    DeviceS2P.S21.resize(S21_.size());
    DeviceS2P.S12.resize(S12_.size());
    DeviceS2P.S22.resize(S22_.size());

    DeviceS2P.S11 = S11_; // Copy data
    DeviceS2P.S21 = S21_;
    DeviceS2P.S12 = S12_;
    DeviceS2P.S22 = S22_;
  }

  // Finally, we calculate conj(Zin) and conj(Zout) for achieving maximum gain
  // on a two-port device

  /***********************************************************************
    (gamma_S)                (gamma_in)    (gamma_out)                (gamma_L)
            _________________       _______        __________________
           |                 | Zin |       | Zout |                  |
      ZS---| INPUT MATCHING  |-----|  TRT  |------| OUTPUT MATCHING  |---ZL
           |_________________|     |_______|      |__________________|

  ************************************************************************/

  vector<complex<double>> delta =
      DeviceS2P.S11 * DeviceS2P.S22 - DeviceS2P.S21 * DeviceS2P.S12;
  vector<double> B1 = 1 + abs(DeviceS2P.S11) * abs(DeviceS2P.S11) -
                      abs(DeviceS2P.S22) * abs(DeviceS2P.S22) -
                      abs(delta) * abs(delta);
  vector<double> B2 = 1 + abs(DeviceS2P.S22) * abs(DeviceS2P.S22) -
                      abs(DeviceS2P.S11) * abs(DeviceS2P.S11) -
                      abs(delta) * abs(delta);
  vector<complex<double>> C1 = DeviceS2P.S11 - delta * conj(DeviceS2P.S22);
  vector<complex<double>> C2 = DeviceS2P.S22 - delta * conj(DeviceS2P.S11);
  complex<double> gamma_S, gamma_L;
  int ext_code = 0;
  Zin_maxg.resize(C1.size());
  Zout_maxg.resize(C1.size());
  // The sign of the square root must be chosen for each frequency so here we
  // cannot apply vector operations
  for (unsigned int i = 0; i < C1.size(); i++) {
    gamma_S = (B1[i] - sqrt(B1[i] * B1[i] - 4. * abs(C1[i]) * abs(C1[i]))) /
              (2. * C1[i]);
    gamma_L = (B2[i] - sqrt(B2[i] * B2[i] - 4. * abs(C2[i]) * abs(C2[i]))) /
              (2. * C2[i]);

    if ((gamma_S != gamma_S) || (gamma_L != gamma_L)) // Check if Nan
    {
      ext_code = -2;
      Zin_maxg[i] = DeviceS2P.Z0 *
                    (complex<double>(1, 0) + conj(DeviceS2P.S11.at(i))) /
                    (complex<double>(1, 0) - conj(DeviceS2P.S11.at(i)));
      Zout_maxg[i] = DeviceS2P.Z0 *
                     (complex<double>(1, 0) + conj(DeviceS2P.S22.at(i))) /
                     (complex<double>(1, 0) - conj(DeviceS2P.S22.at(i)));
    } else {
      Zin_maxg[i] = DeviceS2P.Z0 * (complex<double>(1, 0) + conj(gamma_S)) /
                    (complex<double>(1, 0) - conj(gamma_S));
      Zout_maxg[i] = DeviceS2P.Z0 * (complex<double>(1, 0) + conj(gamma_L)) /
                     (complex<double>(1, 0) - conj(gamma_L));
      continue;
    }
    if (real(Zin_maxg[i]) <= 0) {
      gamma_S = (B1[i] + sqrt(B1[i] * B1[i] - 4. * abs(C1[i]) * abs(C1[i]))) /
                (2. * C1[i]);
      Zin_maxg[i] = ZS[i] * (complex<double>(1, 0) + conj(gamma_S)) /
                    (complex<double>(1, 0) - conj(gamma_S));
    }

    if (real(Zout_maxg[i]) <= 0) {
      gamma_L = (B2[i] + sqrt(B2[i] * B2[i] - 4. * abs(C2[i]) * abs(C2[i]))) /
                (2. * C2[i]);
      Zout_maxg[i] = ZL[i] * (complex<double>(1, 0) + conj(gamma_L)) /
                     (complex<double>(1, 0) - conj(gamma_L));
    }
  }

  // Notice that the equation below difer from the typical equations for
  // two-port matching in the sense that here ZS and ZL are used instead of Z0.
  // Although in general the amplifier is matched to Z0=50 or 75\Ohm this tool
  // aims to treat the impedance matching problem in a more general fashion

  return ext_code;
}

vector<double> IO::getFrequency() { return freq; }

void IO::set_constant_ZS_vs_freq(complex<double> zs) {
  ZS = vector<complex<double>>(1);
  ZS[0] = zs;
}

void IO::set_constant_ZL_vs_freq(complex<double> zl) {
  ZL = vector<complex<double>>(1);
  ZL[0] = zl;
}

void IO::set_matching_band(double fmin, double fmax) {
  fmatching_min = fmin;
  fmatching_max = fmax;
  freq = linspace(fmatching_min, fmatching_max, Nsamples); // Available freqs
}

// Gets the index of a given frequency
int IO::getFreqIndex(double f1) { return closestIndex(freq, f1); }

// Get freq scale from a string line
double IO::getS2PfreqScale(string line) {
  if (line.find("ghz") != std::string::npos) {
    return 1e9;
  } else {
    if (line.find("mhz") != std::string::npos) {
      return 1e6;
    } else {
      if ((line.find("khz") != std::string::npos)) {
        return 1e3;
      } else {
        return 1; // Hz
      }
    }
  }
}

// This function creates a s1p file given a constant impedance. It adds a unique
// point, the S-param simulator should understand that this is constant
void IO::generateConstant_s1p(string datapath, complex<double> Z) {
  std::ofstream s1pFile(datapath.c_str(), ios_base::out);
  s1pFile << "# Hz Z RI R 1\n1000 " + Num2String(Z.real()) + " " +
                 Num2String(Z.imag());
  s1pFile.close();
}

string IO::Num2String(double Num) {
  char c = 0;
  double cal = abs(Num);
  if (cal > 1e-20) {
    cal = log10(cal) / 3.0;
    if (cal < -0.2)
      cal -= 0.98;
    int Expo = int(cal);

    if (Expo >= -5)
      if (Expo <= 4)
        switch (Expo) {
        case -5:
          c = 'f';
          break;
        case -4:
          c = 'p';
          break;
        case -3:
          c = 'n';
          break;
        case -2:
          c = 'u';
          break;
        case -1:
          c = 'm';
          break;
        case 1:
          c = 'k';
          break;
        case 2:
          c = 'M';
          break;
        case 3:
          c = 'G';
          break;
        case 4:
          c = 'T';
          break;
        }

    if (c)
      Num /= pow(10.0, double(3 * Expo));
  }

  std::ostringstream s;
  s << Num;
  string Str = s.str();
  if (c)
    Str += c;

  return Str;
}

string IO::Num2String(int x) {
  std::ostringstream s;
  s << x;
  return s.str();
}
