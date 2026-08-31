#!/usr/bin/env python3

import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import mplhep as hep
import numpy as np
import ROOT


ROOT.gROOT.SetBatch(True)
plt.style.use(hep.cms.style.CMS)


CURVE_STYLES = {
    "part": {
        "default_label": "ParT",
        "tree": "partRaw",
        "color": "black",
        "linestyle": "-",
    },
    "part2": {
        "default_label": "ParT (second sample)",
        "tree": "partRaw",
        "color": "tab:red",
        "linestyle": "-",
    },
    "deeptau": {
        "default_label": "DeepTau",
        "tree": "dtRaw",
        "color": "tab:blue",
        "linestyle": "--",
    },
}


def load_tree(filename, tree_name, pt_min, pt_max, eta_max):
    tree_path = f"tauROC/{tree_name}"
    root_file = ROOT.TFile.Open(filename)
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"Cannot open '{filename}'")
    if not root_file.Get(tree_path):
        root_file.Close()
        raise RuntimeError(f"Tree '{tree_path}' is missing from '{filename}'")
    root_file.Close()

    selection = (
        f"pt >= {pt_min} && pt <= {pt_max} && "
        f"abs(eta) <= {eta_max}"
    )
    arrays = (
        ROOT.RDataFrame(tree_path, filename)
        .Filter(selection)
        .AsNumpy(["score", "label", "pt", "eta"])
    )
    data = {
        "score": np.asarray(arrays["score"], dtype=float),
        "label": np.asarray(arrays["label"], dtype=int),
        "pt": np.asarray(arrays["pt"], dtype=float),
        "eta": np.asarray(arrays["eta"], dtype=float),
    }
    if data["score"].size == 0:
        raise RuntimeError(
            f"No entries in '{tree_path}' from '{filename}' pass {selection}"
        )
    if not np.all((data["label"] == 0) | (data["label"] == 1)):
        raise ValueError(f"Tree '{tree_path}' contains labels other than 0 and 1")

    print(
        f"[LOAD] {tree_path} from {filename}: N={data['score'].size}, "
        f"signal={np.sum(data['label'] == 1)}, "
        f"background={np.sum(data['label'] == 0)}"
    )
    return data


def threshold_scan(scores, labels):
    scores = np.asarray(scores, dtype=float)
    labels = np.asarray(labels, dtype=int)
    valid = np.isfinite(scores)
    scores = scores[valid]
    labels = labels[valid]

    signal_count = int(np.sum(labels == 1))
    background_count = int(np.sum(labels == 0))
    if signal_count == 0 or background_count == 0:
        raise ValueError(
            "Cannot build ROC with "
            f"{signal_count} signal and {background_count} background entries"
        )

    order = np.argsort(-scores, kind="stable")
    sorted_scores = scores[order]
    sorted_labels = labels[order]
    cumulative_signal = np.cumsum(sorted_labels == 1)
    cumulative_background = np.cumsum(sorted_labels == 0)
    last_at_score = np.r_[
        np.flatnonzero(sorted_scores[1:] != sorted_scores[:-1]),
        sorted_scores.size - 1,
    ]
    score_levels = sorted_scores[last_at_score]

    thresholds = np.empty(score_levels.size + 1, dtype=float)
    thresholds[0] = score_levels[0]
    if score_levels.size > 1:
        thresholds[1:-1] = 0.5 * (
            score_levels[:-1] + score_levels[1:]
        )
    thresholds[-1] = np.nextafter(score_levels[-1], -np.inf)

    tpr = np.r_[0.0, cumulative_signal[last_at_score] / signal_count]
    fpr = np.r_[
        0.0,
        cumulative_background[last_at_score] / background_count,
    ]
    return thresholds, fpr, tpr


def auc(fpr, tpr):
    return float(np.trapz(tpr, fpr))


