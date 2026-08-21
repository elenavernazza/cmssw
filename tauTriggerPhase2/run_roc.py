from __future__ import absolute_import, division, print_function

import argparse
import glob
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import mplhep as hep
import numpy as np
import ROOT
from DataFormats.FWLite import Events
from tqdm import tqdm

from modules.EvtData import EvtData, add_product
from modules.GenTools import load_fwlitelibs

DEFAULT_INPUT_GLOB = "/eos/user/a/agruber/samples/HLT_Upgrade_L1filter/ParT_unfiltered_16_1_1/Phase2_L1P2GT_HLT_*.root"

PRODUCTS = (
    ("gen_taus", "std::vector<reco::GenParticle>", "genVisTaus::HLTX"),
    ("jets", "std::vector<reco::PFJet>", "hltAK4PFPuppiJets::HLTX"),
    ("taus", "std::vector<reco::PFTau>", "hltHpsPFTauProducer::HLTX"),
    ("part", "edm::AssociationVector<edm::RefToBaseProd<reco::Jet>,std::vector<float>,edm::RefToBase<reco::Jet>,unsigned int,edm::helper::AssociationIdenticalKeyReference>", "hltParticleTransformerDiscriminatorsJetTags:TauvsAll:HLTX"),
    ("deeptau", "edm::ValueMap<reco::SingleTauDiscriminatorContainer>", "hltHpsPFTauDeepTauProducer:VSjet:HLTX"),
)

##################################################
# Functions for reading input files
##################################################

def build_products():
    products = []
    for name, type_name, tag in PRODUCTS:
        add_product(products, name, type_name, tag)
    return products

def delta_phi(first, second):
    return (first - second + np.pi) % (2.0 * np.pi) - np.pi

def matched_to_any(obj, gen_taus, match_dr2):
    return any(
        (float(obj.eta()) - float(gen.eta())) ** 2
        + delta_phi(float(obj.phi()), float(gen.phi())) ** 2 < match_dr2
        for gen in gen_taus
    )

def raw_deeptau_score(value_map, index):
    # This ValueMap has one keyed collection, so its one-argument overload is
    # equivalent to C++ get(tausHandle.id(), index) without needing ProductID
    # access from the lightweight FWLite Handle wrapper.
    result = value_map.get(index)
    return -1.0 if result.rawValues.empty() else float(result.rawValues.front())

def extract_cache(files, cache_name, max_events, match_dr):
    """Read EDM products once and store only primitive analysis quantities."""
    if not files:
        raise RuntimeError("No input ROOT files were found")

    events = Events(files)
    evtdata = EvtData(build_products(), verbose=False)
    data = {name: {field: [] for field in ("pt", "eta", "score", "label")} for name in ("part", "deeptau")}
    match_dr2 = match_dr * match_dr

    for event_index, event in enumerate(tqdm(events, total=events.size())):
        if max_events >= 0 and event_index >= max_events:
            break

        evtdata.get_handles(event)
        gen_taus = evtdata.get("gen_taus")
        if gen_taus is None:
            continue
        gen_taus = list(gen_taus)

        # Fill Jets + ParT information

        jets = evtdata.get("jets")
        part_scores = evtdata.get("part")

        if jets is not None and part_scores is not None:

            for index in range(int(part_scores.size())):
                jet_ref = part_scores.key(index)

                if jet_ref.isNull() or int(jet_ref.key()) >= len(jets):
                    raise RuntimeError(
                        "ParT score association has an invalid jet reference at event {} (null={}, key={}, jets={}, scores={})".format(
                            event_index, bool(jet_ref.isNull()), int(jet_ref.key()), len(jets), int(part_scores.size()),))

                jet = jets[int(jet_ref.key())]
                # Avoid part_scores[index].second here. PyROOT represents that
                # expression with a transient std::pair proxy which can retain
                # a stale value when FWLite moves to the next event. The
                # AssociationVector's indexed value() API returns the stored
                # float directly and agrees with partScores[index].second in
                # the C++ TauROCAnalyzer.
                score = float(part_scores.value(index))
                if not np.isfinite(score) or score < 0.0 or score > 1.0:
                    continue

                isSignal = int(matched_to_any(jet, gen_taus, match_dr2))
                data["part"]["pt"].append(jet.pt())
                data["part"]["eta"].append(jet.eta())
                data["part"]["score"].append(score)
                data["part"]["label"].append(isSignal)

        # Fill HPS + DeepTau information

        taus = evtdata.get("taus")
        deep_tau = evtdata.get("deeptau")
        
        if taus is not None and deep_tau is not None:

            for index, tau in enumerate(taus):
                score = raw_deeptau_score(deep_tau, index)
                if not np.isfinite(score) or score < 0.0 or score > 1.0:
                    continue

                isSignal = int(matched_to_any(tau, gen_taus, match_dr2))
                data["deeptau"]["pt"].append(tau.pt())
                data["deeptau"]["eta"].append(tau.eta())
                data["deeptau"]["score"].append(score)
                data["deeptau"]["label"].append(isSignal)

    payload = { "cache_version": np.asarray([CACHE_VERSION], dtype=np.int16),
                "match_dr": np.asarray([match_dr], dtype=np.float32),}
    
    for algorithm, fields in data.items():
        for field, values in fields.items():
            dtype = np.int8 if field == "label" else np.float32
            payload[algorithm + "_" + field] = np.asarray(values, dtype=dtype)

    np.savez_compressed(cache_name, **payload)

    print("Saved events to {}".format(cache_name))

