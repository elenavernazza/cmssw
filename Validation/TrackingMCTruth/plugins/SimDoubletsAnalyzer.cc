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
// Original Author:  Luca Ferragina, Elena Vernazza, Jan Schulz
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
#include "Geometry/CommonTopologies/interface/SimplePixelTopology.h"
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

  std::string folder_;  // main folder in the DQM file
  int eventCount_ = 0;  // event counter

  // monitor elements (histograms) to be filled
  MonitorElement* h_layerPairs_;
  MonitorElement* h_numSkippedLayers_;
  MonitorElement* h_z0_;
  MonitorElement* h_curvatureR_;
  MonitorElement* h_pTFromR_;
  MonitorElement* h_YsizeB1_;
  MonitorElement* h_YsizeB2_;
  MonitorElement* h_DYsize12_;
  MonitorElement* h_DYsize_;
  MonitorElement* h_DYPred_;
  std::vector<MonitorElement*> hVector_dr_;
  std::vector<MonitorElement*> hVector_dphi_;
  std::vector<MonitorElement*> hVector_innerZ_;
  std::vector<MonitorElement*> hVector_clusterSizeY_;
  std::vector<MonitorElement*> hVector_dsizeYonlyBarrel_;
  std::vector<MonitorElement*> hVector_dsizeYinnerBarrel_;
};

namespace simdoublets {
  std::pair<std::string, std::string> getInnerOuterLayerNames(int const layerPairId) {
    // make a string from the Id (int)
    std::string index = std::to_string(layerPairId);
    // determine inner and outer layer name
    std::string innerLayerName;
    std::string outerLayerName;
    if (index.size() < 3) {
      innerLayerName = "0";
      outerLayerName = index;
    } else if (index.size() == 3) {
      innerLayerName = index.substr(0, 1);
      outerLayerName = index.substr(1, 3);
    } else {
      innerLayerName = index.substr(0, 2);
      outerLayerName = index.substr(2, 4);
    }
    if (outerLayerName[0] == '0') {
      outerLayerName = outerLayerName.substr(1, 2);
    }

    return {innerLayerName, outerLayerName};
  }
}  // namespace simdoublets

//
// static data member definitions
//
// map that takes the layerPairId as defined in the SimDoublets
// and gives the position of the histogram in the histogram vector
// NOTE: It is absolutely necessary that the map is sorted here,
// otherwise the histograms will not be labeled corresponding to the correct layer pair but are mixed up
static const std::map<int, int> layerPairId2Index{
    {1, 0},     {2, 1},     {4, 2},     {5, 3},     {16, 4},    {17, 5},    {102, 6},   {103, 7},   {104, 8},
    {105, 9},   {116, 10},  {117, 11},  {203, 12},  {204, 13},  {205, 14},  {216, 15},  {217, 16},  {405, 17},
    {406, 18},  {506, 19},  {507, 20},  {607, 21},  {608, 22},  {708, 23},  {709, 24},  {809, 25},  {810, 26},
    {910, 27},  {911, 28},  {1011, 29}, {1012, 30}, {1112, 31}, {1113, 32}, {1213, 33}, {1214, 34}, {1314, 35},
    {1315, 36}, {1415, 37}, {1617, 38}, {1618, 39}, {1718, 40}, {1719, 41}, {1819, 42}, {1820, 43}, {1920, 44},
    {1921, 45}, {2021, 46}, {2022, 47}, {2122, 48}, {2123, 49}, {2223, 50}, {2224, 51}, {2324, 52}, {2325, 53},
    {2425, 54}, {2426, 55}, {2526, 56}, {2527, 57}, {2627, 58}};

// -------------------------------
// constructors and destructor
// -------------------------------
SimDoubletsAnalyzer::SimDoubletsAnalyzer(const edm::ParameterSet& iConfig)
    : geometry_getToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord, edm::Transition::BeginRun>()),
      simDoublets_getToken_(consumes(iConfig.getParameter<edm::InputTag>("simDoubletsSrc"))),
      folder_(iConfig.getParameter<std::string>("folder")) {}

SimDoubletsAnalyzer::~SimDoubletsAnalyzer() {}

// -----------------------
// member functions
// -----------------------

void SimDoubletsAnalyzer::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  trackerGeometry_ = &iSetup.getData(geometry_getToken_);
}

void SimDoubletsAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  eventCount_++;

  // get simDoublets
  SimDoubletsCollection const& simDoubletsCollection = iEvent.get(simDoublets_getToken_);

  // loop over SimDoublets (= loop over TrackingParticles)
  for (auto const& simDoublets : simDoubletsCollection) {
    // create the true RecHit doublets of the TrackingParticle
    auto doublets = simDoublets.getSimDoublets(trackerGeometry_);

    // loop over those doublets
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

      // outer layer vs inner layer of SimDoublets
      h_layerPairs_->Fill(doublet.innerLayerId(), doublet.outerLayerId());

      // number of skipped layers by SimDoublets
      h_numSkippedLayers_->Fill(doublet.numSkippedLayers());

      // longitudinal impact parameter with respect to the beamspot
      h_z0_->Fill(std::abs(inner_r * outer_z - inner_z * outer_r) / dr);

      // radius of the circle defined by the two RecHits and the beamspot
      auto curvature = 1.f / 2.f * std::sqrt((dr / dphi) * (dr / dphi) + (inner_r * outer_r));
      h_curvatureR_->Fill(curvature);

      // pT that this curvature radius corresponds to
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

      // dr = (outer_r - inner_r) histogram
      hVector_dr_[layerPairIdIndex]->Fill(dr);

      // dphi histogram
      hVector_dphi_[layerPairIdIndex]->Fill(dphi);

      // z of the inner RecHit histogram
      hVector_innerZ_[layerPairIdIndex]->Fill(inner_z);

      // cluster size in local y histogram
      auto innerClusterSizeY = doublet.innerRecHit()->cluster()->sizeY();
      hVector_clusterSizeY_[layerPairIdIndex]->Fill(innerClusterSizeY);

      // determine the moduleId
      DetId detIdObject(doublet.innerRecHit()->geographicalId());
      const GeomDetUnit* geomDetUnit = trackerGeometry_->idToDetUnit(detIdObject);
      const uint32_t moduleId = geomDetUnit->index();

      // define bools needed to decide on cutting parameters
      const bool innerInB1 = (doublet.innerLayerId() == 0);
      const bool innerInB2 = (doublet.innerLayerId() == 1);
      const bool isOuterLadder = (0 == (moduleId / 8) % 2);  // check if this even makes sense in Phase-2
      const bool innerInBarrel = (doublet.innerLayerId() < 4);
      const bool outerInBarrel = (doublet.outerLayerId() < 4);

      // histograms for clusterCut
      // cluster size in local y
      if (innerInB1 && isOuterLadder) {
        h_YsizeB1_->Fill(innerClusterSizeY);
      }
      if (innerInB2) {
        h_YsizeB2_->Fill(innerClusterSizeY);
      }

      // histograms for zSizeCut
      if (innerInBarrel) {
        if (outerInBarrel) {  // onlyBarrel
          auto DYsize = std::abs(innerClusterSizeY - doublet.outerRecHit()->cluster()->sizeY());
          if (innerInB1 && isOuterLadder) {
            hVector_dsizeYonlyBarrel_[layerPairIdIndex]->Fill(DYsize);
            h_DYsize12_->Fill(DYsize);
          } else if (!innerInB1) {
            hVector_dsizeYonlyBarrel_[layerPairIdIndex]->Fill(DYsize);
            h_DYsize_->Fill(DYsize);
          }
        } else {  // not onlyBarrel
          int DYsizePred =
              std::abs(innerClusterSizeY - int(std::abs(dz / dr) * pixelTopology::Phase2::dzdrFact + 0.5f));
          hVector_dsizeYinnerBarrel_[layerPairIdIndex]->Fill(DYsizePred);
          h_DYPred_->Fill(DYsizePred);
        }
      }
    }  // end loop over those doublets
  }  // end loop over SimDoublets (= loop over TrackingParticles)
}

