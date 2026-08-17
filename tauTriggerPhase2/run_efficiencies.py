from __future__ import absolute_import, division, print_function

import argparse, glob, os
from array import array
import numpy as np
import ROOT
from DataFormats.FWLite import Events

from modules.EvtData import EvtData, add_product
from modules.GenTools import load_fwlitelibs

from tqdm import tqdm

DEFAULT_INPUT_GLOB = ("/eos/user/a/agruber/samples/HLT_Upgrade_L1filter/ParT_unfiltered_16_1_1/Phase2_L1P2GT_HLT_*.root")
PT_EDGES = array("d", [0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80, 90, 100, 120, 140, 160, 200])
ETA_NBINS = 25
ETA_RANGE = (-2.5, 2.5)

# HLT Triggers under study
TRIGGERS = (
    (
        "HLT_DoubleMediumChargedIsoPFTauHPS40_eta2p1",
        "hltHpsDoublePFTau40TrackPt1MediumChargedIsolation",
    ),
    (
        "HLT_DoubleMediumDeepTauPFTauHPS35_eta2p1",
        "hltHpsDoublePFTau35MediumDitauWPDeepTau",
    ),
    (
        "HLT_DoubleMediumPFPuppiParTTauh30_eta2p1",
        "hltDoublePFJets30ParTTauhTagMediumWPL2DoubleTau",
    ),
)

# L1 seed for the HLT triggers
L1_PATH = "pDoublePuppiTau52_52"

##################################################
# Functions for reading input files
##################################################

def find_trigger_index(trigger_names, path):
    prefix = path + "_v"
    for index in range(trigger_names.size()):
        name = str(trigger_names.triggerName(index))
        if name == path or name.startswith(prefix):
            return index
    return None

def build_products():
    products = []
    add_product(
        products,
        "trig_sum",
        "trigger::TriggerEvent",
        "hltTriggerSummaryAOD::{}".format(TRIGGER_PROCESS),
    )
    add_product(
        products,
        "trig_res",
        "edm::TriggerResults",
        "TriggerResults::{}".format(TRIGGER_PROCESS),
    )
    add_product(
        products,
        "gen_taus",
        "std::vector<reco::GenParticle>",
        "genVisTaus",
    )
    return products

def append_kinematics(objects, pt, eta, phi):
    for obj in objects:
        pt.append(float(obj.pt()))
        eta.append(float(obj.eta()))
        phi.append(float(obj.phi()))

