from __future__ import absolute_import, division, print_function

import glob
import sys
from array import array

import ROOT
from DataFormats.FWLite import Events
from modules.EvtData import EvtData, add_product
from modules.GenTools import load_fwlitelibs

load_fwlitelibs()
ROOT.gSystem.Load("libDataFormatsL1Trigger.so")

TRIGGER_PATHS = [
    ("HLT_DoubleMediumChargedIsoPFTauHPS40_eta2p1", "hltHpsDoublePFTau40TrackPt1MediumChargedIsolation"),
    ("HLT_DoubleMediumDeepTauPFTauHPS35_eta2p1", "hltHpsDoublePFTau35MediumDitauWPDeepTau"),
    ("HLT_DoubleMediumPFPuppiParTTauh30_eta2p1", "hltDoublePFJets30ParTTauhTagMediumWPL2DoubleTau"),
]

CUT_LEVELS = [
    ("baseline", "", "baseline"),
    ("single", "_withSingleCuts", "one-tau orthogonal cuts"),
    ("both", "_withAllCuts", "both-tau N-minus-one cuts"),
]

PT_MIN = 40.0
ETA_MAX = 2.1
MATCH_DR = 0.1
MATCH_DR2 = MATCH_DR * MATCH_DR
TRIGGER_PROCESS = "HLTX"

PT_BINS = array("d", [0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80, 90, 100, 120, 140, 160, 200])
ETA_BINS = 25
ETA_MIN = -2.5
ETA_MAX_HIST = 2.5

GEN_STATES = ("all_gen", "matched_gen", "matched_gen_both")
FILTER_STATES = ("all_filterobj", "matched_filterobj")
TAU_ROLES = ("leading", "subleading")
VARIABLES = ("pt", "eta")


def parse_arguments():
    if len(sys.argv) > 2:
        return [sys.argv[1]], sys.argv[2]
    if len(sys.argv) > 1:
        return [sys.argv[1]], "histograms.root"

    files = glob.glob(
        "/eos/user/a/agruber/samples/HLT_Upgrade_L1filter/"
        "ParT_unfiltered_16_1_1/Phase2_L1P2GT_HLT_*.root"
    )
    return files, "histograms.root"


def make_histogram(name, title, variable):
    if variable == "pt":
        return ROOT.TH1D(name, title, len(PT_BINS) - 1, PT_BINS)
    return ROOT.TH1D(name, title, ETA_BINS, ETA_MIN, ETA_MAX_HIST)


def initialize_histograms():
    histograms = {cut_key: {} for cut_key, _, _ in CUT_LEVELS}

    for cut_key, suffix, cut_title in CUT_LEVELS:
        for path, _ in TRIGGER_PATHS:
            path_histograms = {}

            for variable in VARIABLES:
                for role in TAU_ROLES:
                    for state in GEN_STATES:
                        category = "{}_{}_{}".format(variable, role, state)
                        name = "{}_{}{}".format(category, path, suffix)
                        title = "{} for {} ({})".format(category.replace("_", " ").title(), path, cut_title)
                        path_histograms[category] = make_histogram(name, title, variable)

                if variable == "pt":
                    for state in FILTER_STATES:
                        category = "{}_{}".format(variable, state)
                        name = "{}_{}{}".format(category, path, suffix)
                        title = "{} for {} ({})".format(category.replace("_", " ").title(), path, cut_title)
                        path_histograms[category] = make_histogram(name, title, variable)

                if cut_key == "baseline" and variable == "eta":
                    category = "eta_all_filterobj"
                    name = "{}_{}".format(category, path)
                    title = "{} for {}".format(category.replace("_", " ").title(), path)
                    path_histograms[category] = make_histogram(name, title, variable)

                if cut_key == "baseline":
                    category = "{}_fake_filterobj".format(variable)
                    name = "{}_{}".format(category, path)
                    title = "{} for {}".format(category.replace("_", " ").title(), path)
                    path_histograms[category] = make_histogram(name, title, variable)

            for role in TAU_ROLES:
                category = "pt_gen_vs_reco_matched_{}".format(role)
                name = "{}_{}{}".format(category, path, suffix)
                title = "{} for {} ({})".format(category.replace("_", " ").title(), path, cut_title)
                path_histograms[category] = ROOT.TH2D(
                    name,
                    title,
                    100,
                    0,
                    PT_BINS[-1],
                    100,
                    0,
                    PT_BINS[-1],
                )

            histograms[cut_key][path] = path_histograms

    return histograms


