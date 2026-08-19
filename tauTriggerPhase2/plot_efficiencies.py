import argparse, os, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import mplhep as hep
import numpy as np
import uproot

plt.style.use(hep.style.CMS)

from run_efficiencies import GEN_EVENT_DEN,GEN_EVENT_NUM,RECO_EVENT

TRIGGERS = (
    ("HLT_DoubleMediumChargedIsoPFTauHPS40_eta2p1", "Double HPS 40, charged isolation", "#e42536"),
    ("HLT_DoubleMediumDeepTauPFTauHPS35_eta2p1", "Double HPS 35, DeepTau", "#f89c20"),
    ("HLT_DoubleMediumPFPuppiParTTauh30_eta2p1", "Double Puppi 30, ParT", "#5790fc"),
)

TRIGGER_PT_MIN = {
    "HLT_DoubleMediumChargedIsoPFTauHPS40_eta2p1": 40.0,
    "HLT_DoubleMediumDeepTauPFTauHPS35_eta2p1": 35.0,
    "HLT_DoubleMediumPFPuppiParTTauh30_eta2p1": 30.0,
}

VARIABLE_LABELS = {
    "genTau1_pt": r"Leading genVisTau $p_{\mathrm{T}}$ [GeV]", 
    "genTau2_pt": r"Sub-leading genVisTau $p_{\mathrm{T}}$ [GeV]", 
    "genTau1_eta": r"Leading genVisTau $\eta$", 
    "genTau2_eta": r"Sub-leading genVisTau $\eta$", 
    "recoTau1_pt": r"Leading reco Tau $p_{\mathrm{T}}$ [GeV]", 
    "recoTau2_pt": r"Sub-leading reco Tau $p_{\mathrm{T}}$ [GeV]", 
    "recoTau1_eta": r"Leading reco Tau $\eta$", 
    "recoTau2_eta": r"Sub-leading reco Tau $\eta$", 
}

EVENTS_LABELS = {
    "ev_2tauh": r"Two gen $\tau$'s",
    "ev_2tauh_acc": r"Two gen $\tau$'s with",
    "ev_2tauh_acc_L1": r"Two gen $\tau$'s with passing L1",
    "ev_2tauh_acc_L1_HLT": r"Two gen $\tau$'s with passing L1 and HLT",
    "ev_2tauh_acc_L1_HLT_match": r"Two gen $\tau$'s with passing L1 and HLT and match",
    "ev_L1_HLT": r"Events passing L1 and HLT",
    "ev_L1_HLT_match": r"Events passing L1 and HLT and match",
}

def histogram_is_path_specific(domain, event_name):
    if domain == "reco":
        return True
    return event_name != "ev_2tauh"

def event_label(event_name, domain, variable, tau_index, path):
    """Return an event label containing the exact N-minus-one acceptance."""
    label = EVENTS_LABELS[event_name]
    if domain != "gen" or event_name == "ev_2tauh":
        return label

    pt_min = TRIGGER_PT_MIN[path]
    pt1 = rf"$p_{{T,1}}>{pt_min:g}$ GeV"
    pt2 = rf"$p_{{T,2}}>{pt_min:g}$ GeV"
    eta1 = r"$|\eta_1|<2.1$"
    eta2 = r"$|\eta_2|<2.1$"

    if variable == "pt" and tau_index == 1:
        cuts = (pt2, eta1, eta2)
    elif variable == "pt" and tau_index == 2:
        cuts = (pt1, eta1, eta2)
    elif variable == "eta" and tau_index == 1:
        cuts = (pt1, pt2, eta2)
    elif variable == "eta" and tau_index == 2:
        cuts = (pt1, pt2, eta1)
    elif variable == "pt" and tau_index is None:
        cuts = (eta1, eta2)
    elif variable == "eta" and tau_index is None:
        cuts = (pt1, pt2)
    else:
        raise ValueError(
            f"Unsupported acceptance label: {variable}, tau {tau_index}"
        )

    return label.replace(
        "with",
        "with {}".format(", ".join(cuts)),
    )

def generic_event_label(event_name, domain, variable, tau_index, path):
    """Return an event label containing the exact N-minus-one acceptance without specifying pT."""
    label = EVENTS_LABELS[event_name]
    if domain != "gen" or event_name == "ev_2tauh":
        return label

    pt1 = rf"$p_{{T,1}}>p_{{T}}^{{min}}$ GeV"
    pt2 = rf"$p_{{T,2}}>p_{{T}}^{{min}}$ GeV"
    eta1 = r"$|\eta_1|<2.1$"
    eta2 = r"$|\eta_2|<2.1$"

    if variable == "pt" and tau_index == 1:
        cuts = (pt2, eta1, eta2)
    elif variable == "pt" and tau_index == 2:
        cuts = (pt1, eta1, eta2)
    elif variable == "eta" and tau_index == 1:
        cuts = (pt1, pt2, eta2)
    elif variable == "eta" and tau_index == 2:
        cuts = (pt1, pt2, eta1)
    elif variable == "pt" and tau_index is None:
        cuts = (eta1, eta2)
    elif variable == "eta" and tau_index is None:
        cuts = (pt1, pt2)
    else:
        raise ValueError(
            f"Unsupported acceptance label: {variable}, tau {tau_index}"
        )

    return label.replace(
        "with",
        "with {}".format(", ".join(cuts)),
    )

