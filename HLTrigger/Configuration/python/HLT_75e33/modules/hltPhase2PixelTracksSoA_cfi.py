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
              -20.094, #  0 (00-01) 
                4.177, #  1 (00-04) 
              -21.504, #  2 (00-16) 
              -16.526, #  3 (01-02) 
                6.134, #  4 (01-04) 
              -21.747, #  5 (01-16) 
              -17.702, #  6 (02-03) 
               10.800, #  7 (02-04) 
              -21.652, #  8 (02-16) 
               23.355, #  9 (04-05) 
               30.045, # 10 (05-06) 
               37.726, # 11 (06-07) 
               50.922, # 12 (07-08) 
               65.607, # 13 (08-09) 
               78.790, # 14 (09-10) 
              110.924, # 15 (10-11) 
              -28.109, # 16 (16-17) 
              -35.257, # 17 (17-18) 
              -44.785, # 18 (18-19) 
              -54.435, # 19 (19-20) 
              -69.893, # 20 (20-21) 
              -83.908, # 21 (21-22) 
             -117.134, # 22 (22-23) 
              -16.320, # 23 (00-02) 
                6.822, # 24 (00-05) 
              -22.016, # 25 (00-17) 
              -17.329, # 26 (01-03) 
                9.020, # 27 (01-05) 
              -21.532, # 28 (01-17) 
              137.332, # 29 (11-12) 
              170.243, # 30 (12-13) 
              206.402, # 31 (13-14) 
              233.756, # 32 (14-15) 
             -144.139, # 33 (23-24) 
             -174.183, # 34 (24-25) 
             -208.953, # 35 (25-26) 
             -232.368, # 36 (26-27) 
               22.257, # 37 (04-06) 
               29.980, # 38 (05-07) 
               38.793, # 39 (06-08) 
               49.566, # 40 (07-09) 
               67.237, # 41 (08-10) 
               83.465, # 42 (09-11) 
              106.961, # 43 (10-12) 
              -28.855, # 44 (16-18) 
              -36.356, # 45 (17-19) 
              -45.390, # 46 (18-20) 
              -54.355, # 47 (19-21) 
              -70.843, # 48 (20-22) 
              -86.049, # 49 (21-23) 
             -112.274, # 50 (22-24) 
              -19.491, # 51 (02-28) 
              -20.139, # 52 (03-28) 
             -120.000, # 53 (28-29) 
             -126.793, # 54 (29-30) 
               22.696, # 55 (04-28) 
               29.428, # 56 (05-28) 
               38.318, # 57 (06-28) 
               50.685, # 58 (07-28) 
              -28.271, # 59 (16-28) 
              -34.174, # 60 (17-28) 
              -42.641, # 61 (18-28) 
              -56.240, # 62 (19-28) 
              116.868, # 63 (11-13) 
              110.000, # 64 (11-14) 
              115.303, # 65 (11-15) 
             -152.169, # 66 (23-25) 
             -158.224, # 67 (23-26) 
             -159.016, # 68 (23-27) 
        ),
        maxZ = cms.vdouble(
               20.081, #  0 (00-01) 
               21.237, #  1 (00-04) 
               -3.553, #  2 (00-16) 
               17.531, #  3 (01-02) 
               21.639, #  4 (01-04) 
               -5.971, #  5 (01-16) 
               18.481, #  6 (02-03) 
               21.437, #  7 (02-04) 
              -10.931, #  8 (02-16) 
               29.264, #  9 (04-05) 
               34.567, # 10 (05-06) 
               43.564, # 11 (06-07) 
               55.589, # 12 (07-08) 
               69.197, # 13 (08-09) 
               87.113, # 14 (09-10) 
              116.097, # 15 (10-11) 
              -23.501, # 16 (16-17) 
              -28.568, # 17 (17-18) 
              -39.340, # 18 (18-19) 
              -48.541, # 19 (19-20) 
              -65.175, # 20 (20-21) 
              -82.809, # 21 (21-22) 
             -105.109, # 22 (22-23) 
               16.835, # 23 (00-02) 
               21.720, # 24 (00-05) 
               -7.006, # 25 (00-17) 
               16.786, # 26 (01-03) 
               21.499, # 27 (01-05) 
               -8.806, # 28 (01-17) 
              140.716, # 29 (11-12) 
              185.318, # 30 (12-13) 
              211.857, # 31 (13-14) 
              226.160, # 32 (14-15) 
             -136.330, # 33 (23-24) 
             -173.058, # 34 (24-25) 
             -196.551, # 35 (25-26) 
             -220.990, # 36 (26-27) 
               28.858, # 37 (04-06) 
               35.111, # 38 (05-07) 
               43.707, # 39 (06-08) 
               56.991, # 40 (07-09) 
               68.355, # 41 (08-10) 
               87.163, # 42 (09-11) 
              117.445, # 43 (10-12) 
              -23.541, # 44 (16-18) 
              -30.042, # 45 (17-19) 
              -40.246, # 46 (18-20) 
              -50.196, # 47 (19-21) 
              -65.632, # 48 (20-22) 
              -80.463, # 49 (21-23) 
             -105.553, # 50 (22-24) 
               19.755, # 51 (02-28) 
               20.431, # 52 (03-28) 
              113.045, # 53 (28-29) 
              113.933, # 54 (29-30) 
               28.033, # 55 (04-28) 
               35.368, # 56 (05-28) 
               45.145, # 57 (06-28) 
               54.162, # 58 (07-28) 
              -23.782, # 59 (16-28) 
              -30.364, # 60 (17-28) 
              -39.720, # 61 (18-28) 
              -49.952, # 62 (19-28) 
              152.080, # 63 (11-13) 
              154.687, # 64 (11-14) 
              154.201, # 65 (11-15) 
             -113.400, # 66 (23-25) 
             -113.486, # 67 (23-26) 
             -111.890, # 68 (23-27) 
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
