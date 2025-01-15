#ifndef RecoTracker_PixelSeeding_SimDoublets_h
#define RecoTracker_PixelSeeding_SimDoublets_h


#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"   // includes input data format: RecHit collection 
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticleFwd.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"
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
        Doublet(SimDoublets const&, size_t const, size_t const, const TrackerGeometry*);

        // method to access the layers pair
        std::pair<uint8_t, uint8_t> layerIds() const {
            return layerIds_;
        }

        // method to access the RecHit pair
        std::pair<SiPixelRecHitRef, SiPixelRecHitRef> recHits() const {
            return recHitRefs_;
        }

        // method to access the number of skipped layers
        int8_t numSkippedLayers() const {
            return numSkippedLayers_;
        }
        
        // method to access the layer pair ID
        int8_t layerPairId() const {
            return layerPairId_;
        }

    private:
        const TrackerGeometry* trackerGeometry_ = nullptr;          // pointer to the tracker geometry
        TrackingParticleRef trackingParticleRef_;                   // reference to the TrackingParticle
        std::pair<SiPixelRecHitRef, SiPixelRecHitRef> recHitRefs_;  // reference pair to RecHits of the Doublet
        std::pair<uint8_t, uint8_t> layerIds_;                      // pair of layer IDs corresponding to the RecHits
        int8_t numSkippedLayers_;                                   // number of layers skipped by the Doublet
        int8_t layerPairId_;                                        // ID of the layer pair as defined in the reconstruction for the doublets
    };


    // default contructor
    SimDoublets() {}

    // constructor
    SimDoublets(TrackingParticleRef const trackingParticleRef) : trackingParticleRef_(trackingParticleRef) {}

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
    std::vector<Doublet> getSimDoublets(const TrackerGeometry* trackerGeometry = nullptr) const;

private:
    TrackingParticleRef trackingParticleRef_;               // reference to the TrackingParticle
    SiPixelRecHitRefVector recHitRefVector_;                // reference vector to RecHits associated to the TP (sorted afer building)
    std::vector<uint8_t> layerIdVector_;                    // vector of layer IDs corresponding to the RecHits
    bool recHitsAreSorted_ {false};                         // true if RecHits were sorted
};


#endif
