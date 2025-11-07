import os
import argparse
import uproot
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.collections as coll
from matplotlib.ticker import MultipleLocator
import matplotlib.cm as cm
import matplotlib.colors as mcolors
import mplhep as hep
import pandas as pd
hep.style.use("CMS")
from dataclasses import dataclass
    
def plotGeom(df):
    fig, ax = plt.subplots()
    params = dict(ax=ax, fontsize=15)
    hep.cms.text(' Phase-2 Simulation Preliminary', **params)
    hep.cms.lumitext("ECAL Geometry", **params)

    ax.scatter(df.crystalCenterEta, df.crystalCenterPhi, c='red', s=1)

    pat = []
    for row in df.itertuples(index=False):
        sq = patches.Rectangle((row.crystalCorner2Eta, row.crystalCorner2Phi),
                               abs(row.crystalCorner2Eta-row.crystalCorner0Eta),
                               abs(row.crystalCorner2Phi-row.crystalCorner0Phi),
                               edgecolor='black', fill=False)
        pat.append(sq)
    ax.add_collection(coll.PatchCollection(pat, facecolor='none', edgecolor='black'))
    
    ax.scatter(df.crystalCenterEta, df.crystalCenterPhi, c='red', s=1)

    # ax.xaxis.set_major_locator(MultipleLocator(0.01))
    # ax.xaxis.set_minor_locator(MultipleLocator(0.05))
    # ax.yaxis.set_major_locator(MultipleLocator(0.02))
    # ax.yaxis.set_minor_locator(MultipleLocator(0.005))
    
    plt.xlabel(f'$\eta$')
    plt.ylabel(f'$\phi$')
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    fig.savefig('test.pdf')

def plotEvent(event, geom, out, zoom):
    """
    Plot single event on top of the geometry.
    """
    df_merged = pd.merge(event, geom, how='inner', left_on='detid', right_on='crystalDetId')
    df_merged = df_merged[df_merged.energy>0]
    
    fig, ax = plt.subplots()
    params = dict(ax=ax, fontsize=15)
    hep.cms.text(' Phase-2 Simulation Preliminary', **params)
    hep.cms.lumitext("CloseByElectron | 14 TeV", **params)

    cmap = cm.viridis
    norm = mcolors.Normalize(vmin=df_merged.energy.min(), vmax=df_merged.energy.max())

    # Add rectangles
    pat = []
    for row in df_merged.itertuples(index=False):
        sq = patches.Rectangle((row.crystalCorner2Eta, row.crystalCorner2Phi),
                               abs(row.crystalCorner2Eta-row.crystalCorner0Eta),
                               abs(row.crystalCorner2Phi-row.crystalCorner0Phi))
        pat.append(sq)

    colors = cmap(norm(df_merged.energy.values))
    ax.add_collection(coll.PatchCollection(pat, facecolor=colors, edgecolor='black'))

    # Add colorbar
    sm = cm.ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])
    fig.colorbar(sm, ax=ax, label='PF RecHit Energy [GeV]')

    plt.xlabel(f'$\eta$')
    plt.ylabel(f'$\phi$')
    if zoom:
        ax.set_xlim(0.1, 0.4)
        ax.set_ylim(0.1, 0.4)
    else:
        ax.set_xlim(-1.5, 1.5)
        ax.set_ylim(-3.15, 3.15)
        
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    
    for ext in ('.pdf', '.png'):
        fig.savefig(out + ext)
    
