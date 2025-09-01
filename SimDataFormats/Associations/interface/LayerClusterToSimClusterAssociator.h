#ifndef SimDataFormats_Associations_LayerClusterToSimClusterAssociator_h
#define SimDataFormats_Associations_LayerClusterToSimClusterAssociator_h
// Original Author:  Marco Rovere

// system include files
#include <memory>

// user include files

#include "SimDataFormats/Associations/interface/LayerClusterToSimClusterAssociatorBaseImpl.h"

// forward declarations

namespace ticl {

  template <typename CLUSTER>
  class LayerClusterToSimClusterAssociator {
  public:
    explicit LayerClusterToSimClusterAssociator(
      std::unique_ptr<LayerClusterToSimClusterAssociatorBaseImpl<CLUSTER>> impl)
      : m_impl(std::move(impl)) {}
    LayerClusterToSimClusterAssociator() = default;
    LayerClusterToSimClusterAssociator(LayerClusterToSimClusterAssociator &&) = default;
    LayerClusterToSimClusterAssociator &operator=(LayerClusterToSimClusterAssociator &&) = default;
    LayerClusterToSimClusterAssociator(const LayerClusterToSimClusterAssociator &) = delete;  // stop default

    ~LayerClusterToSimClusterAssociator() = default;
    const LayerClusterToSimClusterAssociator &operator=(const LayerClusterToSimClusterAssociator &) =
        delete;  // stop default
    // ---------- const member functions ---------------------
    /// Associate a LayerCluster to SimClusters
    RecoToSimCollectionWithSimClusters_T<CLUSTER> associateRecoToSim(const edm::Handle<CLUSTER> &cCCH,
                                                                const edm::Handle<SimClusterCollection> &sCCH) const {
      return m_impl->associateRecoToSim(cCCH, sCCH);
    };

    /// Associate a SimCluster to LayerClusters
    SimToRecoCollectionWithSimClusters_T<CLUSTER> associateSimToReco(const edm::Handle<CLUSTER> &cCCH,
                                                                const edm::Handle<SimClusterCollection> &sCCH) const {
      return m_impl->associateSimToReco(cCCH, sCCH);
    }

  private:
    // ---------- member data --------------------------------
    std::unique_ptr<LayerClusterToSimClusterAssociatorBaseImpl<CLUSTER>> m_impl;
  };
}  // namespace ticl

extern template class ticl::LayerClusterToSimClusterAssociator<reco::CaloClusterCollection>;
// extern template class ticl::LayerClusterToSimClusterAssociator<reco::PFClusterCollection>;

using LayerClusterToSimClusterAssociator = ticl::LayerClusterToSimClusterAssociator<reco::CaloClusterCollection>;
// using PFClusterToSimClusterAssociator = ticl::LayerClusterToSimClusterAssociator<reco::PFClusterCollection>;

#endif
