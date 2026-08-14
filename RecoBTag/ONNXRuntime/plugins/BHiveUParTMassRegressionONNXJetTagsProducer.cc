#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/makeRefToBaseProdFrom.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/BTauReco/interface/JetTag.h"
#include "DataFormats/BTauReco/interface/UnifiedParticleTransformerAK4Features.h"
#include "DataFormats/BTauReco/interface/UnifiedParticleTransformerAK4TagInfo.h"

#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"

#include <algorithm>
#include <cassert>

using namespace cms::Ort;

// Adapter for the b-hive UParT_v0_mass_reg input contract:
//   input_1: charged PF candidates [N, 26, 20]
//   input_2: neutral PF candidates [N, 25, 10]
//   input_3: secondary vertices    [N,  5, 15]
// and the two four-component outputs "classification" and "regression".
class BHiveUParTMassRegressionONNXJetTagsProducer
    : public edm::stream::EDProducer<edm::GlobalCache<ONNXRuntime>> {
public:
  explicit BHiveUParTMassRegressionONNXJetTagsProducer(const edm::ParameterSet&, const ONNXRuntime*);
  ~BHiveUParTMassRegressionONNXJetTagsProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions&);
  static std::unique_ptr<ONNXRuntime> initializeGlobalCache(const edm::ParameterSet&);
  static void globalEndJob(const ONNXRuntime*) {}

private:
  using TagInfoCollection = std::vector<reco::UnifiedParticleTransformerAK4TagInfo>;
  using JetTagCollection = reco::JetTagCollection;

  void produce(edm::Event&, const edm::EventSetup&) override;
  void makeInputs(const btagbtvdeep::UnifiedParticleTransformerAK4Features&);

  const edm::EDGetTokenT<TagInfoCollection> src_;
  const std::vector<std::string> output_labels_;
  const std::vector<std::string> input_names_;
  const std::vector<std::string> output_names_;

  static constexpr unsigned n_cpf_ = 26;
  static constexpr unsigned n_features_cpf_ = 20;
  static constexpr unsigned n_npf_ = 25;
  static constexpr unsigned n_features_npf_ = 10;
  static constexpr unsigned n_sv_ = 5;
  static constexpr unsigned n_features_sv_ = 15;

  const std::vector<std::vector<int64_t>> input_shapes_ = {
      {1, n_cpf_, n_features_cpf_},
      {1, n_npf_, n_features_npf_},
      {1, n_sv_, n_features_sv_},
  };
  FloatArrays data_;
};

BHiveUParTMassRegressionONNXJetTagsProducer::BHiveUParTMassRegressionONNXJetTagsProducer(
    const edm::ParameterSet& config, const ONNXRuntime*)
    : src_(consumes<TagInfoCollection>(config.getParameter<edm::InputTag>("src"))),
      output_labels_(config.getParameter<std::vector<std::string>>("output_labels")),
      input_names_(config.getParameter<std::vector<std::string>>("input_names")),
      output_names_(config.getParameter<std::vector<std::string>>("output_names")) {
  if (input_names_.size() != 3 || output_names_.size() != 2 || output_labels_.size() != 8) {
    throw cms::Exception("Configuration")
        << "BHiveUParTMassRegressionONNXJetTagsProducer requires 3 input names, 2 output names, and 8 output labels.";
  }
  for (const auto& label : output_labels_) {
    produces<JetTagCollection>(label);
  }
}

void BHiveUParTMassRegressionONNXJetTagsProducer::fillDescriptions(
    edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("src", edm::InputTag("pfUnifiedParticleTransformerAK4TagInfos"));
  desc.add<std::vector<std::string>>("input_names", {"input_1", "input_2", "input_3"});
  desc.add<edm::FileInPath>(
      "model_path",
      edm::FileInPath("RecoBTag/Combined/data/UParTAK4/PUPPI/BsTauTau/UParT_v0_mass_reg.onnx"));
  desc.add<std::vector<std::string>>("output_names", {"classification", "regression"});
  desc.add<std::vector<std::string>>(
      "output_labels", {"ditauh", "ditaumu", "ditaue", "bkg", "masscentral", "massq16", "massq84", "ptnu"});
  descriptions.add("bhiveUParTMassRegressionJetTags", desc);
}

std::unique_ptr<ONNXRuntime> BHiveUParTMassRegressionONNXJetTagsProducer::initializeGlobalCache(
    const edm::ParameterSet& config) {
  return std::make_unique<ONNXRuntime>(config.getParameter<edm::FileInPath>("model_path").fullPath());
}