def histogram_values(root_file, name):
    try:
        histogram = root_file[name]
    except KeyError as error:
        raise RuntimeError("Missing histogram: {}".format(name)) from error
    return histogram.to_numpy(flow=False)

def save_figure(fig, outdir, stem, formats):
    for extension in formats:
        filename = os.path.join(outdir, "{}.{}".format(stem, extension))
        fig.savefig(filename, bbox_inches="tight")
    print("Saved {}".format(filename))
    plt.close(fig)

def plot_1d_histos(root_file, outdir, args, domain, names, variable, tau_index):

    for path, trigger_label, color in TRIGGERS:
        fig, ax = plt.subplots(figsize=(10, 10))
        max_count = 0
        path_skimmed = path.split("HLT_")[-1]
        for name in names:
            h_name = f"{domain}Tau{tau_index}_{variable}_{name}"
            if histogram_is_path_specific(domain, name):
                h_name = f"{h_name}_{path_skimmed}"

            counts, edges = histogram_values(root_file, h_name)
            counts_err = [np.sqrt(count) for count in counts]
            max_count = max(max(counts), max_count)
            hep.histplot(
                counts, bins=edges, yerr=counts_err, histtype="step", linewidth=2,
                label=event_label(name, domain, variable, tau_index, path), ax=ax,
            )

        ax.set_xlabel(VARIABLE_LABELS[f"{domain}Tau{tau_index}_{variable}"])
        ax.set_ylabel("Number of events")
        ax.set_xlim(edges[0],edges[-1])
        ax.set_ylim(0.0, 1.35*max_count)
        ax.grid(True, alpha=0.25)
        ax.legend(loc="upper left", fontsize=12, title=f"{path}", title_fontsize=12, alignment="left")
        label = hep.cms.label("", data=False, com=14, ax=ax, loc=0)
        if ax.get_ylim()[1] > 100:
            for t in label:
                x, y = t.get_position()
                t.set_position((x + 0.1, y - 0.002))
        ax.ticklabel_format(style="sci", scilimits=(-3, 3), useMathText=True)
        os.system(f"mkdir -p {outdir}/1D_Distributions")
        out_plot = f"{domain}Tau{tau_index}_{variable}_{path_skimmed}"
        save_figure(fig, f"{outdir}/1D_Distributions", out_plot, args.formats)

def plot_2d_histo(root_file, outdir, args, domain, name, variable):

    for path, _, _ in TRIGGERS:
        h_name = f"{domain}Tau1_{variable}_vs_{domain}Tau2_{variable}_{name}"
        label = event_label(name, domain, variable, None, path)
        if histogram_is_path_specific(domain, name):
            path_skimmed = path.split("HLT_")[-1]
            h_name = f"{h_name}_{path_skimmed}"
            label = f"{path}\n{event_label(name, domain, variable, None, path)}"

        counts, x_edges, y_edges = histogram_values(root_file, h_name)
        fig, ax = plt.subplots(figsize=(10, 10))
        image = ax.pcolormesh(x_edges, y_edges, counts.T,
            cmap="viridis", shading="flat")
        colorbar = fig.colorbar(image, ax=ax, pad=0.02)
        colorbar.set_label("Number of events")
        ax.set_xlabel(VARIABLE_LABELS[f"{domain}Tau1_{variable}"])
        ax.set_ylabel(VARIABLE_LABELS[f"{domain}Tau2_{variable}"])
        ax.text(0.03, 0.97, f"{label}", color="white", transform=ax.transAxes, ha="left", va="top", fontsize=12)
        label = hep.cms.label("", data=False, com=14, ax=ax, loc=0)
        os.system(f"mkdir -p {outdir}/2D_Distributions")
        out_plot = h_name
        save_figure(fig, f"{outdir}/2D_Distributions", out_plot, args.formats)

        if not histogram_is_path_specific(domain, name):
            break

