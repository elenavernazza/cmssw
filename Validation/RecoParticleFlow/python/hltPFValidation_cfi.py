import FWCore.ParameterSet.Config as cms

hltPFHBHEScAssocByEnergyScoreProducer = cms.EDProducer("BarrelPCToSCAssociatorByEnergyScoreProducer",
    hardScatterOnly = cms.bool(True),
    hitMapTag = cms.InputTag("hltRecHitMapProducer:barrelRecHitMap"),
    hits = cms.VInputTag("hltParticleFlowRecHitECALUnseeded", "hltParticleFlowRecHitHBHE"), # hltParticleFlowClusterHO
)

hltPFClusterSimClusterAssociationProducer = cms.EDProducer("PCToSCAssociatorEDProducer",
    associator = cms.InputTag("hltPFHBHEScAssocByEnergyScoreProducer"),
    # label_lcl = cms.InputTag("hltParticleFlowClusterECALUncorrected"),
    label_lcl = cms.InputTag("hltParticleFlowClusterHBHE"),
    label_scl = cms.InputTag("mix","MergedCaloTruth")
)

hltPFValidator = cms.EDProducer("PFValidator",
    label_rcl = cms.InputTag("hltParticleFlowClusterHBHE"),
    associator = cms.untracked.InputTag("hltPFClusterSimClusterAssociationProducer"),
    associatorSim = cms.untracked.InputTag("hltPFClusterSimClusterAssociationProducer"),
    label_scl = cms.InputTag("mix","MergedCaloTruth"),
    doCaloParticlePlots = cms.untracked.bool(False),
)

hltPFTester = cms.EDProducer("PFTester",
    PFCand = cms.InputTag("hltParticleFlowTmp"),
    PFClusterHCAL = cms.InputTag("hltParticleFlowClusterHBHE"),
    puppiWeights = cms.InputTag("hltPFPuppi"),
    puppiRawAlphas = cms.InputTag("hltPFPuppi:PuppiRawAlphas"),
    puppiAlphas = cms.InputTag("hltPFPuppi:PuppiAlphas"),
    puppiAlphasMed = cms.InputTag("hltPFPuppi:PuppiAlphasMed"),
    puppiAlphasRms = cms.InputTag("hltPFPuppi:PuppiAlphasRms"),
)

PFValSeq = cms.Sequence(
    hltPFHBHEScAssocByEnergyScoreProducer
    +hltPFClusterSimClusterAssociationProducer
    +hltPFValidator
    +hltPFTester
)