def extract_cache(files, cache_name, max_events=-1):
    """Read EDM products once and store only primitive analysis quantities."""
    if not files:
        raise RuntimeError("No input ROOT files were found")

    events = Events(files)
    evtdata = EvtData(build_products(), verbose=False)
    ntriggers = len(TRIGGERS)

    gen_pt, gen_eta, gen_phi = [], [], []
    gen_offsets = [0]
    reco_pt, reco_eta, reco_phi = [], [], []
    reco_offsets = [0]
    path_present, accepted, filter_present = [], [], []
    l1_double_puppi_tau_fired = []

    total_events = events.size()

    for event_number, event in enumerate(tqdm(events, total=total_events)):
        if max_events >= 0 and event_number >= max_events:
            break

        evtdata.get_handles(event)
        trigger_results = evtdata.get("trig_res")
        trigger_event = evtdata.get("trig_sum")
        gen_taus = list(evtdata.get("gen_taus") or [])
        if trigger_results is None:
            raise RuntimeError("Missing TriggerResults at event {}".format(event_number))
        if trigger_event is None:
            raise RuntimeError(
                "Missing hltTriggerSummaryAOD at event {}".format(event_number)
            )

        gen_taus.sort(key=lambda tau: tau.pt(), reverse=True)
        append_kinematics(gen_taus, gen_pt, gen_eta, gen_phi)
        gen_offsets.append(len(gen_pt))

        trigger_names = event.object().triggerNames(trigger_results)
        l1_index = find_trigger_index(trigger_names, L1_PATH)
        if l1_index is None:
            raise RuntimeError(
                "Missing L1 path {} in {} TriggerResults at event {}".format(
                    L1_PATH, TRIGGER_PROCESS, event_number
                )
            )
        l1_double_puppi_tau_fired.append(
            bool(trigger_results.accept(l1_index))
        )

        all_trigger_objects = trigger_event.getObjects()
        for path, filter_name in TRIGGERS:
            trigger_index = find_trigger_index(trigger_names, path)
            has_path = trigger_index is not None
            path_present.append(has_path)
            accepted.append(has_path and bool(trigger_results.accept(trigger_index)))

            filter_tag = ROOT.edm.InputTag(filter_name, "", TRIGGER_PROCESS)
            filter_index = trigger_event.filterIndex(filter_tag)
            has_filter = filter_index != trigger_event.sizeFilters()
            filter_present.append(has_filter)
            objects = (
                [all_trigger_objects[key] for key in trigger_event.filterKeys(filter_index)]
                if has_filter
                else []
            )
            append_kinematics(objects, reco_pt, reco_eta, reco_phi)
            reco_offsets.append(len(reco_pt))

    nevents = len(gen_offsets) - 1
    np.savez_compressed(
        cache_name,
        cache_version=np.asarray([CACHE_VERSION], dtype=np.int16),
        trigger_paths=np.asarray([item[0] for item in TRIGGERS]),
        trigger_filters=np.asarray([item[1] for item in TRIGGERS]),
        gen_offsets=np.asarray(gen_offsets, dtype=np.int64),
        gen_pt=np.asarray(gen_pt, dtype=np.float32),
        gen_eta=np.asarray(gen_eta, dtype=np.float32),
        gen_phi=np.asarray(gen_phi, dtype=np.float32),
        reco_offsets=np.asarray(reco_offsets, dtype=np.int64),
        reco_pt=np.asarray(reco_pt, dtype=np.float32),
        reco_eta=np.asarray(reco_eta, dtype=np.float32),
        reco_phi=np.asarray(reco_phi, dtype=np.float32),
        path_present=np.asarray(path_present, dtype=np.bool_).reshape(nevents, ntriggers),
        accepted=np.asarray(accepted, dtype=np.bool_).reshape(nevents, ntriggers),
        filter_present=np.asarray(filter_present, dtype=np.bool_).reshape(nevents, ntriggers),
        pDoublePuppiTau52_52=np.asarray(
            l1_double_puppi_tau_fired, dtype=np.bool_
        ),
    )
    print("Saved {} events to cached_data {}".format(nevents, cache_name))
    print(
        "{} fired in {} events".format(
            L1_PATH, int(np.count_nonzero(l1_double_puppi_tau_fired))
        )
    )

def load_cache(cache_name):
    # NpzFile is a lazy ZIP reader. Keeping it in the event loops means that
    # cached_data["gen_pt"], cached_data["reco_eta"], etc. can decompress an entire array
    # on every access. Materialize all arrays once; the loops then use ordinary
    # in-memory ndarrays.
    with np.load(cache_name, allow_pickle=False) as archive:
        cached_data = {name: archive[name] for name in archive.files}
    version = int(cached_data["cache_version"][0])
    if version != CACHE_VERSION:
        raise RuntimeError(
            "Unsupported cached_data version {} (expected {})".format(version, CACHE_VERSION)
        )
    cached_paths = tuple(str(item) for item in cached_data["trigger_paths"])
    configured_paths = tuple(item[0] for item in TRIGGERS)
    if cached_paths != configured_paths:
        raise RuntimeError("Cache trigger configuration does not match this script")
    return cached_data

##################################################
# Functions for producing histograms
##################################################

def make_1d(name, variable):
    if variable == "pt":
        return ROOT.TH1D(name, name, len(PT_EDGES) - 1, PT_EDGES)
    if variable == "eta":
        return ROOT.TH1D(name, name, ETA_NBINS, ETA_RANGE[0], ETA_RANGE[1])
    raise ValueError("Unknown histogram variable: {}".format(variable))

def make_2d_pt(name):
    return ROOT.TH2D(name, name, len(PT_EDGES) - 1, PT_EDGES, len(PT_EDGES) - 1, PT_EDGES)
def make_2d_eta(name):
    return ROOT.TH2D(name, name, ETA_NBINS, ETA_RANGE[0], ETA_RANGE[1], ETA_NBINS, ETA_RANGE[0], ETA_RANGE[1])

