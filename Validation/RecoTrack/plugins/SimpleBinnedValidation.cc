// system include files
#include <memory>

#include "TTree.h"
#include "TFile.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "SimTracker/TrackerHitAssociation/interface/ClusterTPAssociation.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/Associations/interface/TrackToTrackingParticleAssociator.h"

#include "SimTracker/Common/interface/TrackingParticleSelector.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

using reco::TrackCollection;

class SimpleBinnedValidation : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit SimpleBinnedValidation(const edm::ParameterSet&);
  ~SimpleBinnedValidation() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  TH1D* h_rt_eta;
  TH1D* h_at_eta;
  TH1D* h_st_eta;
  TH1D* h_dt_eta;
  TH1D* h_ast_eta;

  TH1D* h_rt_pt;
  TH1D* h_at_pt;
  TH1D* h_st_pt;
  TH1D* h_dt_pt;
  TH1D* h_ast_pt;

  int global_rt_ = 0;
  int global_at_ = 0;
  int global_st_ = 0;
  int global_dt_ = 0;
  int global_ast_ = 0;

  TrackingParticleSelector tpSelector;
  TTree* output_tree_;
  std::vector<edm::InputTag> trackLabels_;
  edm::EDGetTokenT<ClusterTPAssociation> tpMap_;
  //   edm::EDGetTokenT<std::vector<PileupSummaryInfo>>  infoPileUp_;
  std::vector<edm::EDGetTokenT<edm::View<reco::Track>>> trackTokens_;
  edm::EDGetTokenT<reco::TrackToTrackingParticleAssociator> trackAssociatorToken_;
  edm::EDGetTokenT<TrackingParticleCollection> trackingParticleToken_;

  //   const double sharingFraction_;
  //   const double sharingFractionForTriplets_;
};

SimpleBinnedValidation::SimpleBinnedValidation(const edm::ParameterSet& iConfig)
    : trackLabels_(iConfig.getParameter<std::vector<edm::InputTag>>("trackLabels")),
      // tpMap_(consumes(iConfig.getParameter<edm::InputTag>("tpMap"))),
      //   infoPileUp_(consumes(iConfig.getParameter< edm::InputTag >("infoPileUp"))),
      trackAssociatorToken_(consumes<reco::TrackToTrackingParticleAssociator>(
          iConfig.getUntrackedParameter<edm::InputTag>("trackAssociator"))),
      trackingParticleToken_(
          consumes<TrackingParticleCollection>(iConfig.getParameter<edm::InputTag>("trackingParticles")))
//   sharingFraction_(iConfig.getUntrackedParameter<double>("sharingFraction")),
//   sharingFractionForTriplets_(iConfig.getUntrackedParameter<double>("sharingFractionForTriplets"))
{
  for (auto& itag : trackLabels_) {
    trackTokens_.push_back(consumes<edm::View<reco::Track>>(itag));
    // edm::LogPrint("TrackValidator") << itag.label() << "\n";
  }
  tpSelector = TrackingParticleSelector(iConfig.getParameter<double>("ptMinTP"),
                                        iConfig.getParameter<double>("ptMaxTP"),
                                        iConfig.getParameter<double>("minRapidityTP"),
                                        iConfig.getParameter<double>("maxRapidityTP"),
                                        iConfig.getParameter<double>("tipTP"),
                                        iConfig.getParameter<double>("lipTP"),
                                        iConfig.getParameter<int>("minHitTP"),
                                        iConfig.getParameter<bool>("signalOnlyTP"),
                                        iConfig.getParameter<bool>("intimeOnlyTP"),
                                        iConfig.getParameter<bool>("chargedOnlyTP"),
                                        iConfig.getParameter<bool>("stableOnlyTP"),
                                        iConfig.getParameter<std::vector<int>>("pdgIdTP"),
                                        iConfig.getParameter<bool>("invertRapidityCutTP"));
  //now do what ever initialization is needed
}

