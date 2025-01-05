#include "RecoTracker/PixelSeeding/interface/IntermediateHitTriplets.h"
#include "RecoTracker/PixelSeeding/interface/SimDoublets.h"
#include "DataFormats/Common/interface/Wrapper.h"

#include <vector>

namespace RecoPixelVertexing_PixelTriplets {
  struct dictionary {
    IntermediateHitTriplets iht;
    edm::Wrapper<IntermediateHitTriplets> wiht;
  };
}  // namespace RecoPixelVertexing_PixelTriplets


namespace {
  struct simDoubletsDictionary {
    SimDoublets sb;
    edm::Wrapper<SimDoublets> wsb;
    SiPixelRecHitRefVector sprhrv;
  };
}  // namespace
