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
    label_scl = cms.InputTag("mix","HcalCaloTruth") 
)

hltPFClusterSimClusterAssociationProducerECAL = cms.EDProducer("PCToSCAssociatorEDProducer",
    associator = cms.InputTag("hltPFScAssocByEnergyScoreProducer"),
    label_lcl = cms.InputTag("hltParticleFlowClusterECALUnseeded"),
    label_scl = cms.InputTag("mix","EcalCaloTruth") 
)

hltPFCpAssocByEnergyScoreProducer = cms.EDProducer("BarrelPCToCPAssociatorByEnergyScoreProducer",
    hardScatterOnly = cms.bool(True),
    hitMapTag = cms.InputTag("hltRecHitMapProducer:barrelRecHitMap"),
    hits = cms.VInputTag("hltParticleFlowRecHitECALUnseeded", "hltParticleFlowRecHitHBHE") #, "hltParticleFlowRecHitHF", "hltParticleFlowRecHitHO")
)

hltPFClusterCaloParticleAssociationProducerHBHE = cms.EDProducer("PCToCPAssociatorEDProducer",
    associator = cms.InputTag("hltPFCpAssocByEnergyScoreProducer"),
    label_lc = cms.InputTag("hltParticleFlowClusterHBHE"),
    label_cp = cms.InputTag("mix","HcalCaloTruth") 
)

hltPFClusterCaloParticleAssociationProducerECAL = cms.EDProducer("PCToCPAssociatorEDProducer",
    associator = cms.InputTag("hltPFCpAssocByEnergyScoreProducer"),
    label_lc = cms.InputTag("hltParticleFlowClusterECALUnseeded"),
    label_cp = cms.InputTag("mix","EcalCaloTruth") 
)

hltPFTester = cms.EDProducer("PFTester",
    PFCand = cms.InputTag("hltParticleFlowTmp"),
    PFClusterHCAL = cms.InputTag("hltParticleFlowClusterHBHE"),
    SimClusterHCAL = cms.InputTag("mix","HcalCaloTruth"), 
    PFClusterSimClusterAssociatorHCAL = cms.InputTag("hltPFClusterSimClusterAssociationProducerHBHE"),
    PFClusterCaloParticleAssociatorHCAL = cms.InputTag("hltPFClusterCaloParticleAssociationProducerHBHE"),
    PFClusterECAL = cms.InputTag("hltParticleFlowClusterECALUnseeded"),
    SimClusterECAL = cms.InputTag("mix","EcalCaloTruth"), 
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
