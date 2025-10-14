#!/usr/bin/env python3

import os
import argparse
import numpy as np
import hist
import matplotlib.pyplot as plt
from matplotlib.transforms import blended_transform_factory
plt.style.use('tableau-colorblind10')
import array
import ROOT
import mplhep as hep
hep.style.use("CMS")

import warnings
warnings.filterwarnings("ignore", message="The value of the smallest subnormal")

class dotdict(dict):
    """dot.notation access to dictionary attributes"""
    __getattr__ = dict.get
    __setattr__ = dict.__setitem__
    __delattr__ = dict.__delitem__

def createDir(adir):
    if not os.path.exists(adir):
        os.makedirs(adir)
    return adir

def checkRootDir(afile, adir):
    if not afile.Get(adir):
        raise RuntimeError(f"Directory '{adir}' not found in {afile}")

def checkRootFile(afile, hname, rebin=None):
    hist_orig = afile.Get(hname)
    if not hist_orig:
        raise RuntimeError(f"WARNING: Histogram {hname} not found.")

    hist = hist_orig.Clone(hname + "_clone")
    hist.SetDirectory(0) # detach from file

    if rebin is not None:
        if isinstance(rebin, (int, float)):
            hist = hist.Rebin(int(rebin), hname + "_rebin")
        elif hasattr(rebin, '__iter__'):
            bin_edges_c = array.array('d', rebin)
            hist = hist.Rebin(len(bin_edges_c) - 1, hname + "_rebin", bin_edges_c)
        else:
            raise ValueError(f"Unknown type for rebin: {type(rebin)}")

    return hist

def define_bins(h):
    """
    Computes the number of bins, edges, centers and widths of a histogram.
    """
    N = h.GetNbinsX()
    edges = np.array([h.GetBinLowEdge(i+1) for i in range(N)])
    edges = np.append(edges, h.GetBinLowEdge(N+1))
    return N, edges, 0.5*(edges[:-1]+edges[1:]), np.diff(edges)
            
def histo_values_errors(h):
    N = h.GetNbinsX()
    values = np.array([h.GetBinContent(i+1) for i in range(N)])
    errors = np.array([h.GetBinError(i+1) for i in range(N)])
    return values, errors

class Plotter:
    def __init__(self, label, fontsize=18, grid_color='grey'):
        self._fig, self._ax = plt.subplots(figsize=(10, 10))
        self.fontsize = fontsize
        
        hep.cms.text(' Phase-2 Simulation Preliminary', ax=self._ax, fontsize=fontsize)
        hep.cms.lumitext(label + " | 14 TeV", ax=self._ax, fontsize=fontsize)
        if grid_color:
            self._ax.grid(which='major', color=grid_color)
        
        self.extensions = ('png', 'pdf')

    @property
    def fig(self):
        return self._fig

    @property
    def ax(self):
        return self._ax

    def labels(self, x, y, legend_title=None, legend_loc='upper right'):
        self._ax.set_xlabel(x)
        self._ax.set_ylabel(y)
        if legend_title is not None:
            self._ax.legend(title=legend_title,
                            title_fontsize=self.fontsize, fontsize=self.fontsize,
                            loc=legend_loc)

    def limits(self, x=None, y=None, logY=False, logX=False):
        if x:
            self._ax.set_xlim(x)
        if y:
            self._ax.set_ylim(y)
        if logX:
            self._ax.set_xscale('log')
        if logY:
            self._ax.set_yscale('log')

    def limits_with_margin(self, values, errors):
        values[np.isinf(values) | np.isnan(values)] = 0.
        errors[np.isinf(errors) | np.isnan(errors)] = 0.
        
        up = values + errors
        do = values - errors
        diff_step = 0.05 * abs(up.max()-do.min())
        self.limits(y=(do.min() - diff_step, up.max() + 2*diff_step), logY=False)

    def save(self, name):
        for ext in self.extensions:
            print(" ### INFO: Saving " + name + '.' + ext)
            plt.savefig(name + '.' + ext)
        plt.close()

def plot1Dvars(afile, adir, avars, outdir, text, top_text=False):
    """
    Plots 1D distributions.
    The `avars` variables is a dictionary whose values are (xlabel, ylabel, rebin).
    """
    for var, (xlabel, ylabel, rebin) in avars.items():
        plotter = Plotter(args.sample_label)
        root_hist = checkRootFile(afile, f"{adir}/{var}", rebin=rebin)
        nbins, bin_edges, bin_centers, bin_widths = define_bins(root_hist)
        values, errors = histo_values_errors(root_hist)
        errors /= 2
        
        plotter.ax.errorbar(bin_centers, values, xerr=None, yerr=errors,
                            fmt='s', color='black', label=xlabel, **errorbar_kwargs)
        plotter.ax.stairs(values, bin_edges, color='black', linewidth=2, baseline=None)
        plotter.ax.text(0.03, 0.97, text, transform=plotter.ax.transAxes, fontsize=fontsize,
                        verticalalignment='top', horizontalalignment='left')

        print(adir, var)
        plotter.limits_with_margin(values, errors)
        plotter.labels(x=xlabel, y=ylabel)

        if top_text:
            plotter.ax.text(0.97, 0.97, root_hist.GetTitle().replace('ET', r'$E_T$'), transform=plotter.ax.transAxes, fontsize=fontsize,
                            verticalalignment='top', horizontalalignment='right')

        plotter.save( os.path.join(outdir, var) )

