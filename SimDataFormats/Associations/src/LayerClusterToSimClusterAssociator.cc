// Original Author: Marco Rovere

#include "SimDataFormats/Associations/interface/LayerClusterToSimClusterAssociator.h"

template <typename CLUSTER>
ticl::LayerClusterToSimClusterAssociator::LayerClusterToSimClusterAssociator(
    std::unique_ptr<ticl::LayerClusterToSimClusterAssociatorBaseImpl<CLUSTER>> ptr)
    : m_impl(std::move(ptr)) {}