def load_cache(cache_name):
    with np.load(cache_name, allow_pickle=False) as archive:
        data = {name: archive[name] for name in archive.files}
    version = int(data["cache_version"][0])
    if version != CACHE_VERSION:
        raise RuntimeError(
            "Unsupported cache version {} (expected {})".format(version, CACHE_VERSION)
        )
    return data

##################################################
# Functions for producing histograms
##################################################

def select(cache, algorithm, pt_min, pt_max, eta_max):
    pt = cache[algorithm + "_pt"]
    eta = cache[algorithm + "_eta"]
    keep = (pt >= pt_min) & (pt <= pt_max) & (np.abs(eta) <= eta_max)
    result = {
        field: cache[algorithm + "_" + field][keep]
        for field in ("pt", "eta", "score", "label")
    }
    signal = int(np.count_nonzero(result["label"] == 1))
    background = int(np.count_nonzero(result["label"] == 0))
    if not signal or not background:
        raise RuntimeError(
            "{} selection contains {} signal and {} background objects".format(
                algorithm, signal, background
            )
        )
    print("[LOAD] {}: N={}, signal={}, background={}".format(
        algorithm, len(result["score"]), signal, background
    ))
    return result

def dataset_pt_min(args, algorithm):
    specific_pt_min = getattr(args, algorithm + "_pt_min")
    return specific_pt_min

def threshold_scan(scores, labels):
    valid = np.isfinite(scores)
    scores, labels = scores[valid], labels[valid]
    order = np.argsort(-scores, kind="stable")
    scores, labels = scores[order], labels[order]
    last = np.r_[np.flatnonzero(scores[1:] != scores[:-1]), len(scores) - 1]
    signal = np.count_nonzero(labels == 1)
    background = np.count_nonzero(labels == 0)
    tpr = np.r_[0.0, np.cumsum(labels == 1)[last] / signal]
    fpr = np.r_[0.0, np.cumsum(labels == 0)[last] / background]
    return fpr, tpr

def threshold_for_tpr(scores, labels, target_tpr):
    valid = np.isfinite(scores)
    scores, labels = scores[valid], labels[valid]
    signal = np.count_nonzero(labels == 1)
    if not signal:
        raise ValueError("Cannot determine a threshold without signal events")

    order = np.argsort(-scores, kind="stable")
    sorted_scores = scores[order]
    sorted_labels = labels[order]
    last = np.r_[np.flatnonzero(sorted_scores[1:] != sorted_scores[:-1]), len(sorted_scores) - 1]
    tpr = np.cumsum(sorted_labels == 1)[last] / signal
    selected = np.argmin(np.abs(tpr - target_tpr))
    return float(sorted_scores[last[selected]])