def plot1DFilesComparison(adir, avars, outdir, files, files_labels, text):
    """
    Plots 1D distributions.
    The `avars` variables is a dictionary whose values are (xlabel, ylabel, rebin).
    """
    outdir = os.path.join(outdir, 'ComparisonFiles_' + '_'.join(files_labels), text)
    createDir(outdir)

    for var, (xlabel, ylabel, rebin) in avars.items():
        plotter = Plotter(args.sample_label)

        amax, amin = float('-inf'), float('+inf')
        for afile, alabel in zip(files, files_labels):
            afile = ROOT.TFile.Open(afile)
            root_hist = checkRootFile(afile, f"{adir}/{text}/{var}", rebin=rebin)
            nbins, bin_edges, bin_centers, bin_widths = define_bins(root_hist)
            values, errors = histo_values_errors(root_hist)
            errors /= 2 # symmetrize

            patch = plotter.ax.stairs(values, bin_edges, linewidth=2, baseline=None)
            plotter.ax.errorbar(bin_centers, values, xerr=None, yerr=errors,
                                fmt='s', label=alabel, color=patch.get_edgecolor(),
                                **errorbar_kwargs)
            
            amax = max(amax, (values + errors).max())
            amin = min(amin, (values - errors).min())

        diff_step = 0.05 * abs(amax-amin)
        plotter.limits(y=(amin - diff_step, amax + 2*diff_step), logY=False)
        plotter.labels(x=xlabel, y=ylabel)
        plotter.ax.legend(prop={'size': 15})
        plotter.save( os.path.join(outdir, var) )

if __name__ == '__main__':

    def check_list_length(value):
        if len(value) <= 1:
            raise argparse.ArgumentTypeError("List must have more than one item")
        return value

    class DependencyAction(argparse.Action):
        def __call__(self, parser, namespace, values, option_string=None):
            if option_string == "--compare_files_labels" and not namespace.compare_files:
                parser.error("`--compare_files_labels` requires `--compare_files` to be set")
            if option_string == "--compare_files_labels" and not namespace.met:
                parser.error("`--compare_files_labels` requires `--met` to be set")
            if len(namespace.met) > 1:
                parser.error("When comparing files only one MET collection can be used.")
            if len(values) != len(namespace.compare_files):
                parser.error("`--compare_files_labels` must have the same size as `--compare_files`")
            setattr(namespace, self.dest, values)
        
    full_command = 'python3 Validation/RecoParticleFlow/scripts/makeHLTPFValidationPlots.py --odir <your_dir> -l TTbar -f <your_ROOT_file>'
    parser = argparse.ArgumentParser(description='Make HLT PF validation plots. \nExample command:\n' + full_command)
    parser.add_argument('-o', '--odir', default="HLTPFValidationPlots", help='Path to the output directory.')
    parser.add_argument('-l', '--sample_label', default="QCD (200 PU)", help='Sample label for plotting.')

    mutual_excl2 = parser.add_mutually_exclusive_group(required=True)
    mutual_excl2.add_argument('-f', '--file', help='Paths to the DQM ROOT file.')
    mutual_excl2.add_argument('-x', '--compare_files', nargs='+', type=check_list_length,
                              help='Compare the same collection in different DQM files.', )
    parser.add_argument('-y', '--compare_files_labels', nargs='+',
                        action=DependencyAction, help='Compare the same collection in different DQM files.',)
    
    args = parser.parse_args()

    createDir(args.odir)
    
    fontsize = 16    
    colors = hep.style.CMS['axes.prop_cycle'].by_key()['color']
    markers = ('o', 's', 'd')
    errorbar_kwargs = dict(capsize=3, elinewidth=0.8, capthick=2, linewidth=2, linestyle='')

    nEventsLabel = '# Events'
    effLabel = 'Efficiency'
    vars1D = {
        # PF tester producer
        **{x + 'ClustersEnergy': ('Energy [GeV]', nEventsLabel, None) for x in ('Reco', 'Sim')},
        **{x + 'ClustersPt': (r'$p_{T}$ [GeV]', nEventsLabel, None) for x in ('Reco', 'Sim')},
        **{x + 'ClustersEta': (r'$\eta$', nEventsLabel, None) for x in ('Reco', 'Sim')},
        **{x + 'ClustersPhi': (r'$\phi$', nEventsLabel, None) for x in ('Reco', 'Sim')},
        **{x + 'ClustersMult': ('Multiplicity', nEventsLabel, None) for x in ('Reco', 'Sim')},
        # PF post-processing
        'Eff_vs_Energy': ('Energy [GeV]', effLabel, None),
        'Eff_vs_Pt': (r'$p_{T}$ [GeV]', effLabel, None),
        'Eff_vs_Eta': (r'$\eta$', effLabel, None),
        'Eff_vs_Phi': (r'$\phi$', effLabel, None),
        'Eff_vs_Mult': ('Multiplicity', effLabel, None),
    }

    dqm_dir = f"DQMData/Run 1/HLT/Run summary/ParticleFlow/PFClusterValidation"
    if args.compare_files is not None:
        for afile, alabel in zip(args.compare_files, args.compare_files_labels):
            afile = ROOT.TFile.Open(afile)
            checkRootDir(afile, dqm_dir)
        plot1DFilesComparison(dqm_dir, vars1D, outdir=args.odir, text='',
                              files=args.compare_files,
                              files_labels=args.compare_files_labels)

    else:
        afile = ROOT.TFile.Open(args.file)

        # Plot 1D PF variables
        checkRootDir(afile, dqm_dir)
        plot1Dvars(afile, dqm_dir, vars1D, outdir=args.odir, text='')
