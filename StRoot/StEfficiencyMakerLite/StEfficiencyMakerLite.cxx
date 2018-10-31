#include "StEfficiencyMakerLite.hh"

#include "St_base/StMessMgr.h"
#include "StMiniMcEvent/StMiniMcPair.h"
#include "StMiniMcEvent/StTinyMcTrack.h"
#include "StMiniMcEvent/StContamPair.h"

#include "TMath.h"

ClassImp(StEfficiencyMakerLite)

StEfficiencyMakerLite::StEfficiencyMakerLite(TChain* mcTree, std::string outputFile) {
  if (!LoadTree(mcTree)) {
    LOG_ERROR << "load chain failed" << endm;
  }
  
  SetDefaultAxes();
  
  current_ = 0;
  
  minFit_ = 20;
  minFitFrac_ = 0.52;
  maxDCA_ = 3.0;
  
  out_ = new TFile(outputFile.c_str(), "RECREATE");
}

StEfficiencyMakerLite::~StEfficiencyMakerLite() {
  
}

int StEfficiencyMakerLite::Init() {
  
  if (InitInput() != kStOK)
    return kStFatal;
  if (InitOutput() != kStOK)
    return kStFatal;
  return kStOK;
}

void StEfficiencyMakerLite::SetDefaultAxes() {
  cent_axis_ = axisDef(9, 0, 9);
  pt_axis_   = axisDef(100, 0.0, 10.0);
  eta_axis_  = axisDef(10, -1.0, 1.0);
}

void StEfficiencyMakerLite::SetCentralityAxis(unsigned n, double low, double high) {
  cent_axis_ = axisDef(n, low, high);
}

void StEfficiencyMakerLite::SetPtAxis(unsigned n, double low, double high) {
  pt_axis_ = axisDef(n, low, high);
}

void StEfficiencyMakerLite::SetEtaAxis(unsigned n, double low, double high) {
  eta_axis_ = axisDef(n, low, high);
}


bool StEfficiencyMakerLite::LoadTree(TChain* chain) {
  if (chain == nullptr) {
    LOG_INFO << "chain does not exist" << endm;
    return false;
  }
  if (chain->GetBranch("StMiniMcEvent") == nullptr) {
    LOG_ERROR << "chain does not contain StMiniMcEvent branch" << endm;
    return false;
  }
  
  chain_ = chain;
  event_ = new StMiniMcEvent;
  
  chain_->SetBranchAddress("StMiniMcEvent", &event_);
  chain_->GetEntry(0);
  return true;
}

bool StEfficiencyMakerLite::CheckAxes() {
  return cent_axis_.valid() && pt_axis_.valid() && eta_axis_.valid();
}

Int_t StEfficiencyMakerLite::Make() {
  
  if (event_ == nullptr) {
    LOG_ERROR << "StMiniMcEvent Branch not loaded properly: exiting run loop" << endm;
    return kStFatal;
  }
  if (mc_ == nullptr || matched_ == nullptr) {
    LOG_ERROR << "no histograms for analysis exist: was initialization successful?" << endm;
    return kStFatal;
  }
  // load the miniMC event
  if (LoadEvent() == false) {
    LOG_ERROR << "reached end of chain" << endm;
    return kStErr;
  }
  
  // get centrality
  int centBin = event_->centrality();
  double refmult = event_->nUncorrectedPrimaries();
  
  if (centBin < 0 || centBin > 8)
    return kStOK;
  
  refcent_->Fill(centBin, refmult);
  
  TClonesArray* mc_array = event_->tracks(MC);
  TIter next_mc(mc_array);
  StTinyMcTrack* track = nullptr;
  unsigned count_mc = 0;
  
  while ((track = (StTinyMcTrack*) next_mc())) {
    
    geant_id_->Fill(track->geantId());
    
    if (track->parentGeantId() != 0)
      continue;
    
    if (geant_ids_.size() && geant_ids_.find(track->geantId()) == geant_ids_.end())
      continue;
    
    geant_id_embed_->Fill(track->geantId());
    
    fitptmc_->Fill(track->nHitMc());
    
    mcPt_->Fill(track->ptMc());
    count_mc++;
    
    mc_->Fill(track->ptMc(), track->etaMc(), centBin);
  }

  TClonesArray* match_array = event_->tracks(MATCHED);
  TIter next_match(match_array);
  StMiniMcPair* pair = nullptr;
  unsigned count_pair = 0;
  
  while ((pair = (StMiniMcPair*) next_match())) {
    
    int pairGeantId = pair->geantId();
    int parentGeantId = pair->parentGeantId();
    double globalDCA = pair->dcaGl();
    int pairFitPts = pair->fitPts() + 1;
    int pairPossibleFitPts = pair->nPossiblePts() + 1;
    double pairFitFrac = (double) pairFitPts / (double) pairPossibleFitPts;
    
    if (parentGeantId != 0)
      continue;
    
    if (geant_ids_.size() && geant_ids_.find(pairGeantId) == geant_ids_.end())
      continue;
    
    if (globalDCA > maxDCA_ || pairFitPts < minFit_)
      continue;
    
    if (pairFitFrac < minFitFrac_)
      continue;
    
    count_pair++;
    mcPairPt_->Fill(pair->ptMc());
    commonFrac_->Fill(pair->commonFrac());
    
    mcPtvsmatchPt_->Fill(pair->ptMc(), pair->ptPr(), centBin);
    recoMatchPt_->Fill(pair->ptPr());
    fitpt_->Fill(pair->ptPr(), pairFitPts);
    fitptpos_->Fill(pair->ptPr(), pairPossibleFitPts);
    fitptfrac_->Fill(pair->ptPr(), pairFitFrac);
    dcaPt_->Fill(globalDCA, pair->ptPr());
    matched_->Fill(pair->ptPr(), pair->etaPr(), centBin);
  }
  
  nMCvsMatched_->Fill(count_mc, count_pair);
  
  return kStOK;
}

