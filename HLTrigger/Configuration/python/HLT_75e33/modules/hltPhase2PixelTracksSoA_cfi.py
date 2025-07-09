import FWCore.ParameterSet.Config as cms

removeOT = False

hltPhase2PixelTracksSoA = cms.EDProducer('CAHitNtupletAlpakaPhase2OT@alpaka',
    pixelRecHitSrc = cms.InputTag('hltPhase2PixelRecHitsExtendedSoA'),
    ptmin = cms.double(0.9),
    hardCurvCut = cms.double(0.01425), # corresponds to 800 MeV in 3.8T.
    earlyFishbone = cms.bool(True),
    lateFishbone = cms.bool(False),
    fillStatistics = cms.bool(False),
    minHitsPerNtuplet = cms.uint32(5),
    maxNumberOfDoublets = cms.string(str(15*512*1024)),
    maxNumberOfTuples = cms.string(str(4*60*1024)),
    cellPtCut = cms.double(0.85), # Corresponds to 1 GeV * this cut, i.e., 850 MeV, as minimum p_t
    cellZ0Cut = cms.double(12.5), # it's half the BS width! It has nothing to do with the sample!!
    minYsizeB1 = cms.int32(20),
    minYsizeB2 = cms.int32(15),
    maxDYsize12 = cms.int32(12),
    maxDYsize = cms.int32(10),
    maxDYPred = cms.int32(24),
    avgHitsPerTrack = cms.double(10.0),
    avgCellsPerHit = cms.double(25),
    avgCellsPerCell = cms.double(5),
    avgTracksPerCell = cms.double(5),
    minHitsForSharingCut = cms.uint32(10),
    fitNas4 = cms.bool(False),
    useRiemannFit = cms.bool(False),
    doSharedHitCut = cms.bool(True),
    dupPassThrough = cms.bool(False),
    useSimpleTripletCleaner = cms.bool(True),
    trackQualityCuts = cms.PSet(
        maxChi2 = cms.double(5.0),
        minPt   = cms.double(0.9),
        maxTip  = cms.double(0.3),
        maxZip  = cms.double(12),
    ),
    geometry = cms.PSet(
        # This cut also uses the hardCurvCut parameters inside the
        # Kernel_connect "function". This is used to cut connections that have
        # either a too low p_t or that do not intersect the BS+tolerance
        # region. Internally, this cut is compared against the circle.dca0() in
        # natural units divided by circle.curvature(), where circle is the
        # circle passing through the 3 points of the triplet under
        # investigation. Therefore the cut represent the compatibility of the
        # circle in the transverse plane and the units are meant to be cm.
        caDCACuts = cms.vdouble(
            0.15,                       #  0
            0.25,                       #  1
            0.20,                       #  2
            0.20, # End PXB             #  3
            0.25,                       #  4
            0.25,                       #  5
            0.25,                       #  6
            0.25,                       #  7
            0.25,                       #  8
            0.25,                       #  9
            0.25,                       # 10
            0.25,                       # 11
            0.25,                       # 12
            0.25,                       # 13
            0.25,                       # 14
            0.25, # End PXFWD+          # 15
            0.25,                       # 16
            0.25,                       # 17
            0.25,                       # 18
            0.25,                       # 19
            0.25,                       # 20
            0.25,                       # 21
            0.25,                       # 22
            0.25,                       # 23
            0.25,                       # 24
            0.25,                       # 25
            0.25,                       # 26
            0.25, # End PXFWD-          # 27
            0.10,                       # 28
            0.10,                       # 29
            0.10), # End of OT PinPS    # 30
        # caThetaCut is used in the areAlignedRZ function to check if two
        # sibling cell are compatible in the R-Z plane. In that same function,
        # we also use ptmin variable. The caThetaCut is assigned to the SoA of
        # the layers, and is percolated into this compatibility function via
        # the SoA itself.
        caThetaCuts = cms.vdouble(
            0.002,                      #  0
            0.002,                      #  1
            0.002,                      #  2
            0.002,                      #  3
            0.003,                      #  4
            0.003,                      #  5
            0.003,                      #  6
            0.003,                      #  7
            0.003,                      #  8
            0.003,                      #  9
            0.003,                      # 10
            0.003,                      # 11
            0.003,                      # 12
            0.003,                      # 13
            0.003,                      # 14
            0.003,                      # 15
            0.003,                      # 16
            0.003,                      # 17
            0.003,                      # 18
            0.003,                      # 19
            0.003,                      # 20
            0.003,                      # 21
            0.003,                      # 22
            0.003,                      # 23
            0.003,                      # 24
            0.003,                      # 25
            0.003,                      # 26
            0.003,                      # 27
            0.003,                      # 28
            0.003,                      # 29
            0.003),                     # 30
        startingPairs = cms.vint32(
                0,    # PXB0-1
                1,    # PXB0-4
                2,    # PXB0-16
                3,    # PXB1-2
                4,    # PXB1-4
                5,    # PXB1-16
                6,    # PXB2-3
#                7,    # PXB2-4
#                8,    # PXB2-16
                9,
                10,
                11,
                12,
                13,
                14,
                15,
                16,
                17,
                18,
                19,
                20,
                21,
                22,
                23,
                24,
                25,
                26,
#                27,
#                28
#                30,
#                31,
#                32,
                ),
        pairGraph = cms.vint32(
                0, 1,                         # 0
                0, 4,                         # 1
                0, 16,                        # 2
                1, 2,                         # 3
                1, 4,                         # 4
                1, 16,                        # 5
                2, 3,                         # 6
                2, 4,                         # 7
                2, 16,                        # 8
                4, 5,                         # 9
                5, 6,                         # 10
                6, 7,                         # 11
                7, 8,                         # 12
                8, 9,                         # 13
                9, 10,                        # 14
                10, 11,                       # 15
                16, 17,                       # 16
                17, 18,                       # 17
                18, 19,                       # 18
                19, 20,                       # 19
                20, 21,                       # 20
                21, 22,                       # 21
                22, 23,                       # 22
                0, 2,                         # 23
                0, 5,                         # 24
                0, 17,                        # 25
#               0, 6,                         # 26
#               0, 18,                        # 27
                1, 3,                         # 28
                1, 5,                         # 29
                1, 17,                        # 30
#                1, 6,                        # 31
#                1, 18, # last starting pair  # 32
                11, 12,                       # 33
                12, 13,                       # 34
                13, 14,                       # 35
                14, 15,                       # 36
                23, 24,                       # 37
                24, 25,                       # 38
                25, 26,                       # 39
                26, 27,                       # 40
                4, 6,                         # 41
                5, 7,                         # 42
                6, 8,                         # 43
                7, 9,                         # 44
                8, 10,                        # 45
                9, 11,                        # 46
                10, 12,                       # 47
                16, 18,                       # 48
                17, 19,                       # 49
                18, 20,                       # 50
                19, 21,                       # 51
                20, 22,                       # 52
                21, 23,                       # 53
                22, 24,                       # 54
                 2, 28,                       # 55
                 3, 28,                       # 56
#                3, 29,                       # 57
                28, 29,                       # 58
#                28, 30,                      # 59
                29, 30,                       # 60
                 4, 28,                       # 61
                 5, 28,                       # 62
                 6, 28,                       # 63
                 7, 28,                       # 64
#                8, 28,                       # 65
#                9, 28,                       # 66
                16, 28,                       # 67
                17, 28,                       # 68
                18, 28,                       # 69
                19, 28,                       # 70
#                20, 28,                      # 71
#                21, 28,                      # 72
#                4, 29,                       # 73
#                5, 29,                       # 74
#                6, 29,                       # 75
#                7, 29,                       # 76
#                8, 29,                       # 77
#                16, 29,                      # 78
#                17, 29,                      # 79
#                18, 29,                      # 80
#                19, 29,                      # 81
#                20, 29,                      # 82
                11, 13,                       # 83
                11, 14,                       # 84
                11, 15,                       # 85
                23, 25,                       # 86
                23, 26,                       # 87
                23, 27,                       # 88
#                 1, 28,                      # 89
#                 1, 28,                      # 90
                 ),
        phiCuts = cms.vint32(
                522,   # 0
                650,   # 1
                650,   # 2
                626,   # 3
                730,   # 4
                730,   # 5
                626,   # 6
                730,   # 7
                730,   # 8
                522,   # 9
                522,   # 10
                522,   # 11
                522,   # 12
                522,   # 13
                522,   # 14
                522,   # 15
                522,   # 16
                522,   # 17
                522,   # 18
                522,   # 19
                522,   # 20
                522,   # 21
                522,   # 22
                600,   # 23
                522,   # 24
                522,   # 25
#               522,   # 26
#               522,   # 27
                650,   # 28
                730,   # 29
                730,   # 30
#               730,   # 31
#               730,   # 32
                730,   # 33
                730,   # 34
                730,   # 35
                730,   # 36
                730,   # 37
                730,   # 38
                730,   # 39
                730,   # 40
                730,   # 41
                730,   # 42
                730,   # 43
                730,   # 44
                730,   # 45
                730,   # 46
                650,   # 47
                522,   # 48
                522,   # 49
                522,   # 50
                522,   # 51
                522,   # 52
                522,   # 53
                650,   # 54
               1200,   # 55
               1000,   # 56
#              1500,   # 57
               1100,   # 58
#              2000,   # 59
               1250,   # 60
               1000,   # 61
               1000,   # 62
               1000,   # 63
               1000,   # 64
#               1000,  # 65
#               1000,  # 66
               1000,   # 67
               1000,   # 68
               1000,   # 69
               1000,   # 70
#               1000,  # 71
#               1000,  # 72
#               1000,  # 73
#               1000,  # 74
#               1000,  # 75
#               1000,  # 76
#               1000,  # 77
#               1000,  # 78
#               1000,  # 79
#               1000,  # 80
#               1000,  # 81
#               1000,  # 82
                500,   # 83
                300,   # 84
                400,   # 85
                500,   # 86
                300,   # 87
                400,   # 88
#               1300,  # 89
#               1300,  # 90
                ),
        # minZ and maxZ are the limits in Z for the inner cell of a doublets in
        # order to be able to make a doublet with the other layer.
        minZ = cms.vdouble(
              -20.0,     # 0
              4.0,       # 1
              -22.0,     # 2
              -17.0,     # 3
              6.0,       # 4
              -22.0,     # 5
              -18.0,     # 6
              11.0,      # 7
              -22.0,     # 8
              23.0,      # 9
              30.0,      # 10
              39.0,      # 11
              50.0,      # 12
              65.0,      # 13
              82.0,      # 14
              109.0,     # 15
              -28.0,     # 16
              -35.0,     # 17
              -44.0,     # 18
              -55.0,     # 19
              -70.0,     # 20
              -87.0,     # 21
              -113.0,    # 22
              -16.0,     # 23
              7.0,       # 24
              -22.0,     # 25
#             11.0,      # 26
#             -22.0,     # 27
              -17.0,     # 28
               9.0,      # 29
              -22.0,     # 30
#              13.0,     # 31
#              -22.0,    # 32
              137.0,     # 33
              173.0,     # 34
              199.0,     # 35
              229.0,     # 36
              -142.0,    # 37
              -177.0,    # 38
              -203.0,    # 39
              -233.0,    # 40
              23.0,      # 41
              30.0,      # 42
              39.0,      # 43
              50.0,      # 44
              65.0,      # 45
              82.0,      # 46
              109.0,     # 47
              -28.0,     # 48
              -35.0,     # 49
              -44.0,     # 50
              -55.0,     # 51
              -70.0,     # 52
              -87.0,     # 53
              -113.0,    # 54
                -20,     # 55
                -20,     # 56
#               -40,     # 57
                -1200,   # 58
#                -40,    # 59
                -1200,   # 60
                 23,     # 61
                 30,     # 62
                39,      # 63
                50,      # 64
#               -1000,   # 65
#               -1000,   # 66
                -28,     # 67
                -35,     # 68
                -44,     # 69
                -55,     # 70
#               -1000,   # 71
#               -1000,   # 72
#               -1000,   # 73
#               -1000,   # 74
#               -1000,   # 75
#               -1000,   # 76
#               -1000,   # 77
#               -1000,   # 78
#               -1000,   # 79
#               -1000,   # 80
#               -1000,   # 81
#               -1000,   # 82
                -1000,   # 83 
                -1000,   # 84
                -1000,   # 85
                -1000,   # 86
                -1000,   # 87
                -1000,   # 88
#               -1000,   # 89
#                15.0,   # 90
                 ),
        maxZ = cms.vdouble(
              20.0,      # 0
              22.0,      # 1
              -4.0,      # 2
              17.0,      # 3
              22.0,      # 4
              -6.0,      # 5
              18.0,      # 6
              22.0,      # 7
              -11.0,     # 8
              28.0,      # 9
              35.0,      # 10
              44.0,      # 11
              55.0,      # 12
              70.0,      # 13
              87.0,      # 14
              113.0,     # 15
              -23.0,     # 16
              -30.0,     # 17
              -39.0,     # 18
              -50.0,     # 19
              -65.0,     # 20
              -82.0,     # 21
              -109.0,    # 22
              17.0,      # 23
              22.0,      # 24
              -7.0,      # 25
#             22.0,      # 26
#             -10.0,     # 27
              17.0,      # 28
               22.0,     # 29
              -9.0,      # 30
#              22.0,     # 31
#              -13.0,    # 32
              142.0,     # 33
              177.0,     # 34
              203.0,     # 35
              233.0,     # 36
              -137.0,    # 37
              -173.0,    # 38
              -199.0,    # 39
              -229.0,    # 40
              28.0,      # 41
              35.0,      # 42
              44.0,      # 43
              55.0,      # 44
              70.0,      # 45
              87.0,      # 46
              113.0,     # 47
              -23.0,     # 48
              -30.0,     # 49
              -39.0,     # 50
              -50.0,     # 51
              -65.0,     # 52
              -82.0,     # 53
              -109.0,    # 54
                20,      # 55
                20,      # 56
#                 40,    # 57
                1200,    # 58
#                 40,    # 59
                1200,    # 60
                  28,    # 61
                35,      # 62
                44,      # 63
               55,       # 64
#                1000,   # 65
#                1000,   # 66
                -23,     # 67
                -30,     # 68
                -39,     # 69
                -50,     # 70
#                1000,   # 71
#                1000,   # 72
#                1000,   # 73
#                1000,   # 74
#                1000,   # 75
#                1000,   # 76
#                1000,   # 77
#                1000,   # 78
#                1000,   # 79
#                1000,   # 80
#                1000,   # 81
#                1000,   # 82
                 1000,   # 83 
                 1000,   # 84
                 1000,   # 85
                 1000,   # 86
                 1000,   # 87
                 1000,   # 88
#               -15.0,   # 89
#                1000,   # 90
                 ),
        maxR = cms.vdouble(
             4.156, #  0 (00-01) 
             8.000, #  1 (00-04) 
             8.603, #  2 (00-16) 
             6.527, #  3 (01-02) 
             7.452, #  4 (01-04) 
             9.743, #  5 (01-16) 
             7.916, #  6 (02-03) 
             7.624, #  7 (02-04) 
             7.113, #  8 (02-16) 
             6.186, #  9 (04-05) 
             6.130, # 10 (05-06) 
             5.715, # 11 (06-07) 
             6.732, # 12 (07-08) 
             4.979, # 13 (08-09) 
             5.886, # 14 (09-10) 
             5.933, # 15 (10-11) 
             5.739, # 16 (16-17) 
             5.244, # 17 (17-18) 
             5.204, # 18 (18-19) 
             5.141, # 19 (19-20) 
             5.493, # 20 (20-21) 
             5.700, # 21 (21-22) 
             5.795, # 22 (22-23) 
             9.337, # 23 (00-02) 
             5.955, # 24 (00-05) 
             5.280, # 25 (00-17) 
            10.867, # 26 (01-03) 
             8.522, # 27 (01-05) 
            11.768, # 28 (01-17) 
             6.654, # 29 (11-12) 
             4.593, # 30 (12-13) 
             4.716, # 31 (13-14) 
             5.234, # 32 (14-15) 
             6.983, # 33 (23-24) 
             5.302, # 34 (24-25) 
             5.611, # 35 (25-26) 
             5.481, # 36 (26-27) 
             8.272, # 37 (04-06) 
             9.586, # 38 (05-07) 
            10.000, # 39 (06-08) 
             8.414, # 40 (07-09) 
             7.110, # 41 (08-10) 
             9.000, # 42 (09-11) 
            11.321, # 43 (10-12) 
            10.000, # 44 (16-18) 
             8.197, # 45 (17-19) 
             8.782, # 46 (18-20) 
             7.627, # 47 (19-21) 
             7.221, # 48 (20-22) 
             8.677, # 49 (21-23) 
            11.098, # 50 (22-24) 
            20.627, # 51 (02-28) 
            19.817, # 52 (03-28) 
            23.885, # 53 (28-29) 
            24.749, # 54 (29-30) 
            15.000, # 55 (04-28) 
            25.235, # 56 (05-28) 
            19.774, # 57 (06-28) 
            15.092, # 58 (07-28) 
            15.000, # 59 (16-28) 
            27.758, # 60 (17-28) 
            15.000, # 61 (18-28) 
            15.453, # 62 (19-28) 
             8.171, # 63 (11-13) 
             2.238, # 64 (11-14) 
             4.798, # 65 (11-15) 
             7.000, # 66 (23-25) 
             3.106, # 67 (23-26) 
             4.576, # 68 (23-27) 
        )
    ),
    # autoselect the alpaka backend
    alpaka = cms.untracked.PSet(backend = cms.untracked.string(''))
)


