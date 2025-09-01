// Original Author: Marco Rovere

#include "SimDataFormats/Associations/interface/LayerClusterToSimClusterAssociatorBaseImpl.h"

namespace ticl {
  template <typename CLUSTER>
  LayerClusterToSimClusterAssociatorBaseImpl<CLUSTER>::LayerClusterToSimClusterAssociatorBaseImpl() {}
  template <typename CLUSTER>
  LayerClusterToSimClusterAssociatorBaseImpl<CLUSTER>::~LayerClusterToSimClusterAssociatorBaseImpl() {}

  template <typename CLUSTER>
  RecoToSimCollectionWithSimClusters_T<CLUSTER> LayerClusterToSimClusterAssociatorBaseImpl<CLUSTER>::associateRecoToSim(
      const edm::Handle<CLUSTER> &cCCH, const edm::Handle<SimClusterCollection> &sCCH) const {
    return RecoToSimCollectionWithSimClusters_T<CLUSTER>();
  }

  template <typename CLUSTER>
  SimToRecoCollectionWithSimClusters_T<CLUSTER> LayerClusterToSimClusterAssociatorBaseImpl<CLUSTER>::associateSimToReco(
      const edm::Handle<CLUSTER> &cCCH, const edm::Handle<SimClusterCollection> &sCCH) const {
    return SimToRecoCollectionWithSimClusters_T<CLUSTER>();
  }

  template class ticl::LayerClusterToSimClusterAssociatorBaseImpl<reco::CaloClusterCollection>;
  // template class ticl::LayerClusterToSimClusterAssociatorBaseImpl<reco::PFClusterCollection>;

}  // namespace ticl

