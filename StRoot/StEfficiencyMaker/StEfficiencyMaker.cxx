#include "StEfficiencyMaker.hh"

#include "St_base/StMessMgr.h"
#include "StMiniMcEvent/StMiniMcPair.h"
#include "StMiniMcEvent/StTinyMcTrack.h"
#include "StMiniMcEvent/StContamPair.h"

#include "TMath.h"

ClassImp(StEfficiencyMaker)

StEfficiencyMaker::StEfficiencyMaker(TChain* mcTree, std::string outputFile) {
  if (!LoadTree(mcTree)) {
    LOG_ERROR << "load chain failed" << endm;
  }
  
  SetDefaultAxes();
  
  current_ = 0;
  
  refcent_def_ = std::vector<unsigned>{420, 364, 276, 212, 156, 108, 68, 44, 28, 12, 0};
  refcent_string_ = std::vector<std::string>{"0-5%", "5-10%", "10-20%", "20-30%",
                                            "30-40%", "40-50%", "50-60%", "60-70%",
                                            "70-80%", "80-90%", "90-100%"};
  
  
  muDstMaker_ = nullptr;
  muDst_ = nullptr;
  muInputEvent_ = nullptr;
  
  out_ = new TFile(outputFile.c_str(), "RECREATE");
}

StEfficiencyMaker::~StEfficiencyMaker() {
  
}

int StEfficiencyMaker::Init() {
  if (InitInput() != kStOK)
    return kStFatal;
  if (InitOutput() != kStOK)
    return kStFatal;
  return kStOK;
}

void StEfficiencyMaker::SetDefaultAxes() {
  lumi_axis_ = axisDef(3, 0.0, 100.0);
  cent_axis_ = axisDef(11, 0, 1);
  pt_axis_   = axisDef(50, 0.0, 5.0);
  eta_axis_  = axisDef(10, -1.0, 1.0);
  phi_axis_  = axisDef(1, -TMath::Pi(), TMath::Pi());
}

void StEfficiencyMaker::SetLuminosityAxis(unsigned n, double low, double high) {
  lumi_axis_ = axisDef(n, low, high);
}

void StEfficiencyMaker::SetCentralityAxis(unsigned n, double low, double high) {
  cent_axis_ = axisDef(n, low, high);
}

void StEfficiencyMaker::SetPtAxis(unsigned n, double low, double high) {
  pt_axis_ = axisDef(n, low, high);
}

void StEfficiencyMaker::SetEtaAxis(unsigned n, double low, double high) {
  eta_axis_ = axisDef(n, low, high);
}

void StEfficiencyMaker::SetPhiAxis(unsigned n, double low, double high) {
  phi_axis_ = axisDef(n, low, high);
}


