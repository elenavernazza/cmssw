#ifndef RecoTracker_PixelSeeding_SimDoublet_h
#define RecoTracker_PixelSeeding_SimDoublet_h


#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"   // includes input data format: RecHit collection 
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticleFwd.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/RefVector.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>


// persistent reference to a SiPixelRecHit in a SiPixelRecHitCollection
typedef edm::Ref<SiPixelRecHitCollection, SiPixelRecHit> SiPixelRecHitRef;
// persistent vector of references to SiPixelRecHits in a SiPixelRecHitCollection
typedef edm::RefVector<SiPixelRecHitCollection, SiPixelRecHit> SiPixelRecHitRefVector;


// class function that compares two RecHits by position
struct RecHitLessByPosition {
  bool operator()(const SiPixelRecHitRef& h1, const SiPixelRecHitRef& h2) { return h1->globalPosition().mag2() < h2->globalPosition().mag2(); }
};


/** @brief Semi-Monte Carlo truth information used for pixel-tracking opimization.
 *
 * DESCRIPTION FIXME
 *
 * @author Jan Schulz (jan.gerrit.schulz@cern.ch)
 * @date Dec 2024
 */
class SimDoublets {
public:

    /**
     * Sub-class for true doublets of RecHits
     */
    class Doublet {
    public:
        enum layer { inner = 0, outer = 1 };

        // default constructor
        Doublet() {}

        // constructor
        Doublet(SimDoublets const& simDoublets, size_t const innerIndex, size_t const outerIndex) : 
            trackingParticleRef_(simDoublets.trackingParticle()) {
            // fill recHits and layers
            recHitRefs_ = std::make_pair(simDoublets.recHits(innerIndex), simDoublets.recHits(outerIndex));
            layerIds_ = std::make_pair(simDoublets.layerIds(innerIndex), simDoublets.layerIds(outerIndex));

            // determine number of skipped layers
            numSkippedLayers_ = 0;
        }

        // method to access the layers pair
        std::pair<uint8_t, uint8_t> layerIds() const {
            return layerIds_;
        }

    private:
        TrackingParticleRef trackingParticleRef_;                   // reference to the TrackingParticle
        std::pair<SiPixelRecHitRef, SiPixelRecHitRef> recHitRefs_;  // reference pair to RecHits of the Doublet
        std::pair<uint8_t, uint8_t> layerIds_;                      // pair of layer IDs corresponding to the RecHits
        uint8_t numSkippedLayers_;                                  // number of layers skipped by the Doublet
    };


    // default contructor
    SimDoublets() {}

    // constructor
    SimDoublets(TrackingParticleRef const trackingParticleRef) : trackingParticleRef_(trackingParticleRef) {}

    // FIXME: more constructors, copy, move...

    // method to add a RecHitRef with its layer
    void addRecHit(SiPixelRecHitRef const recHitRef, uint8_t const layerId){
        recHitRefVector_.push_back(recHitRef);
        layerIdVector_.push_back(layerId);
        recHitsAreSorted_ = false;  // set sorted to false again
    }

    // method to access the reference to the TrackingParticle
    TrackingParticleRef trackingParticle() const {
        return trackingParticleRef_;
    }

    // method to access the reference vector to the RecHits
    SiPixelRecHitRefVector recHits() const {
        return recHitRefVector_;
    }

    // method to access the RecHit at index i
    SiPixelRecHitRef recHits(size_t i) const {
        return recHitRefVector_[i];
    }

    // method to access the layer id vector
    std::vector<uint8_t> layerIds() const {
        return layerIdVector_;
    }

        // method to access the layer id at index i
    uint8_t layerIds(size_t i) const {
        return layerIdVector_[i];
    }


    // method to sort the RecHits according to the position
    void sortRecHits(const TrackerGeometry*);

    // method to produce the SimDoublets from the RecHits
    std::vector<Doublet> getSimDoublets() const;

private:
    TrackingParticleRef trackingParticleRef_;   // reference to the TrackingParticle
    SiPixelRecHitRefVector recHitRefVector_;    // reference vector to RecHits associated to the TP (sorted afer building)
    std::vector<uint8_t> layerIdVector_;        // vector of layer IDs corresponding to the RecHits
    bool recHitsAreSorted_ {false};             // true if RecHits were sorted
};


void SimDoublets::sortRecHits(const TrackerGeometry* trackerGeometry){
    // get the vector of squared magnitudes of the global RecHit positions
    std::vector<double> recHitMag2;
    recHitMag2.reserve(layerIdVector_.size());
    for (const auto& recHit : recHitRefVector_) {
        DetId detIdObject(recHit->geographicalId());
        const GeomDetUnit* geomDetUnit = trackerGeometry->idToDetUnit(detIdObject);
        auto globalPosition = geomDetUnit->surface().toGlobal(recHit->localPosition());
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


std::vector<SimDoublets::Doublet> SimDoublets::getSimDoublets() const {

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

            doubletVector.push_back(SimDoublets::Doublet(*this, i, j));
        }
    } // end loop over the RecHits/layer Ids

    return doubletVector;
}

#endif
