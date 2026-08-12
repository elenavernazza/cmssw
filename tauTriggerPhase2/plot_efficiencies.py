#!/usr/bin/env python3

import argparse
import os
import sys

import ROOT


TRIGGER_PATHS = [
    "HLT_DoubleMediumChargedIsoPFTauHPS40_eta2p1",
    "HLT_DoubleMediumDeepTauPFTauHPS35_eta2p1",
    "HLT_DoubleMediumPFPuppiParTTauh30_eta2p1",
]

COLOR_MAP = {
    "HLT_DoubleMediumChargedIsoPFTauHPS40_eta2p1": ROOT.kRed,
    "HLT_DoubleMediumDeepTauPFTauHPS35_eta2p1": ROOT.kGreen + 2,
    "HLT_DoubleMediumPFPuppiParTTauh30_eta2p1": ROOT.kOrange + 7,
}

CUT_LEVELS = [
    ("", "baseline"),
    ("_withSingleCuts", "one_tau_orthogonal"),
    ("_withAllCuts", "both_tau_nminus1"),
]

VARIABLES = ("pt", "eta")
TAU_ROLES = ("leading", "subleading")
GEN_STATES = (
    ("all_gen", "All gen taus"),
    ("matched_gen", "Matched gen taus"),
    ("matched_gen_both", "Both gen taus matched"),
)
FILTER_STATES = (
    ("all_filterobj", "All filter objects", ROOT.kBlack),
    ("matched_filterobj", "Gen-matched filter objects", ROOT.kGreen + 2),
)


def load_histogram(root_file, histogram_name):
    histogram = root_file.Get(histogram_name)
    if not histogram:
        print("Missing histogram: {}".format(histogram_name))
        return None
    histogram.SetDirectory(0)
    histogram.SetStats(False)
    return histogram


def create_efficiency(numerator, denominator, color):
    if not ROOT.TEfficiency.CheckConsistency(numerator, denominator):
        print(
            "Inconsistent efficiency histograms: {} / {}".format(
                numerator.GetName(), denominator.GetName()
            )
        )
        return None

    efficiency = ROOT.TEfficiency(numerator, denominator)
    efficiency.SetLineColor(color)
    efficiency.SetLineWidth(2)
    efficiency.SetMarkerColor(color)
    efficiency.SetMarkerStyle(20)
    return efficiency


def setup_canvas(name):
    canvas = ROOT.TCanvas(name, name, 800, 600)
    canvas.SetGridx()
    canvas.SetGridy()
    return canvas


def setup_legend():
    legend = ROOT.TLegend(0.12, 0.69, 0.94, 0.9)
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    legend.SetTextSize(0.022)
    return legend


def draw_efficiency_frame(canvas, variable, y_label):
    if variable == "pt":
        frame = canvas.DrawFrame(0, 0, 200, 1.2)
        frame.GetXaxis().SetTitle("p_{T} [GeV]")
    else:
        frame = canvas.DrawFrame(-2.5, 0, 2.5, 1.2)
        frame.GetXaxis().SetTitle("#eta")
    frame.GetYaxis().SetTitle(y_label)
    return frame


def set_distribution_axes(histogram, variable):
    histogram.GetXaxis().SetTitle("p_{T} [GeV]" if variable == "pt" else "#eta")
    histogram.GetYaxis().SetTitle("Entries")


def save_canvas(canvas, output_dir, filename):
    output_path = os.path.join(output_dir, filename)
    canvas.SaveAs(output_path)
    canvas.Close()
    print("Saved plot: {}".format(output_path))


def plot_gen_efficiencies(root_file, output_dir):
    efficiency_types = (
        ("matched_gen", "matched_gen"),
        ("matched_gen_both", "matched_gen_both"),
    )

    for variable in VARIABLES:
        for role in TAU_ROLES:
            for _, numerator_state in efficiency_types:
                for cut_suffix, _ in CUT_LEVELS:
                    canvas_name = "{}_{}_efficiency_{}{}".format(
                        numerator_state, role, variable, cut_suffix
                    )
                    canvas = setup_canvas(canvas_name)
                    draw_efficiency_frame(canvas, variable, "Efficiency")
                    legend = setup_legend()
                    efficiencies = []

                    for path in TRIGGER_PATHS:
                        numerator_name = "{}_{}_{}_{}{}".format(
                            variable, role, numerator_state, path, cut_suffix
                        )
                        denominator_name = "{}_{}_all_gen_{}{}".format(
                            variable, role, path, cut_suffix
                        )
                        numerator = load_histogram(root_file, numerator_name)
                        denominator = load_histogram(root_file, denominator_name)
                        if numerator is None or denominator is None:
                            continue

                        efficiency = create_efficiency(
                            numerator, denominator, COLOR_MAP.get(path, ROOT.kBlack)
                        )
                        if efficiency is None:
                            continue

                        efficiencies.append(efficiency)
                        efficiency.Draw("P SAME")
                        legend.AddEntry(efficiency, path, "lep")

                    if efficiencies:
                        legend.Draw()
                        save_canvas(canvas, output_dir, "{}.png".format(canvas_name))
                    else:
                        canvas.Close()


