import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester

from Validation.RecoParticleFlow.hltPFValidation_cfi import hltPFTesterECAL
_thresholds = [str(x).replace('.', 'p') for x in hltPFTesterECAL.assocScoreThresholds]

hltPFClusterPostProcessor = DQMEDHarvester(
    "DQMGenericClient",
    subDirs=cms.untracked.vstring("HLT/ParticleFlow/PFClusterValidation"),
    efficiency = cms.vstring(
        "Eff_vs_EnergyEta 'Efficiency vs Energy-#eta' SimClustersMatchedRecoClustersEn_Eta SimClustersEn_Eta",
        "Eff_vs_EnergyPhi 'Efficiency vs Energy-#phi' SimClustersMatchedRecoClustersEn_Phi SimClustersEn_Phi",
        "Eff_vs_EnergyMult 'Efficiency vs Energy-Mult' SimClustersMatchedRecoClustersEn_Mult SimClustersEn_Mult",
        "Eff_vs_PtEta 'Efficiency vs p_{T}-#eta' SimClustersMatchedRecoClustersPt_Eta SimClustersPt_Eta",
        "Eff_vs_PtPhi 'Efficiency vs p_{T}-#phi' SimClustersMatchedRecoClustersPt_Phi SimClustersPt_Phi",
        "Eff_vs_PtMult 'Efficiency vs p_{T}-Mult' SimClustersMatchedRecoClustersPt_Mult SimClustersPt_Mult",
        "Eff_vs_MultEta 'Efficiency vs Mult-#eta' SimClustersMatchedRecoClustersMult_Eta SimClustersMult_Eta",
        "Eff_vs_MultPhi 'Efficiency vs Mult-#phi' SimClustersMatchedRecoClustersMult_Phi SimClustersMult_Phi",
        "Fake_vs_EnergyEta 'Fake Rate vs Energy-#eta' RecoClustersMatchedSimClustersEn_Eta RecoClustersPt_Eta fake",
        "Fake_vs_EnergyPhi 'Fake Rate vs Energy-#phi' RecoClustersMatchedSimClustersEn_Phi RecoClustersPt_Phi fake",
        "Fake_vs_EnergyMult 'Fake Rate vs Energy-Mult' RecoClustersMatchedSimClustersEn_Mult RecoClustersPt_Mult fake",
        "Fake_vs_PtEta 'Fake Rate vs p_{T}-#eta' RecoClustersMatchedSimClustersPt_Eta RecoClustersPt_Eta fake",
        "Fake_vs_PtPhi 'Fake Rate vs p_{T}-#phi' RecoClustersMatchedSimClustersPt_Phi RecoClustersPt_Phi fake",
        "Fake_vs_PtMult 'Fake Rate vs p_{T}-Mult' RecoClustersMatchedSimClustersPt_Mult RecoClustersPt_Mult fake",
        "Fake_vs_MultEta 'Fake Rate vs Mult-#eta' RecoClustersMatchedSimClustersMult_Eta RecoClustersMult_Eta fake",
        "Fake_vs_MultPhi 'Fake Rate vs Mult-#phi' RecoClustersMatchedSimClustersMult_Phi RecoClustersMult_Phi fake",
    ),
    efficiencyProfile = cms.untracked.vstring( # for smoother rebinning
        *[ item
           for thr in _thresholds
           for item in (
                   # Efficiency
                   f"Score{thr}/Eff_vs_Energy 'Efficiency vs Energy' Score{thr}/SimClustersMatchedRecoClustersEn_Score{thr} SimClustersEn",
                   f"Score{thr}/Eff_vs_Pt 'Efficiency vs p_{{T}}' Score{thr}/SimClustersMatchedRecoClustersPt_Score{thr} SimClustersPt",
                   f"Score{thr}/Eff_vs_Eta 'Efficiency vs #eta' Score{thr}/SimClustersMatchedRecoClustersEta_Score{thr} SimClustersEta",
                   f"Score{thr}/Eff_vs_Phi 'Efficiency vs #phi' Score{thr}/SimClustersMatchedRecoClustersPhi_Score{thr} SimClustersPhi",
                   f"Score{thr}/Eff_vs_Mult 'Efficiency vs Multiplicity' Score{thr}/SimClustersMatchedRecoClustersMult_Score{thr} SimClustersMult"    
                   # Fake rate
                   f"Score{thr}/Fake_vs_En 'Fake Rate vs Energy' Score{thr}/RecoClustersMatchedSimClustersEn_Score{thr} RecoClustersEn",
                   f"Score{thr}/Fake_vs_Pt 'Fake Rate vs p_{{T}}' Score{thr}/RecoClustersMatchedSimClustersPt_Score{thr} RecoClustersPt",
                   f"Score{thr}/Fake_vs_Eta 'Fake Rate vs #eta' Score{thr}/RecoClustersMatchedSimClustersEta_Score{thr} RecoClustersEta",
                   f"Score{thr}/Fake_vs_Phi 'Fake Rate vs #phi' Score{thr}/RecoClustersMatchedSimClustersPhi_Score{thr} RecoClustersPhi",
                   f"Score{thr}/Fake_vs_Mult 'Fake Rate vs Multiplicity' Score{thr}/RecoClustersMatchedSimClustersMult_Score{thr} RecoClustersMult",
                   # Duplicate rate
                   f"Score{thr}/Dup_vs_En 'Dup Rate vs Energy' Score{thr}/RecoClustersMultiMatchedSimClustersEn_Score{thr} RecoClustersEn",
                   f"Score{thr}/Dup_vs_Pt 'Dup Rate vs p_{{T}}' Score{thr}/RecoClustersMultiMatchedSimClustersPt_Score{thr} RecoClustersPt",
                   f"Score{thr}/Dup_vs_Eta 'Dup Rate vs #eta' Score{thr}/RecoClustersMultiMatchedSimClustersEta_Score{thr} RecoClustersEta",
                   f"Score{thr}/Dup_vs_Phi 'Dup Rate vs #phi' Score{thr}/RecoClustersMultiMatchedSimClustersPhi_Score{thr} RecoClustersPhi",
                   f"Score{thr}/Dup_vs_Mult 'Dup Rate vs Mult' Score{thr}/RecoClustersMultiMatchedSimClustersMult_Score{thr} RecoClustersMult",
                   # Merge rate
                   f"Score{thr}/Merge_vs_En 'Merge Rate vs Energy' Score{thr}/SimClustersMultiMatchedRecoClustersEn_Score{thr} SimClustersEn",
                   f"Score{thr}/Merge_vs_Pt 'Merge Rate vs p_{{T}}' Score{thr}/SimClustersMultiMatchedRecoClustersPt_Score{thr} SimClustersPt",
                   f"Score{thr}/Merge_vs_Eta 'Merge Rate vs #eta' Score{thr}/SimClustersMultiMatchedRecoClustersEta_Score{thr} SimClustersEta",
                   f"Score{thr}/Merge_vs_Phi 'Merge Rate vs #phi' Score{thr}/SimClustersMultiMatchedRecoClustersPhi_Score{thr} SimClustersPhi",
                   f"Score{thr}/Merge_vs_Mult 'Merge Rate vs Multiplicity' Score{thr}/SimClustersMultiMatchedRecoClustersMult_Score{thr} SimClustersMult",
           )
          ],
    ),
    resolution = cms.vstring(),
    verbose = cms.untracked.uint32(2), 
    outputFileName = cms.untracked.string("")
)
