import FWCore.ParameterSet.Config as cms

process = cms.Process("EcalGeometryAnalyzer")

process.load('Configuration.Geometry.GeometryRecoDB_cff')
# process.load('Configuration.StandardSequences.GeometryRecoDB_cff')

process.TFileService = cms.Service(
    "TFileService", 
    fileName = cms.string("data.root"),
    closeFileFast = cms.untracked.bool(True)
)

process.CaloGeometryBuilder = cms.ESProducer(
    "CaloGeometryBuilder",                                                                         
    SelectedCalos = cms.vstring(
        'HCAL',
        'ZDC',
        # 'CASTOR', # missing the geometry
        'EcalBarrel',
        'EcalEndcap',
        'EcalPreshower',
        'TOWER'
    )
)

process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = '150X_mcRun4_realistic_v1'

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        # 'file:CloseByPGun_Barrel_Front_9p16_PDGID11.root'
        'file:step2p2_SingleElectron.root',
    )
)

process.ecalGeometryAnalyzer = cms.EDAnalyzer(
    'EcalGeometryAnalyzer',
    recHits = cms.InputTag("hltParticleFlowRecHitECALUnseeded"),
    simHits = cms.InputTag("g4SimHits", "EcalHitsEB")
)

process.p = cms.Path(process.ecalGeometryAnalyzer)