void SimDoubletsAnalyzer::bookHistograms(DQMStore::IBooker& ibook, edm::Run const& run, edm::EventSetup const& iSetup) {
  ibook.setCurrentFolder(folder_);

  // ----------------------------------------------------------
  // booking layer pair independent histograms (main folder)
  // ----------------------------------------------------------

  // overview histograms
  h_layerPairs_ = ibook.book2D(
      "layerPairs", "Layer pairs in SimDoublets; Inner layer ID; Outer layer ID", 28, -0.5, 27.5, 28, -0.5, 27.5);
  h_numSkippedLayers_ = ibook.book1D(
      "numSkippedLayers", "Number of skipped layers; Number of skipped layers; Number of SimDoublets", 16, -1.5, 14.5);

  // histogram for z0cutoff  (z0Cut)
  h_z0_ = ibook.book1D("z0", "z_0; Longitudinal impact parameter z_0 [cm]; Number of SimDoublets", 51, -1, 50);

  // histograms for ptcut  (ptCut)
  h_curvatureR_ = ibook.book1D(
      "curvatureR", "Curvature from SimDoublet+beamspot; Curvature radius [cm] ; Number of SimDoublets", 100, 0, 1000);
  h_pTFromR_ = ibook.book1D("pTFromR",
                            "Transverse momentum from curvature; Transverse momentum p_T [GeV]; Number of SimDoublets",
                            500,
                            0,
                            1000);

  // histograms for clusterCut  (minYsizeB1 and minYsizeB2)
  h_YsizeB1_ =
      ibook.book1D("YsizeB1", "Cluster size Y (inner from B1); Cluster Size Y; Number of SimDoublets", 51, -1, 50);
  h_YsizeB2_ =
      ibook.book1D("YsizeB2", "Cluster size Y (inner from B2); Cluster Size Y; Number of SimDoublets", 51, -1, 50);

  // histograms for zSizeCut  (maxDYsize12, maxDYsize and maxDYPred)
  h_DYsize12_ = ibook.book1D("DYsize12",
                             "Difference in cluster y-size (inner from B1); Absolute difference in cluster y-size of "
                             "the two RecHits; Number of SimDoublets",
                             31,
                             -1,
                             30);
  h_DYsize_ = ibook.book1D(
      "DYsize",
      "Difference in cluster y-size; Absolute difference in cluster y-size of the two RecHits; Number of SimDoublets",
      31,
      -1,
      30);
  h_DYPred_ = ibook.book1D("DYPred",
                           "Projected difference in cluster z-size; Absolute difference in projected cluster z-size of "
                           "the two RecHits; Number of SimDoublets",
                           201,
                           -1,
                           200);

  // -----------------------------------------------------------------------
  // booking layer pair dependent histograms (sub-folders for layer pairs)
  // -----------------------------------------------------------------------

  // loop through valid layer pairs and add for each one booked hist per vector
  for (auto id = layerPairId2Index.begin(); id != layerPairId2Index.end(); ++id) {
    // get layer names from the layer pair Id
    auto layerNames = simdoublets::getInnerOuterLayerNames(id->first);
    std::string innerLayerName = layerNames.first;
    std::string outerLayerName = layerNames.second;

    // name the sub-folder for the layer pair "lp_${innerLayerId}_${outerLayerId}"
    std::string subFolderName = "/lp_" + innerLayerName + "_" + outerLayerName;

    // layer mentioning in histogram titles
    std::string layerTitle = "(layers (" + innerLayerName + "," + outerLayerName + "))";

    // set folder to the sub-folder for the layer pair
    ibook.setCurrentFolder(folder_ + subFolderName);

    // histogram for z0cutoff  (maxr)
    hVector_dr_.emplace_back(ibook.book1D(
        "dr",
        "dr of RecHit pair " + layerTitle + "; dr between outer and inner RecHit [cm]; Number of SimDoublets",
        31,
        -1,
        30));

    // histogram for iphicut  (phiCuts)
    hVector_dphi_.emplace_back(ibook.book1D(
        "dphi",
        "dphi of RecHit pair " + layerTitle + "; d#phi between outer and inner RecHit [rad]; Number of SimDoublets",
        50,
        -M_PI,
        M_PI));

    // histogram for z window  (minz and maxz)
    hVector_innerZ_.emplace_back(
        ibook.book1D("innerZ",
                     "z of the inner RecHit " + layerTitle + "; z of inner RecHit [cm]; Number of SimDoublets",
                     100,
                     -300,
                     300));

    // other histograms
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
    hVector_clusterSizeY_.emplace_back(
        ibook.book1D("sizeY", "Cluster Size Y " + layerTitle + "; Cluster Size Y ; Number of SimDoublets", 51, -1, 50));
  }
}

void SimDoubletsAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("folder", "Tracking/TrackingMCTruth/SimDoublets");
  desc.add<edm::InputTag>("simDoubletsSrc", edm::InputTag("simDoubletsProducer"));
  descriptions.addWithDefaultLabel(desc);
}

// define this as a plug-in
DEFINE_FWK_MODULE(SimDoubletsAnalyzer);