// author: Mike Schmitt, University of Florida
// first version 11/7/2007

#include "FWCore/Framework/interface/MakerMacros.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlock.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlockElement.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecTrack.h"
#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <cmath>

class PFTester : public DQMEDAnalyzer {
public:
  explicit PFTester(const edm::ParameterSet&);

protected:
  void bookHistograms(DQMStore::IBooker&, edm::Run const&, edm::EventSetup const&) override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<reco::PFCandidateCollection> PFCandToken_;

  MonitorElement* h_PFCandEt_;
  MonitorElement* h_PFCandEta_;
  MonitorElement* h_PFCandPhi_;
  MonitorElement* h_PFCandCharge_;
  MonitorElement* h_PFCandPdgId_;
  MonitorElement* h_PFCandType_;

  MonitorElement* h_NumElements_;
  MonitorElement* h_NumTrackElements_;
  MonitorElement* h_NumMuonElements_;
  MonitorElement* h_NumPS1Elements_;
  MonitorElement* h_NumPS2Elements_;
  MonitorElement* h_NumECALElements_;
  MonitorElement* h_NumHCALElements_;
  MonitorElement* h_NumHGCALElements_;

  MonitorElement* h_TrackCharge_;
  MonitorElement* h_TrackNumPoints_;
  MonitorElement* h_TrackNumMeasurements_;
  MonitorElement* h_TrackImpactParameter_;

  MonitorElement* h_NumPFClusters_;
  MonitorElement* h_PFClusterE_;
  MonitorElement* h_PFClusterEta_;
  MonitorElement* h_PFClusterPhi_;
  MonitorElement* h_PFClusterDepth_;
  MonitorElement* h_PFClusterNHits_;
  MonitorElement* h_PFClusterType_;
  MonitorElement* h_PFClusterHitFraction_;
  MonitorElement* h_PFClusterHitDetId_;

};

PFTester::PFTester(const edm::ParameterSet &iConfig) 
    : PFCandToken_(consumes<reco::PFCandidateCollection>(iConfig.getParameter<edm::InputTag>("PFCand"))) {}

void PFTester::bookHistograms(DQMStore::IBooker& ibook, edm::Run const&, edm::EventSetup const&) {

  ibook.setCurrentFolder("HLT/ParticleFlow/PFCandidates");
  h_PFCandEt_ = ibook.book1D("PFCandEt", "PFCandEt", 1000, 0, 1000);
  h_PFCandEta_ = ibook.book1D("PFCandEta", "PFCandEta", 200, -5, 5);
  h_PFCandPhi_ = ibook.book1D("PFCandPhi", "PFCandPhi", 200, -M_PI, M_PI);
  h_PFCandCharge_ = ibook.book1D("PFCandCharge", "PFCandCharge", 5, -2, 2);
  h_PFCandPdgId_ = ibook.book1D("PFCandPdgId", "PFCandPdgId", 44, -22, 22);
  h_PFCandType_ = ibook.book1D("PFCandidateType", "PFCandidateType", 10, 0, 10);

  ibook.setCurrentFolder("HLT/ParticleFlow/PFBlocks");
  h_NumElements_ = ibook.book1D("NumElements", "NumElements", 25, 0, 25);
  h_NumTrackElements_ = ibook.book1D("NumTrackElements", "NumTrackElements", 5, 0, 5);
  h_NumMuonElements_ = ibook.book1D("NumMuonElements", "NumMuonElements", 5, 0, 5);
  h_NumPS1Elements_ = ibook.book1D("NumPS1Elements", "NumPS1Elements", 5, 0, 5);
  h_NumPS2Elements_ = ibook.book1D("NumPS2Elements", "NumPS2Elements", 5, 0, 5);
  h_NumECALElements_ = ibook.book1D("NumECALElements", "NumECALElements", 5, 0, 5);
  h_NumHCALElements_ = ibook.book1D("NumHCALElements", "NumHCALElements", 5, 0, 5);
  h_NumHGCALElements_ = ibook.book1D("NumHGCALElements", "NumHGCALElements", 5, 0, 5);

  ibook.setCurrentFolder("HLT/ParticleFlow/PFTracks");
  h_TrackCharge_ = ibook.book1D("TrackCharge", "TrackCharge", 5, -2, 2);
  h_TrackNumPoints_ = ibook.book1D("TrackNumPoints", "TrackNumPoints", 100, 0, 100);
  h_TrackNumMeasurements_ = ibook.book1D("TrackNumMeasurements", "TrackNumMeasurements", 100, 0, 100);
  h_TrackImpactParameter_ = ibook.book1D("TrackImpactParameter", "TrackImpactParameter", 1000, 0, 1);

  ibook.setCurrentFolder("HLT/ParticleFlow/PFClusters");
  h_NumPFClusters_ = ibook.book1D("NumPFClusters", "Number of PFClusters per PFCandidate", 25, 0, 25);
  h_PFClusterE_ = ibook.book1D("PFClusterE", "PFCluster Energy;E [GeV]", 100, 0, 100);
  h_PFClusterEta_ = ibook.book1D("PFClusterEta", "PFCluster Eta;#eta", 120, -6, 6);
  h_PFClusterPhi_ = ibook.book1D("PFClusterPhi", "PFCluster Phi;#phi", 128, -3.2, 3.2);
  h_PFClusterDepth_ = ibook.book1D("PFClusterDepth", "PFCluster Depth;Depth", 10, 0, 10);
  h_PFClusterNHits_ = ibook.book1D("PFClusterNHits", "PFCluster Number of Hits", 100, 0, 100);
  h_PFClusterType_ = ibook.book1D("PFClusterEtaWidth", "PFCluster Eta Width;#sigma_{#eta}", 20, 0, 20);
  h_PFClusterHitFraction_ = ibook.book1D("PFClusterHitFraction", "PFCluster Hit Fraction;Fraction", 100, 0.0, 1.1);
  h_PFClusterHitDetId_ = ibook.book1D("PFClusterHitDetId", "PFCluster Hit DetId modulo 10000;DetId mod 10000", 100, 0, 10000);

}

