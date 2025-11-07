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

#include "DataFormats/ParticleFlowReco/interface/PFRecHit.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecHitFwd.h"
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
  edm::EDGetTokenT<reco::PFRecHitCollection> recHitToken_;
  edm::EDGetTokenT<std::vector<PCaloHit>> simHitToken_;
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
								  
  UMap<unsigned> nHits_;
  UMap<std::vector<float>> energies_;
  UMap<std::vector<unsigned>> detids_;
};

EcalGeometryAnalyzer::EcalGeometryAnalyzer(const edm::ParameterSet& iConfig)
  : caloGeomToken_(esConsumes<CaloGeometry, CaloGeometryRecord>()),
	recHitToken_(consumes<reco::PFRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHits"))),
	simHitToken_(consumes<std::vector<PCaloHit>>(iConfig.getParameter<edm::InputTag>("simHits"))) {
  edm::Service<TFileService> fs;
  geomTree_ = fs->make<TTree>("Geometry", "Geometry data");
  eventTree_ = fs->make<TTree>("Event", "Event data");
}

void EcalGeometryAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("recHits", edm::InputTag("hltParticleFlowRecHitECALUnseeded"));
  desc.add<edm::InputTag>("simHits", edm::InputTag("g4SimHits", "EcalHitsEB"));
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
	eventTree_->Branch(("nHits" + prefix).c_str(), &nHits_[prefix]);
	eventTree_->Branch(("energies" + prefix).c_str(), &energies_[prefix]);
	eventTree_->Branch(("detids" + prefix).c_str(), &detids_[prefix]);
  }
}

// check the detid lies in the ECAL barrel
double EcalGeometryAnalyzer::inBarrel(const DetId& id) {
  return id.det() == DetId::Ecal && id.subdetId() == EcalBarrel;
}

double EcalGeometryAnalyzer::distFromCenter(GlobalPoint point) {
  return std::sqrt(point.x()*point.x() + point.y()*point.y());
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
	  if(did.subdetId() != EcalBarrel) {
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
  iEvent.getByToken(recHitToken_, recHits_);
  if (!recHits_.isValid()) {
    edm::LogInfo("EcalGeometryAnalyzer") << "Input recHit collection not found.";
    return;
  }
  edm::Handle<std::vector<PCaloHit>> simHits_;
  iEvent.getByToken(simHitToken_, simHits_);
  if (!simHits_.isValid()) {
    edm::LogInfo("EcalGeometryAnalyzer") << "Input simHit collection not found.";
    return;
  }

  auto recHits = *recHits_;
  auto simHits = *simHits_;

  // Event fill
  nHits_["Reco"] = recHits.size();
  for (auto& rechit : recHits) {
    DetId id(rechit.detId());
    if (!inBarrel(id))
      continue;
    detids_["Reco"].push_back(rechit.detId());
    energies_["Reco"].push_back(rechit.energy());
  }

  nHits_["Sim"] = simHits.size();
  for (auto& simhit : simHits) {
    DetId id(simhit.id());
    if (!inBarrel(id))
      continue;
    detids_["Sim"].push_back(simhit.id());
    energies_["Sim"].push_back(simhit.energy());
  }

  eventTree_->Fill();
}

DEFINE_FWK_MODULE(EcalGeometryAnalyzer);
