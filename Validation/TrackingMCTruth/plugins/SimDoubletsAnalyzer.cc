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
#include <map>
#include <vector>

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
  MonitorElement* h_z0_;
  std::vector<MonitorElement*> hVector_dr_;
  int eventCount_ = 0;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//
static const std::map<int, int> layerPairId2Index{
    {1, 0},     {102, 1},   {203, 2},   {405, 3},   {506, 4},   {607, 5},   {708, 6},   {809, 7},   {910, 8},
    {1011, 9},  {1112, 10}, {1213, 11}, {1314, 12}, {1415, 13}, {1617, 14}, {1718, 15}, {1819, 16}, {1920, 17},
    {2021, 18}, {2122, 19}, {2223, 20}, {2324, 21}, {2425, 22}, {2526, 23}, {2627, 24}, {2, 25},    {103, 26},
    {406, 27},  {507, 28},  {608, 29},  {709, 30},  {810, 31},  {911, 32},  {1012, 33}, {1113, 34}, {1214, 35},
    {1315, 36}, {1618, 37}, {1719, 38}, {1820, 39}, {1921, 40}, {2022, 41}, {2123, 42}, {2224, 43}, {2325, 44},
    {2426, 45}, {2527, 46}, {4, 47},    {104, 48},  {204, 49},  {5, 50},    {105, 51},  {205, 52},  {16, 53},
    {116, 54},  {216, 55},  {17, 56},   {117, 57},  {217, 58}};

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
      int layerPairId = doublet.layerPairId();

      h_layerPairId_->Fill(doublet.innerLayerId(), doublet.outerLayerId());
      h_numSkippedLayers_->Fill(doublet.numSkippedLayers());
      auto inner_r = doublet.innerGlobalPos().perp();
      auto inner_z = doublet.innerGlobalPos().z();
      auto outer_r = doublet.outerGlobalPos().perp();
      auto outer_z = doublet.outerGlobalPos().z();
      h_z0_->Fill(std::abs(inner_r * outer_z - inner_z * outer_r) / (outer_r - inner_r));

      if (layerPairId2Index.find(layerPairId) == layerPairId2Index.end()) {
        continue;
      }
      int layerPairIdIndex = layerPairId2Index.at(layerPairId);
      // dr histogram
      hVector_dr_[layerPairIdIndex]->Fill(outer_r - inner_r);
    }
  }
}

void SimDoubletsAnalyzer::bookHistograms(DQMStore::IBooker& ibook, edm::Run const& run, edm::EventSetup const& iSetup) {
  ibook.setCurrentFolder(folder_);

  // booking the histograms
  h_layerPairId_ =
      ibook.book2D("layerPairs", "Layer pairs; Inner layer ID; Outer layer ID", 28, -0.5, 27.5, 28, -0.5, 27.5);
  h_numSkippedLayers_ = ibook.book1D(
      "numSkippedLayers", "Number of skipped layers; Number of skipped layers; Number of SimDoublets", 16, -1.5, 14.5);
  h_z0_ = ibook.book1D("z0", "z0; z0 [cm]; Number of SimDoublets", 51, -1, 50);

  //
  for (auto id = layerPairId2Index.begin(); id != layerPairId2Index.end(); ++id) {
    ibook.setCurrentFolder(folder_ + "/layerPair_" + std::to_string(id->first));
    hVector_dr_.emplace_back(
        ibook.book1D("dr", "dr of RecHit pair; dr between outer and inner RecHit [cm]; Number of SimDoublets", 31, -1, 30));
  }
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