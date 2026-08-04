#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1.h"
#include "TH2.h"

#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/LogicalGraphHitIndex.h"
#include "PhysicsTools/TruthInfo/interface/SubgraphHitView.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"

class MyTruthAnalyzer : public edm::one::EDAnalyzer<> {
public:
    explicit MyTruthAnalyzer(edm::ParameterSet const&);
    ~MyTruthAnalyzer() override;
    //static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
    
private:
    void beginJob() override;
    void analyze(edm::Event const& event, edm::EventSetup const&) override;
    void endJob() override;

    //simple map to contain all histograms
    //booked in the beginJob, filled in the analyze
    std::map<std::string, TH1F*> histContainer_;
    std::map<std::string, TH2F*> histContainer2D_;
    
    const edm::EDGetTokenT<truth::Graph> graphToken_;
    const edm::EDGetTokenT<truth::LogicalGraphHitIndex> hitIndexToken_;
    const edm::EDGetTokenT<EcalRecHitCollection> ecalRecHitsToken_;
    const edm::EDGetTokenT<HBHERecHitCollection> hcalRecHitsToken_;

    bool doTenTau, doDYtoLL;

    std::vector<edm::EDGetTokenT<HGCRecHitCollection>> hgcalRecHitsToken_;
    
    //tentau
    mutable int totTau = 0, totTauToMu = 0, totTauToEle = 0, totTauToHadron = 0;
    mutable int totTauTo1Prg0Pi0 = 0, totTauTo1Prg1Pi0 = 0, totTauTo1Prg2Pi0 = 0, totTauTo3Prg0Pi0 = 0,totTauTo3Prg1Pi0 = 0, totTauTo1Prg3Pi0 = 0, totTauTo1Prg4Pi0 = 0, totTauTo3Prg2Pi0 = 0, totTauTo5Prg0Pi0 = 0, totTauTo1Eta1Kstar = 0, totTauToOther = 0;
    //DYtoLL
    mutable int totZ = 0, totZToEle = 0, totZToMu = 0, totZToTau = 0, totZToNot2Particles = 0, totZToNot2Leptons = 0, totZTo1Particle = 0;


};


MyTruthAnalyzer::MyTruthAnalyzer(edm::ParameterSet const& cfg)
        : histContainer_(),
          graphToken_(consumes<truth::Graph>(cfg.getParameter<edm::InputTag>("src"))),
          hitIndexToken_(consumes<truth::LogicalGraphHitIndex>(cfg.getParameter<edm::InputTag>("hitIndex"))),
          ecalRecHitsToken_(consumes<EcalRecHitCollection>(cfg.getParameter<edm::InputTag>("ecalRecHits"))),
          hcalRecHitsToken_(consumes<HBHERecHitCollection>(cfg.getParameter<edm::InputTag>("hcalRecHits"))),
          doTenTau(cfg.getParameter<bool>("doTenTau")),
          doDYtoLL(cfg.getParameter<bool>("doDYtoLL"))
        {
            const auto& hgcalTags = cfg.getParameter<std::vector<edm::InputTag>>("hgcalRecHits");
            for(auto const& tag : hgcalTags)
            {
                hgcalRecHitsToken_.push_back(consumes<HGCRecHitCollection>(tag));
            }
        }

MyTruthAnalyzer::~MyTruthAnalyzer() {}

/*void MyTruthAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc<edm::InputTag>("src", edm::InputTag("truthLogicalGraphProducer"));
    desc<edm::InputTag>("hitIndex", edm::InputTag("truthLogicalGraphHitIndexProducer"));
    descriptions.add("myTruthAnalyzer", desc);
}*/
          