def operating_point(labels, passes):
    labels = np.asarray(labels, dtype=int)
    passes = np.asarray(passes, dtype=bool)
    signal = labels == 1
    background = labels == 0
    signal_count = int(np.sum(signal))
    background_count = int(np.sum(background))
    if signal_count == 0 or background_count == 0:
        raise ValueError("Working points require signal and background entries")

    true_positive = int(np.sum(passes & signal))
    false_positive = int(np.sum(passes & background))
    return {
        "tpr": true_positive / signal_count,
        "fpr": false_positive / background_count,
        "tp": true_positive,
        "fp": false_positive,
        "signal": signal_count,
        "background": background_count,
    }


def fixed_part_working_point(data, threshold):
    return operating_point(data["label"], data["score"] > threshold)


def deeptau_working_point(data):
    thresholds = np.interp(
        data["pt"],
        [35.0, 100.0, 300.0],
        [0.649, 0.441, 0.050],
    )
    passes = (
        (data["pt"] > 35.0)
        & (np.abs(data["eta"]) < 2.1)
        & (data["score"] > thresholds)
    )
    return operating_point(data["label"], passes)


def matched_part_working_point(data, target_efficiency, mode):
    thresholds, fpr, tpr = threshold_scan(data["score"], data["label"])
    if mode == "closest":
        index = int(np.argmin(np.abs(tpr - target_efficiency)))
    elif mode == "at_least":
        candidates = np.flatnonzero(tpr >= target_efficiency)
        index = int(candidates[0]) if candidates.size else len(tpr) - 1
    else:
        candidates = np.flatnonzero(tpr <= target_efficiency)
        index = int(candidates[-1]) if candidates.size else 0

    threshold = float(thresholds[index])
    point = fixed_part_working_point(data, threshold)
    if not np.isclose(point["tpr"], tpr[index]):
        raise RuntimeError("Matched ParT efficiency consistency check failed")
    if not np.isclose(point["fpr"], fpr[index]):
        raise RuntimeError("Matched ParT mistag consistency check failed")
    point["threshold"] = threshold
    return point


def print_working_point(label, description, point):
    rejection = np.inf if point["fpr"] == 0.0 else 1.0 / point["fpr"]
    print(f"\n[WP] {label}: {description}")
    print(
        f"     efficiency={point['tpr']:.6f} "
        f"({point['tp']}/{point['signal']})"
    )
    print(
        f"     mis-id={point['fpr']:.6f} "
        f"({point['fp']}/{point['background']}), "
        f"rejection={rejection:.3f}"
    )


def make_raw_distributions(data, labels, curves, output_path):
    """Plot normalized signal and background discriminator-score shapes."""
    categories = (
        (1, "Signal: gen-matched", "tab:blue"),
        (0, "Background: unmatched", "tab:orange"),
    )
    bins = np.linspace(0.0, 1.0, 51)
    figure_width = max(8.0, 7.0 * len(curves))
    fig, axes = plt.subplots(
        1,
        len(curves),
        figsize=(figure_width, 6),
        sharey=True,
        squeeze=False,
    )

    for axis, name in zip(axes[0], curves):
        scores = data[name]["score"]
        object_labels = data[name]["label"]

        for category, category_label, color in categories:
            selected_scores = scores[
                (object_labels == category) & np.isfinite(scores)
            ]
            if selected_scores.size == 0:
                raise ValueError(
                    f"Cannot plot {labels[name]}: no {category_label} entries"
                )

            # Normalize signal and background independently. This compares
            # their shapes without the much larger background yield hiding
            # the signal; the unnormalized counts remain in the legend.
            weights = np.full(
                selected_scores.size,
                1.0 / selected_scores.size,
            )
            axis.hist(
                selected_scores,
                bins=bins,
                weights=weights,
                histtype="step",
                linewidth=2.0,
                color=color,
                label=f"{category_label} (N={selected_scores.size:,})",
            )

        axis.set_xlabel(f"{labels[name]} score")
        axis.set_xlim(0.0, 1.0)
        axis.set_yscale("log")
        axis.minorticks_on()
        axis.grid(which="major", alpha=0.25)
        axis.grid(which="minor", alpha=0.10)
        axis.legend(frameon=False, fontsize=11)

    axes[0, 0].set_ylabel("Fraction of objects / 0.02")
    hep.cms.label(
        data=False,
        label="Preliminary",
        ax=axes[0, 0],
        loc=0,
        com=14,
    )
    fig.tight_layout()

    raw_output = output_path.with_name(
        f"{output_path.stem}_raw_distributions{output_path.suffix}"
    )
    raw_pdf = raw_output.with_suffix(".pdf")
    fig.savefig(raw_output, bbox_inches="tight")
    if raw_pdf != raw_output:
        fig.savefig(raw_pdf, bbox_inches="tight")
    plt.close(fig)

    print(f"Saved {raw_output}")
    if raw_pdf != raw_output:
        print(f"Saved {raw_pdf}")


