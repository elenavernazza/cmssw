#ifndef SimDataFormats_Associations_LayerClusterToSimClusterAssociatorBaseImpl_h
#define SimDataFormats_Associations_LayerClusterToSimClusterAssociatorBaseImpl_h

/** \class LayerClusterToSimClusterAssociatorBaseImpl
 *
 * Base class for LayerClusterToSimClusterAssociators.  Methods take as input
 * the handle of LayerClusters and the SimCluster collections and return an
 * AssociationMap (oneToManyWithQuality)
 *
 *  \author Marco Rovere
 */

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/CaloRecHit/interface/CaloClusterCollection.h"

#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"

namespace ticl {

  template <typename CLUSTER> 
  using SimToRecoCollectionWithSimClusters_T = edm::AssociationMap<
      edm::OneToManyWithQualityGeneric<SimClusterCollection, CLUSTER, std::pair<float, float>>>;
  template <typename CLUSTER>
  using RecoToSimCollectionWithSimClusters_T = edm::AssociationMap<
      edm::OneToManyWithQualityGeneric<CLUSTER, SimClusterCollection, float>>;

  template <typename CLUSTER>
  class LayerClusterToSimClusterAssociatorBaseImpl {
  public:
    /// Constructor
    LayerClusterToSimClusterAssociatorBaseImpl();
    /// Destructor
    virtual ~LayerClusterToSimClusterAssociatorBaseImpl();

    /// Associate a LayerCluster to SimClusters
    virtual RecoToSimCollectionWithSimClusters_T<CLUSTER> associateRecoToSim(
        const edm::Handle<CLUSTER> &cCH, const edm::Handle<SimClusterCollection> &sCCH) const;

    /// Associate a SimCluster to LayerClusters
    virtual SimToRecoCollectionWithSimClusters_T<CLUSTER> associateSimToReco(
        const edm::Handle<CLUSTER> &cCH, const edm::Handle<SimClusterCollection> &sCCH) const;
  };
}  // namespace ticl

extern template class ticl::LayerClusterToSimClusterAssociatorBaseImpl<reco::CaloClusterCollection>;

#endif
