//
// Original Author:  Leonardo Cristella
//         Created:  Thu Dec  3 10:52:11 CET 2020
//
//

// system include files
#include <memory>
#include <string>

// user include files
#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "SimDataFormats/Associations/interface/LayerClusterToSimClusterAssociator.h"

//
// class decleration
//

template <typename CLUSTER>
class LCToSCAssociatorEDProducer : public edm::global::EDProducer<> {
public:
  explicit LCToSCAssociatorEDProducer(const edm::ParameterSet &);
  ~LCToSCAssociatorEDProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
  void produce(edm::StreamID, edm::Event &, const edm::EventSetup &) const override;

  edm::EDGetTokenT<SimClusterCollection> SCCollectionToken_;
  edm::EDGetTokenT<CLUSTER> LCCollectionToken_;
  edm::EDGetTokenT<ticl::LayerClusterToSimClusterAssociator<CLUSTER>> associatorToken_;
};

template <typename CLUSTER>
LCToSCAssociatorEDProducer<CLUSTER>::LCToSCAssociatorEDProducer(const edm::ParameterSet &pset) {
  produces<ticl::SimToRecoCollectionWithSimClusters_T<CLUSTER>>();
  produces<ticl::RecoToSimCollectionWithSimClusters_T<CLUSTER>>();

  SCCollectionToken_ = consumes<SimClusterCollection>(pset.getParameter<edm::InputTag>("label_scl"));
  LCCollectionToken_ = consumes<CLUSTER>(pset.getParameter<edm::InputTag>("label_lcl"));
  associatorToken_ = consumes<ticl::LayerClusterToSimClusterAssociator<CLUSTER>>(pset.getParameter<edm::InputTag>("associator"));
}

//
// member functions
//

// ------------ method called to produce the data  ------------
template <typename CLUSTER>
void LCToSCAssociatorEDProducer<CLUSTER>::produce(edm::StreamID, edm::Event &iEvent, const edm::EventSetup &iSetup) const {
  using namespace edm;

  edm::Handle<ticl::LayerClusterToSimClusterAssociator<CLUSTER>> theAssociator;
  iEvent.getByToken(associatorToken_, theAssociator);

  if (!theAssociator.isValid()) {
    edm::LogWarning("LCToSCAssociatorEDProducer")
        << "Associator is unavailable.";
    return;
  }

  Handle<SimClusterCollection> SCCollection;
  iEvent.getByToken(SCCollectionToken_, SCCollection);

  if (!SCCollection.isValid()) {
    edm::LogWarning("LCToSCAssociatorEDProducer")
        << "SimCluster collection is unavailable. Producing empty associations.";

    return;
  }

  Handle<CLUSTER> LCCollection;
  iEvent.getByToken(LCCollectionToken_, LCCollection);

  // Protection against missing CaloCluster collection
  if (!LCCollection.isValid()) {
    edm::LogWarning("LCToSCAssociatorEDProducer")
        << "CaloCluster collection is unavailable. Producing empty associations.";

    // Return empty collections
    auto emptyRecSimColl = std::make_unique<ticl::RecoToSimCollectionWithSimClusters_T<CLUSTER>>();
    auto emptySimRecColl = std::make_unique<ticl::SimToRecoCollectionWithSimClusters_T<CLUSTER>>();

    iEvent.put(std::move(emptyRecSimColl));
    iEvent.put(std::move(emptySimRecColl));
    return;
  }

  // associate LC and SC
  LogTrace("AssociatorValidator") << "Calling associateRecoToSim method\n";
  ticl::RecoToSimCollectionWithSimClusters_T<CLUSTER> recSimColl = theAssociator->associateRecoToSim(LCCollection, SCCollection);

  LogTrace("AssociatorValidator") << "Calling associateSimToReco method\n";
  ticl::SimToRecoCollectionWithSimClusters_T<CLUSTER> simRecColl = theAssociator->associateSimToReco(LCCollection, SCCollection);

  auto rts = std::make_unique<ticl::RecoToSimCollectionWithSimClusters_T<CLUSTER>>(recSimColl);
  auto str = std::make_unique<ticl::SimToRecoCollectionWithSimClusters_T<CLUSTER>>(simRecColl);

  iEvent.put(std::move(rts));
  iEvent.put(std::move(str));
}

template <typename CLUSTER>
void LCToSCAssociatorEDProducer<CLUSTER>::fillDescriptions(edm::ConfigurationDescriptions &descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("label_scl", edm::InputTag("scAssocByEnergyScoreProducer"));
  desc.add<edm::InputTag>("label_lcl", edm::InputTag("mix", "MergedCaloTruth"));
  desc.add<edm::InputTag>("associator", edm::InputTag("hgcalMergeLayerClusters"));
  descriptions.addWithDefaultLabel(desc);
}

// define this as a plug-in
typedef LCToSCAssociatorEDProducer<reco::CaloClusterCollection> CaloClusterLCToSCAssociatorEDProducer;
DEFINE_FWK_MODULE(CaloClusterLCToSCAssociatorEDProducer);