def parse_arguments():
    parser = argparse.ArgumentParser(
        description=(
            "Plot native-object DeepTau and one or two ParT ROC curves "
            "with their working points."
        )
    )
    parser.add_argument(
        "input_file",
        help="Primary analyzer ROOT file (ParT and default DeepTau source)",
    )
    parser.add_argument(
        "--part2-file",
        help="Optional analyzer ROOT file supplying a second ParT curve",
    )
    parser.add_argument(
        "--deeptau-file",
        help="Optional DeepTau source; defaults to input_file",
    )
    parser.add_argument(
        "--curves",
        nargs="+",
        choices=tuple(CURVE_STYLES),
        help=(
            "Curves to draw. Defaults to part+deeptau, and automatically "
            "adds part2 when --part2-file is supplied."
        ),
    )
    parser.add_argument("--part-label", default="ParT")
    parser.add_argument("--part2-label", default="ParT (second sample)")
    parser.add_argument("--deeptau-label", default="DeepTau")
    parser.add_argument("--part-wp", type=float, default=0.2)
    parser.add_argument(
        "--part2-wp",
        type=float,
        help="Second ParT threshold; defaults to --part-wp",
    )
    parser.add_argument(
        "--match-mode",
        choices=("closest", "at_least", "at_most"),
        default="closest",
        help="How each displayed ParT curve is matched to the DeepTau WP",
    )
    parser.add_argument("--pt-min", type=float, default=30.0)
    parser.add_argument("--pt-max", type=float, default=1000.0)
    parser.add_argument("--eta-max", type=float, default=2.5)
    parser.add_argument("--xmin", type=float, default=0.4)
    parser.add_argument("--ymin", type=float, default=5e-3)
    parser.add_argument("-o", "--output", default="tau_roc.png")
    args = parser.parse_args()

    if args.curves is None:
        args.curves = ["part", "deeptau"]
        if args.part2_file:
            args.curves.insert(1, "part2")
    args.curves = list(dict.fromkeys(args.curves))

    if "part2" in args.curves and not args.part2_file:
        parser.error("--curves part2 requires --part2-file")
    if args.pt_min > args.pt_max:
        parser.error("--pt-min must not exceed --pt-max")
    if args.eta_max <= 0.0:
        parser.error("--eta-max must be positive")
    if args.ymin <= 0.0:
        parser.error("--ymin must be positive for the logarithmic axis")
    for option, value in (
        ("--part-wp", args.part_wp),
        ("--part2-wp", args.part2_wp),
    ):
        if value is not None and not 0.0 <= value <= 1.0:
            parser.error(f"{option} must lie between zero and one")
    return args


