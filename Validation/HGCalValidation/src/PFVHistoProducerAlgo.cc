#include <numeric>
#include <iomanip>
#include <sstream>

#include "Validation/HGCalValidation/interface/PFVHistoProducerAlgo.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "TMath.h"
#include "TLatex.h"
#include "TF1.h"

using namespace std;

//Parameters for the score cut. Later, this will become part of the
//configuration parameter for the HGCAL associator.
const double ScoreCutRCtoCP_ = 0.1;
const double ScoreCutCPtoRC_ = 0.1;
const double ScoreCutRCtoSC_ = 0.1;
const double ScoreCutSCtoRC_ = 0.1;

PFVHistoProducerAlgo::PFVHistoProducerAlgo(const edm::ParameterSet& pset)
    :  //parameters for eta
      minEta_(pset.getParameter<double>("minEta")),
      maxEta_(pset.getParameter<double>("maxEta")),
      nintEta_(pset.getParameter<int>("nintEta")),
      useFabsEta_(pset.getParameter<bool>("useFabsEta")),

      //parameters for energy
      minEne_(pset.getParameter<double>("minEne")),
      maxEne_(pset.getParameter<double>("maxEne")),
      nintEne_(pset.getParameter<int>("nintEne")),

      //parameters for pt
      minPt_(pset.getParameter<double>("minPt")),
      maxPt_(pset.getParameter<double>("maxPt")),
      nintPt_(pset.getParameter<int>("nintPt")),

      //parameters for phi
      minPhi_(pset.getParameter<double>("minPhi")),
      maxPhi_(pset.getParameter<double>("maxPhi")),
      nintPhi_(pset.getParameter<int>("nintPhi")),

      //parameters for counting mixed hits SimClusters
      minMixedHitsSimCluster_(pset.getParameter<double>("minMixedHitsSimCluster")),
      maxMixedHitsSimCluster_(pset.getParameter<double>("maxMixedHitsSimCluster")),
      nintMixedHitsSimCluster_(pset.getParameter<int>("nintMixedHitsSimCluster")),

      //parameters for counting mixed hits clusters
      minMixedHitsCluster_(pset.getParameter<double>("minMixedHitsCluster")),
      maxMixedHitsCluster_(pset.getParameter<double>("maxMixedHitsCluster")),
      nintMixedHitsCluster_(pset.getParameter<int>("nintMixedHitsCluster")),

      //parameters for the total amount of energy clustered by all layer clusters (fraction over CaloParticles)
      minEneCl_(pset.getParameter<double>("minEneCl")),
      maxEneCl_(pset.getParameter<double>("maxEneCl")),
      nintEneCl_(pset.getParameter<int>("nintEneCl")),

      //parameters for the longitudinal depth barycenter.
      minLongDepBary_(pset.getParameter<double>("minLongDepBary")),
      maxLongDepBary_(pset.getParameter<double>("maxLongDepBary")),
      nintLongDepBary_(pset.getParameter<int>("nintLongDepBary")),

      //parameters for z positionof vertex plots
      minZpos_(pset.getParameter<double>("minZpos")),
      maxZpos_(pset.getParameter<double>("maxZpos")),
      nintZpos_(pset.getParameter<int>("nintZpos")),

      //Parameters for the total number of SimClusters per layer
      minTotNsimClsperlay_(pset.getParameter<double>("minTotNsimClsperlay")),
      maxTotNsimClsperlay_(pset.getParameter<double>("maxTotNsimClsperlay")),
      nintTotNsimClsperlay_(pset.getParameter<int>("nintTotNsimClsperlay")),

      //Parameters for the total number of layer clusters per layer
      minTotNClsperlay_(pset.getParameter<double>("minTotNClsperlay")),
      maxTotNClsperlay_(pset.getParameter<double>("maxTotNClsperlay")),
      nintTotNClsperlay_(pset.getParameter<int>("nintTotNClsperlay")),

      //Parameters for the energy clustered by layer clusters per layer (fraction over CaloParticles)
      minEneClperlay_(pset.getParameter<double>("minEneClperlay")),
      maxEneClperlay_(pset.getParameter<double>("maxEneClperlay")),
      nintEneClperlay_(pset.getParameter<int>("nintEneClperlay")),

      //Parameters for the score both for:
      //1. calo particle to layer clusters association per layer
      //2. layer cluster to calo particles association per layer
      minScore_(pset.getParameter<double>("minScore")),
      maxScore_(pset.getParameter<double>("maxScore")),
      nintScore_(pset.getParameter<int>("nintScore")),

      //Parameters for shared energy fraction. That is:
      //1. Fraction of each of the layer clusters energy related to a
      //calo particle over that calo particle's energy.
      //2. Fraction of each of the calo particles energy
      //related to a layer cluster over that layer cluster's energy.
      minSharedEneFrac_(pset.getParameter<double>("minSharedEneFrac")),
      maxSharedEneFrac_(pset.getParameter<double>("maxSharedEneFrac")),
      nintSharedEneFrac_(pset.getParameter<int>("nintSharedEneFrac")),

      //Parameters for the total number of SimClusters per thickness
      minTotNsimClsperthick_(pset.getParameter<double>("minTotNsimClsperthick")),
      maxTotNsimClsperthick_(pset.getParameter<double>("maxTotNsimClsperthick")),
      nintTotNsimClsperthick_(pset.getParameter<int>("nintTotNsimClsperthick")),

      //Parameters for the total number of clusters per thickness
      minTotNClsperthick_(pset.getParameter<double>("minTotNClsperthick")),
      maxTotNClsperthick_(pset.getParameter<double>("maxTotNClsperthick")),
      nintTotNClsperthick_(pset.getParameter<int>("nintTotNClsperthick")),

      //Parameters for the total number of cells per per thickness per layer
      minTotNcellsperthickperlayer_(pset.getParameter<double>("minTotNcellsperthickperlayer")),
      maxTotNcellsperthickperlayer_(pset.getParameter<double>("maxTotNcellsperthickperlayer")),
      nintTotNcellsperthickperlayer_(pset.getParameter<int>("nintTotNcellsperthickperlayer")),

      //Parameters for the distance of cluster cells to seed cell per thickness per layer
      minDisToSeedperthickperlayer_(pset.getParameter<double>("minDisToSeedperthickperlayer")),
      maxDisToSeedperthickperlayer_(pset.getParameter<double>("maxDisToSeedperthickperlayer")),
      nintDisToSeedperthickperlayer_(pset.getParameter<int>("nintDisToSeedperthickperlayer")),

      //Parameters for the energy weighted distance of cluster cells to seed cell per thickness per layer
      minDisToSeedperthickperlayerenewei_(pset.getParameter<double>("minDisToSeedperthickperlayerenewei")),
      maxDisToSeedperthickperlayerenewei_(pset.getParameter<double>("maxDisToSeedperthickperlayerenewei")),
      nintDisToSeedperthickperlayerenewei_(pset.getParameter<int>("nintDisToSeedperthickperlayerenewei")),

      //Parameters for the distance of cluster cells to max cell per thickness per layer
      minDisToMaxperthickperlayer_(pset.getParameter<double>("minDisToMaxperthickperlayer")),
      maxDisToMaxperthickperlayer_(pset.getParameter<double>("maxDisToMaxperthickperlayer")),
      nintDisToMaxperthickperlayer_(pset.getParameter<int>("nintDisToMaxperthickperlayer")),

      //Parameters for the energy weighted distance of cluster cells to max cell per thickness per layer
      minDisToMaxperthickperlayerenewei_(pset.getParameter<double>("minDisToMaxperthickperlayerenewei")),
      maxDisToMaxperthickperlayerenewei_(pset.getParameter<double>("maxDisToMaxperthickperlayerenewei")),
      nintDisToMaxperthickperlayerenewei_(pset.getParameter<int>("nintDisToMaxperthickperlayerenewei")),

      //Parameters for the distance of seed cell to max cell per thickness per layer
      minDisSeedToMaxperthickperlayer_(pset.getParameter<double>("minDisSeedToMaxperthickperlayer")),
      maxDisSeedToMaxperthickperlayer_(pset.getParameter<double>("maxDisSeedToMaxperthickperlayer")),
      nintDisSeedToMaxperthickperlayer_(pset.getParameter<int>("nintDisSeedToMaxperthickperlayer")),

      //Parameters for the energy of a cluster per thickness per layer
      minClEneperthickperlayer_(pset.getParameter<double>("minClEneperthickperlayer")),
      maxClEneperthickperlayer_(pset.getParameter<double>("maxClEneperthickperlayer")),
      nintClEneperthickperlayer_(pset.getParameter<int>("nintClEneperthickperlayer")),

      //Parameters for the energy density of cluster cells per thickness
      minCellsEneDensperthick_(pset.getParameter<double>("minCellsEneDensperthick")),
      maxCellsEneDensperthick_(pset.getParameter<double>("maxCellsEneDensperthick")),
      nintCellsEneDensperthick_(pset.getParameter<int>("nintCellsEneDensperthick")),

      //Parameters for the energy of a cluster per thickness per layer
      minClEnepermultiplicity_(pset.getParameter<double>("minClEnepermultiplicity")),
      maxClEnepermultiplicity_(pset.getParameter<double>("maxClEnepermultiplicity")),
      nintClEnepermultiplicity_(pset.getParameter<int>("nintClEnepermultiplicity")),

      //parameters for x
      minX_(pset.getParameter<double>("minX")),
      maxX_(pset.getParameter<double>("maxX")),
      nintX_(pset.getParameter<int>("nintX")),

      //parameters for y
      minY_(pset.getParameter<double>("minY")),
      maxY_(pset.getParameter<double>("maxY")),
      nintY_(pset.getParameter<int>("nintY")),

      //parameters for z
      minZ_(pset.getParameter<double>("minZ")),
      maxZ_(pset.getParameter<double>("maxZ")),
      nintZ_(pset.getParameter<int>("nintZ")) {}

PFVHistoProducerAlgo::~PFVHistoProducerAlgo() {}

void PFVHistoProducerAlgo::bookInfo(DQMStore::IBooker& ibook, Histograms& histograms) {
  histograms.lastLayerEEzm = ibook.bookInt("lastLayerEEzm");
  histograms.lastLayerFHzm = ibook.bookInt("lastLayerFHzm");
  histograms.maxlayerzm = ibook.bookInt("maxlayerzm");
  histograms.lastLayerEEzp = ibook.bookInt("lastLayerEEzp");
  histograms.lastLayerFHzp = ibook.bookInt("lastLayerFHzp");
  histograms.maxlayerzp = ibook.bookInt("maxlayerzp");
}

void PFVHistoProducerAlgo::bookCaloParticleHistos(DQMStore::IBooker& ibook,
                                                  Histograms& histograms,
                                                  int pdgid,
                                                  unsigned int layers) {
  histograms.h_caloparticle_eta[pdgid] =
      ibook.book1D("N of caloparticle vs eta", "N of caloParticles vs eta", nintEta_, minEta_, maxEta_);
  histograms.h_caloparticle_eta_Zorigin[pdgid] =
      ibook.book2D("Eta vs Zorigin", "Eta vs Zorigin", nintEta_, minEta_, maxEta_, nintZpos_, minZpos_, maxZpos_);

  histograms.h_caloparticle_energy[pdgid] =
      ibook.book1D("Energy", "Energy of CaloParticles; Energy [GeV]", nintEne_, minEne_, maxEne_);
  histograms.h_caloparticle_pt[pdgid] = ibook.book1D("Pt", "Pt of CaloParticles", nintPt_, minPt_, maxPt_);
  histograms.h_caloparticle_phi[pdgid] = ibook.book1D("Phi", "Phi of CaloParticles", nintPhi_, minPhi_, maxPhi_);
  histograms.h_caloparticle_selfenergy[pdgid] =
      ibook.book1D("SelfEnergy", "Total Energy of Hits in Sim Clusters (matched)", nintEne_, minEne_, maxEne_);
  histograms.h_caloparticle_energyDifference[pdgid] =
      ibook.book1D("EnergyDifference", "(Energy-SelfEnergy)/Energy", 300., -5., 1.);

  histograms.h_caloparticle_nSimClusters[pdgid] =
      ibook.book1D("Num Sim Clusters", "Num Sim Clusters in CaloParticles", 100, 0., 100.);
  histograms.h_caloparticle_nHitsInSimClusters[pdgid] =
      ibook.book1D("Num Hits in Sim Clusters", "Num Hits in Sim Clusters in CaloParticles", 1000, 0., 1000.);
  histograms.h_caloparticle_nHitsInSimClusters_matchedtoRecHit[pdgid] = ibook.book1D(
      "Num Rec-matched Hits in Sim Clusters", "Num Hits in Sim Clusters (matched) in CaloParticles", 1000, 0., 1000.);

  histograms.h_caloparticle_nHits_matched_energy[pdgid] =
      ibook.book1D("Energy of Rec-matched Hits", "Energy of Hits in Sim Clusters (matched)", 100, 0., 10.);
  histograms.h_caloparticle_nHits_matched_energy_layer[pdgid] =
      ibook.book2D("Energy of Rec-matched Hits vs layer",
                   "Energy of Hits in Sim Clusters (matched) vs layer",
                   2 * layers,
                   0.,
                   (float)2 * layers,
                   100,
                   0.,
                   10.);
  histograms.h_caloparticle_nHits_matched_energy_layer_1SimCl[pdgid] =
      ibook.book2D("Energy of Rec-matched Hits vs layer (1SC)",
                   "Energy of Hits only 1 Sim Clusters (matched) vs layer",
                   2 * layers,
                   0.,
                   (float)2 * layers,
                   100,
                   0.,
                   10.);
  histograms.h_caloparticle_sum_energy_layer[pdgid] =
      ibook.book2D("Rec-matched Hits Sum Energy vs layer",
                   "Rescaled Sum Energy of Hits in Sim Clusters (matched) vs layer",
                   2 * layers,
                   0.,
                   (float)2 * layers,
                   110,
                   0.,
                   110.);
  histograms.h_caloparticle_fractions[pdgid] =
      ibook.book2D("HitFractions", "Hit fractions;Hit fraction;E_{hit}^{2} fraction", 101, 0, 1.01, 100, 0, 1);
  histograms.h_caloparticle_fractions_weight[pdgid] = ibook.book2D(
      "HitFractions_weighted", "Hit fractions weighted;Hit fraction;E_{hit}^{2} fraction", 101, 0, 1.01, 100, 0, 1);

  histograms.h_caloparticle_firstlayer[pdgid] =
      ibook.book1D("First Layer", "First layer of the CaloParticles", 2 * layers, 0., (float)2 * layers);
  histograms.h_caloparticle_lastlayer[pdgid] =
      ibook.book1D("Last Layer", "Last layer of the CaloParticles", 2 * layers, 0., (float)2 * layers);
  histograms.h_caloparticle_layersnum[pdgid] =
      ibook.book1D("Number of Layers", "Number of layers of the CaloParticles", 2 * layers, 0., (float)2 * layers);
  histograms.h_caloparticle_firstlayer_matchedtoRecHit[pdgid] = ibook.book1D(
      "First Layer (rec-matched hit)", "First layer of the CaloParticles (matched)", 2 * layers, 0., (float)2 * layers);
  histograms.h_caloparticle_lastlayer_matchedtoRecHit[pdgid] = ibook.book1D(
      "Last Layer (rec-matched hit)", "Last layer of the CaloParticles (matched)", 2 * layers, 0., (float)2 * layers);
  histograms.h_caloparticle_layersnum_matchedtoRecHit[pdgid] =
      ibook.book1D("Number of Layers (rec-matched hit)",
                   "Number of layers of the CaloParticles (matched)",
                   2 * layers,
                   0.,
                   (float)2 * layers);
}

