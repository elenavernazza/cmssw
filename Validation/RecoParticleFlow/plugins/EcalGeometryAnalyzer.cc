#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/CaloTopology/interface/EcalBarrelTopology.h"
#include "Geometry/CaloTopology/interface/EcalEndcapTopology.h"

#include "Geometry/CaloGeometry/interface/EZArrayFL.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"
#include "DataFormats/ParticleFlowReco/interface/PFClusterFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecHit.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecHitFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"

#include <iostream>
#include <array>
#include "TTree.h"

class EcalGeometryAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit EcalGeometryAnalyzer(const edm::ParameterSet&);
  ~EcalGeometryAnalyzer() override {}

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(edm::Event const&, edm::EventSetup const&) override;
  void endJob() override {}
  double inBarrel(const DetId& id);
  double distFromCenter(GlobalPoint point);

  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeomToken_;
  edm::EDGetTokenT<reco::PFRecHitCollection> RecHitToken_;
  edm::EDGetTokenT<reco::PFClusterCollection> PFClusterToken_;
  edm::EDGetTokenT<std::vector<PCaloHit>> SimHitToken_;
  edm::EDGetTokenT<SimClusterCollection> SimClusterToken_;
  TTree *geomTree_, *eventTree_;

  unsigned crystalDetId_;
  float crystalCenterEta_;
  float crystalCenterPhi_;
  float crystalCorner0Eta_;
  float crystalCorner1Eta_;
  float crystalCorner2Eta_;
  float crystalCorner3Eta_;
  float crystalCorner0Phi_;
  float crystalCorner1Phi_;
  float crystalCorner2Phi_;
  float crystalCorner3Phi_;

  static constexpr std::array<std::string, 2> prefixes_ = {{"Reco", "Sim"}};
  unsigned eventId_;

  template <typename T>
  using UMap = std::unordered_map<std::string, T>;
								  
  UMap<unsigned> nClusters_;
  UMap<std::vector<unsigned>> nHitsInCluster_;
  UMap<std::vector<float>> clusterEnergy_;
  UMap<std::vector<float>> energies_;
  UMap<std::vector<float>> fractions_;
  UMap<std::vector<unsigned>> detids_;
};

EcalGeometryAnalyzer::EcalGeometryAnalyzer(const edm::ParameterSet& iConfig)
  : caloGeomToken_(esConsumes<CaloGeometry, CaloGeometryRecord>()),
	RecHitToken_(consumes<reco::PFRecHitCollection>(iConfig.getParameter<edm::InputTag>("RecHits"))),
  PFClusterToken_(consumes<reco::PFClusterCollection>(iConfig.getParameter<edm::InputTag>("PFCluster"))),
	SimHitToken_(consumes<std::vector<PCaloHit>>(iConfig.getParameter<edm::InputTag>("SimHits"))),
  SimClusterToken_(consumes<SimClusterCollection>(iConfig.getParameter<edm::InputTag>("SimCluster"))) {
  edm::Service<TFileService> fs;
  geomTree_ = fs->make<TTree>("Geometry", "Geometry data");
  eventTree_ = fs->make<TTree>("Event", "Event data");
}

void EcalGeometryAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("RecHits", edm::InputTag("hltParticleFlowRecHitECALUnseeded"));
  desc.add<edm::InputTag>("PFCluster", edm::InputTag("hltParticleFlowClusterECALUnseeded"));
  desc.add<edm::InputTag>("SimHits", edm::InputTag("g4SimHits", "EcalHitsEB"));
  desc.add<edm::InputTag>("SimCluster", edm::InputTag("mix", "MergedCaloTruth"));
  descriptions.add("ecalGeometryAnalyzer", desc);
}

void EcalGeometryAnalyzer::beginJob() {
  geomTree_->Branch("crystalDetId", &crystalDetId_);
  geomTree_->Branch("crystalCenterEta", &crystalCenterEta_);
  geomTree_->Branch("crystalCenterPhi", &crystalCenterPhi_);
  geomTree_->Branch("crystalCorner0Eta", &crystalCorner0Eta_);
  geomTree_->Branch("crystalCorner1Eta", &crystalCorner1Eta_);
  geomTree_->Branch("crystalCorner2Eta", &crystalCorner2Eta_);
  geomTree_->Branch("crystalCorner3Eta", &crystalCorner3Eta_);
  geomTree_->Branch("crystalCorner0Phi", &crystalCorner0Phi_);
  geomTree_->Branch("crystalCorner1Phi", &crystalCorner1Phi_);
  geomTree_->Branch("crystalCorner2Phi", &crystalCorner2Phi_);
  geomTree_->Branch("crystalCorner3Phi", &crystalCorner3Phi_);

  eventTree_->Branch("eventId", &eventId_);

  for (auto& prefix : prefixes_) {
	eventTree_->Branch((prefix + "_nClusters").c_str(), &nClusters_[prefix]);
	eventTree_->Branch((prefix + "_clHits").c_str(), &nHitsInCluster_[prefix]);
	eventTree_->Branch((prefix + "_clEnergy").c_str(), &clusterEnergy_[prefix]);
	eventTree_->Branch((prefix + "_energies").c_str(), &energies_[prefix]);
	eventTree_->Branch((prefix + "_fractions").c_str(), &fractions_[prefix]);
	eventTree_->Branch((prefix + "_detids").c_str(), &detids_[prefix]);
  }
}