def main():
    args = parse_arguments()
    filenames = {
        "part": args.input_file,
        "part2": args.part2_file,
        "deeptau": args.deeptau_file or args.input_file,
    }
    labels = {
        "part": args.part_label,
        "part2": args.part2_label,
        "deeptau": args.deeptau_label,
    }
    data = {
        name: load_tree(
            filenames[name],
            CURVE_STYLES[name]["tree"],
            args.pt_min,
            args.pt_max,
            args.eta_max,
        )
        for name in args.curves
    }

    output_path = Path(args.output)
    if output_path.suffix == "":
        output_path = output_path.with_suffix(".png")
    make_raw_distributions(data, labels, args.curves, output_path)

    fig, axis = plt.subplots(figsize=(9, 7))
    for name in args.curves:
        _, fpr, tpr = threshold_scan(data[name]["score"], data[name]["label"])
        style = CURVE_STYLES[name]
        curve_auc = auc(fpr, tpr)
        axis.plot(
            tpr,
            np.clip(fpr, args.ymin, 1.0),
            color=style["color"],
            linestyle=style["linestyle"],
            linewidth=1.8,
            label=f"{labels[name]} (AUC={curve_auc:.3f})",
        )
        print(f"[ROC] {labels[name]}: AUC={curve_auc:.6f}")

    part_thresholds = {
        "part": args.part_wp,
        "part2": args.part2_wp if args.part2_wp is not None else args.part_wp,
    }
    for name in ("part", "part2"):
        if name not in data:
            continue
        threshold = part_thresholds[name]
        point = fixed_part_working_point(data[name], threshold)
        print_working_point(labels[name], f"score > {threshold:g}", point)
        axis.scatter(
            point["tpr"],
            max(point["fpr"], args.ymin),
            marker="o",
            s=58,
            facecolors="white",
            edgecolors=CURVE_STYLES[name]["color"],
            linewidths=1.8,
            zorder=10,
            label=f"{labels[name]} WP > {threshold:g}",
        )

    deep_point = None
    if "deeptau" in data:
        deep_point = deeptau_working_point(data["deeptau"])
        print_working_point(
            labels["deeptau"],
            "pT > 35, |eta| < 2.1, score > T(pT)",
            deep_point,
        )
        axis.scatter(
            deep_point["tpr"],
            max(deep_point["fpr"], args.ymin),
            marker="s",
            s=62,
            color=CURVE_STYLES["deeptau"]["color"],
            zorder=10,
            label=f"{labels['deeptau']} Medium WP",
        )

    if deep_point is not None:
        for name in ("part", "part2"):
            if name not in data:
                continue
            point = matched_part_working_point(
                data[name],
                deep_point["tpr"],
                args.match_mode,
            )
            print_working_point(
                labels[name],
                f"DeepTau-matched score > {point['threshold']:.10g}",
                point,
            )
            axis.scatter(
                point["tpr"],
                max(point["fpr"], args.ymin),
                marker="D",
                s=58,
                color=CURVE_STYLES[name]["color"],
                zorder=11,
                label=(
                    f"{labels[name]} DT-matched "
                    f"> {point['threshold']:.3g}"
                ),
            )

    axis.set_xlabel("Tau efficiency")
    axis.set_ylabel("Background misidentification rate")
    axis.set_yscale("log")
    axis.set_xlim(args.xmin, 1.0)
    axis.set_ylim(args.ymin, 1.0)
    axis.minorticks_on()
    axis.grid(which="major", alpha=0.25)
    axis.grid(which="minor", alpha=0.10)
    hep.cms.label(
        data=False,
        label="Preliminary",
        ax=axis,
        loc=0,
        com=14,
    )
    axis.legend(loc="upper left", frameon=False, fontsize=12)
    fig.tight_layout()

    pdf_path = output_path.with_suffix(".pdf")
    fig.savefig(output_path, bbox_inches="tight")
    if pdf_path != output_path:
        fig.savefig(pdf_path, bbox_inches="tight")
    plt.close(fig)

    print(f"\nSaved {output_path}")
    if pdf_path != output_path:
        print(f"Saved {pdf_path}")


if __name__ == "__main__":
    main()
