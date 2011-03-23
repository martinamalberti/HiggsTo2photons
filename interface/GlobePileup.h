#ifndef GLOBEPILEUP_H
#define GLOBEPILEUP_H

#include "FWCore/Framework/interface/Event.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HiggsAnalysis/HiggsTo2photons/interface/Limits.h"

#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

#include "TTree.h"

#include <iostream>
#include <vector>

class GlobePileup {
 public:
  
  GlobePileup(const edm::ParameterSet&);
  virtual ~GlobePileup() {};

  void defineBranch(TTree* tree);
  bool analyze(const edm::Event&, const edm::EventSetup&);

  int pu_n;
  int pu_bunchcrossing;
  std::vector<float>* pu_zpos;
  std::vector<float>* pu_sumpt_lowpt;
  std::vector<float>* pu_sumpt_highpt;
  std::vector<int>* pu_ntrks_lowpt;
  std::vector<int>* pu_ntrks_highpt;

 private:
   edm::InputTag pileupCollection;
   int debug_level;
};

#endif