void PFVHistoProducerAlgo::bookSimClusterHistos(DQMStore::IBooker& ibook,
                                                Histograms& histograms,
                                                unsigned int layers,
                                                std::vector<int> thicknesses) {
  //---------------------------------------------------------------------------------------------------------------------------
  for (unsigned ilayer = 0; ilayer < 2 * layers; ++ilayer) {
    auto istr1 = std::to_string(ilayer);
    while (istr1.size() < 2) {
      istr1.insert(0, "0");
    }
    // Make a mapping to the regural layer naming plus z- or z+ for convenience
    std::string istr2 = "";
    // first with the -z endcap
    if (ilayer < layers) {
      istr2 = std::to_string(ilayer + 1) + " in z-";
    } else {  // then for the +z
      istr2 = std::to_string(ilayer - (layers - 1)) + " in z+";
    }
    histograms.h_simclusternum_perlayer[ilayer] = ibook.book1D("totsimclusternum_layer_" + istr1,
                                                               "total number of SimClusters for layer " + istr2,
                                                               nintTotNsimClsperlay_,
                                                               minTotNsimClsperlay_,
                                                               maxTotNsimClsperlay_);

  }  //end of loop over layers
  //---------------------------------------------------------------------------------------------------------------------------
  for (std::vector<int>::iterator it = thicknesses.begin(); it != thicknesses.end(); ++it) {
    auto istr = std::to_string(*it);
    histograms.h_simclusternum_perthick[(*it)] = ibook.book1D("totsimclusternum_thick_" + istr,
                                                              "total number of simclusters for thickness " + istr,
                                                              nintTotNsimClsperthick_,
                                                              minTotNsimClsperthick_,
                                                              maxTotNsimClsperthick_);
  }  //end of loop over thicknesses

  //---------------------------------------------------------------------------------------------------------------------------
  //z-
  histograms.h_mixedhitssimcluster_zminus =
      ibook.book1D("mixedhitssimcluster_zminus",
                   "N of simclusters that contain hits of more than one kind in z-",
                   nintMixedHitsSimCluster_,
                   minMixedHitsSimCluster_,
                   maxMixedHitsSimCluster_);
  //z+
  histograms.h_mixedhitssimcluster_zplus =
      ibook.book1D("mixedhitssimcluster_zplus",
                   "N of simclusters that contain hits of more than one kind in z+",
                   nintMixedHitsSimCluster_,
                   minMixedHitsSimCluster_,
                   maxMixedHitsSimCluster_);
}

