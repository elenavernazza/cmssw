#ifndef DataFormats_BTauReco_hltParticleTransformerAK4TagInfo_h
#define DataFormats_BTauReco_hltParticleTransformerAK4TagInfo_h

#include "DataFormats/BTauReco/interface/hltParticleTransformerAK4Features.h"
#include "DataFormats/BTauReco/interface/FeaturesTagInfo.h"

namespace reco {
  typedef FeaturesTagInfo<btagbtvdeep::hltParticleTransformerAK4Features> hltParticleTransformerAK4TagInfo;
  DECLARE_EDM_REFS(hltParticleTransformerAK4TagInfo)
}  // namespace reco

#endif  // DataFormats_BTauReco_hltParticleTransformerAK4TagInfo_h