bool StEfficiencyMaker::LoadTree(TChain* chain) {
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

bool StEfficiencyMaker::CheckAxes() {
  return lumi_axis_.valid() && cent_axis_.valid() && pt_axis_.valid()
         && eta_axis_.valid() && phi_axis_.valid();
}

Int_t StEfficiencyMaker::Make() {
  if (event_ == nullptr) {
    LOG_ERROR << "StMiniMcEvent Branch not loaded properly: exiting run loop" << endm;
    return kStFatal;
  }
  if (mc_.size() == 0 || mc_[0].size() == 0) {
    LOG_ERROR << "no histograms for analysis exist: was initialization successful?" << endm;
    return kStFatal;
  }
  
  // load the matching miniMC event
  if (LoadEvent() == false) {
    LOG_ERROR << "Could not find miniMC event matching muDST event" << endm;
    return kStErr;
  }
  
  // find centrality and luminosity bin
  unsigned refmult = muInputEvent_->refMult();
  int centBin = -1;
  for (unsigned i = 0; i < refcent_def_.size(); ++i) {
    if (refmult > refcent_def_[i]) {
      centBin = i;
      break;
    }
  }
  
  double zdcAnd = muInputEvent_->runInfo().zdcCoincidenceRate() / 1000.0;
  int zdcBin = -1;
  for (unsigned i = 0; i < lumi_axis_.nBins; ++i) {
    if (zdcAnd > lumi_axis_.low + i * lumi_axis_.width() &&
        zdcAnd <= lumi_axis_.low + (i + 1) * lumi_axis_.width()) {
      zdcBin = i;
      break;
    }
  }
  
  if (zdcBin < 0 || centBin < 0)
    return kStOK;
  
  refzdc_->Fill(refmult, zdcAnd);
  
  // get the proper histograms
  TH3D* mc = mc_[zdcBin][centBin];
  TH3D* match = matched_[zdcBin][centBin];
  
  TClonesArray* mc_array = event_->tracks(MC);
  TIter next_mc(mc_array);
  StTinyMcTrack* track = nullptr;
  TClonesArray* match_array = event_->tracks(MATCHED);
  TIter next_match(match_array);
  StMiniMcPair* pair = nullptr;
  
  unsigned count_mc = 0;
  unsigned count_pair = 0;
  while ((track = (StTinyMcTrack*) next_mc())) {
    count_mc++;
    mc->Fill(track->ptMc(), track->etaMc(), track->phiMc());
  }
  
  while ((pair = (StMiniMcPair*) next_match())) {
    count_pair++;
    match->Fill(pair->ptPr(), pair->etaPr(), pair->phiPr());
    
  }
  
  nMCvsMatched_->Fill(count_mc, count_pair);
  
  return kStOK;
}

Int_t StEfficiencyMaker::Finish() {
  if (out_ == nullptr) {
    out_ = new TFile("stefficiencymaker.root", "RECREATE");
  }
  
  out_->cd();
  for (unsigned i = 0; i < mc_.size(); ++i) {
    for (unsigned j = 0; j < mc_[i].size(); ++j) {
      if (mc_[i][j] != nullptr)
        mc_[i][j]->Write();
      if (matched_[i][j] != nullptr)
        matched_[i][j]->Write();
    }
  }
  
  nMCvsMatched_->Write();
  refzdc_->Write();
  
  out_->Close();
  return kStOk;
}


int StEfficiencyMaker::InitInput() {
  muDstMaker_ = (StMuDstMaker*) GetMakerInheritsFrom("StMuDstMaker");
  if (muDstMaker_ == nullptr) {
    LOG_ERROR << "No muDstMaker found in chain: StEfficiencyMaker init failed" << endm;
    return kStFatal;
  }
  return kStOK;
}

int StEfficiencyMaker::InitOutput() {
  if (!CheckAxes()) {
    LOG_ERROR << "axes not valid: could not initialize histograms";
    return kStFatal;
  }
  
  mc_ = std::vector<std::vector<TH3D*>>(lumi_axis_.nBins,
                                        std::vector<TH3D*>(cent_axis_.nBins, nullptr));
  matched_ = std::vector<std::vector<TH3D*>>(lumi_axis_.nBins,
                                             std::vector<TH3D*>(cent_axis_.nBins, nullptr));
  for (unsigned lumi = 0; lumi < lumi_axis_.nBins; ++lumi) {
    for (unsigned cent = 0; cent < cent_axis_.nBins; ++cent) {
      std::string mc_name = "mc_lumi_" + std::to_string(lumi) +
      "_cent_" + std::to_string(cent);
      std::string match_name = "match_lumi_" + std::to_string(lumi) +
      "_cent_" + std::to_string(cent);
      
      mc_[lumi][cent] = new TH3D(mc_name.c_str(), ";p_{T};#eta;#phi",
                                 pt_axis_.nBins, pt_axis_.low, pt_axis_.high,
                                 eta_axis_.nBins, eta_axis_.low, eta_axis_.high,
                                 phi_axis_.nBins, phi_axis_.low, phi_axis_.high);
      matched_[lumi][cent] = new TH3D(match_name.c_str(), ";p_{T};#eta;#phi",
                                      pt_axis_.nBins, pt_axis_.low, pt_axis_.high,
                                      eta_axis_.nBins, eta_axis_.low, eta_axis_.high,
                                      phi_axis_.nBins, phi_axis_.low, phi_axis_.high);
    }
  }
  
  nMCvsMatched_ = new TH2D("mcvsmatched", ";mc;matched", 100, 0, 100, 100, 0, 100);
  refzdc_ = new TH1D("refzdc", ";refmult;zdc Rate [khz]", 200, 0, 800, 100, 0, 100);

  return kStOK;
}

bool StEfficiencyMaker::LoadEvent() {
  muDst_ = muDstMaker_->muDst();
  if (muDst_ == nullptr) {
    LOG_ERROR << "Could not load MuDst" << endm;
    return kStErr;
  }
  muInputEvent_ = muDst_->event();
  if (muInputEvent_ == nullptr) {
    LOG_ERROR << "Could not load MuDstEvent" << endm;
    return kStErr;
  }
  
  int eventID = muInputEvent_->eventId();
  int runID = muInputEvent_->runId();
  
  // now try to match the event to a miniMC event in the chain
  
  int nTries = chain_->GetEntries();
  
  if (event_->eventId() == eventID &&
      event_->runId() == runID)
    return true;
  
  while (nTries >= 0) {
    
    current_++;
    if (current_ >= chain_->GetEntries())
      current_ = 0;
    nTries--;
    
    chain_->GetEntry(current_);
    
    if (event_->eventId() == eventID &&
        event_->runId() == runID)
      return true;
  }
  
  if (nTries < 0) {
    LOG_ERROR << "could not match event to miniMC" << endm;
    return false;
  }
  
  return true;
}


