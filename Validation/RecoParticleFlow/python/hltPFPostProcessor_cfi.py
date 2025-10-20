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
           for recble in ('', 'Reconstructable')
           for item in (
                   # Efficiency
                   f"'Score{thr}/Eff_vs_Energy {recble}' 'Efficiency vs Energy {recble}' Score{thr}/SimClustersMatchedRecoClustersEn_Score{thr} SimClusters{recble}En",
                   f"'Score{thr}/Eff_vs_Pt {recble}' 'Efficiency vs p_{{T}} {recble}' Score{thr}/SimClustersMatchedRecoClustersPt_Score{thr} SimClusters{recble}Pt",
                   f"'Score{thr}/Eff_vs_Eta {recble}' 'Efficiency vs #eta {recble}' Score{thr}/SimClustersMatchedRecoClustersEta_Score{thr} SimClusters{recble}Eta",
                   f"'Score{thr}/Eff_vs_Phi {recble}' 'Efficiency vs #phi {recble}' Score{thr}/SimClustersMatchedRecoClustersPhi_Score{thr} SimClusters{recble}Phi",
                   f"'Score{thr}/Eff_vs_Mult {recble}' 'Efficiency vs Multiplicity {recble}' Score{thr}/SimClustersMatchedRecoClustersMult_Score{thr} SimClusters{recble}Mult"    
                   # Fake rate
                   f"'Score{thr}/Fake_vs_En {recble}' 'Fake Rate vs Energy {recble}' Score{thr}/RecoClustersMatchedSimClustersEn_Score{thr} RecoClusters{recble}En",
                   f"'Score{thr}/Fake_vs_Pt {recble}' 'Fake Rate vs p_{{T}} {recble}' Score{thr}/RecoClustersMatchedSimClustersPt_Score{thr} RecoClusters{recble}Pt",
                   f"'Score{thr}/Fake_vs_Eta {recble}' 'Fake Rate vs #eta {recble}' Score{thr}/RecoClustersMatchedSimClustersEta_Score{thr} RecoClusters{recble}Eta",
                   f"'Score{thr}/Fake_vs_Phi {recble}' 'Fake Rate vs #phi {recble}' Score{thr}/RecoClustersMatchedSimClustersPhi_Score{thr} RecoClusters{recble}Phi",
                   f"'Score{thr}/Fake_vs_Mult {recble}' 'Fake Rate vs Multiplicity {recble}' Score{thr}/RecoClustersMatchedSimClustersMult_Score{thr} RecoClusters{recble}Mult",
                   # Duplicate rate
                   f"'Score{thr}/Dup_vs_En {recble}' 'Dup Rate vs Energy {recble}' Score{thr}/RecoClustersMultiMatchedSimClustersEn_Score{thr} RecoClusters{recble}En",
                   f"'Score{thr}/Dup_vs_Pt {recble}' 'Dup Rate vs p_{{T}} {recble}' Score{thr}/RecoClustersMultiMatchedSimClustersPt_Score{thr} RecoClusters{recble}Pt",
                   f"'Score{thr}/Dup_vs_Eta {recble}' 'Dup Rate vs #eta {recble}' Score{thr}/RecoClustersMultiMatchedSimClustersEta_Score{thr} RecoClusters{recble}Eta",
                   f"'Score{thr}/Dup_vs_Phi {recble}' 'Dup Rate vs #phi {recble}' Score{thr}/RecoClustersMultiMatchedSimClustersPhi_Score{thr} RecoClusters{recble}Phi",
                   f"'Score{thr}/Dup_vs_Mult {recble}' 'Dup Rate vs Mult {recble}' Score{thr}/RecoClustersMultiMatchedSimClustersMult_Score{thr} RecoClusters{recble}Mult",
                   # Merge rate
                   f"'Score{thr}/Merge_vs_En {recble}' 'Merge Rate vs Energy {recble}' Score{thr}/SimClustersMultiMatchedRecoClustersEn_Score{thr} SimClusters{recble}En",
                   f"'Score{thr}/Merge_vs_Pt {recble}' 'Merge Rate vs p_{{T}} {recble}' Score{thr}/SimClustersMultiMatchedRecoClustersPt_Score{thr} SimClusters{recble}Pt",
                   f"'Score{thr}/Merge_vs_Eta {recble}' 'Merge Rate vs #eta {recble}' Score{thr}/SimClustersMultiMatchedRecoClustersEta_Score{thr} SimClusters{recble}Eta",
                   f"'Score{thr}/Merge_vs_Phi {recble}' 'Merge Rate vs #phi {recble}' Score{thr}/SimClustersMultiMatchedRecoClustersPhi_Score{thr} SimClusters{recble}Phi",
                   f"'Score{thr}/Merge_vs_Mult {recble}' 'Merge Rate vs Multiplicity {recble}' Score{thr}/SimClustersMultiMatchedRecoClustersMult_Score{thr} SimClusters{recble}Mult",
           )
          ],
    ),
    resolution = cms.vstring(),
    verbose = cms.untracked.uint32(2), 
    outputFileName = cms.untracked.string("")
)