SimpleBinnedValidation::~SimpleBinnedValidation() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void SimpleBinnedValidation::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  //   auto const& tpClust = iEvent.get(tpMap_);
  auto const& associatorByHits = iEvent.get(trackAssociatorToken_);

  TrackingParticleRefVector tpCollection;
  edm::Handle<TrackingParticleCollection> TPCollectionH;
  iEvent.getByToken(trackingParticleToken_, TPCollectionH);
  //   auto const& tp = iEvent.get(trackingParticleToken_);

  for (size_t i = 0, size = TPCollectionH->size(); i < size; ++i) {
    auto tp = TrackingParticleRef(TPCollectionH, i);
    if (tpSelector(*tp)) {
      tpCollection.push_back(tp);
    }
  }

  for (const auto& trackToken : trackTokens_) {
    edm::Handle<edm::View<reco::Track>> tracksHandle;
    iEvent.getByToken(trackToken, tracksHandle);
    const edm::View<reco::Track>& tracks = *tracksHandle;

    edm::RefToBaseVector<reco::Track> trackRefs;
    for (edm::View<reco::Track>::size_type i = 0; i < tracks.size(); ++i) {
      trackRefs.push_back(tracks.refAt(i));
    }

    reco::RecoToSimCollection recSimColl = associatorByHits.associateRecoToSim(trackRefs, tpCollection);
    reco::SimToRecoCollection simRecColl = associatorByHits.associateSimToReco(trackRefs, tpCollection);
    int rt = 0;
    int at = 0;
    int ast = 0;
    int dt = 0;
    int st = 0;
    for (const auto& track : trackRefs) {
      h_rt_eta->Fill(track->eta());
      h_rt_pt->Fill(track->pt());
      rt++;
      auto foundTP = recSimColl.find(track);
      if (foundTP != recSimColl.end()) {
        const auto& tp = foundTP->val;
        if (!tp.empty()) {
          h_at_eta->Fill(track->eta());
          h_at_pt->Fill(track->pt());
          at++;
        }
        if (simRecColl.find(tp[0].first) != simRecColl.end()) {
          if (simRecColl[tp[0].first].size() > 1) {
            dt++;
            h_dt_eta->Fill(track->eta());
            h_dt_pt->Fill(track->pt());
            }
        }
      }
    }
    for (const TrackingParticleRef& tpr : tpCollection) {
      h_st_eta->Fill(tpr->eta());
      h_st_pt->Fill(tpr->pt());
      st++;
      auto foundTrack = simRecColl.find(tpr);
      if (foundTrack != simRecColl.end()) {
        h_ast_eta->Fill(tpr->eta());
        h_ast_pt->Fill(tpr->pt());  
        ast++;
      }
    }

    // LogPrint("TrackValidator") << "Tag " << trackLabels_[0].label() << " Total simulated "<< st << " Associated tracks " << at << " Total reconstructed " << rt;
    global_rt_ += rt;
    global_st_ += st;
    global_at_ += at;
    global_dt_ += dt;
    global_ast_ += ast;
  }
}

// ------------ method called once each job just before starting event loop  ------------
void SimpleBinnedValidation::beginJob() {
  // please remove this method if not needed
  edm::Service<TFileService> fs;

  std::vector<double> V_bins_eta = {-4, -1.5, 1.5, 4};
  int n_bins_eta = V_bins_eta.size() - 1;
  double* v_bins_eta = &V_bins_eta[0];

  std::vector<double> V_bins_pt = {0, 3, 10, 1000};
  int n_bins_pt = V_bins_pt.size() - 1;
  double* v_bins_pt = &V_bins_pt[0];

  // Counters used for computing the efficiency are filled with the Tracking Particle variables
  // Counters used for computing the fake and duplicate rate are filled with teh Reco Track variables
  h_st_eta = fs->make<TH1D>("h_st_eta", 
                             " ; Tracking Particle #eta; Number of tracking particles", 
                             n_bins_eta, v_bins_eta);
  h_ast_eta = fs->make<TH1D>("h_ast_eta",
                             " ; Tracking Particle #eta; Number of tracking particles associated to at least a reconstructed track",
                             n_bins_eta, v_bins_eta);
  h_rt_eta = fs->make<TH1D>("h_rt_eta", 
                             " ; Reco Track #eta; Number of reconstructed tracks", 
                             n_bins_eta, v_bins_eta);
  h_dt_eta = fs->make<TH1D>("h_dt_eta",
                             " ; Reco Track #eta; Number of duplicates", 
                             n_bins_eta, v_bins_eta);
  h_at_eta = fs->make<TH1D>("h_at_eta",
                             " ; Reco Track #eta; Number of reconstructed tracks associated to a tracking particle",
                             n_bins_eta, v_bins_eta);

  h_st_pt = fs->make<TH1D>("h_st_pt", 
                            " ; Tracking Particle p_{T}; Number of tracking particles", 
                            n_bins_pt, v_bins_pt);  
  h_ast_pt = fs->make<TH1D>("h_ast_pt",
                            " ; Tracking Particle p_{T}; Number of tracking particles associated to at least a reconstructed track",
                            n_bins_pt, v_bins_pt);
  h_rt_pt = fs->make<TH1D>("h_rt_pt", 
                            " ; Reco Track p_{T}; Number of reconstructed tracks", 
                            n_bins_pt, v_bins_pt);
  h_at_pt = fs->make<TH1D>("h_at_pt", 
                            " ; Reco Track p_{T}; Number of reconstructed tracks associated to a tracking particle",
                             n_bins_pt, v_bins_pt);
  h_dt_pt = fs->make<TH1D>("h_dt_pt", 
                            " ; Reco Track p_{T}; Number of duplicates", 
                            n_bins_pt, v_bins_pt);

  output_tree_ = fs->make<TTree>("output", "putput params");

  output_tree_->Branch("rt", &global_rt_);
  output_tree_->Branch("at", &global_at_);
  output_tree_->Branch("st", &global_st_);
  output_tree_->Branch("dt", &global_dt_);
  output_tree_->Branch("ast", &global_ast_);
}

// ------------ method called once each job just after ending the event loop  ------------
void SimpleBinnedValidation::endJob() {
  // please remove this method if not needed
  output_tree_->Fill();
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void SimpleBinnedValidation::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(SimpleBinnedValidation);
