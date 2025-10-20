import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester

from Validation.RecoParticleFlow.hltPFValidation_cfi import hltPFTesterECAL
_thresholds = [str(x).replace('.', 'p') for x in hltPFTesterECAL.assocScoreThresholds]

hltPFClusterPostProcessor = DQMEDHarvester(
    "DQMGenericClient",
    subDirs=cms.untracked.vstring("HLT/ParticleFlow/PFClusterValidation"),
    efficiency = cms.vstring(
        *[ item
           for thr in _thresholds
           for recble in ('', 'Reconstructable')
           for item in (
                   f"'Score{thr}/Eff_vs_EnergyEta {recble}' 'Efficiency vs Energy-#eta {recble}' Score{thr}/SimClustersMatchedRecoClustersEn_Eta_Score{thr} SimClusters{recble}En_Eta",
                   f"'Score{thr}/Eff_vs_EnergyPhi {recble}' 'Efficiency vs Energy-#phi {recble}' Score{thr}/SimClustersMatchedRecoClustersEn_Phi_Score{thr} SimClusters{recble}En_Phi",
                   f"'Score{thr}/Eff_vs_EnergyMult {recble}' 'Efficiency vs Energy-Mult {recble}' Score{thr}/SimClustersMatchedRecoClustersEn_Mult_Score{thr} SimClusters{recble}En_Mult",
                   f"'Score{thr}/Eff_vs_PtEta {recble}' 'Efficiency vs p_{{T}}-#eta {recble}' Score{thr}/SimClustersMatchedRecoClustersPt_Eta_Score{thr} SimClusters{recble}Pt_Eta",
                   f"'Score{thr}/Eff_vs_PtPhi {recble}' 'Efficiency vs p_{{T}}-#phi {recble}' Score{thr}/SimClustersMatchedRecoClustersPt_Phi_Score{thr} SimClusters{recble}Pt_Phi",
                   f"'Score{thr}/Eff_vs_PtMult {recble}' 'Efficiency vs p_{{T}}-Mult {recble}' Score{thr}/SimClustersMatchedRecoClustersPt_Mult_Score{thr} SimClusters{recble}Pt_Mult",
                   f"'Score{thr}/Eff_vs_MultEta {recble}' 'Efficiency vs Mult-#eta {recble}' Score{thr}/SimClustersMatchedRecoClustersMult_Eta_Score{thr} SimClusters{recble}Mult_Eta",
                   f"'Score{thr}/Eff_vs_MultPhi {recble}' 'Efficiency vs Mult-#phi {recble}' Score{thr}/SimClustersMatchedRecoClustersMult_Phi_Score{thr} SimClusters{recble}Mult_Phi",
                   f"'Score{thr}/Fake_vs_EnergyEta {recble}' 'Fake Rate vs Energy-#eta {recble}' Score{thr}/RecoClustersMatchedSimClustersEn_Eta_Score{thr} RecoClusters{recble}Pt_Eta fake",
                   f"'Score{thr}/Fake_vs_EnergyPhi {recble}' 'Fake Rate vs Energy-#phi {recble}' Score{thr}/RecoClustersMatchedSimClustersEn_Phi_Score{thr} RecoClusters{recble}Pt_Phi fake",
                   f"'Score{thr}/Fake_vs_EnergyMult {recble}' 'Fake Rate vs Energy-Mult {recble}' Score{thr}/RecoClustersMatchedSimClustersEn_Mult_Score{thr} RecoClusters{recble}Pt_Mult fake",
                   f"'Score{thr}/Fake_vs_PtEta {recble}' 'Fake Rate vs p_{{T}}-#eta {recble}' Score{thr}/RecoClustersMatchedSimClustersPt_Eta_Score{thr} RecoClusters{recble}Pt_Eta fake",
                   f"'Score{thr}/Fake_vs_PtPhi {recble}' 'Fake Rate vs p_{{T}}-#phi {recble}' Score{thr}/RecoClustersMatchedSimClustersPt_Phi_Score{thr} RecoClusters{recble}Pt_Phi fake",
                   f"'Score{thr}/Fake_vs_PtMult {recble}' 'Fake Rate vs p_{{T}}-Mult {recble}' Score{thr}/RecoClustersMatchedSimClustersPt_Mult_Score{thr} RecoClusters{recble}Pt_Mult fake",
                   f"'Score{thr}/Fake_vs_MultEta {recble}' 'Fake Rate vs Mult-#eta {recble}' Score{thr}/RecoClustersMatchedSimClustersMult_Eta_Score{thr} RecoClusters{recble}Mult_Eta fake",
                   f"'Score{thr}/Fake_vs_MultPhi {recble}' 'Fake Rate vs Mult-#phi {recble}' Score{thr}/RecoClustersMatchedSimClustersMult_Phi_Score{thr} RecoClusters{recble}Mult_Phi fake",
           )
          ],
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
