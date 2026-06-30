#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/makeRefToBaseProdFrom.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/BTauReco/interface/JetTag.h"
#include "DataFormats/BTauReco/interface/UnifiedParticleTransformerAK4Features.h"
#include "DataFormats/BTauReco/interface/UnifiedParticleTransformerAK4TagInfo.h"

#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"

#include <cmath>

using namespace cms::Ort;

class BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer
    : public edm::stream::EDProducer<edm::GlobalCache<ONNXRuntime>> {
public:
  explicit BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer(const edm::ParameterSet&, const ONNXRuntime*);
  ~BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions&);
  static std::unique_ptr<ONNXRuntime> initializeGlobalCache(const edm::ParameterSet&);
  static void globalEndJob(const ONNXRuntime*) {}

private:
  using TagInfoCollection = std::vector<reco::UnifiedParticleTransformerAK4TagInfo>;
  using JetTagCollection = reco::JetTagCollection;

  enum InputIndexes {
    kChargedCandidates = 0,
    kNeutralCandidates = 1,
    kVertices = 2,
    kLostTracks = 3,
    kPairs = 4
  };

  void produce(edm::Event&, const edm::EventSetup&) override;
  void makeInputs(const btagbtvdeep::UnifiedParticleTransformerAK4Features& features);

  const edm::EDGetTokenT<TagInfoCollection> src_;
  const std::vector<std::string> flav_names_;
  const std::vector<std::string> input_names_;
  const std::vector<std::string> output_names_;

  static constexpr unsigned n_cpf_ = 25;
  static constexpr unsigned n_features_cpf_ = 34;
  static constexpr unsigned n_npf_ = 20;
  static constexpr unsigned n_features_npf_ = 14;
  static constexpr unsigned n_sv_ = 5;
  static constexpr unsigned n_features_sv_ = 18;
  static constexpr unsigned n_lt_ = 6;
  static constexpr unsigned n_features_lt_ = 28;
  static constexpr unsigned n_pair_ = 300;
  static constexpr unsigned n_features_pair_ = 10;

  const std::vector<std::vector<int64_t>> input_shapes_ = {
      {1, n_cpf_, n_features_cpf_},
      {1, n_npf_, n_features_npf_},
      {1, n_sv_, n_features_sv_},
      {1, n_lt_, n_features_lt_},
      {1, n_pair_, n_features_pair_},
  };

  FloatArrays data_;
};

BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer::BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer(
    const edm::ParameterSet& iConfig, const ONNXRuntime*)
    : src_(consumes<TagInfoCollection>(iConfig.getParameter<edm::InputTag>("src"))),
      flav_names_(iConfig.getParameter<std::vector<std::string>>("flav_names")),
      input_names_(iConfig.getParameter<std::vector<std::string>>("input_names")),
      output_names_(iConfig.getParameter<std::vector<std::string>>("output_names")) {
  for (const auto& flav_name : flav_names_) {
    produces<JetTagCollection>(flav_name);
  }
}

void BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer::fillDescriptions(
    edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("src", edm::InputTag("pfUnifiedParticleTransformerAK4TagInfos"));
  desc.add<std::vector<std::string>>("input_names", {"input_1", "input_2", "input_3", "input_4", "input_5"});
  desc.add<edm::FileInPath>("model_path", edm::FileInPath("RecoBTag/Combined/data/UParTAK4/PUPPI/V01/modelfile/model.onnx"));
  desc.add<std::vector<std::string>>("output_names", {"output"});
  desc.add<std::vector<std::string>>("flav_names", {});
  descriptions.add("bhiveUnifiedParticleTransformerAK4JetTags", desc);
}

std::unique_ptr<ONNXRuntime> BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer::initializeGlobalCache(
    const edm::ParameterSet& iConfig) {
  return std::make_unique<ONNXRuntime>(iConfig.getParameter<edm::FileInPath>("model_path").fullPath());
}

void BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer::produce(edm::Event& iEvent,
                                                                    const edm::EventSetup&) {
  edm::Handle<TagInfoCollection> tag_infos;
  iEvent.getByToken(src_, tag_infos);

  std::vector<std::unique_ptr<JetTagCollection>> output_tags;
  if (!tag_infos->empty()) {
    auto jet_ref = tag_infos->begin()->jet();
    auto ref2prod = edm::makeRefToBaseProdFrom(jet_ref, iEvent);
    for (std::size_t i = 0; i < flav_names_.size(); ++i) {
      output_tags.emplace_back(std::make_unique<JetTagCollection>(ref2prod));
    }
  } else {
    for (std::size_t i = 0; i < flav_names_.size(); ++i) {
      output_tags.emplace_back(std::make_unique<JetTagCollection>());
    }
  }

  for (unsigned jet_n = 0; jet_n < tag_infos->size(); ++jet_n) {
    const auto& taginfo = (*tag_infos)[jet_n];
    std::vector<float> outputs(flav_names_.size(), -1.0);
    if (taginfo.features().is_filled) {
      makeInputs(taginfo.features());
      outputs = globalCache()->run(input_names_, data_, input_shapes_, output_names_, 1)[0];
      assert(outputs.size() == flav_names_.size());
    }

    const auto& jet_ref = taginfo.jet();
    for (std::size_t flav_n = 0; flav_n < flav_names_.size(); ++flav_n) {
      (*(output_tags[flav_n]))[jet_ref] = outputs[flav_n];
    }
  }

  for (std::size_t flav_n = 0; flav_n < flav_names_.size(); ++flav_n) {
    iEvent.put(std::move(output_tags[flav_n]), flav_names_[flav_n]);
  }
}

void BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer::makeInputs(
    const btagbtvdeep::UnifiedParticleTransformerAK4Features& features) {
  data_.clear();
  data_.emplace_back(n_cpf_ * n_features_cpf_, 0.f);
  data_.emplace_back(n_npf_ * n_features_npf_, 0.f);
  data_.emplace_back(n_sv_ * n_features_sv_, 0.f);
  data_.emplace_back(n_lt_ * n_features_lt_, 0.f);
  data_.emplace_back(n_pair_ * n_features_pair_, 0.f);

  for (std::size_t i = 0; i < std::min(features.c_pf_features.size(), static_cast<std::size_t>(n_cpf_)); ++i) {
    const auto& f = features.c_pf_features.at(i);
    float* ptr = &data_[kChargedCandidates][i * n_features_cpf_];
    *ptr = f.btagPf_trackEtaRel;
    *(++ptr) = f.btagPf_trackPtRel;
    *(++ptr) = f.btagPf_trackPPar;
    *(++ptr) = f.btagPf_trackDeltaR;
    *(++ptr) = f.btagPf_trackPParRatio;
    *(++ptr) = f.btagPf_trackSip2dVal;
    *(++ptr) = f.btagPf_trackSip2dSig;
    *(++ptr) = f.btagPf_trackSip3dVal;
    *(++ptr) = f.btagPf_trackSip3dSig;
    *(++ptr) = f.btagPf_trackJetDistVal;
    *(++ptr) = f.ptrel;
    *(++ptr) = 0.f;  // Cpfcan_qdotp is not stored in UnifiedParticleTransformerAK4Features.
    *(++ptr) = 0.f;  // Cpfcan_qoverp is not stored in UnifiedParticleTransformerAK4Features.
    *(++ptr) = f.drminsv;
    *(++ptr) = f.distminsv;
    *(++ptr) = f.vtx_ass;
    *(++ptr) = f.puppiw;
    *(++ptr) = f.chi2;
    *(++ptr) = f.quality;
    *(++ptr) = f.pt;
    *(++ptr) = f.charge;
    *(++ptr) = f.dz;
    *(++ptr) = f.btagPf_trackDecayLen;
    *(++ptr) = f.HadFrac;
    *(++ptr) = f.CaloFrac;
    *(++ptr) = f.pdgID;
    *(++ptr) = f.lostInnerHits;
    *(++ptr) = f.numberOfPixelHits;
    *(++ptr) = f.numberOfStripHits;
    *(++ptr) = 0.f;  // Cpfcan_tau_signal is not stored in UnifiedParticleTransformerAK4Features.
    *(++ptr) = f.px;
    *(++ptr) = f.py;
    *(++ptr) = f.pz;
    *(++ptr) = f.e;
  }

  for (std::size_t i = 0; i < std::min(features.n_pf_features.size(), static_cast<std::size_t>(n_npf_)); ++i) {
    const auto& f = features.n_pf_features.at(i);
    float* ptr = &data_[kNeutralCandidates][i * n_features_npf_];
    *ptr = f.pt;
    *(++ptr) = f.ptrel;
    *(++ptr) = f.etarel;
    *(++ptr) = f.phirel;
    *(++ptr) = f.deltaR;
    *(++ptr) = f.isGamma;
    *(++ptr) = f.hadFrac;
    *(++ptr) = f.drminsv;
    *(++ptr) = f.puppiw;
    *(++ptr) = 0.f;  // Npfcan_tau_signal is not stored in UnifiedParticleTransformerAK4Features.
    *(++ptr) = f.px;
    *(++ptr) = f.py;
    *(++ptr) = f.pz;
    *(++ptr) = f.e;
  }

  for (std::size_t i = 0; i < std::min(features.sv_features.size(), static_cast<std::size_t>(n_sv_)); ++i) {
    const auto& f = features.sv_features.at(i);
    float* ptr = &data_[kVertices][i * n_features_sv_];
    *ptr = f.pt;
    *(++ptr) = f.deltaR;
    *(++ptr) = f.mass;
    *(++ptr) = f.ntracks;
    *(++ptr) = f.etarel;
    *(++ptr) = f.phirel;
    *(++ptr) = f.chi2;
    *(++ptr) = f.normchi2;
    *(++ptr) = f.dxy;
    *(++ptr) = f.dxysig;
    *(++ptr) = f.d3d;
    *(++ptr) = f.d3dsig;
    *(++ptr) = f.costhetasvpv;
    *(++ptr) = f.enratio;
    *(++ptr) = f.px;
    *(++ptr) = f.py;
    *(++ptr) = f.pz;
    *(++ptr) = f.e;
  }

  for (std::size_t i = 0; i < std::min(features.lt_features.size(), static_cast<std::size_t>(n_lt_)); ++i) {
    const auto& f = features.lt_features.at(i);
    float* ptr = &data_[kLostTracks][i * n_features_lt_];
    *ptr = f.btagPf_trackEtaRel;
    *(++ptr) = f.btagPf_trackPtRel;
    *(++ptr) = f.btagPf_trackPPar;
    *(++ptr) = f.btagPf_trackDeltaR;
    *(++ptr) = f.btagPf_trackPParRatio;
    *(++ptr) = f.btagPf_trackSip2dVal;
    *(++ptr) = f.btagPf_trackSip2dSig;
    *(++ptr) = f.btagPf_trackSip3dVal;
    *(++ptr) = f.btagPf_trackSip3dSig;
    *(++ptr) = f.btagPf_trackJetDistVal;
    *(++ptr) = f.drminsv;
    *(++ptr) = f.puppiw;
    *(++ptr) = f.chi2;
    *(++ptr) = f.quality;
    *(++ptr) = f.charge;
    *(++ptr) = 0.f;  // LT_dz is not stored in LostTracksFeatures.
    *(++ptr) = 0.f;  // LT_BtagPf_trackDecayLen is not stored in LostTracksFeatures.
    *(++ptr) = 0.f;  // LT_HadFrac is not stored in LostTracksFeatures.
    *(++ptr) = 0.f;  // LT_CaloFrac is not stored in LostTracksFeatures.
    *(++ptr) = 0.f;  // LT_pdgID is not stored in LostTracksFeatures.
    *(++ptr) = f.lostInnerHits;
    *(++ptr) = f.numberOfPixelHits;
    *(++ptr) = f.numberOfStripHits;
    *(++ptr) = f.pt;
    *(++ptr) = f.pt * std::cos(f.phi);
    *(++ptr) = f.pt * std::sin(f.phi);
    *(++ptr) = f.pt * std::sinh(f.eta);
    *(++ptr) = f.e;
  }
}

DEFINE_FWK_MODULE(BHiveUnifiedParticleTransformerAK4ONNXJetTagsProducer);
