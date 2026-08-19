import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from pathlib import Path


options = VarParsing("analysis")
options.outputFile = "tau_roc.root"
options.register(
    "inputDir",
    "",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Directory containing input ROOT files",
)
options.parseArguments()

if options.inputDir and options.inputFiles:
    raise RuntimeError("Pass either inputDir=... or inputFiles=..., not both")

input_files = list(options.inputFiles)
if options.inputDir:
    input_files = [
        f"file:{path}"
        for path in sorted(Path(options.inputDir).glob("*.root"))
    ]

if not input_files:
    raise RuntimeError(
        "No input files found; pass inputDir=... or inputFiles=..."
    )

process = cms.Process("ROC")
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents),
)
process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(input_files),
)

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string(options.outputFile),
)

process.tauROC = cms.EDAnalyzer(
    "TauROCAnalyzer",
    jets=cms.InputTag("hltAK4PFPuppiJets", "", "HLTX"),
    partTauvsAll=cms.InputTag(
        "hltParticleTransformerDiscriminatorsJetTags",
        "TauvsAll",
        "HLTX",
    ),
    taus=cms.InputTag("hltHpsPFTauProducer", "", "HLTX"),
    deepTauVSjet=cms.InputTag(
        "hltHpsPFTauDeepTauProducer",
        "VSjet",
        "HLTX",
    ),
    genVisTaus=cms.InputTag("genVisTaus", "", "HLTX"),
    dRmatch=cms.double(0.1),
)

process.path = cms.Path(process.tauROC)