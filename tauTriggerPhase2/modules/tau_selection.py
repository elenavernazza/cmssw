# modules/tau_selection.py

def is_hadronic_tau(tau):
    # Check if the particle is an electron or muon
    for daughter in tau.daughterRefVector():
        pdgId = abs(daughter.pdgId())
        # If the tau decays to an electron or muon, it's a leptonic decay
        if pdgId in [11, 13]:
            return False
    # If no electron or muon daughter is found, it's a hadronic tau
    return True

def passes_cuts(tau, cuts):

    for cut_func in cuts.values():
        if not cut_func(tau):
            return False
    return True