void BHiveUParTMassRegressionONNXJetTagsProducer::produce(edm::Event& event, const edm::EventSetup&) {
  edm::Handle<TagInfoCollection> tag_infos;
  event.getByToken(src_, tag_infos);

  std::vector<std::unique_ptr<JetTagCollection>> output_tags;
  if (!tag_infos->empty()) {
    auto ref2prod = edm::makeRefToBaseProdFrom(tag_infos->front().jet(), event);
    for (std::size_t i = 0; i < output_labels_.size(); ++i)
      output_tags.emplace_back(std::make_unique<JetTagCollection>(ref2prod));
  } else {
    for (std::size_t i = 0; i < output_labels_.size(); ++i)
      output_tags.emplace_back(std::make_unique<JetTagCollection>());
  }

  for (const auto& tag_info : *tag_infos) {
    std::vector<float> flat_outputs(output_labels_.size(), -1.f);
    if (tag_info.features().is_filled) {
      makeInputs(tag_info.features());
      const auto outputs = globalCache()->run(input_names_, data_, input_shapes_, output_names_, 1);
      if (outputs.size() != 2 || outputs[0].size() != 4 || outputs[1].size() != 4) {
        throw cms::Exception("InvalidONNXOutput")
            << "Expected classification[4] and regression[4], got " << outputs.size() << " output tensors.";
      }
      std::copy(outputs[0].begin(), outputs[0].end(), flat_outputs.begin());
      std::copy(outputs[1].begin(), outputs[1].end(), flat_outputs.begin() + 4);
    }

    for (std::size_t i = 0; i < output_labels_.size(); ++i)
      (*output_tags[i])[tag_info.jet()] = flat_outputs[i];
  }

  for (std::size_t i = 0; i < output_labels_.size(); ++i)
    event.put(std::move(output_tags[i]), output_labels_[i]);
}

void BHiveUParTMassRegressionONNXJetTagsProducer::makeInputs(
    const btagbtvdeep::UnifiedParticleTransformerAK4Features& features) {
  data_.assign({std::vector<float>(n_cpf_ * n_features_cpf_, 0.f),
                std::vector<float>(n_npf_ * n_features_npf_, 0.f),
                std::vector<float>(n_sv_ * n_features_sv_, 0.f)});

  for (std::size_t i = 0; i < std::min<std::size_t>(features.c_pf_features.size(), n_cpf_); ++i) {
    const auto& f = features.c_pf_features[i];
    float* p = &data_[0][i * n_features_cpf_];
    *p = f.btagPf_trackEtaRel;
    *(++p) = f.btagPf_trackPtRel;
    *(++p) = f.btagPf_trackPPar;
    *(++p) = f.btagPf_trackDeltaR;
    *(++p) = f.btagPf_trackPParRatio;
    *(++p) = f.btagPf_trackSip2dVal;
    *(++p) = f.btagPf_trackSip2dSig;
    *(++p) = f.btagPf_trackSip3dVal;
    *(++p) = f.btagPf_trackSip3dSig;
    *(++p) = f.btagPf_trackJetDistVal;
    *(++p) = f.ptrel;
    *(++p) = f.drminsv;
    *(++p) = f.vtx_ass;
    *(++p) = f.puppiw;
    *(++p) = f.chi2;
    *(++p) = f.quality;
    *(++p) = f.pt;
    *(++p) = f.eta;
    *(++p) = f.phi;
    *(++p) = f.e;
  }

  for (std::size_t i = 0; i < std::min<std::size_t>(features.n_pf_features.size(), n_npf_); ++i) {
    const auto& f = features.n_pf_features[i];
    float* p = &data_[1][i * n_features_npf_];
    *p = f.ptrel;
    *(++p) = f.deltaR;
    *(++p) = f.isGamma;
    *(++p) = f.hadFrac;
    *(++p) = f.drminsv;
    *(++p) = f.puppiw;
    *(++p) = f.pt;
    *(++p) = f.eta;
    *(++p) = f.phi;
    *(++p) = f.e;
  }

  for (std::size_t i = 0; i < std::min<std::size_t>(features.sv_features.size(), n_sv_); ++i) {
    const auto& f = features.sv_features[i];
    float* p = &data_[2][i * n_features_sv_];
    *p = f.deltaR;
    *(++p) = f.mass;
    *(++p) = f.ntracks;
    *(++p) = f.chi2;
    *(++p) = f.normchi2;
    *(++p) = f.dxy;
    *(++p) = f.dxysig;
    *(++p) = f.d3d;
    *(++p) = f.d3dsig;
    *(++p) = f.costhetasvpv;
    *(++p) = f.enratio;
    *(++p) = f.pt;
    *(++p) = f.eta;
    *(++p) = f.phi;
    *(++p) = f.e;
  }
}

DEFINE_FWK_MODULE(BHiveUParTMassRegressionONNXJetTagsProducer);