Int_t StEfficiencyMakerLite::Finish() {
  if (out_ == nullptr) {
    out_ = new TFile("stefficiencymakerlite.root", "RECREATE");
  }
  
  out_->cd();
  
  mc_->Write();
  matched_->Write();
  mcPtvsmatchPt_->Write();
  nMCvsMatched_->Write();
  refcent_->Write();
  fitpt_->Write();
  fitptpos_->Write();
  fitptfrac_->Write();
  fitptmc_->Write();
  dcaPt_->Write();
  mcPairPt_->Write();
  mcPt_->Write();
  recoMatchPt_->Write();
  commonFrac_->Write();
  geant_id_embed_->Write();
  geant_id_->Write();
  
  out_->Close();
  return kStOk;
}


int StEfficiencyMakerLite::InitInput() {
  return kStOK;
}

int StEfficiencyMakerLite::InitOutput() {
  if (!CheckAxes()) {
    LOG_ERROR << "axes not valid: could not initialize histograms";
    return kStFatal;
  }
  
  mc_= new TH3D("mc", ";p_{T};#eta;centrality",
                pt_axis_.nBins, pt_axis_.low, pt_axis_.high,
                eta_axis_.nBins, eta_axis_.low, eta_axis_.high,
                cent_axis_.nBins, cent_axis_.low, cent_axis_.high);
  matched_= new TH3D("match", ";p_{T};#eta;centrality",
                     pt_axis_.nBins, pt_axis_.low, pt_axis_.high,
                     eta_axis_.nBins, eta_axis_.low, eta_axis_.high,
                     cent_axis_.nBins, cent_axis_.low, cent_axis_.high);
  mc_->Sumw2();
  matched_->Sumw2();
  
  mcPtvsmatchPt_ = new TH3D("mcptvsmatchptvseta", ";mc p_{T};match p_{T};#eta", pt_axis_.nBins,
                            pt_axis_.low, pt_axis_.high, pt_axis_.nBins, pt_axis_.low, pt_axis_.high,
                            cent_axis_.nBins, cent_axis_.low, cent_axis_.high);
  mcPtvsmatchPt_->Sumw2();
  nMCvsMatched_ = new TH2D("mcvsmatched", ";mc;matched", 100, 0, 100, 100, 0, 100);
  nMCvsMatched_->Sumw2();
  refcent_ = new TH2D("refcent", ";cent;refmult", 16, 0, 16, 100, 0, 800);
  fitpt_ = new TH2D("fitpoints", ";p_{T};fit points", 50, 0, 5, 50, 0, 50);
  fitpt_->Sumw2();
  fitptpos_ = new TH2D("fitpointspos", ";p_{T};fit points pos", 50, 0, 5, 50, 0, 50);
  fitptpos_->Sumw2();
  fitptfrac_ = new TH2D("fitpointfrac", ";p_{T};fit point fraction", 50, 0, 5, 50, 0, 1);
  fitptfrac_->Sumw2();
  fitptmc_ = new TH1D("fitpointsmc", ";fit points", 50, 0, 50);
  fitptmc_->Sumw2();
  dcaPt_ = new TH2D("dcapt", ";DCA [cm]", 100, 0, 5, 100, 0, 5);
  dcaPt_->Sumw2();
  mcPt_ = new TH1D("mcpt", ";p_T", 100, 0, 5);
  mcPt_->Sumw2();
  mcPairPt_ = new TH1D("mcptpair", ";p_T", 100, 0, 5);
  mcPairPt_->Sumw2();
  recoMatchPt_ = new TH1D("recopt", ";p_T", 100, 0, 5);
  recoMatchPt_->Sumw2();
  commonFrac_ = new TH1D("commonFrac", "", 100, 0, 1);
  commonFrac_->Sumw2();
  
  geant_id_ = new TH1D("geantid", ";geant ID", 20, 0, 20);
  geant_id_embed_ = new TH1D("geantidembed", ";geant ID", 20, 0, 20);
  return kStOK;
}

bool StEfficiencyMakerLite::LoadEvent() {

  if (current_ >= chain_->GetEntries())
    return false;
    
  chain_->GetEntry(current_);
  current_++;
  return true;
}