def operating_point(labels, passes):
    signal, background = labels == 1, labels == 0
    return (
        np.count_nonzero(passes & signal) / np.count_nonzero(signal),
        np.count_nonzero(passes & background) / np.count_nonzero(background),
    )

def make_raw_distribution(cache, args):

    datasets = {
        name: select(cache, name, dataset_pt_min(args, name), args.pt_max, args.eta_max)
        for name in ("part", "deeptau")
    }
    
    plot_info = {
        "part": ("ParT", "ParT Tau-vs-all score"),
        "deeptau": ("DeepTau", "DeepTau VSjet score"),
    }
    categories = (
        (1, "Signal: gen-matched", "tab:blue"),
        (0, "Background: unmatched", "tab:orange"),
    )
    bins = np.linspace(0.0, 1.0, 51)

    plt.style.use(hep.cms.style.CMS)

    for algorithm in ("part", "deeptau"):
        data = datasets[algorithm]
        title, xlabel = plot_info[algorithm]
        fig, axis = plt.subplots(figsize=(10, 10))

        for category, label, color in categories:
            scores = data["score"][data["label"] == category]
            # Normalize each category independently so that the shapes can be
            # compared despite the very different signal/background yields.
            weights = np.full(len(scores), 1.0 / len(scores))
            axis.hist(
                scores,
                bins=bins,
                weights=weights,
                histtype="step",
                linewidth=2.0,
                color=color,
                label=r"{} ({:} $\tau$'s)".format(label, len(scores)),
            )

        axis.set_xlabel(xlabel)
        axis.set_xlim(0.0, 1.0)
        axis.set_yscale("log")
        axis.grid(which="major", alpha=0.25)
        axis.grid(which="minor", alpha=0.10)
        axis.legend(frameon=False, fontsize=20, title=title, alignment="left", loc='upper left')
        axis.set_ylabel(r"Number of $\tau$'s")
        # axis.text(0.95, 0.05, "$p_T$ > {:g} GeV\n$|\eta| \leq ${:g}".format(
        #         dataset_pt_min(args, algorithm), args.eta_max),
        #     transform=axis.transAxes, ha="right", va="bottom", fontsize=20)

        hep.cms.label(data=False, label="Preliminary", ax=axis, loc=0, com=14)
        fig.tight_layout()

        png = os.path.join(args.outdir, "Distribution_{}_raw_score.png".format(algorithm))
        pdf = os.path.join(args.outdir, "Distribution_{}_raw_score.pdf".format(algorithm))
        fig.savefig(png, bbox_inches="tight")
        fig.savefig(pdf, bbox_inches="tight")
        plt.close(fig)
        print("Saved {} and {}".format(png, pdf))

