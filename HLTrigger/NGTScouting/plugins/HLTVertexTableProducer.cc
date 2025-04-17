#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidate.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "RecoVertex/VertexPrimitives/interface/ConvertToFromReco.h"
#include "RecoVertex/VertexPrimitives/interface/VertexState.h"
#include "DataFormats/Common/interface/ValueMap.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"

//
// class declaration
//

class HLTVertexTableProducer : public edm::stream::EDProducer<> {
public:
  explicit HLTVertexTableProducer(const edm::ParameterSet&);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  // ----------member data ---------------------------
  const edm::EDGetTokenT<std::vector<reco::Vertex>> pvs_;
  const edm::EDGetTokenT<reco::PFCandidateCollection> pfc_;
  const edm::EDGetTokenT<edm::ValueMap<float>> pvsScore_;
  const StringCutObjectSelector<reco::Vertex> goodPvCut_;
  const std::string goodPvCutString_;
  const std::string pvName_;
  const double dlenMin_, dlenSigMin_;
};

//
// constructors
//

HLTVertexTableProducer::HLTVertexTableProducer(const edm::ParameterSet& params)
    : pvs_(consumes<std::vector<reco::Vertex>>(params.getParameter<edm::InputTag>("pvSrc"))),
      pfc_(consumes<reco::PFCandidateCollection>(params.getParameter<edm::InputTag>("pfSrc"))),
      pvsScore_(consumes<edm::ValueMap<float>>(params.getParameter<edm::InputTag>("pvSrc"))),
      goodPvCut_(params.getParameter<std::string>("goodPvCut"), true),
      goodPvCutString_(params.getParameter<std::string>("goodPvCut")),
      pvName_(params.getParameter<std::string>("pvName")),
      dlenMin_(params.getParameter<double>("dlenMin")),
      dlenSigMin_(params.getParameter<double>("dlenSigMin"))

{
  produces<nanoaod::FlatTable>("PV");
  produces<nanoaod::FlatTable>("otherPVs");
  produces<edm::PtrVector<reco::VertexCompositePtrCandidate>>();
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void HLTVertexTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  
  //vertex collection
  auto pvsIn = iEvent.getHandle(pvs_);
  const auto& pvsScoreProd = iEvent.get(pvsScore_);

  //pf candidates collection
  auto pfcIn = iEvent.getHandle(pfc_);
  
  //table for main primary vertex (only 1)
  auto pvTable = std::make_unique<nanoaod::FlatTable>(1, pvName_, true);
  pvTable->addColumnValue<float>("ndof", (*pvsIn)[0].ndof(), "main primary vertex number of degrees of freedom", 8);
  pvTable->addColumnValue<float>("chi2", (*pvsIn)[0].normalizedChi2(), "main primary vertex reduced chi2", 8);
  pvTable->addColumnValue<float>("x", (*pvsIn)[0].x(), "main primary vertex x coordinate", 10);
  pvTable->addColumnValue<float>("y", (*pvsIn)[0].y(), "main primary vertex y coordinate", 10);
  pvTable->addColumnValue<float>("z", (*pvsIn)[0].z(), "main primary vertex z coordinate", 16);
  pvTable->addColumnValue<float>("xError", (*pvsIn)[0].xError(), "main primary vertex error in x coordinate", 10);
  pvTable->addColumnValue<float>("yError", (*pvsIn)[0].yError(), "main primary vertex error in y coordinate", 10);
  pvTable->addColumnValue<float>("zError", (*pvsIn)[0].zError(), "main primary vertex error in z coordinate", 16);
  pvTable->addColumnValue<float>("score", pvsScoreProd.get(pvsIn.id(), 0), "main primary vertex score, i.e. sum pt2 of clustered objects", 8);
  pvTable->addColumnValue<uint8_t>("tracksSize", (*pvsIn)[0].tracksSize(), "main primary vertex number of associated tracks");
  
  float pv_sumpt2 = 0.0, pv_sumpx = 0.0, pv_sumpy = 0.0;
  for (const auto& obj : *pfcIn) {
    // skip neutrals
    if (obj.charge() == 0) continue;
    double dz = fabs(obj.trackRef()->dz((*pvsIn)[0].position()));
    bool include_pfc = false;
    if (dz < 0.2) {
      include_pfc = true;
      for (size_t j = 1; j < (*pvsIn).size(); j++) {
        double newdz = fabs(obj.trackRef()->dz((*pvsIn)[j].position()));
        if (newdz < dz) {
          include_pfc = false;
          break;
        }
      }  // this pf candidate belongs to other PV
    }
    if (include_pfc) {
      float pfc_pt = obj.pt();
      pv_sumpt2 += pfc_pt * pfc_pt;
      pv_sumpx += obj.px();
      pv_sumpy += obj.py();
    }
  }

  pvTable->addColumnValue<float>("sumpt2", pv_sumpt2, "sum pt2 of pf charged candidates within dz=0.2 for the main primary vertex", 10);
  pvTable->addColumnValue<float>("sumpx", pv_sumpx, "sum px of pf charged candidates within dz=0.2 for the main primary vertex", 10);
  pvTable->addColumnValue<float>("sumpy", pv_sumpy, "sum py of pf charged candidates within dz=0.2 for the main primary vertex", 10);
  
  //number of total (and good) primary vertices
  int goodPVs = 0;
  for (const auto& pv : *pvsIn)
    if (goodPvCut_(pv))
      goodPVs++;
  pvTable->addColumnValue<uint8_t>("npvs", pvsIn->size(), "total number of reconstructed primary vertices");
  pvTable->addColumnValue<uint8_t>("npvsGood", goodPVs, "number of good reconstructed primary vertices (selection:" + goodPvCutString_ + ")");

  //table for other primary vertices (only storing maximum three vertices after the main one)
  auto otherPVsTable = std::make_unique<nanoaod::FlatTable>((*pvsIn).size() > 4 ? 3 : (*pvsIn).size() - 1, "Other" + pvName_, false);
  std::vector<float> pvsz;
  std::vector<float> pvscores;
  for (size_t i = 1; i < (*pvsIn).size() && i < 4; i++) {
    pvsz.push_back((*pvsIn)[i].z());
    pvscores.push_back(pvsScoreProd.get(pvsIn.id(), i));
  }
  otherPVsTable->addColumn<float>("z", pvsz, "z coordinate of other primary vertices, excluding the main PV", 8);
  otherPVsTable->addColumn<float>("score", pvscores, "scores of other primary vertices, excluding the main PV, i.e. sum pt2 of clustered objects", 8);

  iEvent.put(std::move(pvTable), "PV");
  iEvent.put(std::move(otherPVsTable), "otherPVs");
}

// ------------ fill 'descriptions' with the allowed parameters for the module ------------
void HLTVertexTableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<std::string>("pvName")->setComment("name of the flat table ouput");
  desc.add<edm::InputTag>("pvSrc")->setComment("std::vector<reco::Vertex> and ValueMap<float> primary vertex input collections");
  desc.add<edm::InputTag>("pfSrc")->setComment("Tracks input collections");
  desc.add<std::string>("goodPvCut")->setComment("selection on the primary vertex");

  desc.add<double>("dlenMin")->setComment("minimum value of dl to select secondary vertex");
  desc.add<double>("dlenSigMin")->setComment("minimum value of dl significance to select secondary vertex");

  descriptions.addWithDefaultLabel(desc);
}

// ------------ define this as a plug-in ------------ 
DEFINE_FWK_MODULE(HLTVertexTableProducer);
