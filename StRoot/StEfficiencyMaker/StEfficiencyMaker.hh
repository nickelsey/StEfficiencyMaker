
#ifndef STEFFICIENCYMAKER__HH
#define STEFFICIENCYMAKER__HH

#include <string>
#include <vector>

#include "StMaker.h"
#include "StMiniMcEvent/StMiniMcEvent.h"
#include "TChain.h"
#include "TFile.h"
#include "TH3D.h"
#include "TH2D.h"

#include "StMaker.h"
#include "StMuDSTMaker/COMMON/StMuDstMaker.h"
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuEvent.h"

struct axisDef {
  unsigned nBins;
  double low;
  double high;
  
  axisDef() : nBins(1), low(0), high(1) {}
  
  axisDef(unsigned n, double l, double h)
  : nBins(n), low(l), high(h) {}
  
  axisDef(const axisDef& rhs)
  : nBins(rhs.nBins), low(rhs.low), high(rhs.high) {};
  
  double width() const {return (high - low) / nBins;}
  
  bool valid() const {return nBins > 0 && width() > 0.0;}
};

class StEfficiencyMaker : public StMaker {
public:
  StEfficiencyMaker(TChain* chain, std::string outputFile = "stefficiencymaker.root");
  
  ~StEfficiencyMaker();
  
  // loads a new chain
  bool LoadTree(TChain* chain);
  
  // set axis bounds
  void SetDefaultAxes();
  void SetLuminosityAxis(unsigned n, double low, double high);
  void SetCentralityAxis(unsigned n, double low, double high);
  void SetPtAxis(unsigned n, double low, double high);
  void SetEtaAxis(unsigned n, double low, double high);
  void SetPhiAxis(unsigned n, double low, double high);
  
  // (re)creates histograms from current axisDefs
  Int_t Init();
  
  // process event
  Int_t Make();
  
  // save result histograms to disk
  Int_t Finish();
  
private:
  
  int InitInput();
  int InitOutput();
  bool LoadEvent();
  
  bool CheckAxes();
  
  TChain* chain_;
  TFile* out_;
  
  std::vector<unsigned> refcent_def_;
  std::vector<std::string> refcent_string_;
  
  unsigned current_;
  
  StMuDstMaker* muDstMaker_;
  StMuDst* muDst_;
  StMuEvent* muInputEvent_;
  
  StMiniMcEvent* event_;
  
  std::vector<std::vector<TH3D*>> mc_;
  std::vector<std::vector<TH3D*>> matched_;
  
  TH1D* refmult_;
  TH1D* zdcRate_;
  
  TH2D* nMCvsMatched_;
  
  axisDef lumi_axis_;
  axisDef cent_axis_;
  axisDef pt_axis_;
  axisDef eta_axis_;
  axisDef phi_axis_;
  
  ClassDef(StEfficiencyMaker,1)
};

#endif // STEFFICIENCYMAKER__HH