void MyTruthAnalyzer::beginJob() {
    // register to the TFileService
    edm::Service<TFileService> fs;

    if(doTenTau)
    {
        //number of taus, photons, electrons at generator, simulation and truth level
        histContainer_["GenTauNum"] = fs->make<TH1F>("GenTauNum", "GenTauNum", 20, 0, 20); 
        histContainer_["SimTauNum"] = fs->make<TH1F>("SimTauNum", "SimTauNum", 20, 0, 20);
        histContainer_["TruthTauNum"] = fs->make<TH1F>("TruthTauNum", "TruthTauNum", 20, 0, 20);

        histContainer_["GenPhtNum"] = fs->make<TH1F>("GenPhtNum", "GenPhtNum", 10000, 0, 10000);
        histContainer_["GenPhtFTauNum"] = fs->make<TH1F>("GenPhtFTauNum", "GenPhtFTauNum", 10000, 0, 10000);
        histContainer_["SimPhtNum"] = fs->make<TH1F>("SimPhtNum", "SimPhtNum", 10000, 0, 10000);
        histContainer_["SimPhtNumHigherPt5p0"] = fs->make<TH1F>("SimPhtNumHigherPt5p0", "SimPhtNumHigherPt5p0", 10000, 0, 10000);
        histContainer_["TruthPhtNum"] = fs->make<TH1F>("TruthPhtNum", "TruthPhtNum", 10000, 0, 10000);
        
        histContainer_["GenEleNum"] = fs->make<TH1F>("GenEleNum", "GenEleNum", 200, 0, 200);
        histContainer_["SimEleNum"] = fs->make<TH1F>("SimEleNum", "SimEleNum", 200, 0, 200);
        histContainer_["SimEleNumHigherPt5p0"] = fs->make<TH1F>("SimEleNumHigherPt5p0", "SimEleNumHigherPt5p0", 200, 0, 200);
        histContainer_["TruthEleNum"] = fs->make<TH1F>("TruthEleNum", "TruthEleNum", 200, 0, 200);

        //radial distance of tau decay vertex
        histContainer_["TauDecayVtxRadius"] = fs->make<TH1F>("TauDecayVtxRadius", "TauDecayVtxRadius", 200, 0, 20);

        //mother particle of taus, photons, electrons in generator, simulation level
        histContainer_["GenTauMother"] = fs->make<TH1F>("GenTauMother", "GenTauMother", 2000, -1000, 1000);
        histContainer_["SimTauMother"] = fs->make<TH1F>("SimTauMother", "SimTauMother", 2000, -1000, 1000);

        histContainer_["GenPhtMother"] = fs->make<TH1F>("GenPhtMother", "GenPhtMother", 2000, -1000, 1000);
        histContainer_["SimPhtMother"] = fs->make<TH1F>("SimPhtMother", "SimPhtMother", 20000, -10000, 10000);

        histContainer_["GenEleMother"] = fs->make<TH1F>("GenEleMother", "GenEleMother", 2000, -1000, 1000);
        histContainer_["SimEleMother"] = fs->make<TH1F>("SimEleMother", "SimEleMother", 20000, -10000, 10000);

        //kinematic variables, pT, eta
        histContainer_["GenTauPt"] = fs->make<TH1F>("GenTauPt", "GenTauPt", 1000, 0, 500);
        histContainer_["GenTauEta"] = fs->make<TH1F>("GenTauEta", "GenTauEta", 100, -5, 5);
        histContainer_["SimTauPt"] = fs->make<TH1F>("SimTauPt", "SimTauPt", 1000, 0, 500);
        histContainer_["SimTauEta"] = fs->make<TH1F>("SimTauEta", "SimTauEta", 100, -5, 5);

        histContainer_["GenPhtPt"] = fs->make<TH1F>("GenPhtPt", "GenPhtPt", 1000, 0, 500);
        histContainer_["GenPhtEta"] = fs->make<TH1F>("GenPhtEta", "GenPhtEta", 100, -5, 5);
        histContainer_["SimPhtPt"] = fs->make<TH1F>("SimPhtPt", "SimPhtPt", 5000, 0, 500);
        histContainer_["SimPhtEta"] = fs->make<TH1F>("SimPhtEta", "SimPhtEta", 100, -5, 5);

        histContainer_["GenElePt"] = fs->make<TH1F>("GenElePt", "GenElePt", 1000, 0, 500);
        histContainer_["GenEleEta"] = fs->make<TH1F>("GenEleEta", "GenEleEta", 100, -5, 5);
        histContainer_["SimElePt"] = fs->make<TH1F>("SimElePt", "SimElePt", 5000, 0, 500);
        histContainer_["SimEleEta"] = fs->make<TH1F>("SimEleEta", "SimEleEta", 100, -5, 5);

        //Total RecHit energy
        histContainer_["TauSimHitE"] = fs->make<TH1F>("TauSimHitE", "TauSimHitE", 1000, 0, 500);
        histContainer_["TauRecHitE"] = fs->make<TH1F>("TauRecHitE", "TauRecHitE", 1000, 0, 500);
        histContainer_["TauEResponse"] = fs->make<TH1F>("TauEResponse", "TauEResponse", 100, 0, 2);

        histContainer2D_["TauEResponseVsEta"] = fs->make<TH2F>("TauEResponseVsEta", "TauEResponseVsEta", 100, -5, 5, 100, 0, 2);
        histContainer2D_["TauEResponseVsPt"] = fs->make<TH2F>("TauEResponseVsPt", "TauEResponseVsPt", 100, 0, 500, 100, 0, 2);
    }

    if(doDYtoLL)
    {
        //Z boson invariant mass
        histContainer_["GenZMass"] = fs->make<TH1F>("GenZMass", "GenZMass", 1000, 0, 200);
        //kinematic variables, pT, eta
        histContainer_["GenZPt"] = fs->make<TH1F>("GenZPt", "GenZPt", 1000, 0, 500);
        histContainer_["GenZEta"] = fs->make<TH1F>("GenZEta", "GenZEta", 200, -10, 10);
        histContainer_["GenEleFromZPt"] = fs->make<TH1F>("GenEleFromZPt", "GenEleFromZPt", 1000, 0, 500);
        histContainer_["GenEleFromZEta"] = fs->make<TH1F>("GenEleFromZEta", "GenEleFromZEta", 200, -10, 10);
        histContainer_["GenMuFromZPt"] = fs->make<TH1F>("GenMuFromZPt", "GenMuFromZPt", 1000, 0, 500);
        histContainer_["GenMuFromZEta"] = fs->make<TH1F>("GenMuFromZEta", "GenMuFromZEta", 200, -10, 10);
        histContainer_["GenTauFromZPt"] = fs->make<TH1F>("GenTauFromZPt", "GenTauFromZPt", 1000, 0, 500);
        histContainer_["GenTauFromZEta"] = fs->make<TH1F>("GenTauFromZEta", "GenTauFromZEta", 200, -10, 10);

    }
}