void PFVHistoProducerAlgo::bookSimClusterAssociationHistos(DQMStore::IBooker& ibook,
                                                           Histograms& histograms,
                                                           unsigned int layers,
                                                           std::vector<int> thicknesses) {
  std::unordered_map<int, dqm::reco::MonitorElement*> denom_recocl_in_simcl_eta_perlayer;
  denom_recocl_in_simcl_eta_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> denom_recocl_in_simcl_phi_perlayer;
  denom_recocl_in_simcl_phi_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> score_recocl2simcl_perlayer;
  score_recocl2simcl_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> sharedenergy_recocl2simcl_perlayer;
  sharedenergy_recocl2simcl_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> energy_vs_score_recocl2simcl_perlayer;
  energy_vs_score_recocl2simcl_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> num_recocl_in_simcl_eta_perlayer;
  num_recocl_in_simcl_eta_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> num_recocl_in_simcl_phi_perlayer;
  num_recocl_in_simcl_phi_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> numMerge_recocl_in_simcl_eta_perlayer;
  numMerge_recocl_in_simcl_eta_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> numMerge_recocl_in_simcl_phi_perlayer;
  numMerge_recocl_in_simcl_phi_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> sharedenergy_recocl2simcl_vs_eta_perlayer;
  sharedenergy_recocl2simcl_vs_eta_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> sharedenergy_recocl2simcl_vs_phi_perlayer;
  sharedenergy_recocl2simcl_vs_phi_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> denom_simcl_eta_perlayer;
  denom_simcl_eta_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> denom_simcl_phi_perlayer;
  denom_simcl_phi_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> score_simcl2recocl_perlayer;
  score_simcl2recocl_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> sharedenergy_simcl2recocl_perlayer;
  sharedenergy_simcl2recocl_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> energy_vs_score_simcl2recocl_perlayer;
  energy_vs_score_simcl2recocl_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> num_simcl_eta_perlayer;
  num_simcl_eta_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> num_simcl_phi_perlayer;
  num_simcl_phi_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> numDup_simcl_eta_perlayer;
  numDup_simcl_eta_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> numDup_simcl_phi_perlayer;
  numDup_simcl_phi_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> sharedenergy_simcl2recocl_vs_eta_perlayer;
  sharedenergy_simcl2recocl_vs_eta_perlayer.clear();
  std::unordered_map<int, dqm::reco::MonitorElement*> sharedenergy_simcl2recocl_vs_phi_perlayer;
  sharedenergy_simcl2recocl_vs_phi_perlayer.clear();

  //---------------------------------------------------------------------------------------------------------------------------
  for (unsigned ilayer = 0; ilayer < 2 * layers; ++ilayer) {
    auto istr1 = std::to_string(ilayer);
    while (istr1.size() < 2) {
      istr1.insert(0, "0");
    }
    // Make a mapping to the regural layer naming plus z- or z+ for convenience
    std::string istr2 = "";
    // first with the -z endcap
    if (ilayer < layers) {
      istr2 = std::to_string(ilayer + 1) + " in z-";
    } else {  // then for the +z
      istr2 = std::to_string(ilayer - (layers - 1)) + " in z+";
    }
    //-------------------------------------------------------------------------------------------------------------------------
    denom_recocl_in_simcl_eta_perlayer[ilayer] =
        ibook.book1D("Denom_RecoCluster_in_simcl_eta_perlayer" + istr1,
                     "Denom RecoCluster in SimCluster Eta per Layer Cluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    //-------------------------------------------------------------------------------------------------------------------------
    denom_recocl_in_simcl_phi_perlayer[ilayer] =
        ibook.book1D("Denom_RecoCluster_in_simcl_phi_perlayer" + istr1,
                     "Denom RecoCluster in SimCluster Phi per Layer Cluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
    //-------------------------------------------------------------------------------------------------------------------------
    score_recocl2simcl_perlayer[ilayer] = ibook.book1D("Score_recocl2simcl_perlayer" + istr1,
                                                             "Score of Layer Cluster per SimCluster for layer " + istr2,
                                                             nintScore_,
                                                             minScore_,
                                                             maxScore_);
    //-------------------------------------------------------------------------------------------------------------------------
    score_simcl2recocl_perlayer[ilayer] = ibook.book1D("Score_simcl2recocl_perlayer" + istr1,
                                                             "Score of SimCluster per Layer Cluster for layer " + istr2,
                                                             nintScore_,
                                                             minScore_,
                                                             maxScore_);
    //-------------------------------------------------------------------------------------------------------------------------
    energy_vs_score_simcl2recocl_perlayer[ilayer] =
        ibook.book2D("Energy_vs_Score_simcl2layer_perlayer" + istr1,
                     "Energy vs Score of SimCluster per Layer Cluster for layer " + istr2,
                     nintScore_,
                     minScore_,
                     maxScore_,
                     nintSharedEneFrac_,
                     minSharedEneFrac_,
                     maxSharedEneFrac_);
    //-------------------------------------------------------------------------------------------------------------------------
    energy_vs_score_recocl2simcl_perlayer[ilayer] =
        ibook.book2D("Energy_vs_Score_layer2simcluster_perlayer" + istr1,
                     "Energy vs Score of Layer Cluster per SimCluster for layer " + istr2,
                     nintScore_,
                     minScore_,
                     maxScore_,
                     nintSharedEneFrac_,
                     minSharedEneFrac_,
                     maxSharedEneFrac_);
    //-------------------------------------------------------------------------------------------------------------------------
    sharedenergy_simcl2recocl_perlayer[ilayer] =
        ibook.book1D("SharedEnergy_simcl2recocl_perlayer" + istr1,
                     "Shared Energy of SimCluster per Layer Cluster for layer " + istr2,
                     nintSharedEneFrac_,
                     minSharedEneFrac_,
                     maxSharedEneFrac_);
    //-------------------------------------------------------------------------------------------------------------------------
    sharedenergy_simcl2recocl_vs_eta_perlayer[ilayer] =
        ibook.bookProfile("SharedEnergy_simcl2recocl_vs_eta_perlayer" + istr1,
                          "Shared Energy of SimCluster vs #eta per best Layer Cluster for layer " + istr2,
                          nintEta_,
                          minEta_,
                          maxEta_,
                          minSharedEneFrac_,
                          maxSharedEneFrac_);
    //-------------------------------------------------------------------------------------------------------------------------
    sharedenergy_simcl2recocl_vs_phi_perlayer[ilayer] =
        ibook.bookProfile("SharedEnergy_simcl2recocl_vs_phi_perlayer" + istr1,
                          "Shared Energy of SimCluster vs #phi per best Layer Cluster for layer " + istr2,
                          nintPhi_,
                          minPhi_,
                          maxPhi_,
                          minSharedEneFrac_,
                          maxSharedEneFrac_);
    //-------------------------------------------------------------------------------------------------------------------------
    sharedenergy_recocl2simcl_perlayer[ilayer] =
        ibook.book1D("SharedEnergy_recocluster2simcluster_perlayer" + istr1,
                     "Shared Energy of Layer Cluster per SimCluster for layer " + istr2,
                     nintSharedEneFrac_,
                     minSharedEneFrac_,
                     maxSharedEneFrac_);
    //-------------------------------------------------------------------------------------------------------------------------
    sharedenergy_recocl2simcl_vs_eta_perlayer[ilayer] =
        ibook.bookProfile("SharedEnergy_recocl2simcl_vs_eta_perlayer" + istr1,
                          "Shared Energy of RecoCluster vs #eta per best SimCluster for layer " + istr2,
                          nintEta_,
                          minEta_,
                          maxEta_,
                          minSharedEneFrac_,
                          maxSharedEneFrac_);
    //-------------------------------------------------------------------------------------------------------------------------
    sharedenergy_recocl2simcl_vs_phi_perlayer[ilayer] =
        ibook.bookProfile("SharedEnergy_recocl2simcl_vs_phi_perlayer" + istr1,
                          "Shared Energy of RecoCluster vs #phi per best SimCluster for layer " + istr2,
                          nintPhi_,
                          minPhi_,
                          maxPhi_,
                          minSharedEneFrac_,
                          maxSharedEneFrac_);
    //-------------------------------------------------------------------------------------------------------------------------
    num_simcl_eta_perlayer[ilayer] = ibook.book1D("Num_simcl_eta_perlayer" + istr1,
                                                       "Num SimCluster Eta per Layer Cluster for layer " + istr2,
                                                       nintEta_,
                                                       minEta_,
                                                       maxEta_);
    //-------------------------------------------------------------------------------------------------------------------------
    numDup_simcl_eta_perlayer[ilayer] =
        ibook.book1D("NumDup_simcl_eta_perlayer" + istr1,
                     "Num Duplicate SimCluster Eta per Layer Cluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    //-------------------------------------------------------------------------------------------------------------------------
    denom_simcl_eta_perlayer[ilayer] = ibook.book1D("Denom_simcl_eta_perlayer" + istr1,
                                                         "Denom SimCluster Eta per Layer Cluster for layer " + istr2,
                                                         nintEta_,
                                                         minEta_,
                                                         maxEta_);
    //-------------------------------------------------------------------------------------------------------------------------
    num_simcl_phi_perlayer[ilayer] = ibook.book1D("Num_simcl_phi_perlayer" + istr1,
                                                       "Num SimCluster Phi per Layer Cluster for layer " + istr2,
                                                       nintPhi_,
                                                       minPhi_,
                                                       maxPhi_);
    //-------------------------------------------------------------------------------------------------------------------------
    numDup_simcl_phi_perlayer[ilayer] =
        ibook.book1D("NumDup_simcl_phi_perlayer" + istr1,
                     "Num Duplicate SimCluster Phi per Layer Cluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
    //-------------------------------------------------------------------------------------------------------------------------
    denom_simcl_phi_perlayer[ilayer] = ibook.book1D("Denom_simcl_phi_perlayer" + istr1,
                                                         "Denom SimCluster Phi per Layer Cluster for layer " + istr2,
                                                         nintPhi_,
                                                         minPhi_,
                                                         maxPhi_);
    //-------------------------------------------------------------------------------------------------------------------------
    num_recocl_in_simcl_eta_perlayer[ilayer] =
        ibook.book1D("Num_RecoCluster_in_simcl_eta_perlayer" + istr1,
                     "Num RecoCluster Eta per Layer Cluster in SimCluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    //-------------------------------------------------------------------------------------------------------------------------
    numMerge_recocl_in_simcl_eta_perlayer[ilayer] =
        ibook.book1D("NumMerge_RecoCluster_in_simcl_eta_perlayer" + istr1,
                     "Num Merge RecoCluster Eta per Layer Cluster in SimCluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    //-------------------------------------------------------------------------------------------------------------------------
    num_recocl_in_simcl_phi_perlayer[ilayer] =
        ibook.book1D("Num_RecoCluster_in_simcl_phi_perlayer" + istr1,
                     "Num RecoCluster Phi per Layer Cluster in SimCluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
    //-------------------------------------------------------------------------------------------------------------------------
    numMerge_recocl_in_simcl_phi_perlayer[ilayer] =
        ibook.book1D("NumMerge_RecoCluster_in_simcl_phi_perlayer" + istr1,
                     "Num Merge RecoCluster Phi per Layer Cluster in SimCluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);

  }  //end of loop over layers

  histograms.h_denom_recocl_in_simcl_eta_perlayer.push_back(std::move(denom_recocl_in_simcl_eta_perlayer));
  histograms.h_denom_recocl_in_simcl_phi_perlayer.push_back(std::move(denom_recocl_in_simcl_phi_perlayer));
  histograms.h_score_recocl2simcl_perlayer.push_back(std::move(score_recocl2simcl_perlayer));
  histograms.h_sharedenergy_recocl2simcl_perlayer.push_back(std::move(sharedenergy_recocl2simcl_perlayer));
  histograms.h_energy_vs_score_recocl2simcl_perlayer.push_back(
      std::move(energy_vs_score_recocl2simcl_perlayer));
  histograms.h_num_recocl_in_simcl_eta_perlayer.push_back(std::move(num_recocl_in_simcl_eta_perlayer));
  histograms.h_num_recocl_in_simcl_phi_perlayer.push_back(std::move(num_recocl_in_simcl_phi_perlayer));
  histograms.h_numMerge_recocl_in_simcl_eta_perlayer.push_back(std::move(numMerge_recocl_in_simcl_eta_perlayer));
  histograms.h_numMerge_recocl_in_simcl_phi_perlayer.push_back(std::move(numMerge_recocl_in_simcl_phi_perlayer));
  histograms.h_sharedenergy_recocl2simcl_vs_eta_perlayer.push_back(
      std::move(sharedenergy_recocl2simcl_vs_eta_perlayer));
  histograms.h_sharedenergy_recocl2simcl_vs_phi_perlayer.push_back(
      std::move(sharedenergy_recocl2simcl_vs_phi_perlayer));
  histograms.h_denom_simcl_eta_perlayer.push_back(std::move(denom_simcl_eta_perlayer));
  histograms.h_denom_simcl_phi_perlayer.push_back(std::move(denom_simcl_phi_perlayer));
  histograms.h_score_simcl2recocl_perlayer.push_back(std::move(score_simcl2recocl_perlayer));
  histograms.h_sharedenergy_simcl2recocl_perlayer.push_back(std::move(sharedenergy_simcl2recocl_perlayer));
  histograms.h_energy_vs_score_simcl2recocl_perlayer.push_back(
      std::move(energy_vs_score_simcl2recocl_perlayer));
  histograms.h_num_simcl_eta_perlayer.push_back(std::move(num_simcl_eta_perlayer));
  histograms.h_num_simcl_phi_perlayer.push_back(std::move(num_simcl_phi_perlayer));
  histograms.h_numDup_simcl_eta_perlayer.push_back(std::move(numDup_simcl_eta_perlayer));
  histograms.h_numDup_simcl_phi_perlayer.push_back(std::move(numDup_simcl_phi_perlayer));
  histograms.h_sharedenergy_simcl2recocl_vs_eta_perlayer.push_back(
      std::move(sharedenergy_simcl2recocl_vs_eta_perlayer));
  histograms.h_sharedenergy_simcl2recocl_vs_phi_perlayer.push_back(
      std::move(sharedenergy_simcl2recocl_vs_phi_perlayer));
}
void PFVHistoProducerAlgo::bookClusterHistos_ClusterLevel(DQMStore::IBooker& ibook,
                                                          Histograms& histograms,
                                                          unsigned int layers,
                                                          std::vector<int> thicknesses,
                                                          std::string pathtomatbudfile) {
  //---------------------------------------------------------------------------------------------------------------------------
  histograms.h_cluster_eta.push_back(
      ibook.book1D("num_reco_cluster_eta", "N of reco clusters vs eta", nintEta_, minEta_, maxEta_));

  //---------------------------------------------------------------------------------------------------------------------------
  //z-
  histograms.h_mixedhitscluster_zminus.push_back(
      ibook.book1D("mixedhitscluster_zminus",
                   "N of reco clusters that contain hits of more than one kind in z-",
                   nintMixedHitsCluster_,
                   minMixedHitsCluster_,
                   maxMixedHitsCluster_));
  //z+
  histograms.h_mixedhitscluster_zplus.push_back(
      ibook.book1D("mixedhitscluster_zplus",
                   "N of reco clusters that contain hits of more than one kind in z+",
                   nintMixedHitsCluster_,
                   minMixedHitsCluster_,
                   maxMixedHitsCluster_));

  //---------------------------------------------------------------------------------------------------------------------------
  //z-
  histograms.h_energyclustered_zminus.push_back(
      ibook.book1D("energyclustered_zminus",
                   "percent of total energy clustered by all layer clusters over CaloParticles energy in z-",
                   nintEneCl_,
                   minEneCl_,
                   maxEneCl_));
  //z+
  histograms.h_energyclustered_zplus.push_back(
      ibook.book1D("energyclustered_zplus",
                   "percent of total energy clustered by all layer clusters over CaloParticles energy in z+",
                   nintEneCl_,
                   minEneCl_,
                   maxEneCl_));

  //---------------------------------------------------------------------------------------------------------------------------
  //z-
  std::string subpathtomat = pathtomatbudfile.substr(pathtomatbudfile.find("Validation"));
  histograms.h_longdepthbarycentre_zminus.push_back(
      ibook.book1D("longdepthbarycentre_zminus",
                   "The longitudinal depth barycentre in z- for " + subpathtomat,
                   nintLongDepBary_,
                   minLongDepBary_,
                   maxLongDepBary_));
  //z+
  histograms.h_longdepthbarycentre_zplus.push_back(
      ibook.book1D("longdepthbarycentre_zplus",
                   "The longitudinal depth barycentre in z+ for " + subpathtomat,
                   nintLongDepBary_,
                   minLongDepBary_,
                   maxLongDepBary_));

  //---------------------------------------------------------------------------------------------------------------------------
  for (unsigned ilayer = 0; ilayer < 2 * layers; ++ilayer) {
    auto istr1 = std::to_string(ilayer);
    while (istr1.size() < 2) {
      istr1.insert(0, "0");
    }
    // Make a mapping to the regural layer naming plus z- or z+ for convenience
    std::string istr2 = "";
    // first with the -z endcap
    if (ilayer < layers) {
      istr2 = std::to_string(ilayer + 1) + " in z-";
    } else {  // then for the +z
      istr2 = std::to_string(ilayer - (layers - 1)) + " in z+";
    }
    histograms.h_clusternum_perlayer[ilayer] = ibook.book1D("totclusternum_layer_" + istr1,
                                                            "total number of layer clusters for layer " + istr2,
                                                            nintTotNClsperlay_,
                                                            minTotNClsperlay_,
                                                            maxTotNClsperlay_);
    histograms.h_energyclustered_perlayer[ilayer] = ibook.book1D(
        "energyclustered_perlayer" + istr1,
        "percent of total energy clustered by layer clusters over CaloParticles energy for layer " + istr2,
        nintEneClperlay_,
        minEneClperlay_,
        maxEneClperlay_);
  }

  //---------------------------------------------------------------------------------------------------------------------------
  for (std::vector<int>::iterator it = thicknesses.begin(); it != thicknesses.end(); ++it) {
    auto istr = std::to_string(*it);
    histograms.h_clusternum_perthick[(*it)] = ibook.book1D("totclusternum_thick_" + istr,
                                                           "total number of layer clusters for thickness " + istr,
                                                           nintTotNClsperthick_,
                                                           minTotNClsperthick_,
                                                           maxTotNClsperthick_);
  }
  //---------------------------------------------------------------------------------------------------------------------------
}

void PFVHistoProducerAlgo::bookClusterHistos_RCtoCP_association(DQMStore::IBooker& ibook,
                                                                Histograms& histograms,
                                                                unsigned int layers) {
  //----------------------------------------------------------------------------------------------------------------------------
  for (unsigned ilayer = 0; ilayer < 2 * layers; ++ilayer) {
    auto istr1 = std::to_string(ilayer);
    while (istr1.size() < 2) {
      istr1.insert(0, "0");
    }
    // Make a mapping to the regural layer naming plus z- or z+ for convenience
    std::string istr2 = "";
    // first with the -z endcap
    if (ilayer < layers) {
      istr2 = std::to_string(ilayer + 1) + " in z-";
    } else {  // then for the +z
      istr2 = std::to_string(ilayer - (layers - 1)) + " in z+";
    }
    histograms.h_score_recocl2caloparticle_perlayer[ilayer] =
        ibook.book1D("Score_recocl2caloparticle_perlayer" + istr1,
                     "Score of Layer Cluster per CaloParticle for layer " + istr2,
                     nintScore_,
                     minScore_,
                     maxScore_);
    histograms.h_score_caloparticle2recocl_perlayer[ilayer] =
        ibook.book1D("Score_caloparticle2recocl_perlayer" + istr1,
                     "Score of CaloParticle per Layer Cluster for layer " + istr2,
                     nintScore_,
                     minScore_,
                     maxScore_);
    histograms.h_energy_vs_score_caloparticle2recocl_perlayer[ilayer] =
        ibook.book2D("Energy_vs_Score_caloparticle2layer_perlayer" + istr1,
                     "Energy vs Score of CaloParticle per Layer Cluster for layer " + istr2,
                     nintScore_,
                     minScore_,
                     maxScore_,
                     nintSharedEneFrac_,
                     minSharedEneFrac_,
                     maxSharedEneFrac_);
    histograms.h_energy_vs_score_recocl2caloparticle_perlayer[ilayer] =
        ibook.book2D("Energy_vs_Score_layer2caloparticle_perlayer" + istr1,
                     "Energy vs Score of Layer Cluster per CaloParticle Layer for layer " + istr2,
                     nintScore_,
                     minScore_,
                     maxScore_,
                     nintSharedEneFrac_,
                     minSharedEneFrac_,
                     maxSharedEneFrac_);
    histograms.h_sharedenergy_caloparticle2recocl_perlayer[ilayer] =
        ibook.book1D("SharedEnergy_caloparticle2recocl_perlayer" + istr1,
                     "Shared Energy of CaloParticle per Layer Cluster for layer " + istr2,
                     nintSharedEneFrac_,
                     minSharedEneFrac_,
                     maxSharedEneFrac_);
    histograms.h_sharedenergy_caloparticle2recocl_vs_eta_perlayer[ilayer] =
        ibook.bookProfile("SharedEnergy_caloparticle2recocl_vs_eta_perlayer" + istr1,
                          "Shared Energy of CaloParticle vs #eta per best Layer Cluster for layer " + istr2,
                          nintEta_,
                          minEta_,
                          maxEta_,
                          minSharedEneFrac_,
                          maxSharedEneFrac_);
    histograms.h_sharedenergy_caloparticle2recocl_vs_phi_perlayer[ilayer] =
        ibook.bookProfile("SharedEnergy_caloparticle2recocl_vs_phi_perlayer" + istr1,
                          "Shared Energy of CaloParticle vs #phi per best Layer Cluster for layer " + istr2,
                          nintPhi_,
                          minPhi_,
                          maxPhi_,
                          minSharedEneFrac_,
                          maxSharedEneFrac_);
    histograms.h_sharedenergy_recocl2caloparticle_perlayer[ilayer] =
        ibook.book1D("SharedEnergy_recocluster2caloparticle_perlayer" + istr1,
                     "Shared Energy of Layer Cluster per Layer Calo Particle for layer " + istr2,
                     nintSharedEneFrac_,
                     minSharedEneFrac_,
                     maxSharedEneFrac_);
    histograms.h_sharedenergy_recocl2caloparticle_vs_eta_perlayer[ilayer] =
        ibook.bookProfile("SharedEnergy_recocl2caloparticle_vs_eta_perlayer" + istr1,
                          "Shared Energy of RecoCluster vs #eta per best Calo Particle for layer " + istr2,
                          nintEta_,
                          minEta_,
                          maxEta_,
                          minSharedEneFrac_,
                          maxSharedEneFrac_);
    histograms.h_sharedenergy_recocl2caloparticle_vs_phi_perlayer[ilayer] =
        ibook.bookProfile("SharedEnergy_recocl2caloparticle_vs_phi_perlayer" + istr1,
                          "Shared Energy of RecoCluster vs #phi per best Calo Particle for layer " + istr2,
                          nintPhi_,
                          minPhi_,
                          maxPhi_,
                          minSharedEneFrac_,
                          maxSharedEneFrac_);
    histograms.h_num_caloparticle_eta_perlayer[ilayer] =
        ibook.book1D("Num_CaloParticle_Eta_perlayer" + istr1,
                     "Num CaloParticle Eta per Layer Cluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    histograms.h_numDup_caloparticle_eta_perlayer[ilayer] =
        ibook.book1D("NumDup_CaloParticle_Eta_perlayer" + istr1,
                     "Num Duplicate CaloParticle Eta per Layer Cluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    histograms.h_denom_caloparticle_eta_perlayer[ilayer] =
        ibook.book1D("Denom_CaloParticle_Eta_perlayer" + istr1,
                     "Denom CaloParticle Eta per Layer Cluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    histograms.h_num_caloparticle_phi_perlayer[ilayer] =
        ibook.book1D("Num_CaloParticle_Phi_perlayer" + istr1,
                     "Num CaloParticle Phi per Layer Cluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
    histograms.h_numDup_caloparticle_phi_perlayer[ilayer] =
        ibook.book1D("NumDup_CaloParticle_Phi_perlayer" + istr1,
                     "Num Duplicate CaloParticle Phi per Layer Cluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
    histograms.h_denom_caloparticle_phi_perlayer[ilayer] =
        ibook.book1D("Denom_CaloParticle_Phi_perlayer" + istr1,
                     "Denom CaloParticle Phi per Layer Cluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
    histograms.h_num_recocl_eta_perlayer[ilayer] =
        ibook.book1D("Num_RecoCluster_Eta_perlayer" + istr1,
                     "Num RecoCluster Eta per Layer Cluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    histograms.h_numMerge_recocl_eta_perlayer[ilayer] =
        ibook.book1D("NumMerge_RecoCluster_Eta_perlayer" + istr1,
                     "Num Merge RecoCluster Eta per Layer Cluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    histograms.h_denom_recocl_eta_perlayer[ilayer] =
        ibook.book1D("Denom_RecoCluster_Eta_perlayer" + istr1,
                     "Denom RecoCluster Eta per Layer Cluster for layer " + istr2,
                     nintEta_,
                     minEta_,
                     maxEta_);
    histograms.h_num_recocl_phi_perlayer[ilayer] =
        ibook.book1D("Num_RecoCluster_Phi_perlayer" + istr1,
                     "Num RecoCluster Phi per Layer Cluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
    histograms.h_numMerge_recocl_phi_perlayer[ilayer] =
        ibook.book1D("NumMerge_RecoCluster_Phi_perlayer" + istr1,
                     "Num Merge RecoCluster Phi per Layer Cluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
    histograms.h_denom_recocl_phi_perlayer[ilayer] =
        ibook.book1D("Denom_RecoCluster_Phi_perlayer" + istr1,
                     "Denom RecoCluster Phi per Layer Cluster for layer " + istr2,
                     nintPhi_,
                     minPhi_,
                     maxPhi_);
  }
  //---------------------------------------------------------------------------------------------------------------------------
}

void PFVHistoProducerAlgo::bookClusterHistos_CellLevel(DQMStore::IBooker& ibook,
                                                       Histograms& histograms,
                                                       unsigned int layers,
                                                       std::vector<int> thicknesses) {
  //----------------------------------------------------------------------------------------------------------------------------
  for (unsigned ilayer = 0; ilayer < 2 * layers; ++ilayer) {
    auto istr1 = std::to_string(ilayer);
    while (istr1.size() < 2) {
      istr1.insert(0, "0");
    }
    // Make a mapping to the regural layer naming plus z- or z+ for convenience
    std::string istr2 = "";
    // first with the -z endcap
    if (ilayer < layers) {
      istr2 = std::to_string(ilayer + 1) + " in z-";
    } else {  // then for the +z
      istr2 = std::to_string(ilayer - (layers - 1)) + " in z+";
    }
    histograms.h_cellAssociation_perlayer[ilayer] =
        ibook.book1D("cellAssociation_perlayer" + istr1, "Cell Association for layer " + istr2, 5, -4., 1.);
    histograms.h_cellAssociation_perlayer[ilayer]->setBinLabel(2, "TN(purity)");
    histograms.h_cellAssociation_perlayer[ilayer]->setBinLabel(3, "FN(ineff.)");
    histograms.h_cellAssociation_perlayer[ilayer]->setBinLabel(4, "FP(fake)");
    histograms.h_cellAssociation_perlayer[ilayer]->setBinLabel(5, "TP(eff.)");
  }
  //----------------------------------------------------------------------------------------------------------------------------
  for (std::vector<int>::iterator it = thicknesses.begin(); it != thicknesses.end(); ++it) {
    auto istr = std::to_string(*it);
    histograms.h_cellsenedens_perthick[(*it)] = ibook.book1D("cellsenedens_thick_" + istr,
                                                             "energy density of cluster cells for thickness " + istr,
                                                             nintCellsEneDensperthick_,
                                                             minCellsEneDensperthick_,
                                                             maxCellsEneDensperthick_);
  }
  //----------------------------------------------------------------------------------------------------------------------------
  //Not all combination exists but should keep them all for cross checking reason.
  for (std::vector<int>::iterator it = thicknesses.begin(); it != thicknesses.end(); ++it) {
    for (unsigned ilayer = 0; ilayer < 2 * layers; ++ilayer) {
      auto istr1 = std::to_string(*it);
      auto istr2 = std::to_string(ilayer);
      while (istr2.size() < 2)
        istr2.insert(0, "0");
      auto istr = istr1 + "_" + istr2;
      // Make a mapping to the regural layer naming plus z- or z+ for convenience
      std::string istr3 = "";
      // first with the -z endcap
      if (ilayer < layers) {
        istr3 = std::to_string(ilayer + 1) + " in z- ";
      } else {  // then for the +z
        istr3 = std::to_string(ilayer - (layers - 1)) + " in z+ ";
      }
      //---
      histograms.h_cellsnum_perthickperlayer[istr] =
          ibook.book1D("cellsnum_perthick_perlayer_" + istr,
                       "total number of cells for layer " + istr3 + " for thickness " + istr1,
                       nintTotNcellsperthickperlayer_,
                       minTotNcellsperthickperlayer_,
                       maxTotNcellsperthickperlayer_);
      //---
      histograms.h_distancetoseedcell_perthickperlayer[istr] =
          ibook.book1D("distancetoseedcell_perthickperlayer_" + istr,
                       "distance of cluster cells to seed cell for layer " + istr3 + " for thickness " + istr1,
                       nintDisToSeedperthickperlayer_,
                       minDisToSeedperthickperlayer_,
                       maxDisToSeedperthickperlayer_);
      //---
      histograms.h_distancetoseedcell_perthickperlayer_eneweighted[istr] = ibook.book1D(
          "distancetoseedcell_perthickperlayer_eneweighted_" + istr,
          "energy weighted distance of cluster cells to seed cell for layer " + istr3 + " for thickness " + istr1,
          nintDisToSeedperthickperlayerenewei_,
          minDisToSeedperthickperlayerenewei_,
          maxDisToSeedperthickperlayerenewei_);
      //---
      histograms.h_distancetomaxcell_perthickperlayer[istr] =
          ibook.book1D("distancetomaxcell_perthickperlayer_" + istr,
                       "distance of cluster cells to max cell for layer " + istr3 + " for thickness " + istr1,
                       nintDisToMaxperthickperlayer_,
                       minDisToMaxperthickperlayer_,
                       maxDisToMaxperthickperlayer_);
      //---
      histograms.h_distancetomaxcell_perthickperlayer_eneweighted[istr] = ibook.book1D(
          "distancetomaxcell_perthickperlayer_eneweighted_" + istr,
          "energy weighted distance of cluster cells to max cell for layer " + istr3 + " for thickness " + istr1,
          nintDisToMaxperthickperlayerenewei_,
          minDisToMaxperthickperlayerenewei_,
          maxDisToMaxperthickperlayerenewei_);
      //---
      histograms.h_distancebetseedandmaxcell_perthickperlayer[istr] =
          ibook.book1D("distancebetseedandmaxcell_perthickperlayer_" + istr,
                       "distance of seed cell to max cell for layer " + istr3 + " for thickness " + istr1,
                       nintDisSeedToMaxperthickperlayer_,
                       minDisSeedToMaxperthickperlayer_,
                       maxDisSeedToMaxperthickperlayer_);
      //---
      histograms.h_distancebetseedandmaxcellvsclusterenergy_perthickperlayer[istr] = ibook.book2D(
          "distancebetseedandmaxcellvsclusterenergy_perthickperlayer_" + istr,
          "distance of seed cell to max cell vs cluster energy for layer " + istr3 + " for thickness " + istr1,
          nintDisSeedToMaxperthickperlayer_,
          minDisSeedToMaxperthickperlayer_,
          maxDisSeedToMaxperthickperlayer_,
          nintClEneperthickperlayer_,
          minClEneperthickperlayer_,
          maxClEneperthickperlayer_);
    }
  }
}

void PFVHistoProducerAlgo::fill_info_histos(const Histograms& histograms, unsigned int layers) const {
  // Save some info straight from geometry to avoid mistakes from updates
  //----------- TODO ----------------------------------------------------------
  // For now values returned for 'lastLayerFHzp': '104', 'lastLayerFHzm': '52' are not the one expected.
  // Will come back to this when there will be info in CMSSW to put in DQM file.
  histograms.lastLayerEEzm->Fill(recHitTools_->lastLayerEE());
  histograms.lastLayerFHzm->Fill(recHitTools_->lastLayerFH());
  histograms.maxlayerzm->Fill(layers);
  histograms.lastLayerEEzp->Fill(recHitTools_->lastLayerEE() + layers);
  histograms.lastLayerFHzp->Fill(recHitTools_->lastLayerFH() + layers);
  histograms.maxlayerzp->Fill(layers + layers);
}

void PFVHistoProducerAlgo::fill_caloparticle_histos(const Histograms& histograms,
                                                    int pdgid,
                                                    const CaloParticle& caloParticle,
                                                    std::vector<SimVertex> const& simVertices,
                                                    unsigned int layers,
                                                    std::unordered_map<DetId, const unsigned int> const& hitMap,
                                                    MultiVectorManager<HGCRecHit> const& hits) const {
  const auto eta = getEta(caloParticle.eta());
  if (histograms.h_caloparticle_eta.count(pdgid)) {
    histograms.h_caloparticle_eta.at(pdgid)->Fill(eta);
  }
  if (histograms.h_caloparticle_eta_Zorigin.count(pdgid)) {
    histograms.h_caloparticle_eta_Zorigin.at(pdgid)->Fill(
        simVertices.at(caloParticle.g4Tracks()[0].vertIndex()).position().z(), eta);
  }

  if (histograms.h_caloparticle_energy.count(pdgid)) {
    histograms.h_caloparticle_energy.at(pdgid)->Fill(caloParticle.energy());
  }
  if (histograms.h_caloparticle_pt.count(pdgid)) {
    histograms.h_caloparticle_pt.at(pdgid)->Fill(caloParticle.pt());
  }
  if (histograms.h_caloparticle_phi.count(pdgid)) {
    histograms.h_caloparticle_phi.at(pdgid)->Fill(caloParticle.phi());
  }

  if (histograms.h_caloparticle_nSimClusters.count(pdgid)) {
    histograms.h_caloparticle_nSimClusters.at(pdgid)->Fill(caloParticle.simClusters().size());

    int simHits = 0;
    int minLayerId = 999;
    int maxLayerId = 0;

    int simHits_matched = 0;
    int minLayerId_matched = 999;
    int maxLayerId_matched = 0;

    float energy = 0.;
    std::map<int, double> totenergy_layer;

    float hitEnergyWeight_invSum = 0;
    std::vector<std::pair<DetId, float>> haf_cp;
    for (const auto& sc : caloParticle.simClusters()) {
      LogDebug("HGCalValidator") << " This sim cluster has " << sc->hits_and_fractions().size() << " simHits and "
                                 << sc->energy() << " energy. " << std::endl;
      simHits += sc->endcap_hits_and_fractions().size();
      for (auto const& h_and_f : sc->endcap_hits_and_fractions()) {
        const auto hitDetId = h_and_f.first;
        if (recHitTools_->isBarrel(hitDetId))
          continue;
        const int layerId =
            recHitTools_->getLayerWithOffset(hitDetId) + layers * ((recHitTools_->zside(hitDetId) + 1) >> 1) - 1;
        // set to 0 if matched RecHit not found
        int layerId_matched_min = 999;
        int layerId_matched_max = 0;
        std::unordered_map<DetId, const unsigned int>::const_iterator itcheck = hitMap.find(hitDetId);
        if (itcheck != hitMap.end()) {
          layerId_matched_min = layerId;
          layerId_matched_max = layerId;
          simHits_matched++;

          const auto hitEn = (hits[itcheck->second]).energy();
          hitEnergyWeight_invSum += pow(hitEn, 2);
          const auto hitFr = h_and_f.second;
          const auto hitEnFr = hitEn * hitFr;
          energy += hitEnFr;
          histograms.h_caloparticle_nHits_matched_energy.at(pdgid)->Fill(hitEnFr);
          histograms.h_caloparticle_nHits_matched_energy_layer.at(pdgid)->Fill(layerId, hitEnFr);

          if (totenergy_layer.find(layerId) != totenergy_layer.end()) {
            totenergy_layer[layerId] = totenergy_layer.at(layerId) + hitEn;
          } else {
            totenergy_layer.emplace(layerId, hitEn);
          }
          if (caloParticle.simClusters().size() == 1)
            histograms.h_caloparticle_nHits_matched_energy_layer_1SimCl.at(pdgid)->Fill(layerId, hitEnFr);

          auto found = std::find_if(std::begin(haf_cp),
                                    std::end(haf_cp),
                                    [&hitDetId](const std::pair<DetId, float>& v) { return v.first == hitDetId; });
          if (found != haf_cp.end())
            found->second += hitFr;
          else
            haf_cp.emplace_back(hitDetId, hitFr);

        } else {
          LogDebug("HGCalValidator") << "   matched to RecHit NOT found !" << std::endl;
        }

        minLayerId = std::min(minLayerId, layerId);
        maxLayerId = std::max(maxLayerId, layerId);
        minLayerId_matched = std::min(minLayerId_matched, layerId_matched_min);
        maxLayerId_matched = std::max(maxLayerId_matched, layerId_matched_max);
      }
      LogDebug("HGCalValidator") << std::endl;
    }  // End loop over SimClusters of CaloParticle
    if (hitEnergyWeight_invSum)
      hitEnergyWeight_invSum = 1 / hitEnergyWeight_invSum;

    if (minLayerId == 999)
      return;
    histograms.h_caloparticle_firstlayer.at(pdgid)->Fill(minLayerId);
    histograms.h_caloparticle_lastlayer.at(pdgid)->Fill(maxLayerId);
    histograms.h_caloparticle_layersnum.at(pdgid)->Fill(int(maxLayerId - minLayerId));

    histograms.h_caloparticle_firstlayer_matchedtoRecHit.at(pdgid)->Fill(minLayerId_matched);
    histograms.h_caloparticle_lastlayer_matchedtoRecHit.at(pdgid)->Fill(maxLayerId_matched);
    histograms.h_caloparticle_layersnum_matchedtoRecHit.at(pdgid)->Fill(int(maxLayerId_matched - minLayerId_matched));

    histograms.h_caloparticle_nHitsInSimClusters.at(pdgid)->Fill((float)simHits);
    histograms.h_caloparticle_nHitsInSimClusters_matchedtoRecHit.at(pdgid)->Fill((float)simHits_matched);
    histograms.h_caloparticle_selfenergy.at(pdgid)->Fill((float)energy);
    histograms.h_caloparticle_energyDifference.at(pdgid)->Fill((float)1. - energy / caloParticle.energy());

    //Calculate sum energy per-layer
    auto i = totenergy_layer.begin();
    double sum_energy = 0.0;
    while (i != totenergy_layer.end()) {
      sum_energy += i->second;
      histograms.h_caloparticle_sum_energy_layer.at(pdgid)->Fill(i->first, sum_energy / caloParticle.energy() * 100.);
      i++;
    }

    for (auto const& haf : haf_cp) {
      const auto hitEn = (hits[hitMap.find(haf.first)->second]).energy();
      const auto weight = pow(hitEn, 2);
      histograms.h_caloparticle_fractions.at(pdgid)->Fill(haf.second, weight * hitEnergyWeight_invSum);
      histograms.h_caloparticle_fractions_weight.at(pdgid)->Fill(haf.second, weight * hitEnergyWeight_invSum, weight);
    }
  }
}

void PFVHistoProducerAlgo::PFVHistoProducerAlgo::fill_simCluster_histos(const Histograms& histograms,
                                                                        std::vector<SimCluster> const& simClusters,
                                                                        unsigned int layers,
                                                                        std::vector<int> thicknesses) const {
  //Each event to be treated as two events: an event in +ve endcap,
  //plus another event in -ve endcap. In this spirit there will be
  //a layer variable (layerid) that maps the layers in :
  //-z: 0->49
  //+z: 50->99

  //To keep track of total num of simClusters per layer
  //tnscpl[layerid]
  std::vector<int> tnscpl(1000, 0);  //tnscpl.clear(); tnscpl.reserve(1000);

  //To keep track of the total num of clusters per thickness in plus and in minus endcaps
  std::map<std::string, int> tnscpthplus;
  tnscpthplus.clear();
  std::map<std::string, int> tnscpthminus;
  tnscpthminus.clear();
  //At the beginning of the event all layers should be initialized to zero total clusters per thickness
  for (std::vector<int>::iterator it = thicknesses.begin(); it != thicknesses.end(); ++it) {
    tnscpthplus.insert(std::pair<std::string, int>(std::to_string(*it), 0));
    tnscpthminus.insert(std::pair<std::string, int>(std::to_string(*it), 0));
  }
  //To keep track of the total num of simClusters with mixed thickness hits per event
  tnscpthplus.insert(std::pair<std::string, int>("mixed", 0));
  tnscpthminus.insert(std::pair<std::string, int>("mixed", 0));

  //loop through simClusters
  for (const auto& sc : simClusters) {
    //Auxillary variables to count the number of different kind of hits in each simCluster
    int nthhits120p = 0;
    int nthhits200p = 0;
    int nthhits300p = 0;
    int nthhitsscintp = 0;
    int nthhits120m = 0;
    int nthhits200m = 0;
    int nthhits300m = 0;
    int nthhitsscintm = 0;
    //For the hits thickness of the layer cluster.
    double thickness = 0.;
    //To keep track if we added the simCluster in a specific layer
    std::vector<int> occurenceSCinlayer(1000, 0);  //[layerid][0 if not added]

    //loop through hits of the simCluster
    for (const auto& hAndF : sc.hits_and_fractions()) {
      const DetId sh_detid = hAndF.first;

      if (sh_detid.det() == DetId::Forward || sh_detid.det() == DetId::HGCalEE || sh_detid.det() == DetId::HGCalHSi ||
          sh_detid.det() == DetId::HGCalHSc) {
        //The layer the cluster belongs to. As mentioned in the mapping above, it takes into account -z and +z.
        int layerid =
            recHitTools_->getLayerWithOffset(sh_detid) + layers * ((recHitTools_->zside(sh_detid) + 1) >> 1) - 1;
        //zside that the current cluster belongs to.
        int zside = recHitTools_->zside(sh_detid);

        //add the simCluster to the relevant layer. A SimCluster may give contribution to several layers.
        if (occurenceSCinlayer[layerid] == 0) {
          tnscpl[layerid]++;
        }
        occurenceSCinlayer[layerid]++;

        if (sh_detid.det() == DetId::HGCalHSc)
          thickness = -1;
        else
          thickness = recHitTools_->getSiThickness(sh_detid);

        if ((thickness == 120.) && (zside > 0.)) {
          nthhits120p++;
        } else if ((thickness == 120.) && (zside < 0.)) {
          nthhits120m++;
        } else if ((thickness == 200.) && (zside > 0.)) {
          nthhits200p++;
        } else if ((thickness == 200.) && (zside < 0.)) {
          nthhits200m++;
        } else if ((thickness == 300.) && (zside > 0.)) {
          nthhits300p++;
        } else if ((thickness == 300.) && (zside < 0.)) {
          nthhits300m++;
        } else if ((thickness == -1) && (zside > 0.)) {
          nthhitsscintp++;
        } else if ((thickness == -1) && (zside < 0.)) {
          nthhitsscintm++;
        } else {  //assert(0);
          LogDebug("HGCalValidator")
              << " You are running a geometry that contains thicknesses different than the normal ones. "
              << "\n";
        }
      }
    }  //end of loop through hits

    //Check for simultaneously having hits of different kind. Checking at least two combinations is sufficient.
    if ((nthhits120p != 0 && nthhits200p != 0) || (nthhits120p != 0 && nthhits300p != 0) ||
        (nthhits120p != 0 && nthhitsscintp != 0) || (nthhits200p != 0 && nthhits300p != 0) ||
        (nthhits200p != 0 && nthhitsscintp != 0) || (nthhits300p != 0 && nthhitsscintp != 0)) {
      tnscpthplus["mixed"]++;
    } else if ((nthhits120p != 0 || nthhits200p != 0 || nthhits300p != 0 || nthhitsscintp != 0)) {
      //This is a cluster with hits of one kind
      tnscpthplus[std::to_string((int)thickness)]++;
    }
    if ((nthhits120m != 0 && nthhits200m != 0) || (nthhits120m != 0 && nthhits300m != 0) ||
        (nthhits120m != 0 && nthhitsscintm != 0) || (nthhits200m != 0 && nthhits300m != 0) ||
        (nthhits200m != 0 && nthhitsscintm != 0) || (nthhits300m != 0 && nthhitsscintm != 0)) {
      tnscpthminus["mixed"]++;
    } else if ((nthhits120m != 0 || nthhits200m != 0 || nthhits300m != 0 || nthhitsscintm != 0)) {
      //This is a cluster with hits of one kind
      tnscpthminus[std::to_string((int)thickness)]++;
    }
  }  //end of loop through SimClusters of the event

  //Per layer : Loop 0->99
  for (unsigned ilayer = 0; ilayer < layers * 2; ++ilayer)
    if (histograms.h_simclusternum_perlayer.count(ilayer))
      histograms.h_simclusternum_perlayer.at(ilayer)->Fill(tnscpl[ilayer]);

  //Per thickness
  for (std::vector<int>::iterator it = thicknesses.begin(); it != thicknesses.end(); ++it) {
    if (histograms.h_simclusternum_perthick.count(*it)) {
      histograms.h_simclusternum_perthick.at(*it)->Fill(tnscpthplus[std::to_string(*it)]);
      histograms.h_simclusternum_perthick.at(*it)->Fill(tnscpthminus[std::to_string(*it)]);
    }
  }
  //Mixed thickness clusters
  histograms.h_mixedhitssimcluster_zplus->Fill(tnscpthplus["mixed"]);
  histograms.h_mixedhitssimcluster_zminus->Fill(tnscpthminus["mixed"]);
}

void PFVHistoProducerAlgo::fill_cluster_histos(const Histograms& histograms,
                                               const int count,
                                               const reco::PFCluster& cluster) const {
  const auto eta = getEta(cluster.eta());
  histograms.h_cluster_eta[count]->Fill(eta);
}

void PFVHistoProducerAlgo::recoClusters_to_CaloParticles(const Histograms& histograms,
														 edm::Handle<reco::PFClusterCollection> clusterHandle,
														 const reco::PFClusterCollection& clusters,
														 edm::Handle<std::vector<CaloParticle>> caloParticleHandle,
														 std::vector<CaloParticle> const& cP,
														 std::vector<size_t> const& cPIndices,
														 std::vector<size_t> const& cPSelectedIndices,
														 std::unordered_map<DetId, const unsigned int> const& hitMap,
														 unsigned int layers,
														 const ticl::RecoToSimCollectionT<reco::PFClusterCollection>& cpsInClusterMap,
														 const ticl::SimToRecoCollectionT<reco::PFClusterCollection>& cPOnLayerMap,
														 MultiVectorManager<HGCRecHit> const& hits) const {
  const auto nClusters = clusters.size();

  std::unordered_map<DetId, std::vector<PFVHistoProducerAlgo::detIdInfoInCluster>> detIdToCaloParticleId_Map;
  std::unordered_map<DetId, std::vector<PFVHistoProducerAlgo::detIdInfoInCluster>> detIdToClusterId_Map;

  // The association has to be done in an all-vs-all fashion.
  // For this reason use the full set of CaloParticles, with the only filter on bx
  for (const auto& cpId : cPIndices) {
    for (const auto& simCluster : cP[cpId].simClusters()) {
      for (const auto& it_haf : simCluster->hits_and_fractions()) {
        const DetId hitid = (it_haf.first);
        if (recHitTools_->isBarrel(hitid))
          continue;
        if (hitMap.find(hitid) != hitMap.end()) {
          if (detIdToCaloParticleId_Map.find(hitid) == detIdToCaloParticleId_Map.end()) {
            detIdToCaloParticleId_Map[hitid] = std::vector<PFVHistoProducerAlgo::detIdInfoInCluster>();
            detIdToCaloParticleId_Map[hitid].emplace_back(
                PFVHistoProducerAlgo::detIdInfoInCluster{cpId, it_haf.second});
          } else {
            auto findHitIt =
                std::find(detIdToCaloParticleId_Map[hitid].begin(),
                          detIdToCaloParticleId_Map[hitid].end(),
                          PFVHistoProducerAlgo::detIdInfoInCluster{
                              cpId, 0.f});  // only the first element is used for the matching (overloaded operator==)
            if (findHitIt != detIdToCaloParticleId_Map[hitid].end())
              findHitIt->fraction += it_haf.second;
            else
              detIdToCaloParticleId_Map[hitid].emplace_back(
                  PFVHistoProducerAlgo::detIdInfoInCluster{cpId, it_haf.second});
          }
        }
      }
    }
  }

  for (unsigned int cId = 0; cId < nClusters; ++cId) {
    const auto& hits_and_fractions = clusters[cId].hitsAndFractions();
    const auto numberOfHitsInCluster = hits_and_fractions.size();

    // This vector will store, for each hit in the cluster, the index of
    // the CaloParticle that contributed the most, in terms of energy, to it.
    // Special values are:
    //
    // -2  --> the reconstruction fraction of the RecHit is 0 (used in the past to monitor Halo Hits)
    // -3  --> same as before with the added condition that no CaloParticle has been linked to this RecHit
    // -1  --> the reco fraction is >0, but no CaloParticle has been linked to it
    // >=0 --> index of the linked CaloParticle
    std::vector<int> hitsToCaloParticleId(numberOfHitsInCluster);
    const auto firstHitDetId = hits_and_fractions[0].first;
    if (recHitTools_->isBarrel(firstHitDetId))
      continue;
    int clusterLayerId =
        recHitTools_->getLayerWithOffset(firstHitDetId) + layers * ((recHitTools_->zside(firstHitDetId) + 1) >> 1) - 1;

    // This will store the fraction of the CaloParticle energy shared with the cluster: e_shared/cp_energy
    std::unordered_map<unsigned, float> CPEnergyInRC;

    for (unsigned int iHit = 0; iHit < numberOfHitsInCluster; iHit++) {
      const DetId rh_detid = hits_and_fractions[iHit].first;
      const auto rhFraction = hits_and_fractions[iHit].second;

      std::unordered_map<DetId, const unsigned int>::const_iterator itcheck = hitMap.find(rh_detid);
      const HGCRecHit* hit = &(hits[itcheck->second]);

      if (detIdToClusterId_Map.find(rh_detid) == detIdToClusterId_Map.end()) {
        detIdToClusterId_Map[rh_detid] = std::vector<PFVHistoProducerAlgo::detIdInfoInCluster>();
      }
      detIdToClusterId_Map[rh_detid].emplace_back(PFVHistoProducerAlgo::detIdInfoInCluster{cId, rhFraction});

      const auto& hit_find_in_CP = detIdToCaloParticleId_Map.find(rh_detid);

      // if the fraction is zero or the hit does not belong to any calo
      // particle, set the caloparticleId for the hit to -1 this will
      // contribute to the number of noise hits

      // MR Remove the case in which the fraction is 0, since this could be a
      // real hit that has been marked as halo.
      if (rhFraction == 0.) {
        hitsToCaloParticleId[iHit] = -2;
      }
      if (hit_find_in_CP == detIdToCaloParticleId_Map.end()) {
        hitsToCaloParticleId[iHit] -= 1;
      } else {
        auto maxCPEnergyInRC = 0.f;
        auto maxCPId = -1;
        for (auto& h : hit_find_in_CP->second) {
          const auto iCP = h.clusterId;
          CPEnergyInRC[iCP] += h.fraction * hit->energy();
          // Keep track of which CaloParticle contributed the most, in terms
          // of energy, to this specific Cluster.
          if (CPEnergyInRC[iCP] > maxCPEnergyInRC) {
            maxCPEnergyInRC = CPEnergyInRC[iCP];
            maxCPId = iCP;
          }
        }
        hitsToCaloParticleId[iHit] = maxCPId;
      }
      histograms.h_cellAssociation_perlayer.at(clusterLayerId)->Fill(
          hitsToCaloParticleId[iHit] > 0. ? 0. : hitsToCaloParticleId[iHit]);
    }  // End loop over hits on a Cluster

  }  // End of loop over Clusters

  // Fill the plots to compute the different metrics linked to
  // reco-level, namely fake-rate an merge-rate. In this loop should *not*
  // restrict only to the selected caloParaticles.
  for (unsigned int cId = 0; cId < nClusters; ++cId) {
    const auto firstHitDetId = (clusters[cId].hitsAndFractions())[0].first;
    if (recHitTools_->isBarrel(firstHitDetId))
      continue;
    const int clusterLayerId =
        recHitTools_->getLayerWithOffset(firstHitDetId) + layers * ((recHitTools_->zside(firstHitDetId) + 1) >> 1) - 1;
    histograms.h_denom_recocl_eta_perlayer.at(clusterLayerId)->Fill(clusters[cId].eta());
    histograms.h_denom_recocl_phi_perlayer.at(clusterLayerId)->Fill(clusters[cId].phi());
    //
    const edm::Ref<reco::PFClusterCollection> clusterRef(clusterHandle, cId);
    const auto& cpsIt = cpsInClusterMap.find(clusterRef);
    if (cpsIt == cpsInClusterMap.end())
      continue;

    const auto cluster_en = clusters[cId].energy();
    const auto& cps = cpsIt->val;
    if (cluster_en == 0. && !cps.empty()) {
      for (const auto& cpPair : cps)
        histograms.h_score_recocl2caloparticle_perlayer.at(clusterLayerId)->Fill(cpPair.second);
      continue;
    }
    for (const auto& cpPair : cps) {
      LogDebug("HGCalValidator") << "cluster Id: \t" << cId << "\t CP id: \t" << cpPair.first.index()
                                 << "\t score \t" << cpPair.second << std::endl;
      histograms.h_score_recocl2caloparticle_perlayer.at(clusterLayerId)->Fill(cpPair.second);
      auto const& cp_linked =
          std::find_if(std::begin(cPOnLayerMap[cpPair.first]),
                       std::end(cPOnLayerMap[cpPair.first]),
                       [&clusterRef](const std::pair<edm::Ref<reco::PFClusterCollection>, std::pair<float, float>>& p) {
                         return p.first == clusterRef;
                       });
      if (cp_linked ==
          cPOnLayerMap[cpPair.first].end())  // This should never happen by construction of the association maps
        continue;
      histograms.h_sharedenergy_recocl2caloparticle_perlayer.at(clusterLayerId)->Fill(cp_linked->second.first / cluster_en,
                                                                                  cluster_en);
      histograms.h_energy_vs_score_recocl2caloparticle_perlayer.at(clusterLayerId)->Fill(cpPair.second,
                                                                                     cp_linked->second.first / cluster_en);
    }
    const auto assoc =
        std::count_if(std::begin(cps), std::end(cps), [](const auto& obj) { return obj.second < ScoreCutRCtoCP_; });
    if (assoc) {
      histograms.h_num_recocl_eta_perlayer.at(clusterLayerId)->Fill(clusters[cId].eta());
      histograms.h_num_recocl_phi_perlayer.at(clusterLayerId)->Fill(clusters[cId].phi());
      if (assoc > 1) {
        histograms.h_numMerge_recocl_eta_perlayer.at(clusterLayerId)->Fill(clusters[cId].eta());
        histograms.h_numMerge_recocl_phi_perlayer.at(clusterLayerId)->Fill(clusters[cId].phi());
      }
      const auto& best = std::min_element(
          std::begin(cps), std::end(cps), [](const auto& obj1, const auto& obj2) { return obj1.second < obj2.second; });
      const auto& best_cp_linked =
          std::find_if(std::begin(cPOnLayerMap[best->first]),
                       std::end(cPOnLayerMap[best->first]),
                       [&clusterRef](const std::pair<edm::Ref<reco::PFClusterCollection>, std::pair<float, float>>& p) {
                         return p.first == clusterRef;
                       });
      if (best_cp_linked ==
          cPOnLayerMap[best->first].end())  // This should never happen by construction of the association maps
        continue;
      histograms.h_sharedenergy_recocl2caloparticle_vs_eta_perlayer.at(clusterLayerId)->Fill(
          clusters[cId].eta(), best_cp_linked->second.first / cluster_en);
      histograms.h_sharedenergy_recocl2caloparticle_vs_phi_perlayer.at(clusterLayerId)->Fill(
          clusters[cId].phi(), best_cp_linked->second.first / cluster_en);
    }
  }  // End of loop over clusters

  // Here Fill the plots to compute the different metrics linked to
  // gen-level, namely efficiency and duplicate. In this loop should restrict
  // only to the selected caloParticles.
  for (const auto& cpId : cPSelectedIndices) {
    const edm::Ref<CaloParticleCollection> cpRef(caloParticleHandle, cpId);
    const auto& rcsIt = cPOnLayerMap.find(cpRef);

    std::map<unsigned int, float> cPEnergyOnLayer;
    for (unsigned int layerId = 0; layerId < layers * 2; ++layerId)
      cPEnergyOnLayer[layerId] = 0;

    for (const auto& simCluster : cP[cpId].simClusters()) {
      for (const auto& it_haf : simCluster->hits_and_fractions()) {
        const DetId hitid = (it_haf.first);
        if (recHitTools_->isBarrel(hitid))
          continue;
        const auto hitLayerId =
            recHitTools_->getLayerWithOffset(hitid) + layers * ((recHitTools_->zside(hitid) + 1) >> 1) - 1;
        std::unordered_map<DetId, const unsigned int>::const_iterator itcheck = hitMap.find(hitid);
        if (itcheck != hitMap.end()) {
          const HGCRecHit* hit = &(hits[itcheck->second]);
          cPEnergyOnLayer[hitLayerId] += it_haf.second * hit->energy();
        }
      }
    }

    for (unsigned int layerId = 0; layerId < layers * 2; ++layerId) {
      if (!cPEnergyOnLayer[layerId])
        continue;

      histograms.h_denom_caloparticle_eta_perlayer.at(layerId)->Fill(cP[cpId].g4Tracks()[0].momentum().eta());
      histograms.h_denom_caloparticle_phi_perlayer.at(layerId)->Fill(cP[cpId].g4Tracks()[0].momentum().phi());

      if (rcsIt == cPOnLayerMap.end())
        continue;
      const auto& rcs = rcsIt->val;

      auto getRCLayerId = [&](const unsigned int cId) {
        const auto firstHitDetId = (clusters[cId].hitsAndFractions())[0].first;
        if (recHitTools_->isBarrel(firstHitDetId))
          return 9999u;
        const auto clusterLayerId = recHitTools_->getLayerWithOffset(firstHitDetId) +
		  layers * ((recHitTools_->zside(firstHitDetId) + 1) >> 1) - 1;
        return clusterLayerId;
      };

      for (const auto& rcPair : rcs) {
        if (recHitTools_->isBarrel(clusters[rcPair.first.index()].seed()))
          continue;
        if (getRCLayerId(rcPair.first.index()) != layerId)
          continue;
        histograms.h_score_caloparticle2recocl_perlayer.at(layerId)->Fill(rcPair.second.second);
        histograms.h_sharedenergy_caloparticle2recocl_perlayer.at(layerId)->Fill(
            rcPair.second.first / cPEnergyOnLayer[layerId], cPEnergyOnLayer[layerId]);
        histograms.h_energy_vs_score_caloparticle2recocl_perlayer.at(layerId)->Fill(
            rcPair.second.second, rcPair.second.first / cPEnergyOnLayer[layerId]);
      }
      const auto assoc = std::count_if(std::begin(rcs), std::end(rcs), [&](const auto& obj) {
        if (getRCLayerId(obj.first.index()) != layerId)
          return false;
        else
          return obj.second.second < ScoreCutCPtoRC_;
      });
      if (assoc) {
        histograms.h_num_caloparticle_eta_perlayer.at(layerId)->Fill(cP[cpId].g4Tracks()[0].momentum().eta());
        histograms.h_num_caloparticle_phi_perlayer.at(layerId)->Fill(cP[cpId].g4Tracks()[0].momentum().phi());
        if (assoc > 1) {
          histograms.h_numDup_caloparticle_eta_perlayer.at(layerId)->Fill(cP[cpId].g4Tracks()[0].momentum().eta());
          histograms.h_numDup_caloparticle_phi_perlayer.at(layerId)->Fill(cP[cpId].g4Tracks()[0].momentum().phi());
        }
        const auto best = std::min_element(std::begin(rcs), std::end(rcs), [&](const auto& obj1, const auto& obj2) {
          if (getRCLayerId(obj1.first.index()) != layerId)
            return false;
          else if (getRCLayerId(obj2.first.index()) == layerId)
            return obj1.second.second < obj2.second.second;
          else
            return true;
        });
        histograms.h_sharedenergy_caloparticle2recocl_vs_eta_perlayer.at(layerId)->Fill(
            cP[cpId].g4Tracks()[0].momentum().eta(), best->second.first / cPEnergyOnLayer[layerId]);
        histograms.h_sharedenergy_caloparticle2recocl_vs_phi_perlayer.at(layerId)->Fill(
            cP[cpId].g4Tracks()[0].momentum().phi(), best->second.first / cPEnergyOnLayer[layerId]);
      }
    }
  }
}

void PFVHistoProducerAlgo::fill_RecoClusters_to_SimClusters(
    const Histograms& histograms,
    const int count,
    edm::Handle<reco::PFClusterCollection> clusterHandle,
    const reco::PFClusterCollection& clusters,
    edm::Handle<std::vector<SimCluster>> simClusterHandle,
    std::vector<SimCluster> const& sC,
    std::vector<size_t> const& sCIndices,
    const std::vector<float>& mask,
    std::unordered_map<DetId, const unsigned int> const& hitMap,
    unsigned int layers,
    const ticl::RecoToSimCollectionWithSimClustersT<reco::PFClusterCollection>& simClustersInRecoClusterMap,
    const ticl::SimToRecoCollectionWithSimClustersT<reco::PFClusterCollection>& recoClustersInSimClusterMap,
    MultiVectorManager<HGCRecHit> const& hits) const {
  // Here fill the plots to compute the different metrics linked to
  // reco-level, namely fake-rate and merge-rate. In this loop should *not*
  // restrict only to the selected SimClusters.
  for (unsigned int cId = 0; cId < clusters.size(); ++cId) {
    if (mask[cId] != 0.) {
      LogDebug("HGCalValidator") << "Skipping cluster " << cId << " not belonging to mask" << std::endl;
      continue;
    }
    const auto firstHitDetId = (clusters[cId].hitsAndFractions())[0].first;
    if (recHitTools_->isBarrel(firstHitDetId))
      continue;
    const auto clusterLayerId =
        recHitTools_->getLayerWithOffset(firstHitDetId) + layers * ((recHitTools_->zside(firstHitDetId) + 1) >> 1) - 1;
    //Although the ones below are already created in the LC to CP association, will
    //recreate them here since in the post processor it looks in a specific directory.
    histograms.h_denom_recocl_in_simcl_eta_perlayer[count].at(clusterLayerId)->Fill(clusters[cId].eta());
    histograms.h_denom_recocl_in_simcl_phi_perlayer[count].at(clusterLayerId)->Fill(clusters[cId].phi());
    //
    const edm::Ref<reco::PFClusterCollection> clusterRef(clusterHandle, cId);
    const auto& scsIt = simClustersInRecoClusterMap.find(clusterRef);
    if (scsIt == simClustersInRecoClusterMap.end())
      continue;

    const auto cluster_en = clusters[cId].energy();
    const auto& scs = scsIt->val;
    // If a reconstructed cluster has energy 0 but is linked to at least a
    // SimCluster, then his score should be 1 as set in the associator
    if (cluster_en == 0. && !scs.empty()) {
      for (const auto& scPair : scs) {
        histograms.h_score_recocl2simcl_perlayer[count].at(clusterLayerId)->Fill(scPair.second);
      }
      continue;
    }
    //Loop through all SimClusters linked to the layer cluster under study
    for (const auto& scPair : scs) {
      LogDebug("PFVHistoProducerAlgo") << "cluster Id: \t" << cId << "\t SC id: \t" << scPair.first.index()
                                 << "\t score \t" << scPair.second << std::endl;
      //This should be filled #clusters in layer x #linked SimClusters
      histograms.h_score_recocl2simcl_perlayer[count].at(clusterLayerId)->Fill(scPair.second);
      auto const& sc_linked =
          std::find_if(std::begin(recoClustersInSimClusterMap[scPair.first]),
                       std::end(recoClustersInSimClusterMap[scPair.first]),
                       [&clusterRef](const std::pair<edm::Ref<reco::PFClusterCollection>, std::pair<float, float>>& p) {
                         return p.first == clusterRef;
                       });
      if (sc_linked ==
          recoClustersInSimClusterMap[scPair.first].end())  // This should never happen by construction of the association maps
        continue;
      histograms.h_sharedenergy_recocl2simcl_perlayer[count].at(clusterLayerId)->Fill(sc_linked->second.first / cluster_en,
                                                                                       cluster_en);
      histograms.h_energy_vs_score_recocl2simcl_perlayer[count].at(clusterLayerId)->Fill(
          scPair.second, sc_linked->second.first / cluster_en);
    }
    //Here he counts how many of the linked SimClusters of the layer cluster under study have a score above a certain value.
    const auto assoc =
        std::count_if(std::begin(scs), std::end(scs), [](const auto& obj) { return obj.second < ScoreCutRCtoSC_; });
    if (assoc) {
      histograms.h_num_recocl_in_simcl_eta_perlayer[count].at(clusterLayerId)->Fill(clusters[cId].eta());
      histograms.h_num_recocl_in_simcl_phi_perlayer[count].at(clusterLayerId)->Fill(clusters[cId].phi());
      if (assoc > 1) {
        histograms.h_numMerge_recocl_in_simcl_eta_perlayer[count].at(clusterLayerId)->Fill(clusters[cId].eta());
        histograms.h_numMerge_recocl_in_simcl_phi_perlayer[count].at(clusterLayerId)->Fill(clusters[cId].phi());
      }
      const auto& best = std::min_element(
          std::begin(scs), std::end(scs), [](const auto& obj1, const auto& obj2) { return obj1.second < obj2.second; });
      //From all SimClusters he founds the one with the best (lowest) score and takes his scId
      const auto& best_sc_linked =
          std::find_if(std::begin(recoClustersInSimClusterMap[best->first]),
                       std::end(recoClustersInSimClusterMap[best->first]),
                       [&clusterRef](const std::pair<edm::Ref<reco::PFClusterCollection>, std::pair<float, float>>& p) {
                         return p.first == clusterRef;
                       });
      if (best_sc_linked ==
          recoClustersInSimClusterMap[best->first].end())  // This should never happen by construction of the association maps
        continue;
      histograms.h_sharedenergy_recocl2simcl_vs_eta_perlayer[count].at(clusterLayerId)->Fill(
          clusters[cId].eta(), best_sc_linked->second.first / cluster_en);
      histograms.h_sharedenergy_recocl2simcl_vs_phi_perlayer[count].at(clusterLayerId)->Fill(
          clusters[cId].phi(), best_sc_linked->second.first / cluster_en);
    }
  }  // End of loop over clusters

  // Fill the plots to compute the different metrics linked to
  // gen-level, namely efficiency and duplicate. In this loop should restrict
  // only to the selected SimClusters.
  for (const auto& scId : sCIndices) {
    const edm::Ref<SimClusterCollection> scRef(simClusterHandle, scId);
    const auto& rcsIt = recoClustersInSimClusterMap.find(scRef);

    std::map<unsigned int, float> sCEnergyOnLayer;
    for (unsigned int layerId = 0; layerId < layers * 2; ++layerId)
      sCEnergyOnLayer[layerId] = 0;

    for (const auto& it_haf : sC[scId].hits_and_fractions()) {
      const DetId hitid = (it_haf.first);
      if (recHitTools_->isBarrel(hitid))
        continue;
      const auto scLayerId =
          recHitTools_->getLayerWithOffset(hitid) + layers * ((recHitTools_->zside(hitid) + 1) >> 1) - 1;
      std::unordered_map<DetId, const unsigned int>::const_iterator itcheck = hitMap.find(hitid);
      if (itcheck != hitMap.end()) {
        const HGCRecHit* hit = &(hits[itcheck->second]);
        sCEnergyOnLayer[scLayerId] += it_haf.second * hit->energy();
      }
    }

    for (unsigned int layerId = 0; layerId < layers * 2; ++layerId) {
      if (!sCEnergyOnLayer[layerId])
        continue;

      histograms.h_denom_simcl_eta_perlayer[count].at(layerId)->Fill(sC[scId].eta());
      histograms.h_denom_simcl_phi_perlayer[count].at(layerId)->Fill(sC[scId].phi());

      if (rcsIt == recoClustersInSimClusterMap.end())
        continue;
      const auto& rcs = rcsIt->val;

      auto getRCLayerId = [&](const unsigned int cId) {
        const auto firstHitDetId = (clusters[cId].hitsAndFractions())[0].first;
        if (recHitTools_->isBarrel(firstHitDetId))
          return 9999u;
        const unsigned int clusterLayerId = recHitTools_->getLayerWithOffset(firstHitDetId) +
		  layers * ((recHitTools_->zside(firstHitDetId) + 1) >> 1) - 1;
        return clusterLayerId;
      };

      //Loop through layer clusters linked to the SimCluster under study
      for (const auto& rcPair : rcs) {
        auto cId = rcPair.first.index();
        if (mask[cId] != 0.) {
          LogDebug("HGCalValidator") << "Skipping layer cluster " << cId << " not belonging to mask" << std::endl;
          continue;
        }

        if (getRCLayerId(cId) != layerId)
          continue;
        histograms.h_score_simcl2recocl_perlayer[count].at(layerId)->Fill(rcPair.second.second);
        histograms.h_sharedenergy_simcl2recocl_perlayer[count].at(layerId)->Fill(
            rcPair.second.first / sCEnergyOnLayer[layerId], sCEnergyOnLayer[layerId]);
        histograms.h_energy_vs_score_simcl2recocl_perlayer[count].at(layerId)->Fill(
            rcPair.second.second, rcPair.second.first / sCEnergyOnLayer[layerId]);
      }
      const auto assoc = std::count_if(std::begin(rcs), std::end(rcs), [&](const auto& obj) {
        if (getRCLayerId(obj.first.index()) != layerId)
          return false;
        else
          return obj.second.second < ScoreCutSCtoRC_;
      });
      if (assoc) {
        histograms.h_num_simcl_eta_perlayer[count].at(layerId)->Fill(sC[scId].eta());
        histograms.h_num_simcl_phi_perlayer[count].at(layerId)->Fill(sC[scId].phi());
        if (assoc > 1) {
          histograms.h_numDup_simcl_eta_perlayer[count].at(layerId)->Fill(sC[scId].eta());
          histograms.h_numDup_simcl_phi_perlayer[count].at(layerId)->Fill(sC[scId].phi());
        }
        const auto best = std::min_element(std::begin(rcs), std::end(rcs), [&](const auto& obj1, const auto& obj2) {
          if (getRCLayerId(obj1.first.index()) != layerId)
            return false;
          else if (getRCLayerId(obj2.first.index()) == layerId)
            return obj1.second.second < obj2.second.second;
          else
            return true;
        });
        histograms.h_sharedenergy_simcl2recocl_vs_eta_perlayer[count].at(layerId)->Fill(
            sC[scId].eta(), best->second.first / sCEnergyOnLayer[layerId]);
        histograms.h_sharedenergy_simcl2recocl_vs_phi_perlayer[count].at(layerId)->Fill(
            sC[scId].phi(), best->second.first / sCEnergyOnLayer[layerId]);
      }
    }
  }
}

void PFVHistoProducerAlgo::fill_generic_cluster_histos(const Histograms& histograms,
                                                       const int count,
                                                       edm::Handle<reco::PFClusterCollection> clusterHandle,
                                                       const reco::PFClusterCollection& clusters,
                                                       edm::Handle<std::vector<CaloParticle>> caloParticleHandle,
                                                       std::vector<CaloParticle> const& cP,
                                                       std::vector<size_t> const& cPIndices,
                                                       std::vector<size_t> const& cPSelectedIndices,
                                                       std::unordered_map<DetId, const unsigned int> const& hitMap,
                                                       std::map<double, double> cummatbudg,
                                                       unsigned int layers,
                                                       std::vector<int> thicknesses,
                                                       const ticl::RecoToSimCollectionT<reco::PFClusterCollection>& recSimColl,
                                                       const ticl::SimToRecoCollectionT<reco::PFClusterCollection>& simRecColl,
                                                       MultiVectorManager<HGCRecHit> const& hits) const {
  //Each event to be treated as two events: an event in +ve endcap,
  //plus another event in -ve endcap. In this spirit there will be
  //a layer variable (layerid) that maps the layers in :
  //-z: 0->51
  //+z: 52->103

  //To keep track of total num of layer clusters per layer
  //tnlcpl[layerid]
  std::vector<int> tnlcpl(1000, 0);  //tnlcpl.clear(); tnlcpl.reserve(1000);

  //To keep track of the total num of clusters per thickness in plus and in minus endcaps
  std::map<std::string, int> tnlcpthplus;
  tnlcpthplus.clear();
  std::map<std::string, int> tnlcpthminus;
  tnlcpthminus.clear();
  //At the beginning of the event all layers should be initialized to zero total clusters per thickness
  for (std::vector<int>::iterator it = thicknesses.begin(); it != thicknesses.end(); ++it) {
    tnlcpthplus.insert(std::pair<std::string, int>(std::to_string(*it), 0));
    tnlcpthminus.insert(std::pair<std::string, int>(std::to_string(*it), 0));
  }
  //To keep track of the total num of clusters with mixed thickness hits per event
  tnlcpthplus.insert(std::pair<std::string, int>("mixed", 0));
  tnlcpthminus.insert(std::pair<std::string, int>("mixed", 0));

  recoClusters_to_CaloParticles(histograms,
								clusterHandle,
								clusters,
								caloParticleHandle,
								cP,
								cPIndices,
								cPSelectedIndices,
								hitMap,
								layers,
								recSimColl,
								simRecColl,
								hits);

  //To find out the total amount of energy clustered per layer
  //Initialize with zeros because I see clear gives weird numbers.
  std::vector<double> tecpl(1000, 0.0);  //tecpl.clear(); tecpl.reserve(1000);
  //for the longitudinal depth barycenter
  std::vector<double> ldbar(1000, 0.0);  //ldbar.clear(); ldbar.reserve(1000);

  // Need to compare with the total amount of energy coming from CaloParticles
  double caloparteneplus = 0.;
  double caloparteneminus = 0.;
  for (const auto& cpId : cPIndices) {
    if (cP[cpId].eta() >= 0.) {
      caloparteneplus = caloparteneplus + cP[cpId].energy();
    } else if (cP[cpId].eta() < 0.) {
      caloparteneminus = caloparteneminus + cP[cpId].energy();
    }
  }

  // loop through clusters of the event
  for (const auto& cId : clusters) {
    const auto seedid = cId.seed();
    if (recHitTools_->isBarrel(seedid))
      continue;
    const double seedx = recHitTools_->getPosition(seedid).x();
    const double seedy = recHitTools_->getPosition(seedid).y();
    DetId maxid = findmaxhit(cId, hitMap, hits);

    // const DetId maxid = cId.max();
    double maxx = recHitTools_->getPosition(maxid).x();
    double maxy = recHitTools_->getPosition(maxid).y();

    //Auxillary variables to count the number of different kind of hits in each cluster
    int nthhits120p = 0;
    int nthhits200p = 0;
    int nthhits300p = 0;
    int nthhitsscintp = 0;
    int nthhits120m = 0;
    int nthhits200m = 0;
    int nthhits300m = 0;
    int nthhitsscintm = 0;
    //For the hits thickness of the layer cluster.
    double thickness = 0.;
    //The layer the cluster belongs to. As mentioned in the mapping above, it takes into account -z and +z.
    int layerid = 0;
    // Need another layer variable for the longitudinal material budget file reading.
    //In this case need no distinction between -z and +z.
    int lay = 0;
    // Need to save the combination thick_lay
    std::string istr = "";
    //boolean to check for the layer that the cluster belong to. Maybe later will check all the layer hits.
    bool cluslay = true;
    //zside that the current cluster belongs to.
    int zside = 0;

    const auto& hits_and_fractions = cId.hitsAndFractions();
    for (std::vector<std::pair<DetId, float>>::const_iterator it_haf = hits_and_fractions.begin();
         it_haf != hits_and_fractions.end();
         ++it_haf) {
      const DetId rh_detid = it_haf->first;
      //The layer that the current hit belongs to
      layerid = recHitTools_->getLayerWithOffset(rh_detid) + layers * ((recHitTools_->zside(rh_detid) + 1) >> 1) - 1;
      lay = recHitTools_->getLayerWithOffset(rh_detid);
      zside = recHitTools_->zside(rh_detid);
      if (rh_detid.det() == DetId::Forward || rh_detid.det() == DetId::HGCalEE || rh_detid.det() == DetId::HGCalHSi) {
        thickness = recHitTools_->getSiThickness(rh_detid);
      } else if (rh_detid.det() == DetId::HGCalHSc) {
        thickness = -1;
      } else {
        LogDebug("HGCalValidator") << "These are HGCal layer clusters, you shouldn't be here !!! " << layerid << "\n";
        continue;
      }

      //Count here only once the layer cluster and save the combination thick_layerid
      std::string curistr = std::to_string((int)thickness);
      std::string lay_string = std::to_string(layerid);
      while (lay_string.size() < 2)
        lay_string.insert(0, "0");
      curistr += "_" + lay_string;
      if (cluslay) {
        tnlcpl[layerid]++;
        istr = curistr;
        cluslay = false;
      }

      if ((thickness == 120.) && (recHitTools_->zside(rh_detid) > 0.)) {
        nthhits120p++;
      } else if ((thickness == 120.) && (recHitTools_->zside(rh_detid) < 0.)) {
        nthhits120m++;
      } else if ((thickness == 200.) && (recHitTools_->zside(rh_detid) > 0.)) {
        nthhits200p++;
      } else if ((thickness == 200.) && (recHitTools_->zside(rh_detid) < 0.)) {
        nthhits200m++;
      } else if ((thickness == 300.) && (recHitTools_->zside(rh_detid) > 0.)) {
        nthhits300p++;
      } else if ((thickness == 300.) && (recHitTools_->zside(rh_detid) < 0.)) {
        nthhits300m++;
      } else if ((thickness == -1) && (recHitTools_->zside(rh_detid) > 0.)) {
        nthhitsscintp++;
      } else if ((thickness == -1) && (recHitTools_->zside(rh_detid) < 0.)) {
        nthhitsscintm++;
      } else {  //assert(0);
        LogDebug("HGCalValidator")
            << " You are running a geometry that contains thicknesses different than the normal ones. "
            << "\n";
      }

      std::unordered_map<DetId, const unsigned int>::const_iterator itcheck = hitMap.find(rh_detid);
      if (itcheck == hitMap.end()) {
        std::ostringstream st1;
        if ((rh_detid.det() == DetId::HGCalEE) || (rh_detid.det() == DetId::HGCalHSi)) {
          st1 << HGCSiliconDetId(rh_detid);
        } else if (rh_detid.det() == DetId::HGCalHSc) {
          st1 << HGCScintillatorDetId(rh_detid);
        } else {
          st1 << HFNoseDetId(rh_detid);
        }
        LogDebug("HGCalValidator") << " You shouldn't be here - Unable to find a hit " << rh_detid.rawId() << " "
                                   << rh_detid.det() << " " << st1.str() << "\n";
        continue;
      }

      const HGCRecHit* hit = &(hits[itcheck->second]);
      //Here for the per cell plots
      //----
      const double hit_x = recHitTools_->getPosition(rh_detid).x();
      const double hit_y = recHitTools_->getPosition(rh_detid).y();
      double distancetoseed = distance(seedx, seedy, hit_x, hit_y);
      double distancetomax = distance(maxx, maxy, hit_x, hit_y);
      if (distancetoseed != 0. && histograms.h_distancetoseedcell_perthickperlayer.count(curistr)) {
        histograms.h_distancetoseedcell_perthickperlayer.at(curistr)->Fill(distancetoseed);
      }
      //----
      if (distancetoseed != 0. && histograms.h_distancetoseedcell_perthickperlayer_eneweighted.count(curistr)) {
        histograms.h_distancetoseedcell_perthickperlayer_eneweighted.at(curistr)->Fill(distancetoseed, hit->energy());
      }
      //----
      if (distancetomax != 0. && histograms.h_distancetomaxcell_perthickperlayer.count(curistr)) {
        histograms.h_distancetomaxcell_perthickperlayer.at(curistr)->Fill(distancetomax);
      }
      //----
      if (distancetomax != 0. && histograms.h_distancetomaxcell_perthickperlayer_eneweighted.count(curistr)) {
        histograms.h_distancetomaxcell_perthickperlayer_eneweighted.at(curistr)->Fill(distancetomax, hit->energy());
      }

    }  // end of loop through hits and fractions

    //Check for simultaneously having hits of different kind. Checking at least two combinations is sufficient.
    if ((nthhits120p != 0 && nthhits200p != 0) || (nthhits120p != 0 && nthhits300p != 0) ||
        (nthhits120p != 0 && nthhitsscintp != 0) || (nthhits200p != 0 && nthhits300p != 0) ||
        (nthhits200p != 0 && nthhitsscintp != 0) || (nthhits300p != 0 && nthhitsscintp != 0)) {
      tnlcpthplus["mixed"]++;
    } else if ((nthhits120p != 0 || nthhits200p != 0 || nthhits300p != 0 || nthhitsscintp != 0)) {
      //This is a cluster with hits of one kind
      tnlcpthplus[std::to_string((int)thickness)]++;
    }
    if ((nthhits120m != 0 && nthhits200m != 0) || (nthhits120m != 0 && nthhits300m != 0) ||
        (nthhits120m != 0 && nthhitsscintm != 0) || (nthhits200m != 0 && nthhits300m != 0) ||
        (nthhits200m != 0 && nthhitsscintm != 0) || (nthhits300m != 0 && nthhitsscintm != 0)) {
      tnlcpthminus["mixed"]++;
    } else if ((nthhits120m != 0 || nthhits200m != 0 || nthhits300m != 0 || nthhitsscintm != 0)) {
      //This is a cluster with hits of one kind
      tnlcpthminus[std::to_string((int)thickness)]++;
    }

    //To find the thickness with the biggest amount of cells
    std::vector<int> bigamoth;
    bigamoth.clear();
    if (zside > 0) {
      bigamoth.push_back(nthhits120p);
      bigamoth.push_back(nthhits200p);
      bigamoth.push_back(nthhits300p);
      bigamoth.push_back(nthhitsscintp);
    } else if (zside < 0) {
      bigamoth.push_back(nthhits120m);
      bigamoth.push_back(nthhits200m);
      bigamoth.push_back(nthhits300m);
      bigamoth.push_back(nthhitsscintm);
    }
    auto bgth = std::max_element(bigamoth.begin(), bigamoth.end());
    istr = std::to_string(thicknesses[std::distance(bigamoth.begin(), bgth)]);
    std::string lay_string = std::to_string(layerid);
    while (lay_string.size() < 2)
      lay_string.insert(0, "0");
    istr += "_" + lay_string;

    //Here for the per cluster plots that need the thickness_layer info
    if (histograms.h_cellsnum_perthickperlayer.count(istr)) {
      histograms.h_cellsnum_perthickperlayer.at(istr)->Fill(hits_and_fractions.size());
    }

    //Now, with the distance between seed and max cell.
    double distancebetseedandmax = distance(seedx, seedy, maxx, maxy);
    //The thickness_layer combination in this case will use the thickness of the seed as a convention.
    std::string seedstr = std::to_string((int)recHitTools_->getSiThickness(seedid)) + "_" + std::to_string(layerid);
    seedstr += "_" + lay_string;
    if (histograms.h_distancebetseedandmaxcell_perthickperlayer.count(seedstr)) {
      histograms.h_distancebetseedandmaxcell_perthickperlayer.at(seedstr)->Fill(distancebetseedandmax);
    }
    const auto cluster_en = cId.energy();
    if (histograms.h_distancebetseedandmaxcellvsclusterenergy_perthickperlayer.count(seedstr)) {
      histograms.h_distancebetseedandmaxcellvsclusterenergy_perthickperlayer.at(seedstr)->Fill(distancebetseedandmax,
                                                                                               cluster_en);
    }

    //Energy clustered per layer
    tecpl[layerid] = tecpl[layerid] + cluster_en;
    ldbar[layerid] = ldbar[layerid] + cluster_en * cummatbudg[(double)lay];

  }  //end of loop through clusters of the event

  // First a couple of variables to keep the sum of the energy of all clusters
  double sumeneallcluspl = 0.;
  double sumeneallclusmi = 0.;
  // and the longitudinal variable
  double sumldbarpl = 0.;
  double sumldbarmi = 0.;
  //Per layer : Loop 0->103
  for (unsigned ilayer = 0; ilayer < layers * 2; ++ilayer) {
    if (histograms.h_clusternum_perlayer.count(ilayer)) {
      histograms.h_clusternum_perlayer.at(ilayer)->Fill(tnlcpl[ilayer]);
    }
    // Two times one for plus and one for minus
    //First with the -z endcap
    if (ilayer < layers) {
      if (histograms.h_energyclustered_perlayer.count(ilayer)) {
        if (caloparteneminus != 0.) {
          histograms.h_energyclustered_perlayer.at(ilayer)->Fill(100. * tecpl[ilayer] / caloparteneminus);
        }
      }
      //Keep here the total energy for the event in -z
      sumeneallclusmi = sumeneallclusmi + tecpl[ilayer];
      //And for the longitudinal variable
      sumldbarmi = sumldbarmi + ldbar[ilayer];
    } else {  //Then for the +z
      if (histograms.h_energyclustered_perlayer.count(ilayer)) {
        if (caloparteneplus != 0.) {
          histograms.h_energyclustered_perlayer.at(ilayer)->Fill(100. * tecpl[ilayer] / caloparteneplus);
        }
      }
      //Keep here the total energy for the event in -z
      sumeneallcluspl = sumeneallcluspl + tecpl[ilayer];
      //And for the longitudinal variable
      sumldbarpl = sumldbarpl + ldbar[ilayer];
    }  //end of +z loop

  }  //end of loop over layers

  //Per thickness
  for (std::vector<int>::iterator it = thicknesses.begin(); it != thicknesses.end(); ++it) {
    if (histograms.h_clusternum_perthick.count(*it)) {
      histograms.h_clusternum_perthick.at(*it)->Fill(tnlcpthplus[std::to_string(*it)]);
      histograms.h_clusternum_perthick.at(*it)->Fill(tnlcpthminus[std::to_string(*it)]);
    }
  }
  //Mixed thickness clusters
  histograms.h_mixedhitscluster_zplus[count]->Fill(tnlcpthplus["mixed"]);
  histograms.h_mixedhitscluster_zminus[count]->Fill(tnlcpthminus["mixed"]);

  //Total energy clustered from all layer clusters (fraction)
  if (caloparteneplus != 0.) {
    histograms.h_energyclustered_zplus[count]->Fill(100. * sumeneallcluspl / caloparteneplus);
  }
  if (caloparteneminus != 0.) {
    histograms.h_energyclustered_zminus[count]->Fill(100. * sumeneallclusmi / caloparteneminus);
  }

  //For the longitudinal depth barycenter
  histograms.h_longdepthbarycentre_zplus[count]->Fill(sumldbarpl / sumeneallcluspl);
  histograms.h_longdepthbarycentre_zminus[count]->Fill(sumldbarmi / sumeneallclusmi);
}

double PFVHistoProducerAlgo::distance2(const double x1,
                                       const double y1,
                                       const double x2,
                                       const double y2) const {  //distance squared
  const double dx = x1 - x2;
  const double dy = y1 - y2;
  return (dx * dx + dy * dy);
}  //distance squaredq
double PFVHistoProducerAlgo::distance(const double x1,
                                      const double y1,
                                      const double x2,
                                      const double y2) const {  //2-d distance on the layer (x-y)
  return std::sqrt(distance2(x1, y1, x2, y2));
}

void PFVHistoProducerAlgo::setRecHitTools(std::shared_ptr<hgcal::RecHitTools> recHitTools) {
  recHitTools_ = recHitTools;
}

DetId PFVHistoProducerAlgo::findmaxhit(const reco::PFCluster& cluster,
                                       std::unordered_map<DetId, const unsigned int> const& hitMap,
                                       MultiVectorManager<HGCRecHit> const& hits) const {
  const auto& hits_and_fractions = cluster.hitsAndFractions();

  DetId themaxid;
  double maxene = 0.;
  for (std::vector<std::pair<DetId, float>>::const_iterator it_haf = hits_and_fractions.begin();
       it_haf != hits_and_fractions.end();
       ++it_haf) {
    const DetId rh_detid = it_haf->first;
    if (recHitTools_->isBarrel(rh_detid))
      continue;
    const auto hitEn = (hits[hitMap.find(rh_detid)->second]).energy();
    if (maxene < hitEn) {
      maxene = hitEn;
      themaxid = rh_detid;
    }
  }

  return themaxid;
}

double PFVHistoProducerAlgo::getEta(double eta) const {
  if (useFabsEta_)
    return fabs(eta);
  else
    return eta;
}