void PFTester::analyze(const edm::Event &iEvent, const edm::EventSetup &) {

  const reco::PFCandidateCollection *pf_candidates; 
  edm::Handle<reco::PFCandidateCollection> PFCand;
  iEvent.getByToken(PFCandToken_, PFCand);
  pf_candidates = PFCand.product();

  if (!pf_candidates) {
    edm::LogInfo("OutputInfo") << " Failed to retrieve data required by PFTester.cc";
    return;
  }

  // Loop Over Particle Flow Candidates
  for (const auto& particle : *pf_candidates) {

    h_PFCandEt_->Fill(particle.et());
    h_PFCandEta_->Fill(particle.eta());
    h_PFCandPhi_->Fill(particle.phi());
    h_PFCandCharge_->Fill(particle.charge());
    h_PFCandPdgId_->Fill(particle.pdgId());
    h_PFCandType_->Fill(particle.particleId());

    // Get the PFBlock and Elements
    const reco::PFCandidate::ElementsInBlocks& elementsInBlocks = particle.elementsInBlocks();
    int numElements = elementsInBlocks.size();
    int numTrackElements = 0;
    int numMuonElements = 0;
    int numPS1Elements = 0;
    int numPS2Elements = 0;
    int numECALElements = 0;
    int numHCALElements = 0;
    int numHGCALElements = 0;
    int numPFClusters = 0;

    // Loop over Elements in Block
    for (const auto& elemBlockPair : elementsInBlocks) {
      reco::PFBlockRef blockRef = elemBlockPair.first;
      unsigned elementIndex = elemBlockPair.second;
      const reco::PFBlockElement& element = blockRef->elements()[elementIndex];
      int element_type = element.type();

      // Element is a Tracker Track
      if (element_type == reco::PFBlockElement::TRACK) {

        // Get General Information about the Track
        reco::PFRecTrack track = *(element.trackRefPF());
        h_TrackCharge_->Fill(track.charge());
        h_TrackNumPoints_->Fill(track.nTrajectoryPoints());
        h_TrackNumMeasurements_->Fill(track.nTrajectoryMeasurements());

        // Loop Over Points in the Track
        std::vector<reco::PFTrajectoryPoint> points = track.trajectoryPoints();
        std::vector<reco::PFTrajectoryPoint>::iterator point;
        for (point = points.begin(); point != points.end(); point++) {
          int point_layer = point->layer();
          double x = point->position().x();
          double y = point->position().y();
          double z = point->position().z();
          if (point_layer == reco::PFTrajectoryPoint::ClosestApproach) {
            h_TrackImpactParameter_->Fill(sqrt(x*x + y*y + z*z)); // [FIXME]
          }
        }
        numTrackElements++;
      } 
      else if (element_type == reco::PFBlockElement::MUON) {
        numMuonElements++; // Element is a Muon Track
      } else { 
        if (element_type == reco::PFBlockElement::PS1) numPS1Elements++; // Element is a PreShower1 Cluster
        if (element_type == reco::PFBlockElement::PS2) numPS2Elements++; // Element is a PreShower2 Cluster
        if (element_type == reco::PFBlockElement::ECAL) numECALElements++; // Element is an ECAL Cluster
        if (element_type == reco::PFBlockElement::HCAL) numHCALElements++; // Element is a HCAL Cluster
        if (element_type == reco::PFBlockElement::HGCAL) numHGCALElements++; // Element is a HGCAL Cluster

        if (element.clusterRef().isNonnull()) {
          auto const& cluster = *(element.clusterRef());
          numPFClusters++;
          h_PFClusterE_->Fill(cluster.energy());
          h_PFClusterEta_->Fill(cluster.eta());
          h_PFClusterPhi_->Fill(cluster.phi());
          h_PFClusterDepth_->Fill(cluster.depth());
          h_PFClusterNHits_->Fill(cluster.recHitFractions().size());
          h_PFClusterType_->Fill(element_type);
          for (const auto& hitFracPair : cluster.hitsAndFractions()) {
            DetId hitId = hitFracPair.first;
            float fraction = hitFracPair.second;
            h_PFClusterHitFraction_->Fill(fraction);
            h_PFClusterHitDetId_->Fill(hitId.rawId() % 10000); // modulo for visualization
          }
        }
      }
    }

    // Fill the Respective Elements Sizes
    h_NumElements_->Fill(numElements);
    h_NumPFClusters_->Fill(numPFClusters);
    h_NumTrackElements_->Fill(numTrackElements);
    h_NumMuonElements_->Fill(numMuonElements);
    h_NumPS1Elements_->Fill(numPS1Elements);
    h_NumPS2Elements_->Fill(numPS2Elements);
    h_NumECALElements_->Fill(numECALElements);
    h_NumHCALElements_->Fill(numHCALElements);
    h_NumHGCALElements_->Fill(numHGCALElements);

  }
}

DEFINE_FWK_MODULE(PFTester);