void MyTruthAnalyzer::analyze(edm::Event const& event, edm::EventSetup const&) {
        //truth graph collection
        auto const& graph = event.get(graphToken_);
        auto const& hitIndex  = event.get(hitIndexToken_);
        truth::SubgraphHitView subgraphHitView(hitIndex);

    if(doTenTau)
    {

        int nGenTau = 0, nSimTau = 0, nTruthTau = 0;
        int nGenPht = 0, nGenPhtFTau = 0, nSimPht = 0, nSimPhtHigherPt5p0 = 0, nTruthPht = 0; 
        int nGenEle = 0, nSimEle = 0, nSimEleHigherPt5p0 = 0, nTruthEle = 0;

        for (truth::Particle p : graph.particleViews())
        {
            if(!p.valid())
                continue;
            //number of photons, electrons, taus at generator, simulation level
            if(std::abs(p.pdgId()) == 15 && p.hasGen())    ++nGenTau;
            if(std::abs(p.pdgId()) == 15 && p.hasSim())    ++nSimTau;
            if(std::abs(p.pdgId()) == 15 && (p.hasGen() || p.hasSim()))    ++nTruthTau;

            if(std::abs(p.pdgId()) == 22 && p.hasGen())    ++nGenPht;
            if(std::abs(p.pdgId()) == 22 && p.hasGen() && (p.hasAncestorPdgId(15) || p.hasAncestorPdgId(-15)))
                ++nGenPhtFTau;
            if(std::abs(p.pdgId()) == 22 && p.hasSim())    ++nSimPht;
            if(std::abs(p.pdgId()) == 22 && p.hasSim() && p.momentum().pt() > 5.0)   ++nSimPhtHigherPt5p0;
            if(std::abs(p.pdgId()) == 22 && (p.hasGen() || p.hasSim()))    ++nTruthPht;
            
            if(std::abs(p.pdgId()) == 11 && p.hasGen())    ++nGenEle;
            if(std::abs(p.pdgId()) == 11 && p.hasSim())    ++nSimEle;
            if(std::abs(p.pdgId()) == 11 && p.hasSim() && p.momentum().pt() > 5.0)   ++nSimEleHigherPt5p0;
            if(std::abs(p.pdgId()) == 11 && (p.hasGen() || p.hasSim()))    ++nTruthEle;

            //radial distance of tau decay vertex
            if(std::abs(p.pdgId()) == 15 && p.hasGen())
            {
                auto produceVertices = p.productionVertices();
                auto decayVertices = p.decayVertices();
                if(produceVertices.size() == 1 && decayVertices.size() == 1)
                {
                    if(!produceVertices[0].valid() || !decayVertices[0].valid()) continue;
                    auto producepos = produceVertices[0].position();
                    auto decaypos = decayVertices[0].position();
                    double r = std::hypot(producepos.x()-decaypos.x(), producepos.y()-decaypos.y());
                    histContainer_["TauDecayVtxRadius"]->Fill(r);
                }
            }

            //mother particle of taus, photons, electrons in generator, simulation level
            if(std::abs(p.pdgId()) == 15 && p.hasGen()) 
            {
                std::vector<truth::Particle> mother = p.parents();
                if(mother.size() == 1) 
                {
                    histContainer_["GenTauMother"]->Fill(mother[0].pdgId());
                    //std::cout << "PDG ID of Tau in generation level is " << mother[0].pdgId() << std::endl;
                }
            }
            if(std::abs(p.pdgId()) == 15 && p.hasSim())
            {
                std::vector<truth::Particle> mother = p.parents();
                if(mother.size() == 1) 
                {
                    histContainer_["SimTauMother"]->Fill(mother[0].pdgId());
                    //std::cout << "PDG ID of Tau in simulation level is " << mother[0].pdgId() << std::endl;
                }
            }

            if(std::abs(p.pdgId()) == 22 && p.hasGen())
            {
                std::vector<truth::Particle> mother = p.parents();
                if(mother.size() == 1)
                {
                    histContainer_["GenPhtMother"]->Fill(mother[0].pdgId());
                }
            }
            if(std::abs(p.pdgId()) == 22 && p.hasSim())
            {
                std::vector<truth::Particle> mother = p.parents();
                if(mother.size() == 1)
                {
                    histContainer_["SimPhtMother"]->Fill(mother[0].pdgId());
                }
            }

            if(std::abs(p.pdgId()) == 11 && p.hasGen())
            {
                std::vector<truth::Particle> mother = p.parents();
                if(mother.size() == 1)
                {
                    histContainer_["GenEleMother"]->Fill(mother[0].pdgId());
                }
            }
            if(std::abs(p.pdgId()) == 11 && p.hasSim())
            {
                std::vector<truth::Particle> mother = p.parents();
                if(mother.size() == 1)
                {
                    histContainer_["SimEleMother"]->Fill(mother[0].pdgId());
                }
            }


            //kinematic distribution: pt, eta
            if(std::abs(p.pdgId()) == 15 && p.hasGen()){ histContainer_["GenTauPt"]->Fill(p.momentum().pt()); histContainer_["GenTauEta"]->Fill(p.momentum().eta());}
            if(std::abs(p.pdgId()) == 15 && p.hasSim()){ histContainer_["SimTauPt"]->Fill(p.momentum().pt()); histContainer_["SimTauEta"]->Fill(p.momentum().eta());}
            if(std::abs(p.pdgId()) == 22 && p.hasGen()){ histContainer_["GenPhtPt"]->Fill(p.momentum().pt()); histContainer_["GenPhtEta"]->Fill(p.momentum().eta());}
            if(std::abs(p.pdgId()) == 22 && p.hasSim()){ histContainer_["SimPhtPt"]->Fill(p.momentum().pt()); histContainer_["SimPhtEta"]->Fill(p.momentum().eta());}
            if(std::abs(p.pdgId()) == 11 && p.hasGen()){ histContainer_["GenElePt"]->Fill(p.momentum().pt()); histContainer_["GenEleEta"]->Fill(p.momentum().eta());}
            if(std::abs(p.pdgId()) == 11 && p.hasSim()){ histContainer_["SimElePt"]->Fill(p.momentum().pt()); histContainer_["SimEleEta"]->Fill(p.momentum().eta());}

            //fractions of tau decays    
            if(std::abs(p.pdgId()) == 15 && p.hasGen())
            {
                ++totTau;
                int nProng = 0, nPi0 = 0, nEta = 0, nKstar = 0;
                bool isMuDecay = 0, isEleDecay = 0;
                std::vector<truth::Particle> kids = p.children();
                for(const auto& kid : kids)
                {
                    if(!kid.valid() || !kid.hasGen()) continue;
                    if(std::abs(kid.pdgId()) == 13)
                    {    
                        ++totTauToMu;
                        isMuDecay = 1;
                        break;
                    }
                    if(std::abs(kid.pdgId()) == 11)
                    {
                        ++totTauToEle;
                        isEleDecay = 1;
                        break;
                    }
                    //hadronic decay
                    if(std::abs(kid.pdgId()) == 211 || std::abs(kid.pdgId()) == 321) ++nProng;
                    if(std::abs(kid.pdgId()) == 111) ++nPi0;
                    if(std::abs(kid.pdgId()) == 221) ++nEta;
                    if(std::abs(kid.pdgId()) == 323) ++nKstar;
                }
                if(isMuDecay == 0 && isEleDecay == 0)
                {
                    ++totTauToHadron;
                    if(nProng == 1 && nPi0 == 0) ++totTauTo1Prg0Pi0;
                    else if(nProng == 1 && nPi0 == 1) ++totTauTo1Prg1Pi0;
                    else if(nProng == 1 && nPi0 == 2) ++totTauTo1Prg2Pi0;
                    else if(nProng == 3 && nPi0 == 0) ++totTauTo3Prg0Pi0;
                    else if(nProng == 3 && nPi0 == 1) ++totTauTo3Prg1Pi0;
                    else if(nProng == 1 && nPi0 == 3) ++totTauTo1Prg3Pi0;
                    else if(nProng == 1 && nPi0 == 4) ++totTauTo1Prg4Pi0;
                    else if(nProng == 3 && nPi0 == 2) ++totTauTo3Prg2Pi0;
                    else if(nProng == 5 && nPi0 == 0) ++totTauTo5Prg0Pi0;
                    else if(nEta == 1 && nKstar == 1) ++totTauTo1Eta1Kstar;
                    else
                    {
                        ++totTauToOther;
                        for(const auto& kid : kids)
                            std::cout << "The PDGID of other hadronic decay of tau is " << kid.pdgId() << std::endl;
                    }
                }
            }

            //truth::Branch tau(&graph, p.id());
        
        }//go through all particles

        histContainer_["GenTauNum"]->Fill(nGenTau);
        histContainer_["SimTauNum"]->Fill(nSimTau);
        histContainer_["TruthTauNum"]->Fill(nTruthTau);

        histContainer_["GenPhtNum"]->Fill(nGenPht);
        histContainer_["GenPhtFTauNum"]->Fill(nGenPhtFTau);
        histContainer_["SimPhtNum"]->Fill(nSimPht);
        histContainer_["SimPhtNumHigherPt5p0"]->Fill(nSimPhtHigherPt5p0);  
        histContainer_["TruthPhtNum"]->Fill(nTruthPht);

        histContainer_["GenEleNum"]->Fill(nGenEle);
        histContainer_["SimEleNum"]->Fill(nSimEle);
        histContainer_["SimEleNumHigherPt5p0"]->Fill(nSimEleHigherPt5p0);
        histContainer_["TruthEleNum"]->Fill(nTruthEle);

        // Make one simple lookup: reconstructed detector cell -> energy.
        std::unordered_map<uint32_t, float> recEnergyByDetId;

        for (auto const& token : hgcalRecHitsToken_) {
            auto const& recHits = event.get(token);
            for (auto const& hit : recHits)
                recEnergyByDetId[hit.detid().rawId()] = hit.energy();
        }

        auto const& ecalRecHits = event.get(ecalRecHitsToken_);
        for (auto const& hit : ecalRecHits)
            recEnergyByDetId[hit.detid().rawId()] = hit.energy();

        auto const& hcalRecHits = event.get(hcalRecHitsToken_);
        for (auto const& hit : hcalRecHits)
            recEnergyByDetId[hit.id().rawId()] = hit.energy();

        for (uint32_t pid = 0; pid < hitIndex.nParticles(); ++pid)
        {
            auto const& p = graph.particle(pid);
            if (!p.valid() || std::abs(p.pdgId()) != 15)
                continue;

            auto const caloHits = subgraphHitView.subgraphHits(truth::HitChannel::Calo, pid);
            double simHitEnergy = 0.;
            double recHitEnergy = 0.;
            for (auto const& hit : caloHits)
            {
                // hit.energy is the truth energy attributed to this subgraph
                // in the cell; it is not the full reconstructed cell energy.
                simHitEnergy += hit.energy;

                auto const recHit = recEnergyByDetId.find(hit.detId);
                DetId const detectorId(hit.detId);
                if (recHit != recEnergyByDetId.end()) {
                    recHitEnergy += recHit->second;
                    std::cout << "Matched hit " << hit.detId << " in detector " << detectorId.det()
                              << " with sim energy " << hit.energy << " and rec energy " << recHit->second
                              << std::endl;
                } else {
                    std::cout << "Missing RecHit for truth hit " << hit.detId << " in detector " << detectorId.det()
                              << " with sim energy " << hit.energy << std::endl;
                }
            }

            histContainer_["TauSimHitE"]->Fill(simHitEnergy);
            histContainer_["TauRecHitE"]->Fill(recHitEnergy);

            // Calorimeter response is reconstructed energy / truth-particle
            // energy.  Protect the division for malformed/zero-energy nodes.
            const double particleEnergy = p.momentum().energy();
            if (particleEnergy > 0.) {
                const double response = recHitEnergy / particleEnergy;
                histContainer_["TauEResponse"]->Fill(response);
                histContainer2D_["TauEResponseVsEta"]->Fill(p.momentum().eta(), response);
                histContainer2D_["TauEResponseVsPt"]->Fill(p.momentum().pt(), response);
            }
        }


   }//doTenTau

   if(doDYtoLL)
   {
        for (truth::Particle p : graph.particleViews())
        {
            if(!p.valid())
                continue;
            if(std::abs(p.pdgId()) == 23 && p.hasGen())
            {
                ++totZ;
                std::vector<truth::Particle> kids = p.children();
                if(kids.size() == 1) ++totZTo1Particle;
                if(kids.size() == 2)
                {
                    if((kids[0].pdgId() + kids[1].pdgId()) != 0) continue;
                    if(std::abs(kids[0].pdgId()) == 11 && std::abs(kids[1].pdgId()) == 11) 
                    {
                        ++totZToEle;
                        histContainer_["GenEleFromZPt"]->Fill(kids[0].momentum().pt());
                        histContainer_["GenEleFromZPt"]->Fill(kids[1].momentum().pt());
                        histContainer_["GenEleFromZEta"]->Fill(kids[0].momentum().eta());
                        histContainer_["GenEleFromZEta"]->Fill(kids[1].momentum().eta());
                    }
                    else if(std::abs(kids[0].pdgId()) == 13 && std::abs(kids[1].pdgId()) == 13) 
                    {
                        ++totZToMu;
                        histContainer_["GenMuFromZPt"]->Fill(kids[0].momentum().pt());
                        histContainer_["GenMuFromZPt"]->Fill(kids[1].momentum().pt());
                        histContainer_["GenMuFromZEta"]->Fill(kids[0].momentum().eta());
                        histContainer_["GenMuFromZEta"]->Fill(kids[1].momentum().eta());
                    }
                    else if(std::abs(kids[0].pdgId()) == 15 && std::abs(kids[1].pdgId()) == 15) 
                    {
                        ++totZToTau;
                        histContainer_["GenTauFromZPt"]->Fill(kids[0].momentum().pt());
                        histContainer_["GenTauFromZPt"]->Fill(kids[1].momentum().pt());
                        histContainer_["GenTauFromZEta"]->Fill(kids[0].momentum().eta());
                        histContainer_["GenTauFromZEta"]->Fill(kids[1].momentum().eta());
                    }
                    else ++totZToNot2Leptons;

                    histContainer_["GenZMass"]->Fill((kids[0].momentum()+kids[1].momentum()).mass());
                    histContainer_["GenZPt"]->Fill((kids[0].momentum()+kids[1].momentum()).pt());
                    histContainer_["GenZEta"]->Fill((kids[0].momentum()+kids[1].momentum()).eta());

                    
                }

            }

        }//go through all the particles
   
   }//doDYtoLL

}

