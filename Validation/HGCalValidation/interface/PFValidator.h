#ifndef HGCalValidator_h
#define HGCalValidator_h

/** \class HGCalValidator
 *  Class that produces histograms to validate HGCal Reconstruction performances
 *
 *  \author HGCal
 */
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"

#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"
#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"

#include "DQMServices/Core/interface/DQMGlobalEDAnalyzer.h"

#include "Validation/HGCalValidation/interface/PFVHistoProducerAlgo.h"
#include "Validation/HGCalValidation/interface/CaloParticleSelector.h"
#include "RecoLocalCalo/HGCalRecProducers/interface/HGCalClusteringAlgoBase.h"

#include "SimDataFormats/Associations/interface/LayerClusterToCaloParticleAssociator.h"
#include "SimDataFormats/Associations/interface/LayerClusterToSimClusterAssociator.h"
#include "SimDataFormats/Associations/interface/TICLAssociationMap.h"

#include "DataFormats/HGCalReco/interface/MultiVectorManager.h"

class PileupSummaryInfo;

struct PFValidatorHistograms {
  PFVHistoProducerAlgoHistograms histoProducerAlgo;
  // PFCandidateValidatorHistograms histoPFCandidates;
  std::vector<dqm::reco::MonitorElement*> h_recoclusters_coll;
};

class PFValidator : public DQMGlobalEDAnalyzer<PFValidatorHistograms> {
public:
  using Histograms = PFValidatorHistograms;
  using SimClusterToCaloParticleMap =
      ticl::AssociationMap<ticl::oneToOneMapWithFraction, std::vector<SimCluster>, std::vector<CaloParticle>>;

  /// Constructor
  PFValidator(const edm::ParameterSet& pset);

  /// Destructor
  ~PFValidator() override;

  /// Method called once per event
  void dqmAnalyze(const edm::Event&, const edm::EventSetup&, const Histograms&) const override;
  /// Method called to book the DQM histograms
  void bookHistograms(DQMStore::IBooker&, edm::Run const&, edm::EventSetup const&, Histograms&) const override;

  void cpParametersAndSelection(const Histograms& histograms,
                                std::vector<CaloParticle> const& cPeff,
                                std::vector<SimVertex> const& simVertices,
                                std::vector<size_t>& selected_cPeff,
                                unsigned int layers,
                                std::unordered_map<DetId, const unsigned int> const&,
                                MultiVectorManager<HGCRecHit> const& hits) const;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

protected:
  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeomToken_;
  edm::InputTag label_rcl;
  edm::InputTag associator_;
  edm::InputTag associatorSim_;
  const bool SaveGeneralInfo_;
  const bool doCaloParticlePlots_;
  const bool doCaloParticleSelection_;
  const bool doSimClustersPlots_;
  edm::InputTag label_SimClustersPlots_, label_SimClustersLevel_;
  const bool doRecoClustersPlots_;
  edm::InputTag label_recoClustersPlots_, label_RCToCPLinking_;
  std::string label_TS_, label_TSbyHitsCP_, label_TSbyHits_, label_TSbyLCsCP_, label_TSbyLCs_;
  std::vector<edm::InputTag> label_clustersmask;
  const bool doCandidatesPlots_;
  std::string label_candidates_;
  const edm::FileInPath cummatbudinxo_;

  std::vector<edm::EDGetTokenT<reco::PFClusterCollection>> labelToken;
  edm::EDGetTokenT<std::vector<SimCluster>> simClusters_;
  edm::EDGetTokenT<reco::PFClusterCollection> recoClusters_;
  edm::EDGetTokenT<std::vector<CaloParticle>> label_cp_effic;
  edm::EDGetTokenT<std::vector<CaloParticle>> label_cp_fake;
  edm::EDGetTokenT<std::vector<SimVertex>> simVertices_;
  std::vector<edm::EDGetTokenT<std::vector<float>>> clustersMaskTokens_;
  edm::EDGetTokenT<std::unordered_map<DetId, const unsigned int>> hitMap_;
  edm::EDGetTokenT<ticl::RecoToSimCollectionT<reco::PFClusterCollection>> associatorMapRtS;
  edm::EDGetTokenT<ticl::SimToRecoCollectionT<reco::PFClusterCollection>> associatorMapStR;
  edm::EDGetTokenT<ticl::SimToRecoCollectionWithSimClustersT<reco::PFClusterCollection>> associatorMapSimtR;
  edm::EDGetTokenT<ticl::RecoToSimCollectionWithSimClustersT<reco::PFClusterCollection>> associatorMapRtSim;
  std::unique_ptr<PFVHistoProducerAlgo> histoProducerAlgo_;
  std::vector<edm::InputTag> hits_label_;
  std::vector<edm::EDGetTokenT<HGCRecHitCollection>> hits_tokens_;
  // std::unique_ptr<PFCandidateValidator> candidateVal_;

private:
  CaloParticleSelector cpSelector;
  std::shared_ptr<hgcal::RecHitTools> tools_;
  std::map<double, double> cumulative_material_budget;
  std::vector<int> particles_to_monitor_;
  unsigned totallayers_to_monitor_;
  std::vector<int> thicknesses_to_monitor_;
  std::string dirName_;
};

#endif