def value(tau, variable):
    return tau.pt() if variable == "pt" else tau.eta()


def passes_all_cuts(tau):
    return tau.pt() > PT_MIN and abs(tau.eta()) < ETA_MAX


def passes_orthogonal_cut(obj, variable):
    if variable == "pt":
        return abs(obj.eta()) < ETA_MAX
    return obj.pt() > PT_MIN


def passes_gen_selection(tau, other_tau, variable, cut_key):
    if cut_key == "baseline":
        return True
    if not passes_orthogonal_cut(tau, variable):
        return False
    if cut_key == "single":
        return True
    return passes_all_cuts(other_tau)


def passes_filter_selection(obj, variable, cut_key, both_taus_pass_cuts):
    if cut_key == "baseline":
        return True
    if not passes_orthogonal_cut(obj, variable):
        return False
    return cut_key == "single" or both_taus_pass_cuts


def delta_r2(first, second):
    return ROOT.reco.deltaR2(first.eta(), first.phi(), second.eta(), second.phi())


def match_gen_taus(gen_taus, trigger_objects):
    candidates = [[] for _ in gen_taus]
    for gen_index, tau in enumerate(gen_taus):
        for object_index, trigger_object in enumerate(trigger_objects):
            distance = delta_r2(tau, trigger_object)
            if distance < MATCH_DR2:
                candidates[gen_index].append((object_index, distance))

    best_matches = [None] * len(gen_taus)
    best_score = (-1, float("inf"))

    def assign(gen_index, assigned_objects, matches, total_distance):
        nonlocal best_matches, best_score
        if gen_index == len(gen_taus):
            score = (sum(match is not None for match in matches), total_distance)
            if score[0] > best_score[0] or (score[0] == best_score[0] and score[1] < best_score[1]):
                best_matches = list(matches)
                best_score = score
            return

        assign(gen_index + 1, assigned_objects, matches + [None], total_distance)
        for object_index, distance in candidates[gen_index]:
            if object_index in assigned_objects:
                continue
            assign(
                gen_index + 1,
                assigned_objects | {object_index},
                matches + [trigger_objects[object_index]],
                total_distance + distance,
            )

    assign(0, set(), [], 0.0)
    return best_matches


def find_trigger_index(trigger_names, path):
    versioned_prefix = "{}_v".format(path)
    for index in range(trigger_names.size()):
        trigger_name = str(trigger_names.triggerName(index))
        if trigger_name == path or trigger_name.startswith(versioned_prefix):
            return index
    return None


def fill_gen_histograms(histograms, path, taus, matches, trigger_accept):
    both_matched = all(match is not None for match in matches)

    for role_index, role in enumerate(TAU_ROLES):
        tau = taus[role_index]
        other_tau = taus[1 - role_index]
        matched = matches[role_index] is not None

        for cut_key, _, _ in CUT_LEVELS:
            path_histograms = histograms[cut_key][path]
            for variable in VARIABLES:
                if not passes_gen_selection(tau, other_tau, variable, cut_key):
                    continue

                path_histograms["{}_{}_all_gen".format(variable, role)].Fill(value(tau, variable))
                if trigger_accept and matched:
                    path_histograms["{}_{}_matched_gen".format(variable, role)].Fill(value(tau, variable))
                if trigger_accept and both_matched:
                    path_histograms["{}_{}_matched_gen_both".format(variable, role)].Fill(value(tau, variable))

            if (
                trigger_accept
                and matched
                and passes_gen_selection(tau, other_tau, "pt", cut_key)
            ):
                path_histograms["pt_gen_vs_reco_matched_{}".format(role)].Fill(
                    tau.pt(), matches[role_index].pt()
                )


def fill_filter_histograms(histograms, path, trigger_objects, all_gen_taus, both_taus_pass_cuts, trigger_accept):
    if not trigger_accept:
        return

    for trigger_object in trigger_objects:
        matched = any(delta_r2(tau, trigger_object) < MATCH_DR2 for tau in all_gen_taus)
        histograms["baseline"][path]["eta_all_filterobj"].Fill(trigger_object.eta())

        if not matched:
            for variable in VARIABLES:
                histograms["baseline"][path]["{}_fake_filterobj".format(variable)].Fill(
                    value(trigger_object, variable)
                )

        for cut_key, _, _ in CUT_LEVELS:
            path_histograms = histograms[cut_key][path]
            if not passes_filter_selection(trigger_object, "pt", cut_key, both_taus_pass_cuts):
                continue

            path_histograms["pt_all_filterobj"].Fill(trigger_object.pt())
            if matched:
                path_histograms["pt_matched_filterobj"].Fill(trigger_object.pt())


