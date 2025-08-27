import FWCore.ParameterSet.Config as cms

hltJetTester = cms.EDProducer("PFTester",
    PFCand = cms.InputTag("hltParticleFlowTmp"),
)

PFValSeq = cms.Sequence(hltJetTester)