def make_roc_curve(cache, args):

    datasets = {
        name: select(cache, name, dataset_pt_min(args, name), args.pt_max, args.eta_max)
        for name in ("deeptau", "part")
    }

    styles = {
        "deeptau": ("DeepTau", "grey", "--"),
        "part": ("ParT", "tab:blue", "-"),
    }

    plt.style.use(hep.cms.style.CMS)

    fig, axis = plt.subplots(figsize=(10, 9))
    for name, data in datasets.items():
        fpr, tpr = threshold_scan(data["score"], data["label"])
        area = float(np.trapz(tpr, fpr))
        label, color, linestyle = styles[name]
        axis.plot(np.clip(fpr, args.ymin, 1.0), tpr, color=color,
                  linestyle=linestyle, linewidth=1.8,
                  label=r"{} ($AUC={:.3f}$)".format(label, area))
        print("[ROC] {}: AUC={:.6f}".format(label, area))

    deep = datasets["deeptau"]
    thresholds = np.interp(deep["pt"], [35.0, 100.0, 300.0],
                           [0.649, 0.441, 0.050])
    deep_point = operating_point(deep["label"], (deep["pt"] > dataset_pt_min(args, "deeptau")) & (np.abs(deep["eta"]) < args.eta_max) & (deep["score"] > thresholds))
    axis.scatter(max(deep_point[1], args.ymin), deep_point[0], marker="o", s=90,
                 facecolors="white", edgecolors="grey", linewidths=2, 
                 label="DeepTau Medium WP", zorder=10)
    print("[WP] DeepTau Medium: efficiency={:.6f}, mis-id={:.6f}".format(*deep_point))

    part = datasets["part"]
    part_wp = threshold_for_tpr(part["score"], part["label"], deep_point[0])
    part_point = operating_point(part["label"], part["score"] >= part_wp)
    axis.scatter(max(part_point[1], args.ymin), part_point[0], marker="o", s=90,
                 facecolors="white", edgecolors="tab:blue", linewidths=2,
                 label="ParT WP $\\geq$ {:.3f}".format(part_wp), zorder=10)
    print("[WP] ParT score >= {:.6f}: efficiency={:.6f}, mis-id={:.6f} (target TPR={:.6f})".format(
        part_wp, part_point[0], part_point[1], deep_point[0]))

    axis.set_xlabel("False Positive Rate (FPR)")
    axis.set_ylabel("True Positive Rate (TPR)")
    # axis.set_yscale("log")
    axis.set_xlim(args.xmin, 1.0)
    axis.set_ylim(args.ymin, 1.0)
    axis.grid(which="major", alpha=0.25)
    axis.grid(which="minor", alpha=0.10)
    hep.cms.label(data=False, label="Preliminary", ax=axis, loc=0, com=14)
    axis.legend(loc="lower right", frameon=False, fontsize=20)
    fig.tight_layout()
    png = os.path.join(args.outdir, "tau_roc.png")
    pdf = os.path.join(args.outdir, "tau_roc.pdf")
    fig.savefig(png, bbox_inches="tight")
    fig.savefig(pdf, bbox_inches="tight")
    plt.close(fig)
    print("Saved {} and {}".format(png, pdf))

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="*", help="input EDM ROOT files")
    parser.add_argument("-o", "--outdir", default="TauROC")
    parser.add_argument("--cache", help="NumPy cache path (default: OUTDIR/Events.npz)")
    parser.add_argument("--reuse-cache", action="store_true", help="skip EDM reading and analyse an existing cache")
    parser.add_argument("--max-events", type=int, default=-1)
    parser.add_argument("--deltaR", type=float, default=0.1, help="gen-to-reco matching radius")
    parser.add_argument("--part-pt-min", type=float, default=30, help="ParT-specific lower pT selection")
    parser.add_argument("--deeptau-pt-min", type=float, default=30, help="DeepTau-specific lower pT selection")
    parser.add_argument("--pt-max", type=float, default=1000.0)
    parser.add_argument("--eta-max", type=float, default=2.1)
    parser.add_argument("--xmin", type=float, default=0.0)
    parser.add_argument("--ymin", type=float, default=0.0)
    args = parser.parse_args()

    if args.deltaR <= 0 or args.eta_max <= 0:
        parser.error("--deltaR, --eta-max, and --ymin must be positive")
    for algorithm in ("part", "deeptau"):
        pt_min = dataset_pt_min(args, algorithm)
        if pt_min > args.pt_max:
            parser.error("--{}-pt-min must not exceed --pt-max".format(algorithm))
    ROOT.gROOT.SetBatch(True)
    load_fwlitelibs()
    
    os.makedirs(args.outdir, exist_ok=True)

    CACHE_VERSION = 2
    
    ##### Read inputs and produce cache

    cache_name = args.cache or os.path.join(args.outdir, "Events.npz")
    if args.reuse_cache:
        if not os.path.isfile(cache_name):
            raise RuntimeError("Cache does not exist: {}".format(cache_name))
    else:
        print("Re-running cache")
        files = args.inputs or sorted(glob.glob(DEFAULT_INPUT_GLOB))
        extract_cache(files, cache_name, args.max_events, args.deltaR)

    ##### Define and fill histograms

    cached_data = load_cache(cache_name)
    make_raw_distribution(cached_data, args)
    make_roc_curve(cached_data, args)
