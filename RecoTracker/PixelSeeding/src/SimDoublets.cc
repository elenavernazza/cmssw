#include "RecoTracker/PixelSeeding/interface/SimDoublets.h"

#include "Geometry/CommonTopologies/interface/SimplePixelTopology.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/MeasurementPoint.h"    // includes MeasurementPoint
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

namespace simdoublets {

    // function that converts layerId from one convention (1 to 212) to another (0 to 27)
    uint8_t convertLayerIdToRange0to27(uint8_t const layerIdRange1to212) {
        // if layer is in barrel (1,4)
        if (layerIdRange1to212 < 5) {
            // subtract 1 to get to (0,3)
            return (layerIdRange1to212 - 1);
        }
        // if layer is in backward direction (101,112)
        else if (layerIdRange1to212 < 200) {
            // subtract  to get to (16,27)
            return (layerIdRange1to212 - 85);
        }
        // if layer is in backward direction (201,212)
        else {
            // subtract  to get to (4,15)
            return (layerIdRange1to212 - 197);
        }
    }


    // function that gets the global position of a RecHit based on its Cluster position
    GlobalPoint getGlobalHitPosition(SiPixelRecHitRef const& recHit, const TrackerGeometry* trackerGeometry) {
        // get DetUnit of the RecHit
        DetId detIdObject(recHit->geographicalId());
        const GeomDetUnit* geomDetUnit = trackerGeometry->idToDetUnit(detIdObject);

        // get position from cluster local position and DetUnit global position
        auto cluster = recHit->cluster();
        MeasurementPoint measurementPoint(cluster->x(), cluster->y());
        LocalPoint localPosition = geomDetUnit->topology().localPosition(measurementPoint);

        return geomDetUnit->surface().toGlobal(localPosition);
    }


    // function that determines the number of skipped layers for a given pair of layer IDs
    // layerIds cover the ranges:
    // 1 to 4 (barrel), 101 to 112 (backward), 201 to 212 (forward)
    int getNumSkippedLayers(std::pair<uint8_t, uint8_t> const& layerIds) {

        // Possibility 0: invalid case (outer layer is not the outer one), set to -1
        if (layerIds.first >= layerIds.second) {
            return -1;
        }
        // Possibility 1: both RecHits lie in the same detector part (barrel, forward or backward)
        else if ((layerIds.second - layerIds.first) < 50) {
            return (layerIds.second - layerIds.first - 1);
        }
        // Possibility 2: the inner RecHit is in the barrel while the outer is in either forward or backward
        else if (layerIds.first < 5) {
            return (layerIds.second % 100) - 1;
        }
        // Possibility 3: invalid case (one is forward and the other in backward), set to -1
        else {
            return -1;
        }
    }


    // function that, for a pair of two layers, determines the pair index that is used in the true reconstruction;
    // if that layer pair is not considered in reconstruction, return -1
    int getLayerPairId(std::pair<uint8_t, uint8_t> const& layerIds) {

        // first, convert the 1 to 212 ranged layer Id into the reco range 0 to 27
        uint8_t innerLayerId = convertLayerIdToRange0to27(layerIds.first);
        uint8_t outerLayerId = convertLayerIdToRange0to27(layerIds.second);

        // then, loop through all layer pairs considered in the reconstruction
        // and find the one corresponding to the present pair
        int layerPairId = -1;
        for (size_t i=0; i < pixelTopology::Phase2::nPairs; i++) {
            if ((pixelTopology::Phase2::layerPairs[2*i] == innerLayerId) && (pixelTopology::Phase2::layerPairs[2*i+1] == outerLayerId)) {
                layerPairId = i;
                break;
            }
        }

        return layerPairId;
    }

} // end namespace simdoublets




SimDoublets::Doublet::Doublet(SimDoublets const& simDoublets, size_t const innerIndex, size_t const outerIndex, const TrackerGeometry* trackerGeometry) : 
    trackerGeometry_(trackerGeometry),
    trackingParticleRef_(simDoublets.trackingParticle())
{
    // fill recHits and layers
    recHitRefs_ = std::make_pair(simDoublets.recHits(innerIndex), simDoublets.recHits(outerIndex));
    layerIds_ = std::make_pair(simDoublets.layerIds(innerIndex), simDoublets.layerIds(outerIndex));

    // determine number of skipped layers
    numSkippedLayers_ = simdoublets::getNumSkippedLayers(layerIds_);

    // determine Id of the layer pair
    layerPairId_ = simdoublets::getLayerPairId(layerIds_);
}



void SimDoublets::sortRecHits(const TrackerGeometry* trackerGeometry) {
    // get the vector of squared magnitudes of the global RecHit positions
    std::vector<double> recHitMag2;
    recHitMag2.reserve(layerIdVector_.size());
    for (const auto& recHit : recHitRefVector_) {
        Global3DPoint globalPosition = simdoublets::getGlobalHitPosition(recHit, trackerGeometry);
        recHitMag2.push_back(globalPosition.mag2());
    }

    // find the permutation vector that sort the magnitudes
    std::vector<std::size_t> sortedPerm(recHitMag2.size());
    std::iota(sortedPerm.begin(), sortedPerm.end(), 0);
    std::sort(sortedPerm.begin(), sortedPerm.end(),
        [&](std::size_t i, std::size_t j){ return (recHitMag2[i] < recHitMag2[j]); });
    
    // create the sorted recHitRefVector and the sorted layerIdVector accordingly
    SiPixelRecHitRefVector sorted_recHitRefVector;
    sorted_recHitRefVector.reserve(sortedPerm.size());
    for (size_t i : sortedPerm) {
        sorted_recHitRefVector.push_back(recHitRefVector_[i]);
    }
    std::vector<uint8_t> sorted_layerIdVector(sortedPerm.size());
    std::transform(sortedPerm.begin(), sortedPerm.end(), sorted_layerIdVector.begin(),
        [&](std::size_t i){ return layerIdVector_[i]; });

    // swap them with the class member
    recHitRefVector_.swap(sorted_recHitRefVector);
    layerIdVector_.swap(sorted_layerIdVector);

    // set sorted bool to true
    recHitsAreSorted_ = true;
}



std::vector<SimDoublets::Doublet> SimDoublets::getSimDoublets(const TrackerGeometry* trackerGeometry) const {

    // create output vector for the doublets
    std::vector<SimDoublets::Doublet> doubletVector;

    // confirm that the RecHits are sorted
    if (!recHitsAreSorted_){
        return doubletVector;
    }

    // loop over the RecHits/layer Ids
    for (size_t i=0; i<layerIdVector_.size(); i++) {
        uint8_t innerLayerId = layerIdVector_[i];
        uint8_t outerLayerId {};
        size_t outerLayerStart {};

        // find the next layer Id + at which hit this layer starts
        for (size_t j=i+1; j<layerIdVector_.size(); j++) {
            if (innerLayerId != layerIdVector_[j]) {
                outerLayerId = layerIdVector_[j];
                outerLayerStart = j;
                break;
            }
        }

        // build the doublets of the inner hit i with all outer hits in the layer outerLayerId
        for (size_t j=outerLayerStart; j<layerIdVector_.size(); j++) {
            
            // break if the hit doesn't belong to the outer layer anymore
            if (outerLayerId != layerIdVector_[j]) {
                break;
            }

            doubletVector.push_back(SimDoublets::Doublet(*this, i, j, trackerGeometry));
        }
    } // end loop over the RecHits/layer Ids

    return doubletVector;
}