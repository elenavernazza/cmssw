#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1F.h"

#include "FWCore/Framework/interface/global/EDAnalyzer.h"
#include "SimDataFormats/TruthInfo/interface/Graph.h"
#include "SimDataFormats/TruthInfo/interface/LogicalGraphHitIndex.h"

class MyTruthAnalyzer : public edm::global::EDAnalyzer<> {
public:
    explicit MyTruthAnalyzer(edm::ParameterSet const& cfg)
        : graphToken_(consumes<truth::Graph>(cfg.getParameter<edm::InputTag>("src"))),
          hitIndexToken_(consumes<truth::LogicalGraphHitIndex>(cfg.getParameter<edm::InputTag>("hitIndex"))) 
          {
              edm::Service<TFileService> fs;
              hist_tauPt = fs->make<TH1D>("hist_tauPt", "hist_tauPt", 100, 0, 500);
              hist_nTau = fs->make<TH1D>("hist_nTau", "hist_nTau", 20, 0, 20); 
              hist_nPhoton = fs->make<TH1D>("hist_nPhoton", "hist_nPhoton", 10000, 0, 10000);
              hist_nPhotonfromTau = fs->make<TH1D>("hist_nPhotonfromTau", "hist_nPhotonfromTau", 10000, 0, 10000);
              hist_nElectron = fs->make<TH1D>("hist_nElectron", "hist_nElectron", 200, 0, 200);

          }

    void analyze(edm::StreamID, edm::Event const& event, edm::EventSetup const&) const override {
        auto const& graph = event.get(graphToken_);
        auto const& hits  = event.get(hitIndexToken_);

        int nTau = 0;
        int nPhoton = 0;
        int nPhotonfromTau = 0;
        int nElectron = 0;

        for (truth::Particle p : graph.particleViews())
        {
            if(!p.valid())
                continue;
            if(std::abs(p.pdgId()) == 22 && p.hasGen())
                ++nPhoton;
            if(std::abs(p.pdgId()) == 22 && p.hasGen() && (p.hasAncestorPdgId(15) || p.hasAncestorPdgId(15)))
                ++nPhotonfromTau;
            if(std::abs(p.pdgId()) == 11 && p.hasGen())
                ++nElectron;
            if(std::abs(p.pdgId()) == 15 && p.hasGen())
            {    
                ++nTau;
                const auto& p4 = p.momentum();
                hist_tauPt->Fill(p4.pt());
                std::cout << "pt = " << p4.pt() << std::endl;


                std::vector<truth::Particle> kids = p.children();
                for(kids)
                {

                }
            }

            //Tau decay braching fraction begin
            //std::vector<truth::Particle> par = p.parents();
            //if(par.empty())
            //    continue;
            if(std::abs(par[0].pdgId()) == 15 && par[0].hasGen())
            {
                ++totalTau;
                if(std::abs(p.pdgId()) == 13 && p.hasGen())
                    ++totalTauToMuon;
                if(std::abs(p.pdgId()) == 11 && p.hasGen())
                    ++totalTauToEle;
                if(std::abs(p.pdgId()) != 13 && std::abs(p.pdgId()) != 11 && p.hasGen())
                    ++totalTauToHadron;

            }
            //Tau decay braching fraction end



            //truth::Branch tau(&graph, p.id());
        }

        hist_nTau->Fill(nTau);
        hist_nPhoton->Fill(nPhoton);
        hist_nPhotonfromTau->Fill(nPhotonfromTau);
        hist_nElectron->Fill(nElectron);
    }

    void endJob() override
    {
        std::cout << "========= Tau decay summary =======" << std::endl;
        std::cout << "Tau = " << totalTau << std::endl;
        std::cout << "Tau--->hadron = " << totalTauToHadron << std::endl;
        std::cout << "Tau--->ele = " << totalTauToEle << std::endl;
        std::cout << "Tau--->muon = " << totalTauToMuon << std::endl;

    }

private:
    const edm::EDGetTokenT<truth::Graph> graphToken_;
    const edm::EDGetTokenT<truth::LogicalGraphHitIndex> hitIndexToken_;
    TH1D* hist_tauPt, *hist_nTau, *hist_nPhoton, *hist_nPhotonfromTau, *hist_nElectron;

    mutable int totalTau = 0, totalTauToEle = 0, totalTauToMuon = 0, totalTauToHadron = 0;
};

DEFINE_FWK_MODULE(MyTruthAnalyzer);

