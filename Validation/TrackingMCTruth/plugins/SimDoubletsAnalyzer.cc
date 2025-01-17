// -*- C++ -*-
//
// Package:    Validation/TrackingMCTruth
// Class:      SimDoubletsAnalyzer
//
/**\class SimDoubletsAnalyzer SimDoubletsAnalyzer.cc Validation/TrackingMCTruth/plugins/SimDoubletsAnalyzer.cc

 Description: DQM analyzer for true RecHit doublets (SimDoublets) of the inner tracker in Phase-2 HLT

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Luca Ferragina
//         Created:  Thu, 16 Jan 2025 13:46:21 GMT
//
//

#include <string>

// user include files
#include "DataFormats/Histograms/interface/MonitorElementCollection.h"
#include "SimDataFormats/TrackingAnalysis/interface/SimDoublets.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/MonitorElement.h"

//
// class declaration
//

class SimDoubletsAnalyzer : public DQMEDAnalyzer {
public:
  explicit SimDoubletsAnalyzer(const edm::ParameterSet&);
  ~SimDoubletsAnalyzer() override;

  void dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) override;
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void bookHistograms(DQMStore::IBooker&, edm::Run const&, edm::EventSetup const&) override;

  void analyze(const edm::Event&, const edm::EventSetup&) override;

  // ------------ member data ------------
  
  const TrackerGeometry* trackerGeometry_ = nullptr;
  const edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geometry_getToken_;
  const edm::EDGetTokenT<SimDoubletsCollection> simDoublets_getToken_;
  std::string folder_;
  MonitorElement* h_layerPairId_;
  MonitorElement* h_numSkippedLayers_;
  int eventCount_ = 0;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
SimDoubletsAnalyzer::SimDoubletsAnalyzer(const edm::ParameterSet& iConfig)
    : geometry_getToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord, edm::Transition::BeginRun>()),
      simDoublets_getToken_(consumes(iConfig.getParameter<edm::InputTag>("simDoubletsSrc"))),
      folder_(iConfig.getParameter<std::string>("folder")) {
  // now do what ever initialization is needed
}

SimDoubletsAnalyzer::~SimDoubletsAnalyzer() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}


//
// member functions
// ----------------

void SimDoubletsAnalyzer::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  trackerGeometry_ = &iSetup.getData(geometry_getToken_);
}


void SimDoubletsAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  eventCount_++;

  // get simDoublets
  SimDoubletsCollection const& simDoubletsCollection = iEvent.get(simDoublets_getToken_);

  for (auto const& simDoublets : simDoubletsCollection) {
    auto doublets = simDoublets.getSimDoublets(trackerGeometry_);
    for (auto const& doublet : doublets) {
      h_layerPairId_->Fill(doublet.innerLayerId(), doublet.outerLayerId());
      h_numSkippedLayers_->Fill(doublet.numSkippedLayers());
    }
  }
}


void SimDoubletsAnalyzer::bookHistograms(DQMStore::IBooker& ibook,
                                    edm::Run const& run,
                                    edm::EventSetup const& iSetup) {
  ibook.setCurrentFolder(folder_);

  // booking the histograms  
  h_layerPairId_ = ibook.book2D("layerPairs", "Layer pairs; Inner layer ID; Outer layer ID", 28, -0.5, 27.5, 28, -0.5, 27.5);
  h_numSkippedLayers_ = ibook.book1D("numSkippedLayers", "Number of skipped layers; Number of skipped layers; Number of SimDoublets", 16, -1.5, 14.5);
}



void SimDoubletsAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  // The following says we do not know what parameters are allowed so do no
  // validation
  // Please change this to state exactly what you do use, even if it is no
  // parameters
  edm::ParameterSetDescription desc;
  desc.add<std::string>("folder", "Tracking/TrackingMCTruth/SimDoublets");
  desc.add<edm::InputTag>("simDoubletsSrc", edm::InputTag("simDoubletsProducer"));
  descriptions.addWithDefaultLabel(desc);
}

// define this as a plug-in
DEFINE_FWK_MODULE(SimDoubletsAnalyzer);