def plot_gen_distributions(root_file, output_dir):
    role_colors = {"leading": ROOT.kRed, "subleading": ROOT.kBlue}

    for path in TRIGGER_PATHS:
        for cut_suffix, _ in CUT_LEVELS:
            for state, state_label in GEN_STATES:
                canvas_name = "{}_{}_pt_distribution{}".format(path, state, cut_suffix)
                canvas = setup_canvas(canvas_name)
                legend = setup_legend()
                plotted_histograms = []

                for role in TAU_ROLES:
                    histogram_name = "pt_{}_{}_{}{}".format(
                        role, state, path, cut_suffix
                    )
                    histogram = load_histogram(root_file, histogram_name)
                    if histogram is None:
                        continue

                    color = role_colors[role]
                    histogram.SetLineColor(color)
                    histogram.SetLineWidth(2)
                    set_distribution_axes(histogram, "pt")
                    histogram.Draw("HIST" if not plotted_histograms else "HIST SAME")
                    plotted_histograms.append(histogram)
                    legend.AddEntry(
                        histogram,
                        "{}: {}".format(role.capitalize(), state_label),
                        "l",
                    )

                if plotted_histograms:
                    legend.Draw()
                    save_canvas(canvas, output_dir, "z_{}.png".format(canvas_name))
                else:
                    canvas.Close()


def plot_filter_distributions(root_file, output_dir):
    for path in TRIGGER_PATHS:
        for cut_suffix, _ in CUT_LEVELS:
            for state, label, color in FILTER_STATES:
                canvas_name = "{}_{}_pt_distribution{}".format(
                    path, state, cut_suffix
                )
                canvas = setup_canvas(canvas_name)
                legend = setup_legend()
                histogram_name = "pt_{}_{}{}".format(state, path, cut_suffix)
                histogram = load_histogram(root_file, histogram_name)
                if histogram is None:
                    canvas.Close()
                    continue

                histogram.SetLineColor(color)
                histogram.SetLineWidth(2)
                set_distribution_axes(histogram, "pt")
                histogram.Draw("HIST")
                legend.AddEntry(histogram, label, "l")
                legend.Draw()
                save_canvas(canvas, output_dir, "z_{}.png".format(canvas_name))


def plot_fake_rates(root_file, output_dir):
    for variable in VARIABLES:
        canvas_name = "fake_efficiency_{}".format(variable)
        canvas = setup_canvas(canvas_name)
        draw_efficiency_frame(canvas, variable, "Fake rate")
        legend = setup_legend()
        efficiencies = []

        for path in TRIGGER_PATHS:
            numerator = load_histogram(
                root_file, "{}_fake_filterobj_{}".format(variable, path)
            )
            denominator = load_histogram(
                root_file, "{}_all_filterobj_{}".format(variable, path)
            )
            if numerator is None or denominator is None:
                continue

            efficiency = create_efficiency(
                numerator, denominator, COLOR_MAP.get(path, ROOT.kBlack)
            )
            if efficiency is None:
                continue

            efficiencies.append(efficiency)
            efficiency.Draw("P SAME")
            legend.AddEntry(efficiency, path, "lep")

        if efficiencies:
            legend.Draw()
            save_canvas(canvas, output_dir, "{}.png".format(canvas_name))
        else:
            canvas.Close()


def plot_response_histograms(root_file, output_dir):
    for path in TRIGGER_PATHS:
        for cut_suffix, _ in CUT_LEVELS:
            for role in TAU_ROLES:
                histogram_name = "pt_gen_vs_reco_matched_{}_{}{}".format(
                    role, path, cut_suffix
                )
                histogram = load_histogram(root_file, histogram_name)
                if histogram is None:
                    continue

                canvas = setup_canvas(histogram_name)
                histogram.GetXaxis().SetTitle("Gen p_{T} [GeV]")
                histogram.GetYaxis().SetTitle("Filter-object p_{T} [GeV]")
                histogram.Draw("COLZ")

                diagonal = ROOT.TLine(
                    histogram.GetXaxis().GetXmin(),
                    histogram.GetYaxis().GetXmin(),
                    histogram.GetXaxis().GetXmax(),
                    histogram.GetYaxis().GetXmax(),
                )
                diagonal.SetLineColor(ROOT.kRed)
                diagonal.SetLineWidth(2)
                diagonal.Draw("SAME")
                save_canvas(canvas, output_dir, "{}.png".format(histogram_name))


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("root_file", help="ROOT file produced by main.py")
    parser.add_argument(
        "--distributions",
        action="store_true",
        help="also create the optional basic gen and filter-object distributions",
    )
    return parser.parse_args()


def main():
    args = parse_arguments()
    root_file_path = args.root_file
    if not os.path.isfile(root_file_path):
        print("Error: file does not exist: {}".format(root_file_path))
        sys.exit(1)

    ROOT.gROOT.SetBatch(True)
    ROOT.gStyle.SetOptTitle(0)
    ROOT.gStyle.SetOptStat(0)

    root_file = ROOT.TFile.Open(root_file_path, "READ")
    if not root_file or root_file.IsZombie():
        print("Error: cannot open ROOT file: {}".format(root_file_path))
        sys.exit(1)

    output_dir = "efficiency_plots"
    os.makedirs(output_dir, exist_ok=True)

    plot_gen_efficiencies(root_file, output_dir)
    if args.distributions:
        plot_gen_distributions(root_file, output_dir)
        plot_filter_distributions(root_file, output_dir)
    plot_fake_rates(root_file, output_dir)
    plot_response_histograms(root_file, output_dir)

    root_file.Close()
    print("All plots have been saved in '{}'.".format(output_dir))


if __name__ == "__main__":
    main()
