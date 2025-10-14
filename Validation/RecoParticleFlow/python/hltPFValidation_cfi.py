import FWCore.ParameterSet.Config as cms

# For HCAL: currently hltParticleFlowClusterHBHE, but probably better hltParticleFlowClusterHCAL
# For ECAL: currently hltParticleFlowClusterECALUnseeded, but probably better hltParticleFlowSuperClusterECALUnseeded

hltPFScAssocByEnergyScoreProducer = cms.EDProducer("BarrelPCToSCAssociatorByEnergyScoreProducer",
    hardScatterOnly = cms.bool(True),
    hitMapTag = cms.InputTag("hltRecHitMapProducer:barrelRecHitMap"),
    hits = cms.VInputTag("hltParticleFlowRecHitECALUnseeded", "hltParticleFlowRecHitHBHE") #, "hltParticleFlowRecHitHF", "hltParticleFlowRecHitHO")
)

hltPFClusterSimClusterAssociationProducerHBHE = cms.EDProducer("PCToSCAssociatorEDProducer",
    associator = cms.InputTag("hltPFScAssocByEnergyScoreProducer"),
    label_lcl = cms.InputTag("hltParticleFlowClusterHBHE"),
    label_scl = cms.InputTag("mix","MergedCaloTruth") # FIXME: we will have different collections for ECAL and HCAL
)

hltPFClusterSimClusterAssociationProducerECAL = cms.EDProducer("PCToSCAssociatorEDProducer",
    associator = cms.InputTag("hltPFScAssocByEnergyScoreProducer"),
    label_lcl = cms.InputTag("hltParticleFlowRecHitECALUnseeded"),
    label_scl = cms.InputTag("mix","MergedCaloTruth") # FIXME: we will have different collections for ECAL and HCAL
)

hltPFCpAssocByEnergyScoreProducer = cms.EDProducer("BarrelPCToCPAssociatorByEnergyScoreProducer",
    hardScatterOnly = cms.bool(True),
    hitMapTag = cms.InputTag("hltRecHitMapProducer:barrelRecHitMap"),
    hits = cms.VInputTag("hltParticleFlowRecHitECALUnseeded", "hltParticleFlowRecHitHBHE") #, "hltParticleFlowRecHitHF", "hltParticleFlowRecHitHO")
)

hltPFClusterCaloParticleAssociationProducerHBHE = cms.EDProducer("PCToCPAssociatorEDProducer",
    associator = cms.InputTag("hltPFCpAssocByEnergyScoreProducer"),
    label_lc = cms.InputTag("hltParticleFlowClusterHBHE"),
    label_cp = cms.InputTag("mix","MergedCaloTruth") # FIXME: we will have different collections for ECAL and HCAL
)

hltPFClusterCaloParticleAssociationProducerECAL = cms.EDProducer("PCToCPAssociatorEDProducer",
    associator = cms.InputTag("hltPFCpAssocByEnergyScoreProducer"),
    label_lc = cms.InputTag("hltParticleFlowRecHitECALUnseeded"),
    label_cp = cms.InputTag("mix","MergedCaloTruth") # FIXME: we will have different collections for ECAL and HCAL
)

hltPFTester = cms.EDProducer("PFTester",
    PFCand = cms.InputTag("hltParticleFlowTmp"),
    PFClusterHCAL = cms.InputTag("hltParticleFlowClusterHBHE"),
    SimClusterHCAL = cms.InputTag("mix","MergedCaloTruth"), # FIXME: we will have different collections for ECAL and HCAL
    PFClusterSimClusterAssociatorHCAL = cms.InputTag("hltPFClusterSimClusterAssociationProducerHBHE"),
    PFClusterCaloParticleAssociatorHCAL = cms.InputTag("hltPFClusterCaloParticleAssociationProducerHBHE"),
    PFClusterECAL = cms.InputTag("hltParticleFlowClusterECALUnseeded"),
    SimClusterECAL = cms.InputTag("mix","MergedCaloTruth"), # FIXME: we will have different collections for ECAL and HCAL
    PFClusterSimClusterAssociatorECAL = cms.InputTag("hltPFClusterSimClusterAssociationProducerECAL"),
    PFClusterCaloParticleAssociatorECAL = cms.InputTag("hltPFClusterCaloParticleAssociationProducerECAL"),
)

PFValSeq = cms.Sequence(
    hltPFScAssocByEnergyScoreProducer
    +hltPFClusterSimClusterAssociationProducerHBHE
    +hltPFClusterSimClusterAssociationProducerECAL
    +hltPFCpAssocByEnergyScoreProducer
    +hltPFClusterCaloParticleAssociationProducerHBHE
    +hltPFClusterCaloParticleAssociationProducerECAL
    +hltPFTester
)