def write_histograms(histograms, output_filename):
    output_file = ROOT.TFile(output_filename, "RECREATE")
    if not output_file or output_file.IsZombie():
        raise RuntimeError("Could not create output file: {}".format(output_filename))

    for cut_key, _, _ in CUT_LEVELS:
        for path_histograms in histograms[cut_key].values():
            for histogram in path_histograms.values():
                histogram.Write()

    output_file.Close()


def main():
    files, output_filename = parse_arguments()
    if not files:
        raise RuntimeError("No input ROOT files were found")

    products = []
    add_product(products, "trig_sum", "trigger::TriggerEvent", "hltTriggerSummaryAOD")
    add_product(products, "trig_res", "edm::TriggerResults", "TriggerResults")
    add_product(products, "genVisTaus", "std::vector<reco::GenParticle>", "genVisTaus")

    events = Events(files)
    evtdata = EvtData(products, verbose=False)
    histograms = initialize_histograms()

    cutflow = {
        "TotalEvents": 0,
        "TwoGenTaus": 0,
        "BothTausPassCuts": 0,
    }
    for path, _ in TRIGGER_PATHS:
        cutflow["{}_accepted".format(path)] = 0
        cutflow["{}_lead_matched".format(path)] = 0
        cutflow["{}_sub_matched".format(path)] = 0
        cutflow["{}_both_matched".format(path)] = 0
        cutflow["{}_missing_path".format(path)] = 0
        cutflow["{}_missing_filter".format(path)] = 0

    for event_number, event in enumerate(events):
        cutflow["TotalEvents"] += 1
        if event_number % 100 == 0:
            print("Processing event {}".format(event_number))

        evtdata.get_handles(event)
        trigger_results = evtdata.get("trig_res")
        trigger_event = evtdata.get("trig_sum")
        all_gen_taus = list(evtdata.get("genVisTaus") or [])

        if len(all_gen_taus) < 2:
            continue

        cutflow["TwoGenTaus"] += 1
        all_gen_taus.sort(key=lambda tau: tau.pt(), reverse=True)
        taus = all_gen_taus[:2]
        both_taus_pass_cuts = all(passes_all_cuts(tau) for tau in taus)
        if both_taus_pass_cuts:
            cutflow["BothTausPassCuts"] += 1

        trigger_names = event.object().triggerNames(trigger_results)

        for path, filter_name in TRIGGER_PATHS:
            trigger_index = find_trigger_index(trigger_names, path)
            if trigger_index is None:
                cutflow["{}_missing_path".format(path)] += 1
                continue

            trigger_accept = trigger_results.accept(trigger_index)
            if trigger_accept:
                cutflow["{}_accepted".format(path)] += 1

            filter_tag = ROOT.edm.InputTag(filter_name, "", TRIGGER_PROCESS)
            filter_index = trigger_event.filterIndex(filter_tag)
            if filter_index == trigger_event.sizeFilters():
                cutflow["{}_missing_filter".format(path)] += 1
                trigger_objects = []
            else:
                all_trigger_objects = trigger_event.getObjects()
                trigger_objects = [
                    all_trigger_objects[key]
                    for key in trigger_event.filterKeys(filter_index)
                ]

            matches = match_gen_taus(taus, trigger_objects)
            lead_matched = matches[0] is not None
            sub_matched = matches[1] is not None
            if trigger_accept and lead_matched:
                cutflow["{}_lead_matched".format(path)] += 1
            if trigger_accept and sub_matched:
                cutflow["{}_sub_matched".format(path)] += 1
            if trigger_accept and lead_matched and sub_matched:
                cutflow["{}_both_matched".format(path)] += 1

            fill_gen_histograms(histograms, path, taus, matches, trigger_accept)
            fill_filter_histograms(
                histograms,
                path,
                trigger_objects,
                all_gen_taus,
                both_taus_pass_cuts,
                trigger_accept,
            )

    write_histograms(histograms, output_filename)

    print("Cutflow Summary:")
    for key, count in cutflow.items():
        print("{}: {}".format(key, count))
    print("Histograms have been saved to {}.".format(output_filename))


if __name__ == "__main__":
    main()
