// author: Mike Schmitt, University of Florida
// first version 11/7/2007

#include "FWCore/Framework/interface/MakerMacros.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/RefToBase.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "DataFormats/METReco/interface/GenMET.h"
#include "DataFormats/METReco/interface/GenMETCollection.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlock.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlockElement.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecTrack.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

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
  MonitorElement* h_NumPS1Elements_;
  MonitorElement* h_NumPS2Elements_;
  MonitorElement* h_NumECALElements_;
  MonitorElement* h_NumHCALElements_;
  MonitorElement* h_NumMuonElements_;

  MonitorElement* h_TrackCharge_;
  MonitorElement* h_TrackNumPoints_;
  MonitorElement* h_TrackNumMeasurements_;
  MonitorElement* h_TrackImpactParameter_;

};

PFTester::PFTester(const edm::ParameterSet &iConfig) 
    : PFCandToken_(consumes<reco::PFCandidateCollection>(iConfig.getParameter<edm::InputTag>("PFCand"))) {}

// PFTester::~PFTester() {}

void PFTester::bookHistograms(DQMStore::IBooker& ibook, edm::Run const&, edm::EventSetup const&) {

  ibook.setCurrentFolder("HLT/ParticleFlow/PFValidation/PFCandidates");
  h_PFCandEt_ = ibook.book1D("PFCandEt", "PFCandEt", 1000, 0, 1000);
  h_PFCandEta_ = ibook.book1D("PFCandEta", "PFCandEta", 200, -5, 5);
  h_PFCandPhi_ = ibook.book1D("PFCandPhi", "PFCandPhi", 200, -M_PI, M_PI);
  h_PFCandCharge_ = ibook.book1D("PFCandCharge", "PFCandCharge", 5, -2, 2);
  h_PFCandPdgId_ = ibook.book1D("PFCandPdgId", "PFCandPdgId", 44, -22, 22);
  h_PFCandType_ = ibook.book1D("PFCandidateType", "PFCandidateType", 10, 0, 10);

  ibook.setCurrentFolder("HLT/ParticleFlow/PFValidation/PFBlocks");
  h_NumElements_ = ibook.book1D("NumElements", "NumElements", 25, 0, 25);
  h_NumTrackElements_ = ibook.book1D("NumTrackElements", "NumTrackElements", 5, 0, 5);
  h_NumPS1Elements_ = ibook.book1D("NumPS1Elements", "NumPS1Elements", 5, 0, 5);
  h_NumPS2Elements_ = ibook.book1D("NumPS2Elements", "NumPS2Elements", 5, 0, 5);
  h_NumECALElements_ = ibook.book1D("NumECALElements", "NumECALElements", 5, 0, 5);
  h_NumHCALElements_ = ibook.book1D("NumHCALElements", "NumHCALElements", 5, 0, 5);
  h_NumMuonElements_ = ibook.book1D("NumMuonElements", "NumMuonElements", 5, 0, 5);


  ibook.setCurrentFolder("HLT/ParticleFlow/PFValidation/PFTracks");
  h_TrackCharge_ = ibook.book1D("TrackCharge", "TrackCharge", 5, -2, 2);
  h_TrackNumPoints_ = ibook.book1D("TrackNumPoints", "TrackNumPoints", 100, 0, 100);
  h_TrackNumMeasurements_ = ibook.book1D("TrackNumMeasurements", "TrackNumMeasurements", 100, 0, 100);
  h_TrackImpactParameter_ = ibook.book1D("TrackImpactParameter", "TrackImpactParameter", 1000, 0, 1);

  ibook.setCurrentFolder("HLT/ParticleFlow/PFValidation/PFClusters");
  // to be done

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
  reco::PFCandidateCollection::const_iterator pf;
  for (pf = pf_candidates->begin(); pf != pf_candidates->end(); pf++) {
    const reco::PFCandidate *particle = &(*pf);

    h_PFCandEt_->Fill(particle->et());
    h_PFCandEta_->Fill(particle->eta());
    h_PFCandPhi_->Fill(particle->phi());
    h_PFCandCharge_->Fill(particle->charge());
    h_PFCandPdgId_->Fill(particle->pdgId());
    h_PFCandType_->Fill(particle->particleId());
    // particle->elementsInBlocks();

    // Get the PFBlock and Elements
    // reco::PFBlock block = *(particle->Blocks());
    // edm::OwnVector<reco::PFBlockElement> elements = block.elements();
    const reco::PFCandidate::ElementsInBlocks& elementsInBlocks = particle->elementsInBlocks();
    int numElements = elementsInBlocks.size();
    int numTrackElements = 0;
    int numPS1Elements = 0;
    int numPS2Elements = 0;
    int numECALElements = 0;
    int numHCALElements = 0;
    int numMuonElements = 0;

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
          //switch (point_layer) {
          //case PFTrajectoryPoint::ClosestApproach:
          // Fill the Track's D0
          if (point_layer == reco::PFTrajectoryPoint::ClosestApproach) {
            h_TrackImpactParameter_->Fill(sqrt(x*x + y*y + z*z));
          }
        }
        numTrackElements++;
      }

      // Element is an ECAL Cluster
      else if (element_type == reco::PFBlockElement::ECAL) {
        numECALElements++;
      }
      // Element is a HCAL Cluster
      else if (element_type == reco::PFBlockElement::HCAL) {
        numHCALElements++;
      }
      // Element is a Muon Track
      else if (element_type == reco::PFBlockElement::MUON) {
        numMuonElements++;
      }
      // // Element is a PS1 Cluster
      // else if (element_type == PFBlockElement::PS1) {
      //   numPS1Elements++;
      // }
      // Fill the Respective Elements Sizes
      h_NumElements_->Fill(numElements);
      h_NumTrackElements_->Fill(numTrackElements);
      h_NumPS1Elements_->Fill(numPS1Elements);
      h_NumPS2Elements_->Fill(numPS2Elements);
      h_NumECALElements_->Fill(numECALElements);
      h_NumHCALElements_->Fill(numHCALElements);
      h_NumMuonElements_->Fill(numMuonElements);
    }
  }
}

DEFINE_FWK_MODULE(PFTester);