void MyTruthAnalyzer::endJob() {
    
    if(doTenTau)
    {
        std::cout << "========= Tau decay summary =======" << std::endl;
        std::cout << "Tau = " << totTau << std::endl;
        std::cout << "Tau--->muon = " << totTauToMu << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauToMu*1.0/totTau << std::endl;
        std::cout << "Tau--->ele = " << totTauToEle << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauToEle*1.0/totTau << std::endl;
        std::cout << "Tau--->hadron = " << totTauToHadron << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauToHadron*1.0/totTau << std::endl;
        std::cout << "========= Tau hadronic decay =======" << std::endl;
        std::cout << "Tau--->1prong + 0pi0 = " << totTauTo1Prg0Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo1Prg0Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->1prong + 1pi0 = " << totTauTo1Prg1Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo1Prg1Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->1prong + 2pi0 = " << totTauTo1Prg2Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo1Prg2Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->3prong + 0pi0 = " << totTauTo3Prg0Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo3Prg0Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->3prong + 1pi0 = " << totTauTo3Prg1Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo3Prg1Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->1prong + 3pi0 = " << totTauTo1Prg3Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo1Prg3Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->1prong + 4pi0 = " << totTauTo1Prg4Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo1Prg4Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->3prong + 2pi0 = " << totTauTo3Prg2Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo3Prg2Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->5prong + 0pi0 = " << totTauTo5Prg0Pi0 << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo5Prg0Pi0*1.0/totTau << std::endl;
        std::cout << "Tau--->1eta + 1kstar = " << totTauTo1Eta1Kstar << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauTo1Eta1Kstar*1.0/totTau << std::endl;
        std::cout << "Tau--->other = " << totTauToOther << " | " << "BF = " << std::fixed << std::setprecision(3) << totTauToOther*1.0/totTau << std::endl;
    }

    if(doDYtoLL)
    {
        std::cout << "========= Z boson decay summary ========" << std::endl;
        std::cout << "Z boson = " << totZ << std::endl;
        std::cout << "Z--->e+e- = " << totZToEle << " | " << "BF = " << std::fixed << std::setprecision(3) << totZToEle*1.0/totZ << std::endl;
        std::cout << "Z--->mu+mu- = " << totZToMu << " | " << "BF = " << std::fixed << std::setprecision(3) << totZToMu*1.0/totZ << std::endl;
        std::cout << "Z--->tau+tau- = " << totZToTau << " | " << "BF = " << std::fixed << std::setprecision(3) << totZToTau*1.0/totZ << std::endl;
        std::cout << "Z--->not2particles = " << totZToNot2Particles << " | " << "BF = " << std::fixed << std::setprecision(3) << totZToNot2Particles*1.0/totZ << std::endl;
        std::cout << "Z--->not2leptons = " << totZToNot2Leptons << " | " << "BF = " << std::fixed << std::setprecision(3) << totZToNot2Leptons*1.0/totZ << std::endl;
        std::cout << "Z--->1 particle = " << totZTo1Particle << " | " << "BF = " << std::fixed << std::setprecision(3) << totZTo1Particle*1.0/totZ << std::endl;

    }
}

DEFINE_FWK_MODULE(MyTruthAnalyzer);
