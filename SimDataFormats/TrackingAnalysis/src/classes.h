#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticleFwd.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingVertex.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingVertexContainer.h"
#include "SimDataFormats/TrackingAnalysis/interface/UniqueSimTrackId.h"
#include "SimDataFormats/TrackingAnalysis/interface/SimDoublets.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/Common/interface/AssociationMapHelpers.h"
#include "DataFormats/Common/interface/Wrapper.h"


namespace {
  struct simDoubletsDictionary {
    SimDoublets sb;
    edm::Wrapper<SimDoublets> wsb;
    SiPixelRecHitRefVector sprhrv;
  };
}  // namespace
