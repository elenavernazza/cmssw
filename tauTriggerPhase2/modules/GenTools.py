# taken from Sam James Harper: https://gitlab.cern.ch/sharper/HLTAnalyserPy

import ROOT

def get_lastcopy(part):
    for daughter in part.daughterRefVector():
        if daughter.pdgId() == part.pdgId():
            return get_lastcopy(daughter.get())
    return part

def deltaR2(obj1, obj2):
    """Calculate deltaR squared between two objects."""
    deta = obj1.eta() - obj2.eta()
    dphi = ROOT.TVector2.Phi_mpi_pi(obj1.phi() - obj2.phi())
    return deta * deta + dphi * dphi

def match_trigger_objects(tau, trigger_objects, dR2limit):
    min_dR2 = float('inf')
    for trig_obj in trigger_objects:
        dR2 = deltaR2(tau, trig_obj)
        if dR2 < min_dR2:
            min_dR2 = dR2
    return min_dR2 < dR2limit

# Load FWLite libraries
def load_fwlitelibs():
    ROOT.gSystem.Load("libFWCoreFWLite.so")
    ROOT.FWLiteEnabler.enable()
    ROOT.reco.GsfElectron  # Ensure necessary libraries are loaded
