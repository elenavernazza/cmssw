// Native-object inputs for ParT and DeepTau ROC comparisons.

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/Common/interface/AssociationVector.h"
#include "DataFormats/Common/interface/RefToBase.h"
#include "DataFormats/Common/interface/RefToBaseProd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/TauReco/interface/PFTau.h"
#include "DataFormats/TauReco/interface/TauDiscriminatorContainer.h"

#include "TTree.h"

#include <cmath>
#include <vector>

class TauROCAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  using JetScoreAssociation = edm::AssociationVector<edm::RefToBaseProd<reco::Jet>,
                                                     std::vector<float>,
                                                     edm::RefToBase<reco::Jet>,
                                                     unsigned int,
                                                     edm::helper::AssociationIdenticalKeyReference>;

  explicit TauROCAnalyzer(const edm::ParameterSet& config)
      : jetsTag_(config.getParameter<edm::InputTag>("jets")),
        partTag_(config.getParameter<edm::InputTag>("partTauvsAll")),
        tausTag_(config.getParameter<edm::InputTag>("taus")),
        deepTauTag_(config.getParameter<edm::InputTag>("deepTauVSjet")),
        genTausTag_(config.getParameter<edm::InputTag>("genVisTaus")),
        matchDR_(config.getParameter<double>("dRmatch")),
        jetsToken_(consumes<std::vector<reco::PFJet>>(jetsTag_)),
        partToken_(consumes<JetScoreAssociation>(partTag_)),
        tausToken_(consumes<std::vector<reco::PFTau>>(tausTag_)),
        deepTauToken_(consumes<reco::TauDiscriminatorContainer>(deepTauTag_)),
        genTausToken_(consumes<std::vector<reco::GenParticle>>(genTausTag_)) {
    usesResource("TFileService");
  }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription description;
    description.add<edm::InputTag>("jets");
    description.add<edm::InputTag>("partTauvsAll");
    description.add<edm::InputTag>("taus");
    description.add<edm::InputTag>("deepTauVSjet");
    description.add<edm::InputTag>("genVisTaus");
    description.add<double>("dRmatch", 0.1);
    descriptions.addWithDefaultLabel(description);
  }

private:
  void beginJob() override {
    edm::Service<TFileService> fileService;

    partTree_ = fileService->make<TTree>("partRaw", "ParT scores on PUPPI jets");
    addBranches_(partTree_, partBranches_);

    deepTauTree_ = fileService->make<TTree>("dtRaw", "DeepTau VSjet scores on PFTaus");
    addBranches_(deepTauTree_, deepTauBranches_);
  }

  void analyze(const edm::Event& event, const edm::EventSetup&) override {
    ++events_;

    const auto genTausHandle = event.getHandle(genTausToken_);
    if (!genTausHandle.isValid())
      return;

    ++genTauProductEvents_;
    const auto& genTaus = *genTausHandle;

    const auto jetsHandle = event.getHandle(jetsToken_);
    const auto partHandle = event.getHandle(partToken_);

    if (jetsHandle.isValid())
      ++jetProductEvents_;
    if (partHandle.isValid())
      ++partProductEvents_;

    if (jetsHandle.isValid() && partHandle.isValid()) {
      const auto& jets = *jetsHandle;
      const auto& partScores = *partHandle;
      for (std::size_t index = 0; index < partScores.size(); ++index) {
        const auto jetRef = partScores.key(index);
        if (jetRef.isNull() || jetRef.id() != jetsHandle.id() || jetRef.key() >= jets.size()) {
          throw cms::Exception("ProductMismatch")
              << "ParT association " << partTag_.encode() << " is not keyed to " << jetsTag_.encode();
        }

        const float score = partScores[index].second;
        if (!validScore_(score))
          continue;

        fillBranches_(partBranches_, jets[jetRef.key()], score, genTaus);
        partTree_->Fill();
      }
    }

    const auto tausHandle = event.getHandle(tausToken_);
    const auto deepTauHandle = event.getHandle(deepTauToken_);

    if (tausHandle.isValid())
      ++tauProductEvents_;
    if (deepTauHandle.isValid())
      ++deepTauProductEvents_;

    if (tausHandle.isValid() && deepTauHandle.isValid()) {
      const auto& taus = *tausHandle;
      const auto& deepTau = *deepTauHandle;
      for (std::size_t index = 0; index < taus.size(); ++index) {
        const auto& result = deepTau.get(tausHandle.id(), index);
        const float score = result.rawValues.empty() ? -1.f : result.rawValues.front();
        if (!validScore_(score))
          continue;

        fillBranches_(deepTauBranches_, taus[index], score, genTaus);
        deepTauTree_->Fill();
      }
    }
  }

  void endJob() override {
    edm::LogPrint("TauROCAnalyzer")
        << "Processed " << events_ << " events; gen taus present in " << genTauProductEvents_
        << ", PUPPI jets present in " << jetProductEvents_
        << ", ParT scores present in " << partProductEvents_
        << ", PFTaus present in " << tauProductEvents_
        << ", DeepTau scores present in " << deepTauProductEvents_;
  }

  struct Branches {
    float pt = 0.f;
    float eta = 0.f;
    float score = 0.f;
    int label = 0;
  };

  static void addBranches_(TTree* tree, Branches& branches) {
    tree->Branch("pt", &branches.pt, "pt/F");
    tree->Branch("eta", &branches.eta, "eta/F");
    tree->Branch("score", &branches.score, "score/F");
    tree->Branch("label", &branches.label, "label/I");
  }

  template <typename Object>
  void fillBranches_(Branches& branches,
                     const Object& object,
                     float score,
                     const std::vector<reco::GenParticle>& genTaus) const {
    branches.pt = object.pt();
    branches.eta = object.eta();
    branches.score = score;
    branches.label = matchedToAny_(object.eta(), object.phi(), genTaus) ? 1 : 0;
  }

  static bool validScore_(float score) { return std::isfinite(score) && score >= 0.f && score <= 1.f; }

  bool matchedToAny_(float eta,
                     float phi,
                     const std::vector<reco::GenParticle>& genTaus) const {
    for (const auto& genTau : genTaus) {
      if (reco::deltaR(eta, phi, genTau.eta(), genTau.phi()) < matchDR_)
        return true;
    }
    return false;
  }

  const edm::InputTag jetsTag_;
  const edm::InputTag partTag_;
  const edm::InputTag tausTag_;
  const edm::InputTag deepTauTag_;
  const edm::InputTag genTausTag_;
  const double matchDR_;

  const edm::EDGetTokenT<std::vector<reco::PFJet>> jetsToken_;
  const edm::EDGetTokenT<JetScoreAssociation> partToken_;
  const edm::EDGetTokenT<std::vector<reco::PFTau>> tausToken_;
  const edm::EDGetTokenT<reco::TauDiscriminatorContainer> deepTauToken_;
  const edm::EDGetTokenT<std::vector<reco::GenParticle>> genTausToken_;

  TTree* partTree_ = nullptr;
  TTree* deepTauTree_ = nullptr;
  Branches partBranches_;
  Branches deepTauBranches_;
  std::size_t events_ = 0;
  std::size_t genTauProductEvents_ = 0;
  std::size_t jetProductEvents_ = 0;
  std::size_t partProductEvents_ = 0;
  std::size_t tauProductEvents_ = 0;
  std::size_t deepTauProductEvents_ = 0;
};

DEFINE_FWK_MODULE(TauROCAnalyzer);