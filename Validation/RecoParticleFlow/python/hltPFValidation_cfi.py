import FWCore.ParameterSet.Config as cms

# lcAssocByEnergyScoreProducer = _lcAssocByEnergyScoreProducer.clone(hardScatterOnly = cms.bool(True))
# scAssocByEnergyScoreProducer = _scAssocByEnergyScoreProducer.clone(hardScatterOnly = cms.bool(True))

# layerClusterCaloParticleAssociation = cms.EDProducer("LCToCPAssociatorEDProducer",
#     associator = cms.InputTag('lcAssocByEnergyScoreProducer'),
#     label_cp = cms.InputTag("mix","MergedCaloTruth"),
#     label_lc = cms.InputTag("hgcalMergeLayerClusters")
# )

# hltLcAssocByEnergyScoreProducer = _lcAssocByEnergyScoreProducer.clone(
#     hits = cms.VInputTag("hltHGCalRecHit:HGCEERecHits", "hltHGCalRecHit:HGCHEFRecHits", "hltHGCalRecHit:HGCHEBRecHits"),
#     hitMapTag = cms.InputTag("hltRecHitMapProducer","hgcalRecHitMap"),
# )

# hltScAssocByEnergyScoreProducer = _scAssocByEnergyScoreProducer.clone(
#     hits = cms.VInputTag("hltHGCalRecHit:HGCEERecHits", "hltHGCalRecHit:HGCHEFRecHits", "hltHGCalRecHit:HGCHEBRecHits"),
#     hitMapTag = cms.InputTag("hltRecHitMapProducer","hgcalRecHitMap"),
# )

# hltLayerClusterCaloParticleAssociationProducer = layerClusterCaloParticleAssociation.clone(
#     associator = cms.InputTag("hltLcAssocByEnergyScoreProducer"),
#     label_lc = cms.InputTag("hltMergeLayerClusters")
# )

# hltLayerClusterSimClusterAssociationProducer = _layerClusterSimClusterAssociationProducer.clone(
#     associator = cms.InputTag("hltScAssocByEnergyScoreProducer"),
#     label_lcl = cms.InputTag("hltMergeLayerClusters")
# )

hltPFHBHEScAssocByEnergyScoreProducer = cms.EDProducer("BarrelPCToSCAssociatorByEnergyScoreProducer",
    hardScatterOnly = cms.bool(True),
    hitMapTag = cms.InputTag("hltRecHitMapProducer:barrelRecHitMap"),
    hits = cms.VInputTag("hltParticleFlowRecHitECALUnseeded", "hltParticleFlowRecHitHBHE"), # hltParticleFlowClusterHO
)

hltPFClusterSimClusterAssociationProducer = cms.EDProducer("PCToSCAssociatorEDProducer",
    associator = cms.InputTag("hltPFHBHEScAssocByEnergyScoreProducer"),
    label_lcl = cms.InputTag("hltParticleFlowClusterHBHE"),
    label_scl = cms.InputTag("mix","MergedCaloTruth")
)

hltPFHBHECpAssocByEnergyScoreProducer = cms.EDProducer("BarrelPCToCPAssociatorByEnergyScoreProducer",
    hardScatterOnly = cms.bool(True),
    hitMapTag = cms.InputTag("hltRecHitMapProducer:barrelRecHitMap"),
    hits = cms.VInputTag("hltParticleFlowRecHitECALUnseeded", "hltParticleFlowRecHitHBHE"), # hltParticleFlowClusterHO
)

hltPFClusterCaloParticleAssociationProducer = cms.EDProducer("PCToCPAssociatorEDProducer",
    associator = cms.InputTag("hltPFHBHECpAssocByEnergyScoreProducer"),
    label_lc = cms.InputTag("hltParticleFlowClusterHBHE"),
    label_cp = cms.InputTag("mix","MergedCaloTruth")
)

hltPFTester = cms.EDProducer("PFTester",
    PFCand = cms.InputTag("hltParticleFlowTmp"),
    PFClusterHCAL = cms.InputTag("hltParticleFlowClusterHBHE"),
    SimClusterHCAL = cms.InputTag("mix","MergedCaloTruth"),
    PFClusterSimClusterAssociatorHCAL = cms.InputTag("hltPFClusterSimClusterAssociationProducer"),
    PFClusterCaloParticleAssociatorHCAL = cms.InputTag("hltPFClusterCaloParticleAssociationProducer"),
)

PFValSeq = cms.Sequence(
    hltPFHBHEScAssocByEnergyScoreProducer
    +hltPFClusterSimClusterAssociationProducer
    +hltPFHBHECpAssocByEnergyScoreProducer
    +hltPFClusterCaloParticleAssociationProducer
    +hltPFTester
)
