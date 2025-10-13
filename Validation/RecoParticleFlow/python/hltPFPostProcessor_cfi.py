import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester

hltPFClusterPostProcessor = DQMEDHarvester("DQMGenericClient",
    subDirs=cms.untracked.vstring("HLT/ParticleFlow/PFClusterValidation"),
    efficiency = cms.vstring(),
    #     "Eff_vs_EtaPt 'Efficiency vs #eta-p_{T}' MatchedGenEtaPt GenEtaPt",
    #     "Eff_vs_PhiPt 'Efficiency vs #phi-p_{T}' MatchedGenPhiPt GenPhiPt",
    #     "Fake_vs_EtaPt 'Fake Rate vs #eta-p_{T}' MatchedJetEtaPt JetEtaPt fake",
    #     "Fake_vs_PhiPt 'Fake Rate vs #phi-p_{T}' MatchedJetPhiPt JetPhiPt fake",
    #     "Dup_vs_EtaPt 'Duplicate Rate vs #eta-p_{T}' DuplicatesJetEtaPt JetEtaPt",
    #     "Dup_vs_PhiPt 'Duplicate Rate vs #phi-p_{T}' DuplicatesJetPhiPt JetPhiPt",
    #     "Dup_gen_vs_EtaPt 'Duplicate Gen Rate vs #eta-p_{T}' DuplicatesGenEtaPt GenEtaPt",
    #     "Dup_gen_vs_PhiPt 'Duplicate Gen Rate vs #phi-p_{T}' DuplicatesGenPhiPt GenPhiPt",
    # ),
    efficiencyProfile = cms.untracked.vstring( # for smoother rebinning
        # Efficiency
        "Eff_vs_Energy 'Efficiency vs Energy' SimClustersMatchedRecoClustersEnergy SimClustersEnergy ",
        "Eff_vs_Pt 'Efficiency vs p_{T}' SimClustersMatchedRecoClustersPt SimClustersPt ",
        "Eff_vs_Eta 'Efficiency vs #eta' SimClustersMatchedRecoClustersEta SimClustersEta ",
        "Eff_vs_Phi 'Efficiency vs #phi' SimClustersMatchedRecoClustersPhi SimClustersPhi ",
        "Eff_vs_Mult 'Efficiency vs Multiplicity' SimClustersMatchedRecoClustersMult SimClustersMult ",
        # # Fake rate
        # "Fake_vs_Eta 'Fake Rate vs #eta' MatchedJetEta JetEta fake",
        # # Duplicate rate
        # "Dup_vs_Eta 'Duplicate Rate vs #eta' DuplicatesJetEta JetEta",
        # # Duplicate gen rate
        # "DupGen_vs_Eta 'Duplicate Gen Rate vs #eta' DuplicatesGenEta GenEta",
    ),
    resolution = cms.vstring(),
    verbose = cms.untracked.uint32(2), 
    outputFileName = cms.untracked.string("")
)
