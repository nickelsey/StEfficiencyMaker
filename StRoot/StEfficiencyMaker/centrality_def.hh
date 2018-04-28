
#ifndef CENTRALITY_DEF_HH
#define CENTRALITY_DEF_HH

// defines a lightweight class that can handle StRefMultCorr corrections
// allows similar cuts to be set, but by hand instead of reading from a
// table

// used to test refmultcorr & centrality definitions in sct

#include <vector>

class CentralityDef {
public:
  CentralityDef();
  ~CentralityDef();
  
  // sets the parameters necessary for refmultcorr calculations, must
  // be called before refMultCorr(), weight(), etc
  void setEvent(int runid, double refmult, double zdc, double vz);
  
  // given a luminosity, a vz position, and a refmult, calculate
  // the corrected refmult
  double refMultCorr() const {return refmultcorr_;}
  
  // get the weight associated with the corrected reference multiplicity
  double weight() {return weight_;}
  
  // given a corrected refmult, calculate the 16 or 9 bin centrality
  int centrality16() {return centrality_16_;}
  int centrality9() {return centrality_9_;}
  
  // two parameters for luminosity correction -
  // [0] + [1] * zdcRate
  void setZDCParameters(double par0, double par1);
  void setZDCParameters(const std::vector<double>& pars);
  std::vector<double> ZDCParameters() const {return zdc_par_;}
  
  // 6 parameters for vz correction - sixth order polynomial
  void setVzParameters(double par0, double par1, double par2, double par3,
                       double par4, double par5, double par6);
  void setVzParameters(const std::vector<double>& pars);
  std::vector<double> VzParameters() const {return vz_par_;}
  
  // set ZDC range for which the fits were performed
  void setZDCRange(double min, double max) {min_zdc_ = min; max_zdc_ = max;}
  double ZDCMin() const {return min_zdc_;}
  double ZDCMax() const {return max_zdc_;}
  void setZDCNormalizationPoint(double norm) {zdc_norm_ = norm;}
  double ZDCNormalizationPoint() const {return zdc_norm_;}
  
  // set Vz range for which the fits were performed
  void setVzRange(double min, double max) {min_vz_ = min; max_vz_ = max;}
  double VzMin() const {return min_vz_;}
  double VzMax() const {return max_vz_;}
  void setVzNormalizationPoint(double norm) {vz_norm_ = norm;}
  double VzNormalizationPoint() const {return vz_norm_;}
  
  // set minimum and maximum run numbers that the corrections are valid for
  void setRunRange(int min, int max) {min_run_ = min; max_run_ = max;}
  int runMin() const {return min_run_;}
  int runMax() const {return max_run_;}
  
  // load refmultcorr centrality bin edges
  void setCentralityBounds16Bin(const std::vector<unsigned>& bounds);
  std::vector<unsigned> CentralityBounds16Bin() const {return cent_bin_16_;}
  std::vector<unsigned> CentralityBounds9Bin() const {return cent_bin_9_;}
  
  // parameters for refmultcorr weighting
  // bound is the cutoff for normalization: beyond this point, weight = 1.0
  void setWeightParameters(const std::vector<double>& pars, double bound = 400);
  std::vector<double> weightParameters() const {return weight_par_;}
  double reweightingBound() const {return weight_bound_;}
  
private:
  
  bool checkEvent(int runid, double refmult, double zdc, double vz);
  void calculateCentrality(double refmult, double zdc, double vz);
  
  double refmultcorr_;
  int centrality_16_;
  int centrality_9_;
  double weight_;
  
  double min_vz_;
  double max_vz_;
  double min_zdc_;
  double max_zdc_;
  int min_run_;
  int max_run_;
  double weight_bound_;
  
  double vz_norm_;
  double zdc_norm_;
  
  std::vector<double> zdc_par_;
  std::vector<double> vz_par_;
  std::vector<double> weight_par_;
  std::vector<unsigned> cent_bin_16_;
  std::vector<unsigned> cent_bin_9_;
  
};

#endif // CENTRALITY_DEF_HH
