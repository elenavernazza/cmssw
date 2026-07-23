import FWCore.ParameterSet.Config as cms

process = cms.Process("MYTRUTHANALYZER")

# Messages
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Validation.Configuration.truthPrevalidation_cff")

process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(100)
)

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(
        "file:34087.88_TenTau_15_500+Run4D120_enableTruth/step3.root"
        #"file:34044.88_DYToLL_M_50_14TeV+Run4D120_enableTruth/step3.root"
    )
)

process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("myTenTau.root")
    #fileName = cms.string("myDYtoLL.root")
)

process.myTruthAnalyzer = cms.EDAnalyzer(
    "MyTruthAnalyzer",
    src = cms.InputTag("truthLogicalGraphProducer"),
    hitIndex = cms.InputTag("truthLogicalGraphHitIndexProducer"),
    recHitMap = cms.InputTag("detIdToRecHitMapProducer"),
    doTenTau = cms.bool(True),
    doDYtoLL = cms.bool(False),
)

process.truthanalyzer = cms.Path(
    process.truthGraphPrevalidation
    + process.myTruthAnalyzer
)
