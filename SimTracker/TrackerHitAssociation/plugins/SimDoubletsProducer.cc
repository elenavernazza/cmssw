#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHit.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/IndexSet.h"
#include "FWCore/Utilities/interface/ESGetToken.h"

#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonTopologies/interface/SimplePixelTopology.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"

#include "SimTracker/Common/interface/TrackingParticleSelector.h"            // include the selector for TrackingParticles
#include "SimTracker/TrackerHitAssociation/interface/ClusterTPAssociation.h" // include cluster to TrackingParticle association

#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"   // includes input data format: RecHit collection 
#include "DataFormats/TrackerRecHit2D/interface/OmniClusterRef.h"            // include OmniClusterRef
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"      // includes input data format: TrackingParticleCollection, TrackingParticle

#include "SimDataFormats/TrackingAnalysis/interface/SimDoublets.h"

#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>
#include <memory>
#include <typeinfo>





/** @brief Produces SimDoublets (MC-info based PixelRecHit doublets) for selected TrackingParticles.
 *
 * DESCRIPTION FIXME
 *
 * @author Jan Schulz (jan.gerrit.schulz@cern.ch)
 * @date Dec 2024
 */
class SimDoubletsProducer : public edm::stream::EDProducer<> {
public:
  explicit SimDoubletsProducer(const edm::ParameterSet&);
  static void fillDescriptions(edm::ConfigurationDescriptions&);

  void produce(edm::Event&, const edm::EventSetup&) override;
  void beginRun(const edm::Run&, const edm::EventSetup&) override;

private:
  TrackingParticleSelector trackingParticleSelector;
  const TrackerGeometry* trackerGeometry_ = nullptr;
  const TrackerTopology* trackerTopology_ = nullptr;

  const edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geometry_getToken_;
  const edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> topology_getToken_;
  const edm::EDGetTokenT<ClusterTPAssociation> clusterTPAssociation_getToken_;
  const edm::EDGetTokenT<TrackingParticleCollection> trackingParticles_getToken_;
  const edm::EDGetTokenT<SiPixelRecHitCollection> pixelRecHits_getToken_;
  const edm::EDPutTokenT<SimDoubletsCollection> simDoublets_putToken_;
};





SimDoubletsProducer::SimDoubletsProducer(const edm::ParameterSet& pSet)
    : geometry_getToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord, edm::Transition::BeginRun>()),
      topology_getToken_(esConsumes<TrackerTopology, TrackerTopologyRcd, edm::Transition::BeginRun>()),
      clusterTPAssociation_getToken_(consumes<ClusterTPAssociation>(pSet.getParameter<edm::InputTag>("clusterTPAssociationSrc"))),
      trackingParticles_getToken_(consumes(pSet.getParameter<edm::InputTag>("trackingParticleSrc"))),
      pixelRecHits_getToken_(consumes(pSet.getParameter<edm::InputTag>("pixelRecHitSrc"))),
      simDoublets_putToken_(produces<SimDoubletsCollection>()) {

  // initialize the selector for TrackingParticles used to create SimHitDoublets
  const edm::ParameterSet& pSetTPSel = pSet.getParameter<edm::ParameterSet>("TrackingParticleSelectionConfig");
  trackingParticleSelector = TrackingParticleSelector(pSetTPSel.getParameter<double>("ptMin"),
                                                      pSetTPSel.getParameter<double>("ptMax"),
                                                      pSetTPSel.getParameter<double>("minRapidity"),
                                                      pSetTPSel.getParameter<double>("maxRapidity"),
                                                      pSetTPSel.getParameter<double>("tip"),
                                                      pSetTPSel.getParameter<double>("lip"),
                                                      pSetTPSel.getParameter<int>("minHit"),
                                                      pSetTPSel.getParameter<bool>("signalOnly"),
                                                      pSetTPSel.getParameter<bool>("intimeOnly"),
                                                      pSetTPSel.getParameter<bool>("chargedOnly"),
                                                      pSetTPSel.getParameter<bool>("stableOnly"),
                                                      pSetTPSel.getParameter<std::vector<int>>("pdgId"),
                                                      pSetTPSel.getParameter<bool>("invertRapidityCut"),
                                                      pSetTPSel.getParameter<double>("minPhi"),
                                                      pSetTPSel.getParameter<double>("maxPhi"));
}




void SimDoubletsProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  
  // sources for cluster-TrackingParticle association, TrackingParticles and RecHits
  desc.add<edm::InputTag>("clusterTPAssociationSrc", edm::InputTag("hltTPClusterProducer"));
  desc.add<edm::InputTag>("trackingParticleSrc", edm::InputTag("mix", "MergedTrackTruth"));
  desc.add<edm::InputTag>("pixelRecHitSrc", edm::InputTag("hltSiPixelRecHits"));

  // FIXME : maybe move those settings to a separate file
  // parameter set for the selection of TrackingParticles that will be used for SimHitDoublets
  edm::ParameterSetDescription descTPSelector;
  descTPSelector.add<double>("ptMin", 0.005);
  descTPSelector.add<double>("ptMax", 1e100);
  descTPSelector.add<double>("minRapidity", -5.);
  descTPSelector.add<double>("maxRapidity", 5.);
  descTPSelector.add<double>("tip", 100.); // FIXME : for now it's just a random large value
  descTPSelector.add<double>("lip", 100.); // FIXME : for now it's just a random large value
  descTPSelector.add<int>("minHit", 0);
  descTPSelector.add<bool>("signalOnly", true);
  descTPSelector.add<bool>("intimeOnly", true);
  descTPSelector.add<bool>("chargedOnly", true);
  descTPSelector.add<bool>("stableOnly", false);
  descTPSelector.add<std::vector<int>>("pdgId", {});
  descTPSelector.add<bool>("invertRapidityCut", false);
  descTPSelector.add<double>("minPhi", -3.2);
  descTPSelector.add<double>("maxPhi", 3.2);
  desc.add<edm::ParameterSetDescription>("TrackingParticleSelectionConfig", descTPSelector);

  descriptions.addWithDefaultLabel(desc);
}



void SimDoubletsProducer::beginRun(const edm::Run& run, const edm::EventSetup& eventSetup) {
  // get TrackerGeometry and TrackerTopology
  trackerGeometry_ = &eventSetup.getData(geometry_getToken_);
  trackerTopology_ = &eventSetup.getData(topology_getToken_);
}



void SimDoubletsProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

  // get cluster to TrackingParticle association
  ClusterTPAssociation const& clusterTPAssociation = event.get(clusterTPAssociation_getToken_);

  // get the pixel RecHit collection from the event
  edm::Handle<SiPixelRecHitCollection> hits;
  event.getByToken(pixelRecHits_getToken_, hits);
  if (!hits.isValid()){
    return;
  }

  // get TrackingParticles from the event
  edm::Handle<TrackingParticleCollection> trackingParticles;
  event.getByToken(trackingParticles_getToken_, trackingParticles);
  if (!trackingParticles.isValid()){
    return;
  }

  // create collection of SimDoublets
  // each element will correspond to one selected TrackingParticle
  SimDoubletsCollection simDoubletsCollection;
  simDoubletsCollection.reserve(500);

  // loop over TrackingParticles
  for (size_t i = 0; i < trackingParticles->size(); ++i) {
    TrackingParticle const& trackingParticle = trackingParticles->at(i);

    // select reasonable TrackingParticles for the study (e.g., only signal)
    if (trackingParticleSelector(trackingParticle)){
      simDoubletsCollection.push_back(SimDoublets(TrackingParticleRef(trackingParticles, i)));
    }
  }

  // create a set of the keys of the selected TrackingParticles
  edm::IndexSet selectedTrackingParticleKeys;
  selectedTrackingParticleKeys.reserve(simDoubletsCollection.size());
  for (const auto& simDoublets : simDoubletsCollection){
    TrackingParticleRef trackingParticleRef = simDoublets.trackingParticle();
    selectedTrackingParticleKeys.insert(trackingParticleRef.key());
  }


  // loop over pixel RecHit collections of the different pixel modules
  int count_totRecHits = 0;
  int count = 0;
  for (const auto& detSet : *hits) {
    // get layer Id
    unsigned int const layerId = trackerTopology_->getITPixelLayerNumber(detSet.detId());

    // loop over RecHits
    for (auto const& hit : detSet) {
      count_totRecHits++;

      auto range = clusterTPAssociation.equal_range(OmniClusterRef(hit.cluster()));

      // if the RecHit has associated TrackingParticles
      if (range.first != range.second) {
        for (auto assocTrackingParticleIter = range.first; assocTrackingParticleIter != range.second; assocTrackingParticleIter++) {
          const TrackingParticleRef assocTrackingParticle = (assocTrackingParticleIter->second);

          // if the associated TrackingParticle is among the selected ones
          if (selectedTrackingParticleKeys.has(assocTrackingParticle.key())) {
            SiPixelRecHitRef hitRef = edmNew::makeRefTo(hits, &hit);

            // loop over collection of SimDoublets and find the one of the associated TrackingParticle
            for (auto& simDoublets : simDoubletsCollection){
              TrackingParticleRef trackingParticleRef = simDoublets.trackingParticle();
              if (assocTrackingParticle.key() == trackingParticleRef.key()){
                simDoublets.addRecHit(hitRef, layerId);
                count++;
              }
            }
          }
        }
      }
    }  // end loop over RecHits
  }  // end loop over pixel RecHit collections of the different pixel modules

  // loop over collection of SimDoublets and sort the RecHits according to their global position
  for (auto& simDoublets : simDoubletsCollection){
    simDoublets.sortRecHits(trackerGeometry_);
  }
  
  // put the produced simDoublets collection in the event
  event.emplace(simDoublets_putToken_, std::move(simDoubletsCollection));
}


DEFINE_FWK_MODULE(SimDoubletsProducer);