// check the detid lies in the ECAL barrel
double EcalGeometryAnalyzer::inBarrel(const DetId& id) {
  return id.det() == DetId::Ecal && id.subdetId() == EcalBarrel;
}

double EcalGeometryAnalyzer::distFromCenter(GlobalPoint point) {
  return std::sqrt(point.x() * point.x() + point.y() * point.y());
}

void EcalGeometryAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Get the ECAL geometry
  const auto& caloGeom = iSetup.getData(caloGeomToken_);
  const auto& barrelGeom = caloGeom.getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const std::vector<DetId> detids = barrelGeom->getValidDetIds();

  unsigned eventId = iEvent.id().event();
  eventId_ = eventId;

  // Geometry fill
  if (eventId == 1) {
    for (auto& did : detids) {
      if (did.subdetId() != EcalBarrel) {
        throw std::runtime_error("Error");
        continue;
      }

      const CaloCellGeometry* cellGeom = barrelGeom->getGeometry(did);
      crystalDetId_ = did.rawId();
      crystalCenterEta_ = cellGeom->getPosition().eta();
      crystalCenterPhi_ = cellGeom->getPosition().phi();

      const EZArrayFL<GlobalPoint> corners = cellGeom->getCorners();
      crystalCorner0Eta_ = corners[0].eta();
      crystalCorner1Eta_ = corners[1].eta();
      crystalCorner2Eta_ = corners[2].eta();
      crystalCorner3Eta_ = corners[3].eta();
      crystalCorner0Phi_ = corners[0].phi();
      crystalCorner1Phi_ = corners[1].phi();
      crystalCorner2Phi_ = corners[2].phi();
      crystalCorner3Phi_ = corners[3].phi();

      geomTree_->Fill();
    }
  }  // if (eventId == 1)

  edm::Handle<reco::PFRecHitCollection> recHits_;
  iEvent.getByToken(RecHitToken_, recHits_);
  if (!recHits_.isValid()) {
    edm::LogInfo("EcalGeometryAnalyzer") << "Input recHit collection not found.";
    return;
  }
  edm::Handle<reco::PFClusterCollection> pfClusters_;
  iEvent.getByToken(PFClusterToken_, pfClusters_);
  if (!pfClusters_.isValid()) {
    edm::LogInfo("PFTester") << "Input PFCluster collection not found.";
    return;
  }
  edm::Handle<std::vector<PCaloHit>> simHits_;
  iEvent.getByToken(SimHitToken_, simHits_);
  if (!simHits_.isValid()) {
    edm::LogInfo("EcalGeometryAnalyzer") << "Input simHit collection not found.";
    return;
  }
  edm::Handle<SimClusterCollection> simClusters_;
  iEvent.getByToken(SimClusterToken_, simClusters_);
  if (!simClusters_.isValid()) {
    edm::LogInfo("PFTester") << "Input SimCluster collection not found.";
    return;
  }
  
  auto recHits = *recHits_;
  auto recoClusters = *pfClusters_;
  auto simHits = *simHits_;
  auto simClusters = *simClusters_;

  // Clear vectors
  for (auto& prefix : prefixes_) {
    nClusters_[prefix] = 0;
    nHitsInCluster_[prefix].clear();
    clusterEnergy_[prefix].clear();
    energies_[prefix].clear();
    fractions_[prefix].clear();
    detids_[prefix].clear();
  }

  // Event fill
  float fraction = 0;
  float energy = 0;
  float rec_energy = 0;
  nClusters_["Reco"] = recoClusters.size();
  for (auto& recoCluster : recoClusters) {
    for (const auto& hitFracPair : recoCluster.hitsAndFractions()) {
      DetId hitId = hitFracPair.first;
      if (!inBarrel(hitId))
        continue;
      fraction = hitFracPair.second;
      for (auto& rechit : recHits) {
        DetId id(rechit.detId());
        if (hitId == id) {
          energy = rechit.energy();
          rec_energy += energy;
          break;
        }
      }
      nHitsInCluster_["Reco"].push_back(recoCluster.recHitFractions().size());
      clusterEnergy_["Reco"].push_back(rec_energy);
      energies_["Reco"].push_back(energy);
      fractions_["Reco"].push_back(fraction);
      detids_["Reco"].push_back(hitId);
    }
  }

  float sim_energy = 0;
  nClusters_["Sim"] = simClusters.size();
  for (auto& simCluster : simClusters) {
    const auto& hits_fractions = simCluster.hits_and_fractions();
    const auto& hits_energies = simCluster.hits_and_energies();
    auto itF = hits_fractions.begin();
    auto itE = hits_energies.begin();
    for (; itF != hits_fractions.end() && itE != hits_energies.end(); ++itF, ++itE) {
      DetId hitId = itF->first;
      if (!inBarrel(hitId))
        continue;
      sim_energy += itE->second*itF->second;
      nHitsInCluster_["Sim"].push_back(simCluster.hits_and_fractions().size());
      energies_["Sim"].push_back(itE->second);
      fractions_["Sim"].push_back(itF->second);
      detids_["Sim"].push_back(hitId);
    }
    clusterEnergy_["Sim"].resize(simCluster.hits_and_fractions().size(), sim_energy);
  }

  eventTree_->Fill();
}

DEFINE_FWK_MODULE(EcalGeometryAnalyzer);
