import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *

hltVertexTable = cms.EDProducer("HLTVertexTableProducer",
                                skipNonExistingSrc = cms.bool(True),
                                pvSrc = cms.InputTag("hltOfflinePrimaryVertices"),
                                goodPvCut = cms.string("!isFake && ndof >= 4.0 && abs(z) <= 24.0 && abs(position.Rho) <= 2.0"), 
                                pfSrc = cms.InputTag("hltParticleFlowTmp"),
                                dlenMin = cms.double(0),
                                dlenSigMin = cms.double(3),
                                pvName = cms.string("hltPrimaryVertex"),
                                )

hltPixelVertexTable = cms.EDProducer("SimpleVertexFlatTableProducer",
    skipNonExistingSrc = cms.bool(True),
    src = cms.InputTag("hltPhase2PixelVertices"),
    name = cms.string("hltPhase2PixelVertices"),
    cut = cms.string(""),
    doc = cms.string("HLT Pixel Vertices information"),
    singleton = cms.bool(False),
    extension = cms.bool(False),
    variables = cms.PSet(
      x = Var("x()", "float", doc="x coordinate of the vertex"),
      y = Var("y()", "float", doc="y coordinate of the vertex"),
      z = Var("z()", "float", doc="z coordinate of the vertex"),
      dz = Var("zError()", "float", doc="z error of the vertex"),
      tracksSize = Var("tracksSize()", "uint", doc="number of tracks associated to the vertex"),
    ),
  )

hltSimVertexTable = cms.EDProducer("SimpleTrackingVertexFlatTableProducer",
    skipNonExistingSrc = cms.bool(True),
    src = cms.InputTag("mix","MergedTrackTruth"),
    name = cms.string("hltSimVertices"),
    cut = cms.string(""),
    doc = cms.string("Sim Vertices information"),
    singleton = cms.bool(False),
    extension = cms.bool(False),
    variables = cms.PSet(
      x = Var("position().x()", "float", doc="x coordinate of the vertex"),
      y = Var("position().y()", "float", doc="y coordinate of the vertex"),
      z = Var("position().z()", "float", doc="z coordinate of the vertex"),
      tracksSize = Var("nDaughterTracks()", "uint", doc="number of tracks associated to the vertex"),
    ),
  )