def checked_ratio(numerator, denominator, numerator_name, denominator_name):
    numerator = np.asarray(numerator, dtype=np.float64)
    denominator = np.asarray(denominator, dtype=np.float64)
    invalid = numerator > denominator + 1.0e-9

    if np.any(invalid):
        raise RuntimeError(
            "{} is larger than {} in {} bins".format(
                numerator_name, denominator_name, int(np.count_nonzero(invalid))
            )
        )
    
    ratio = np.full(denominator.shape, np.nan, dtype=np.float64)
    uncertainty = np.full(denominator.shape, np.nan, dtype=np.float64)
    valid = denominator > 0.0
    ratio[valid] = numerator[valid] / denominator[valid]
    uncertainty[valid] = np.sqrt(ratio[valid] * (1.0 - ratio[valid]) / denominator[valid])

    return ratio, uncertainty

def plot_1d_ratio(root_file, outdir, args, domain, variable, tau_index, den_name, num_name, quantity):
    fig, ax = plt.subplots(figsize=(10, 10))

    for path, trigger_label, color in TRIGGERS:
        path_skimmed = path.split("HLT_")[-1]
        h_den_name = f"{domain}Tau{tau_index}_{variable}_{den_name}"
        if histogram_is_path_specific(domain, den_name):
            h_den_name = f"{h_den_name}_{path_skimmed}"
        h_num_name = f"{domain}Tau{tau_index}_{variable}_{num_name}"
        if histogram_is_path_specific(domain, num_name):
            h_num_name = f"{h_num_name}_{path_skimmed}"

        den_counts, den_edges = histogram_values(root_file, h_den_name)
        num_counts, num_edges = histogram_values(root_file, h_num_name)
        if not np.array_equal(den_edges, num_edges):
            raise RuntimeError(
                f"Histogram binning mismatch: {h_num_name} / {h_den_name}"
            )

        ratio, uncertainty = checked_ratio(num_counts, den_counts, h_num_name, h_den_name)

        bin_centers = 0.5 * (den_edges[:-1] + den_edges[1:])
        bin_half_widths = 0.5 * np.diff(den_edges)
        valid = np.isfinite(ratio)
        if not np.any(valid):
            print(f"Skipping empty ratio: {h_num_name} / {h_den_name}")
            continue

        ax.errorbar(bin_centers[valid], ratio[valid], xerr=bin_half_widths[valid], yerr=uncertainty[valid],
            color=color, marker="o", markersize=5, linestyle="-", linewidth=1, capsize=2, label=path)

    ax.set_xlabel(VARIABLE_LABELS[f"{domain}Tau{tau_index}_{variable}"])
    ax.set_ylabel(f"{quantity}")
    ax.set_xlim(den_edges[0], den_edges[-1])
    ax.set_ylim(0.0, 1.0)
    ax.grid(True, alpha=0.25)
    ax.legend(loc="upper left", fontsize=12, alignment="left")
    # The three gen curves have different thresholds, so list their exact
    # denominator definitions rather than using a separate generic cut box.
    if domain == "gen":
        ratio_description = f"Den: {generic_event_label(den_name, domain, variable, tau_index, path)}"\
        f"\nNum: {generic_event_label(num_name, domain, variable, tau_index, path)}"
    else:
        ratio_description = (
            f"Den: {EVENTS_LABELS[den_name]}\nNum: {EVENTS_LABELS[num_name]}"
        )
    ax.text(
        0.03, 0.03, ratio_description,
        transform=ax.transAxes, ha="left", va="bottom", fontsize=12,
        bbox=dict(facecolor="white", alpha=0.85, edgecolor="none"),
    )
    label = hep.cms.label("", data=False, com=14, ax=ax, loc=0)

    ratio_outdir = f'{outdir}/{quantity.replace(" ", "")}'
    os.system(f"mkdir -p {ratio_outdir}")
    out_plot = f"{domain}Tau{tau_index}_{variable}"
    save_figure(fig, ratio_outdir, out_plot, args.formats)

