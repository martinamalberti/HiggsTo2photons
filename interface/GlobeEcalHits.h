#ifndef GLOBEECALHITS_H
#define GLOBEECALHITS_H

#include "FWCore/Framework/interface/Event.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HiggsAnalysis/HiggsTo2photons/interface/Limits.h"
#include "HiggsAnalysis/HiggsTo2photons/interface/GlobeCuts.h"
#include "HiggsAnalysis/HiggsTo2photons/interface/GlobeElectrons.h"
#include "HiggsAnalysis/HiggsTo2photons/interface/GlobeMuons.h"

#include "HiggsAnalysis/HiggsTo2photons/interface/GlobePhotons.h"
#include "HiggsAnalysis/HiggsTo2photons/interface/GlobeLeptons.h"

#include "TTree.h"
#include "TClonesArray.h"
#include "TLorentzVector.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include <iostream>

class GlobeEcalHits {
 public:
  
  GlobeEcalHits(const edm::ParameterSet&, const char* n="unused");
  virtual ~GlobeEcalHits() {};

  void defineBranch(TTree* tree);
  bool analyze(const edm::Event&, const edm::EventSetup&, GlobeLeptons*, GlobeElectrons*, GlobeMuons*, GlobePhotons*);


  // variables

  TClonesArray *ecalhit_p4;
  Int_t ecalhit_n;
  Int_t ecalhit_type[MAX_ECALRECHITS];
  Int_t ecalhit_flag[MAX_ECALRECHITS];
  Float_t ecalhit_time[MAX_ECALRECHITS];
  
 private:
  const char* nome;
  bool doPreshowerHits;
  GlobeCuts *gCUT;
  edm::InputTag ecalHitEBColl, ecalHitEEColl, ecalHitESColl;
  int debug_level;
};


#endif