def make_2d(name, variable):
    if variable == "pt":
        return make_2d_pt(name)
    if variable == "eta":
        return make_2d_eta(name)
    raise ValueError("Unknown histogram variable: {}".format(variable))

def add_pair_histograms(histograms, domain, events):
    """Book four 1D and two 2D histograms for one event category."""

    for variable in ("pt", "eta"):
        h2D_name = f"{domain}Tau1_{variable}_vs_{domain}Tau2_{variable}_{events}"
        histograms[h2D_name] = make_2d(f"{h2D_name}", variable)
        for index in (1, 2):
            h1D_name = f"{domain}Tau{index}_{variable}_{events}"
            histograms[h1D_name] = make_1d(f"{h1D_name}", variable)

def fill_pair_histograms(histograms, domain, events, pt, eta):
    """Fill the six histograms representing a two-object event."""
    for index in (0, 1):
        histograms[f"{domain}Tau{index+1}_pt_{events}"].Fill(pt[index])
        histograms[f"{domain}Tau{index+1}_eta_{events}"].Fill(eta[index])

    histograms[f"{domain}Tau1_pt_vs_{domain}Tau2_pt_{events}"].Fill(pt[0], pt[1])
    histograms[f"{domain}Tau1_eta_vs_{domain}Tau2_eta_{events}"].Fill(eta[0], eta[1])

GEN_EVENT_DEN = (
    "ev_2tauh",
    "ev_2tauh_acc",
    "ev_2tauh_acc_L1",
) 
GEN_EVENT_NUM = (
    "ev_2tauh_acc_L1_HLT",
    "ev_2tauh_acc_L1_HLT_match",
)
RECO_EVENT = (
    "ev_L1_HLT",
    "ev_L1_HLT_match",
)

def initialize_histograms():
    histograms = {}

    for name in GEN_EVENT_DEN:
        add_pair_histograms(histograms, "gen", name)
    for name in GEN_EVENT_NUM:
        for path, _ in TRIGGERS:
            path_skimmed = path.split("HLT_")[-1]
            add_pair_histograms(histograms, "gen", f"{name}_{path_skimmed}")

    for name in RECO_EVENT:
        for path, _ in TRIGGERS:
            path_skimmed = path.split("HLT_")[-1]
            add_pair_histograms(histograms, "reco", f"{name}_{path_skimmed}")

    return histograms

def delta_phi(first, second):
    difference = first - second
    return (difference + np.pi) % (2.0 * np.pi) - np.pi

def delta_r2(eta1, phi1, eta2, phi2):
    return (eta1 - eta2) ** 2 + delta_phi(phi1, phi2) ** 2

def is_within_acceptance(pt, eta):
    return ( pt > GEN_PT_MIN and abs(eta) < GEN_ETA_MAX)

def one_to_one_matches(gen_eta, gen_phi, reco_eta, reco_phi):
    """Return a reco index (or -1) for each gen tau, maximizing match count."""
    candidates = []
    for eta, phi in zip(gen_eta, gen_phi):
        choices = []
        for reco_index, (other_eta, other_phi) in enumerate(zip(reco_eta, reco_phi)):
            distance = delta_r2(eta, phi, other_eta, other_phi)
            if distance < MATCH_DR2:
                choices.append((reco_index, distance))
        candidates.append(choices)

    best_matches = [-1] * len(candidates)
    best_score = (-1, float("inf"))

    def assign(index, used, matches, total_distance):
        nonlocal best_matches, best_score
        if index == len(candidates):
            score = (sum(match >= 0 for match in matches), total_distance)
            if score[0] > best_score[0] or (
                score[0] == best_score[0] and score[1] < best_score[1]
            ):
                best_matches = list(matches)
                best_score = score
            return
        assign(index + 1, used, matches + [-1], total_distance)
        for reco_index, distance in candidates[index]:
            if reco_index not in used:
                assign(
                    index + 1,
                    used | {reco_index},
                    matches + [reco_index],
                    total_distance + distance,
                )

    assign(0, set(), [], 0.0)
    return best_matches

def event_gen(cached_data, event_index):
    begin, end = cached_data["gen_offsets"][event_index:event_index + 2]
    return (
        cached_data["gen_pt"][begin:end],
        cached_data["gen_eta"][begin:end],
        cached_data["gen_phi"][begin:end],
    )

