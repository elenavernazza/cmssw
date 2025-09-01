import FWCore.ParameterSet.Config as cms
from SimCalorimetry.HGCalAssociatorProducers.LCToSCAssociation_cfi import layerClusterSimClusterAssociation as _layerClusterSimClusterAssociationProducer

hltPFHBHEScAssocByEnergyScoreProducer = cms.EDProducer("BarrelLCToSCAssociatorByEnergyScoreProducer",
    hardScatterOnly = cms.bool(True),
    hitMapTag = cms.InputTag("hltRecHitMapProducer:barrelRecHitMap"),
    hits = cms.VInputTag("hltParticleFlowRecHitECALUnseeded", "hltParticleFlowRecHitHBHE"), # hltParticleFlowClusterHO
)

hltPFClusterSimClusterAssociationProducer = _layerClusterSimClusterAssociationProducer.clone(
    associator = cms.InputTag("hltPFHBHEScAssocByEnergyScoreProducer"),
    # label_lcl = cms.InputTag("hltParticleFlowClusterECALUncorrected"),
    label_lcl = cms.InputTag("hltParticleFlowClusterHBHE"),
    label_scl = cms.InputTag("mix","MergedCaloTruth")
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
    +hltPFTester
)