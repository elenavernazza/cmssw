import FWCore.ParameterSet.Config as cms

hltPFTester = cms.EDProducer("PFTester",
    PFCand = cms.InputTag("hltParticleFlowTmp"),
)

PFValSeq = cms.Sequence(hltPFTester)