def event_reco(cached_data, event_index, trigger_index):
    flat_index = event_index * len(TRIGGERS) + trigger_index
    begin, end = cached_data["reco_offsets"][flat_index:flat_index + 2]
    return (
        cached_data["reco_pt"][begin:end],
        cached_data["reco_eta"][begin:end],
        cached_data["reco_phi"][begin:end],
    )

def run_gen_loop(cached_data, histograms, cutflow):
    """Classify each gen pair using matches to all final-filter objects."""

    nevents = len(cached_data["gen_offsets"]) - 1
    for event_index in range(nevents):

        # DEN 0. All events
        cutflow["all_events"] += 1

        # DEN 1. Check that there are two gen taus (decaying hadronically by definition)
        gen_pt, gen_eta, gen_phi = event_gen(cached_data, event_index)
        if len(gen_pt) < 2:
            continue
        cutflow["events_with_two_gen_taus_h"] += 1
        selected_pt, selected_eta, selected_phi = gen_pt[:2], gen_eta[:2], gen_phi[:2]
        fill_pair_histograms(histograms, "gen", "ev_2tauh", selected_pt, selected_eta)

        # DEN 2. Check that the two gen taus (decaying hadronically by definition) are within acceptance in eta and pt
        if not all(is_within_acceptance(pt, eta) for pt, eta in zip(selected_pt, selected_eta)):
            continue
        cutflow["events_with_two_gen_taus_h_within_acceptance"] += 1
        fill_pair_histograms(histograms, "gen", "ev_2tauh_acc", selected_pt, selected_eta)

        # DEN 3. Check that the event passes L1 trigger bit
        if not bool(cached_data["pDoublePuppiTau52_52"][event_index]):
            continue
        cutflow["events_with_two_gen_taus_h_within_acceptance_firing_L1"] += 1
        fill_pair_histograms(histograms, "gen", "ev_2tauh_acc_L1", selected_pt, selected_eta)

        for trigger_index, (path, _) in enumerate(TRIGGERS):
            path_skimmed = path.split("HLT_")[-1]

            if not cached_data["path_present"][event_index, trigger_index]:
                continue
            if not cached_data["filter_present"][event_index, trigger_index]:
                continue

            # NUM 1. Check that the event fired the HLT path
            accepted = bool(cached_data["accepted"][event_index, trigger_index])
            if not accepted:
                continue
            cutflow["events_with_two_gen_taus_h_within_acceptance_firing_L1_HLT_" + path] += 1
            fill_pair_histograms(histograms, "gen", f"ev_2tauh_acc_L1_HLT_{path_skimmed}", selected_pt, selected_eta)

            # NUM 2. Check that the event fired the HLT path and it was fired by the signal taus
            reco_pt, reco_eta, reco_phi = event_reco(cached_data, event_index, trigger_index)
            matches = one_to_one_matches(selected_eta, selected_phi, reco_eta, reco_phi)
            if matches[0] >= 0 and matches[1] >= 0:
                cutflow["events_with_two_gen_taus_h_within_acceptance_firing_L1_HLT_matched_" + path] += 1
                fill_pair_histograms(histograms, "gen", f"ev_2tauh_acc_L1_HLT_match_{path_skimmed}", selected_pt, selected_eta)


def run_reco_loop(cached_data, histograms, cutflow):
    """Classify the two leading objects from each accepted final filter."""

    nevents = len(cached_data["gen_offsets"]) - 1
    for event_index in range(nevents):
        gen_pt, gen_eta, gen_phi = event_gen(cached_data, event_index)

        for trigger_index, (path, _) in enumerate(TRIGGERS):
            path_skimmed = path.split("HLT_")[-1]

            if not cached_data["path_present"][event_index, trigger_index]:
                continue
            if not cached_data["filter_present"][event_index, trigger_index]:
                continue

            reco_pt, reco_eta, reco_phi = event_reco(cached_data, event_index, trigger_index)
            if len(reco_pt) < 2:
                continue
            order = np.argsort(-reco_pt, kind="stable")[:2]
            selected_pt = reco_pt[order]
            selected_eta = reco_eta[order]
            selected_phi = reco_phi[order]

            # DEN 1. Check that event fired the HLT path
            if not cached_data["accepted"][event_index, trigger_index]:
                continue
            cutflow["events_firing_L1_HLT_" + path] += 1
            fill_pair_histograms(histograms, "reco", f"ev_L1_HLT_{path_skimmed}", selected_pt, selected_eta)

            # NUM 1. Check that event fired the HLT path and it was fired by the signal taus
            matches = one_to_one_matches(selected_eta, selected_phi, gen_eta, gen_phi)
            if matches[0] >= 0 and matches[1] >= 0:
                cutflow["events_firing_L1_HLT_matched_" + path] += 1
                fill_pair_histograms(histograms, "reco", f"ev_L1_HLT_match_{path_skimmed}", selected_pt, selected_eta)

