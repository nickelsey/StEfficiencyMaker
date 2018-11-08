
#ifndef STEFFICIENCYMAKERLITE__HH
#define STEFFICIENCYMAKERLITE__HH


#include <string>
#include <vector>
#include <set>

#include "StMaker.h"
#include "StMiniMcEvent/StMiniMcEvent.h"
#include "TChain.h"
#include "TFile.h"
#include "TH3D.h"
#include "TH2D.h"

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

class StEfficiencyMakerLite : public StMaker {
public:
  StEfficiencyMakerLite(TChain* chain, std::string outputFile = "stefficiencymakerlite.root");
  
  ~StEfficiencyMakerLite();
  
  // loads a new chain
  bool LoadTree(TChain* chain);
  
  // set axis bounds
  void SetDefaultAxes();
  void SetCentralityAxis(unsigned n, double low, double high);
  void SetPtAxis(unsigned n, double low, double high);
  void SetEtaAxis(unsigned n, double low, double high);
  
  // set track cuts for matched tracks
  void SetDCAMax(double dca) {maxDCA_ = dca;}
  double DCAMax() const      {return maxDCA_;}
  
  void SetMinFitPoints(unsigned fit) {minFit_ = fit;}
  unsigned MinFitPoints() const      {return minFit_;}
  
  void SetMinFitPointFraction(double frac) {minFitFrac_ = frac;}
  double MinFitPointFraction() const       {return minFitFrac_;}
  
  void AddGeantId(int id)   {geant_ids_.insert(id);}
  std::set<int>& GeantIds() {return geant_ids_;}
  
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
  
  StMiniMcEvent* event_;

  int current_;
  
  TH3D* mc_;
  TH3D* matched_;

  TH1D* test_mc_;
  TH1D* test_match_;
  TH1D* test_ref_;
  
  TH3D* mcPtvsmatchPt_;
  TH2D* nMCvsMatched_;
  TH2D* refzdc_;
  TH2D* refcent_;
  TH2D* fitpt_;
  TH2D* fitptpos_;
  TH2D* fitptfrac_;
  TH1D* fitptmc_;
  TH2D* dcaPt_;
  TH1D* mcPt_;
  TH1D* mcPairPt_;
  TH1D* recoMatchPt_;
  TH1D* commonFrac_;
  
  TH1D* geant_id_;
  TH1D* geant_id_embed_;
  
  axisDef cent_axis_;
  axisDef pt_axis_;
  axisDef eta_axis_;
  
  int minFit_;
  double minFitFrac_;
  double maxDCA_;
  std::set<int> geant_ids_;
  
  ClassDef(StEfficiencyMakerLite,1)
};

#endif // STEFFICIENCYMAKERLITE__HH