def plot_2d_ratio(root_file, outdir, args, domain, variable, den_name, num_name, quantity):

    for path, trigger_label, _ in TRIGGERS:
        path_skimmed = path.split("HLT_")[-1]
        h_den_name = f"{domain}Tau1_{variable}_vs_{domain}Tau2_{variable}_{den_name}"
        if histogram_is_path_specific(domain, den_name):
            h_den_name = f"{h_den_name}_{path_skimmed}"
        h_num_name = f"{domain}Tau1_{variable}_vs_{domain}Tau2_{variable}_{num_name}"
        if histogram_is_path_specific(domain, num_name):
            h_num_name = f"{h_num_name}_{path_skimmed}"

        den_counts, den_x_edges, den_y_edges = histogram_values(root_file, h_den_name)
        num_counts, num_x_edges, num_y_edges = histogram_values(root_file, h_num_name)
        if not (np.array_equal(den_x_edges, num_x_edges) and np.array_equal(den_y_edges, num_y_edges)):
            raise RuntimeError(
                f"Histogram binning mismatch: {h_num_name} / {h_den_name}"
            )

        ratio, _ = checked_ratio(num_counts, den_counts, h_num_name, h_den_name)
        masked_ratio = np.ma.masked_invalid(ratio)
        cmap = matplotlib.colormaps["viridis"].copy()
        # cmap.set_bad(color="white")

        fig, ax = plt.subplots(figsize=(10, 10))
        image = ax.pcolormesh(den_x_edges, den_y_edges, masked_ratio.T, cmap=cmap, vmin=0.0, vmax=1.0, shading="flat")
        colorbar = fig.colorbar(image, ax=ax, pad=0.02)
        colorbar.set_label(quantity)

        ax.set_xlabel(VARIABLE_LABELS[f"{domain}Tau1_{variable}"])
        ax.set_ylabel(VARIABLE_LABELS[f"{domain}Tau2_{variable}"])
        ax.set_xlim(den_x_edges[0], den_x_edges[-1])
        ax.set_ylim(den_y_edges[0], den_y_edges[-1])
        denominator_label = event_label(
            den_name, domain, variable, None, path
        )
        numerator_label = event_label(
            num_name, domain, variable, None, path
        )
        ax.text(0.03, 0.97, (f"{path}\nDen: {denominator_label}\nNum: {numerator_label}"),
            transform=ax.transAxes, ha="left", va="top", fontsize=12, color='black',
            bbox=dict(facecolor="white", alpha=0.85, edgecolor="none"),)
        hep.cms.label("", data=False, com=14, ax=ax, loc=0)

        ratio_outdir = os.path.join(outdir, quantity.replace(" ", ""), "2D")
        os.makedirs(ratio_outdir, exist_ok=True)
        out_plot = (f"{domain}Tau1_{variable}_vs_{domain}Tau2_{variable}_{path}")
        save_figure(fig, ratio_outdir, out_plot, args.formats)

def plot_gen_histos(root_file, outdir, args):
    for variable in ("pt", "eta"):
        for tau_index in (1, 2):
            plot_1d_histos(root_file, outdir, args, "gen",  GEN_EVENT_DEN + GEN_EVENT_NUM, variable, tau_index)
        for name in GEN_EVENT_DEN + GEN_EVENT_NUM:
            plot_2d_histo(root_file, outdir, args, "gen", name, variable)

def plot_reco_histos(root_file, outdir, args):
    for variable in ("pt", "eta"):
        for tau_index in (1, 2):
            plot_1d_histos(root_file, outdir, args, "reco",  RECO_EVENT, variable, tau_index)
        for name in RECO_EVENT:
            plot_2d_histo(root_file, outdir, args, "reco", name, variable)

def plot_efficiency(root_file, outdir, args):
    den_name = "ev_2tauh_acc_L1"
    num_name = "ev_2tauh_acc_L1_HLT_match"
    for variable in ("pt", "eta"):
        for tau_index in (1, 2):
            plot_1d_ratio(root_file, outdir, args, "gen", variable, tau_index, den_name, num_name, "HLT Efficiency")
        plot_2d_ratio(root_file, outdir, args, "gen", variable, den_name, num_name, "HLT Efficiency")
    den_name = "ev_2tauh_acc"
    num_name = "ev_2tauh_acc_L1_HLT_match"
    for variable in ("pt", "eta"):
        for tau_index in (1, 2):
            plot_1d_ratio(root_file, outdir, args, "gen", variable, tau_index, den_name, num_name, "L1 HLT Efficiency")
        plot_2d_ratio(root_file, outdir, args, "gen", variable, den_name, num_name, "L1 HLT Efficiency")

def plot_purity(root_file, outdir, args):
    den_name = "ev_L1_HLT"
    num_name = "ev_L1_HLT_match"
    for variable in ("pt", "eta"):
        for tau_index in (1, 2):
            plot_1d_ratio(root_file, outdir, args, "reco", variable, tau_index, den_name, num_name, "HLT Purity")
        plot_2d_ratio(root_file, outdir, args, "reco", variable, den_name, num_name, "HLT Purity")

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default="TauPerformancePlots/histograms.root", help="ROOT file from run_efficiencies.py")
    parser.add_argument("--outdir", default="TauPerformancePlots", help="Directory for output plots")
    parser.add_argument("--formats", nargs="+", default=("png","pdf"), choices=("png", "pdf"), help="one or more output formats")
    args =  parser.parse_args()

    if not os.path.isfile(args.root):
        print("Error: ROOT file does not exist: {}".format(args.root))
        sys.exit()

    os.makedirs(args.outdir, exist_ok=True)

    with uproot.open(args.root) as root_file:
        plot_gen_histos(root_file, args.outdir, args)
        plot_reco_histos(root_file, args.outdir, args)
        plot_efficiency(root_file, args.outdir, args)
        plot_purity(root_file, args.outdir, args)

    print("All plots saved in {}".format(args.outdir))