def initialize_cutflow():

    cutflow = {
        "all_events": 0,
        "events_with_two_gen_taus_h": 0,
        "events_with_two_gen_taus_h_within_acceptance": 0,
        "events_with_two_gen_taus_h_within_acceptance_firing_L1": 0,
    }
    for path, _ in TRIGGERS:
        cutflow["events_with_two_gen_taus_h_within_acceptance_firing_L1_HLT_" + path] = 0
        cutflow["events_with_two_gen_taus_h_within_acceptance_firing_L1_HLT_matched_" + path] = 0
    
        cutflow["events_firing_L1_HLT_" + path] = 0
        cutflow["events_firing_L1_HLT_matched_" + path] = 0

    return cutflow

def write_histograms(histograms, output_name):
    output = ROOT.TFile.Open(output_name, "RECREATE")
    if not output or output.IsZombie():
        raise RuntimeError("Could not create output file: {}".format(output_name))
    try:
        for name in sorted(histograms):
            histogram = histograms[name]
            histogram.Write()
    finally:
        output.Close()

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="*", help="input EDM ROOT files")
    parser.add_argument("-o", "--outdir", default="TauPerformancePlots")
    parser.add_argument("--cache", help="NumPy cache path (default: Events.npz)")
    parser.add_argument("--reuse-cache", action="store_true", help="skip EDM reading and analyze an existing --cache file")
    parser.add_argument("--max-events", type=int, default=-1, help="limit extraction for testing")
    parser.add_argument("--cache-version", type=int, default=1, help="Cache version")
    parser.add_argument("--gen-pt-min", type=float, default=40, help="Minimum pT for gen taus")
    parser.add_argument("--gen-eta-max", type=float, default=2.1, help="Maximum |eta| for gen taus")
    parser.add_argument("--deltaR", type=float, default=0.1, help="Threshold for deltaR macthing gen-reco")
    args = parser.parse_args()

    ROOT.gROOT.SetBatch(True)
    load_fwlitelibs()
    ROOT.gSystem.Load("libDataFormatsL1Trigger.so")

    os.makedirs(args.outdir, exist_ok=True)

    TRIGGER_PROCESS = "HLTX"
    CACHE_VERSION = args.cache_version
    GEN_PT_MIN = args.gen_pt_min
    GEN_ETA_MAX = args.gen_eta_max
    MATCH_DR = args.deltaR
    MATCH_DR2 = MATCH_DR * MATCH_DR

    ##### Read inputs and produce cache

    cache_name = args.cache or os.path.join(args.outdir, "Events.npz")
    if args.reuse_cache:
        if not os.path.isfile(cache_name):
            raise RuntimeError("Cache does not exist: {}".format(cache_name))
    else:
        print("Re-running cache")
        files = args.inputs or sorted(glob.glob(DEFAULT_INPUT_GLOB))
        extract_cache(files, cache_name, args.max_events)

    ##### Define and fill histograms

    output = f"{args.outdir}/histograms.root"
    cached_data = load_cache(cache_name)
    histograms = initialize_histograms()
    cutflow = initialize_cutflow()
    run_gen_loop(cached_data, histograms, cutflow)
    run_reco_loop(cached_data, histograms, cutflow)
    write_histograms(histograms, output)
    print("Saved histograms to {}".format(output))

    print("Cutflow summary:")
    for key, value in cutflow.items():
        print("{}: {}".format(key, value))
