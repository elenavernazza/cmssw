import FWCore.ParameterSet.Config as cms

hltParticleTransformerAK4TagInfos = cms.EDProducer("hltParticleTransformerAK4TagInfoProducer",
    candidates = cms.InputTag("hltParticleFlowTmp"),
    flip = cms.bool(False),
    jet_radius = cms.double(0.4),
    jets = cms.InputTag("hltAK4PFPuppiJets"),
    mightGet = cms.optional.untracked.vstring,
    min_candidate_pt = cms.double(0.1),
    min_jet_pt = cms.double(5),
    secondary_vertices = cms.InputTag("hltDeepInclusiveMergedVerticesPF"),
    vertex_associator = cms.InputTag("hltPrimaryVertexAssociation","original"),
    vertices = cms.InputTag("hltGoodOfflinePrimaryVertices")
)