def exclude_layers(hltPhase2PixelTracksSoA, layers_to_exclude):
    keep_indices = []
    num_pairs = len(hltPhase2PixelTracksSoA.geometry.pairGraph) // 2
    for i in range(num_pairs):
        a = hltPhase2PixelTracksSoA.geometry.pairGraph[2*i]
        b = hltPhase2PixelTracksSoA.geometry.pairGraph[2*i + 1]
        if a not in layers_to_exclude and b not in layers_to_exclude:
            keep_indices.append(i)
    # Now update in place
    # For pairGraph, build the new flat list from kept pairs
    new_pairGraph = []
    for i in keep_indices:
        new_pairGraph.extend([hltPhase2PixelTracksSoA.geometry.pairGraph[2*i], hltPhase2PixelTracksSoA.geometry.pairGraph[2*i+1]])

    hltPhase2PixelTracksSoA.geometry.pairGraph[:] = new_pairGraph
    # Update all other lists in place
    hltPhase2PixelTracksSoA.geometry.phiCuts[:] = [hltPhase2PixelTracksSoA.geometry.phiCuts[i] for i in keep_indices]
    hltPhase2PixelTracksSoA.geometry.minZ[:] = [hltPhase2PixelTracksSoA.geometry.minZ[i] for i in keep_indices]
    hltPhase2PixelTracksSoA.geometry.maxZ[:] = [hltPhase2PixelTracksSoA.geometry.maxZ[i] for i in keep_indices]
    hltPhase2PixelTracksSoA.geometry.maxR[:] = [hltPhase2PixelTracksSoA.geometry.maxR[i] for i in keep_indices]


if removeOT:
    ot_layers_ = [28, 29, 30]
    exclude_layers(hltPhase2PixelTracksSoA, layers_to_exclude=ot_layers_)

print("Using {} pair connections: {}".format(len(hltPhase2PixelTracksSoA.geometry.pairGraph), hltPhase2PixelTracksSoA.geometry.pairGraph))