def showECAL(infile, indir, outfile, props):
    varsGeom = ['crystalDetId', 'crystalCenterEta', 'crystalCenterPhi',
                'crystalCorner0Eta', 'crystalCorner1Eta', 'crystalCorner2Eta', 'crystalCorner3Eta',
                'crystalCorner0Phi', 'crystalCorner1Phi', 'crystalCorner2Phi', 'crystalCorner3Phi']

    varsEventCommon = ['eventId']
    varsEvent = {"Reco": [], "Sim": []}
    for prefix in ("Reco", "Sim"):
        varsEvent[prefix].extend([x+prefix for x in ('energies', 'detids', 'nHits')])
    varsEventAll = varsEventCommon + varsEvent['Reco'] + varsEvent['Sim']
        
    with uproot.open(infile) as file:
        dfGeom = file['ecalGeometryAnalyzer/Geometry'].arrays(varsGeom, library="pandas")
        dfEvent = file['ecalGeometryAnalyzer/Event'].arrays(varsEventAll, library="awkward")

    # filters
    # if zoom:
    #     filt = lambda x : ((x.crystalCenterEta > 0.18) & (x.crystalCenterEta < 0.3)
    #                        & (x.crystalCenterPhi > 0.1) & (x.crystalCenterPhi < 0.3))
    # else:
    #     filt = lambda x : x

    # dfGeom = dfGeom[filt(dfGeom)]
    plotGeom(dfGeom)

    for t in range(1, props.nevents+1):
        print(f'INFO: Processing event {t}...')

        for prefix in ("Reco", "Sim"):
            outname = "event" + prefix + str(t)
            if props.zoom:
                outname += "_zoom"

            dfEventTmp = dfEvent[dfEvent.eventId==t]
            if prefix == "Sim": # energy cut to reduce hit multiplicity in plot
                en_mask = dfEventTmp.energiesSim > 0.2
                dfEventTmp['energies'+prefix] = dfEventTmp['energies'+prefix][en_mask]
                dfEventTmp['detids'+prefix] = dfEventTmp['detids'+prefix][en_mask]

            dfEventTmp = pd.DataFrame({'energy': dfEventTmp['energies'+prefix][0],
                                       'detid': dfEventTmp['detids'+prefix][0]})

            plotEvent(dfEventTmp, dfGeom, out=os.path.join(outfile,outname), zoom=props.zoom)
        
    print('INFO: Done.')

    
@dataclass
class InputArgs:
    zoom: bool
    nevents: int
    
if __name__ == '__main__':

    full_command = 'python3 Validation/RecoParticleFlow/scripts/showECALcrystals.py --file <input_root_file>'
    parser = argparse.ArgumentParser(description='Show position of crystals. \nExample command:\n' + full_command)

    parser.add_argument('-i', '--file', help='Path to the input ROOT file.')
    parser.add_argument('-o', '--outdir', help='Path to the output folder where the events will be stored.')
    parser.add_argument('-z', '--zoom', help='Zoom over a hard-coded eta/phi region.', action='store_true')
    parser.add_argument('-n', '--nevents', help='Number of events to plot.', default=6)
    
    args = parser.parse_args()
    props = InputArgs(zoom=args.zoom, nevents=args.nevents)
    
    showECAL(args.file, f"ecalGeometryAnalyzer/Geometry", args.outdir, props)






# from scipy.spatial import ConvexHull
# def plot_convex_hull(points, ax, **kwargs):
#     hull = ConvexHull(points)
#     for simplex in hull.simplices:
#         ax.plot(points[simplex, 0], points[simplex, 1], **kwargs)

# fig, ax = plt.subplots(figsize=(10, 8))
# h = ax.hist2d(all_hits[:, 0], all_hits[:, 1], bins=20, weights=all_energies, cmap='viridis')
# plt.colorbar(h[3], label='Energy')

# plot_convex_hull(cluster1, ax, color='red', linestyle='--', label='Cluster 1')
# plot_convex_hull(cluster2, ax, color='blue', linestyle='--', label='Cluster 2')

# ax.legend()
# ax.set_title('Energy Hits with Cluster Boundaries')
# ax.set_xlabel('X position')
# ax.set_ylabel('Y position')

# OR

# from scipy.stats import gaussian_kde

# def plot_cluster_contour(points, ax, **kwargs):
#     xy = np.vstack([points[:, 0], points[:, 1]])
#     kde = gaussian_kde(xy)
#     xgrid, ygrid = np.mgrid[points[:, 0].min():points[:, 0].max():100j,
#                             points[:, 1].min():points[:, 1].max():100j]
#     z = kde(np.vstack([xgrid.ravel(), ygrid.ravel()]))
#     ax.contour(xgrid, ygrid, z.reshape(xgrid.shape), **kwargs)

# fig, ax = plt.subplots(figsize=(10, 8))
# h = ax.hist2d(all_hits[:, 0], all_hits[:, 1], bins=20, weights=all_energies, cmap='viridis')
# plt.colorbar(h[3], label='Energy')

# plot_cluster_contour(cluster1, ax, colors='red', linestyles='--', levels=1, label='Cluster 1')
# plot_cluster_contour(cluster2, ax, colors='blue', linestyles='--', levels=1, label='Cluster 2')

# ax.legend()
# ax.set_title('Energy Hits with Cluster Contours')
# ax.set_xlabel('X position')
# ax.set_ylabel('Y position')
