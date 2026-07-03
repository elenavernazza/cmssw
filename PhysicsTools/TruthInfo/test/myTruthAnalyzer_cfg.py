import FWCore.ParameterSet.Config as cms

process = cms.Process("MYTRUTHANALYZER")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(10000)
)

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(
        "file:34087.88_TenTau_15_500+Run4D120_enableTruth/step3.root"
    )
)

process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("mytest.root")
)

process.load("Validation.Configuration.truthPrevalidation_cff")

process.myTruthAnalyzer = cms.EDAnalyzer(
    "MyTruthAnalyzer",
    src = cms.InputTag("truthLogicalGraphProducer"),
    hitIndex = cms.InputTag("truthLogicalGraphHitIndexProducer"),
)

process.truthanalyzer = cms.Path(
    process.truthGraphPrevalidation
    + process.myTruthAnalyzer
)
