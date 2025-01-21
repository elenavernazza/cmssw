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
#include "DataFormats/Math/interface/deltaPhi.h"
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
  MonitorElement* h_curvatureR_;
  MonitorElement* h_pTFromR_;
  MonitorElement* h_sizeYinnerB1_;
  MonitorElement* h_sizeYinnerB2_;
  std::vector<MonitorElement*> hVector_dr_;
  std::vector<MonitorElement*> hVector_dphi_;
  std::vector<MonitorElement*> hVector_innerZ_;
  std::vector<MonitorElement*> hVector_sizeY_;
  std::vector<MonitorElement*> hVector_dsizeYonlyBarrel_;
  std::vector<MonitorElement*> hVector_dsizeYinnerBarrel_;
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
      // RecHit properties
      auto inner_r = doublet.innerGlobalPos().perp();
      auto inner_z = doublet.innerGlobalPos().z();
      auto inner_phi = doublet.innerGlobalPos().barePhi();  // returns float, whereas .phi() returns phi object
      auto outer_r = doublet.outerGlobalPos().perp();
      auto outer_z = doublet.outerGlobalPos().z();
      auto outer_phi = doublet.outerGlobalPos().barePhi();

      auto dz = outer_z - inner_z;
      auto dr = outer_r - inner_r;
      auto dphi = reco::deltaPhi(inner_phi, outer_phi);

      // ----------------------------------------------------------
      // layer pair independent plots (main folder)
      // ----------------------------------------------------------

      h_layerPairId_->Fill(doublet.innerLayerId(), doublet.outerLayerId());
      h_numSkippedLayers_->Fill(doublet.numSkippedLayers());
      // impact parameter histogram
      h_z0_->Fill(std::abs(inner_r * outer_z - inner_z * outer_r) / dr);
      auto curvature = 1.f / 4.f * ((dr / dphi) * (dr / dphi) + (dr));
      h_curvatureR_->Fill(curvature);
      h_pTFromR_->Fill(curvature / 87.78f);

      // ----------------------------------------------------------
      // layer pair dependent plots (sub-folders for layer pairs)
      // ----------------------------------------------------------

      // first, get layer pair Id and exclude layer pairs that are not considered
      int layerPairId = doublet.layerPairId();
      if (layerPairId2Index.find(layerPairId) == layerPairId2Index.end()) {
        continue;
      }

      // get the position of the layer pair in the vectors of histograms
      int layerPairIdIndex = layerPairId2Index.at(layerPairId);

      // dr histogram
      hVector_dr_[layerPairIdIndex]->Fill(dr);

      // dphi histogram
      hVector_dphi_[layerPairIdIndex]->Fill(dphi);

      // inner z histogram
      hVector_innerZ_[layerPairIdIndex]->Fill(inner_z);

      // cluster size y histogram
      hVector_sizeY_[layerPairIdIndex]->Fill(doublet.innerRecHit()->cluster()->sizeY());

      DetId detIdObject(doublet.innerRecHit()->geographicalId());
      const GeomDetUnit* geomDetUnit = trackerGeometry_->idToDetUnit(detIdObject);
      const uint32_t moduleId = geomDetUnit->index();
      bool innerInB1 = (doublet.innerLayerId() == 0);
      bool innerInB2 = (doublet.innerLayerId() == 1);
      bool isOuterLadder = (0 == (moduleId / 8) % 2);
      bool innerInBarrel = (doublet.innerLayerId() < 4);
      bool bothInBarrel = innerInBarrel && (doublet.outerLayerId() < 4);

      // cluster size in local y
      if (innerInB1 && isOuterLadder) {
        h_sizeYinnerB1_->Fill(doublet.innerRecHit()->cluster()->sizeY());
      }
      if (innerInB2) {
        h_sizeYinnerB2_->Fill(doublet.innerRecHit()->cluster()->sizeY());
      }

      if (bothInBarrel) {
        auto realDsizeY =
            std::abs(doublet.innerRecHit()->cluster()->sizeY() - doublet.outerRecHit()->cluster()->sizeY());
        if (innerInB1 && isOuterLadder) {
          hVector_dsizeYonlyBarrel_[layerPairIdIndex]->Fill(realDsizeY);
        } else if (!innerInB1) {
          hVector_dsizeYonlyBarrel_[layerPairIdIndex]->Fill(realDsizeY);
        }
      } else if (innerInBarrel) {
        int projectedDsizeY = std::abs(doublet.innerRecHit()->cluster()->sizeY() -
                                       int(std::abs(dz / dr) * 8.f * 0.0285f / 0.015f + 0.5f));
        hVector_dsizeYinnerBarrel_[layerPairIdIndex]->Fill(std::abs(
            doublet.innerRecHit()->cluster()->sizeY() - int(std::abs(dz / dr) * 8.f * 0.0285f / 0.015f + 0.5f)));
        std::cout << layerPairId << " " << projectedDsizeY << " "
                  << hVector_dsizeYinnerBarrel_[layerPairIdIndex]->getEntries() << std::endl;
      }
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
  h_curvatureR_ =
      ibook.book1D("curvatureR", "Curvature Radius; Curvature Radius [cm] ; Number of SimDoublets", 40, -20, 20);
  h_pTFromR_ = ibook.book1D(
      "pTFromR", "Transverse Momentum from curvature; Transverse Momentum [GeV] ; Number of SimDoublets", 500, 0, 1000);

  h_sizeYinnerB1_ =
      ibook.book1D("sizeYinnerB1", "Cluster Size Y (inner from B1); Cluster Size Y; Number of SimDoublets", 51, -1, 50);
  h_sizeYinnerB2_ =
      ibook.book1D("sizeYinnerB2", "Cluster Size Y (inner from B2); Cluster Size Y; Number of SimDoublets", 51, -1, 50);

  // booking the vector of histograms for each valid layer pair
  for (auto id = layerPairId2Index.begin(); id != layerPairId2Index.end(); ++id) {
    std::string index = std::to_string(id->first);
    // name of the sub-folder: "lp_${innerLayerId}_${outerLayerId}"
    std::string name;
    std::string innerLayerName;
    std::string outerLayerName;
    if (index.size() < 3) {
      innerLayerName = "0";
      outerLayerName = index;
    } else if (index.size() == 3) {
      innerLayerName = index.substr(0, 1);
      if (index.substr(1, 2) == "0") {
        outerLayerName = index.substr(2, 3);
      } else {
        outerLayerName = index.substr(1, 3);
      }
    } else {
      innerLayerName = index.substr(0, 2);
      if (index.substr(2, 3) == "0") {
        outerLayerName = index.substr(3, 4);
      } else {
        outerLayerName = index.substr(2, 4);
      }
    }
    name = "/lp_" + innerLayerName + "_" + outerLayerName;

    // layer mentioning in histogram titles
    std::string layerTitle = "(layers (" + innerLayerName + "," + outerLayerName + "))";

    ibook.setCurrentFolder(folder_ + name);
    hVector_dr_.emplace_back(ibook.book1D(
        "dr",
        "dr of RecHit pair " + layerTitle + "; dr between outer and inner RecHit [cm]; Number of SimDoublets",
        31,
        -1,
        30));
    hVector_dphi_.emplace_back(ibook.book1D(
        "dphi",
        "dphi of RecHit pair " + layerTitle + "; d#phi between outer and inner RecHit [rad]; Number of SimDoublets",
        50,
        -M_PI,
        M_PI));
    hVector_innerZ_.emplace_back(
        ibook.book1D("innerZ",
                     "z of the inner RecHit " + layerTitle + "; z of inner RecHit [cm]; Number of SimDoublets",
                     100,
                     -300,
                     300));

    hVector_dsizeYonlyBarrel_.emplace_back(ibook.book1D("dsizeYonlyBarrel",
                                                        "Cluster Size Y Difference between outer and inner RecHit " +
                                                            layerTitle +
                                                            "; Cluster Size Y Difference ; Number of SimDoublets",
                                                        51,
                                                        -1,
                                                        50));

    hVector_dsizeYinnerBarrel_.emplace_back(
        ibook.book1D("dsizeYinnerBarrel",
                     "Projected Cluster Size Y Difference between outer and inner RecHit " + layerTitle +
                         "; Cluster Size Y Difference ; Number of SimDoublets",
                     51,
                     -1,
                     50));

    hVector_sizeY_.emplace_back(
        ibook.book1D("sizeY", "Cluster Size Y " + layerTitle + "; Cluster Size Y ; Number of SimDoublets", 51, -1, 50));
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