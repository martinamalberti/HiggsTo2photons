#include "HiggsAnalysis/HiggsTo2photons/interface/Limits.h"

#include "HiggsAnalysis/HiggsTo2photons/interface/GlobeMuons.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include <iostream>

GlobeMuons::GlobeMuons(const edm::ParameterSet& iConfig, const char* n): nome(n) {

  muonColl =  iConfig.getParameter<edm::InputTag>("MuonColl");
  trackColl =  iConfig.getParameter<edm::InputTag>("TrackColl");
  debug_level = iConfig.getParameter<int>("Debug_Level");
  doAodSim = iConfig.getParameter<bool>("doAodSim");
  doEgammaSummer09Skim = iConfig.getParameter<bool>("doEgammaSummer09Skim");

  vertexColl = iConfig.getParameter<edm::InputTag>("VertexColl_std");
  psetTAP = iConfig.getParameter<edm::ParameterSet>("TrackAssociatorParameters");

  theAssociatorParameters = new TrackAssociatorParameters(psetTAP);
  theAssociator = new TrackDetectorAssociator;
  theAssociator->useDefaultPropagator();

  // get cut thresholds
  gCUT = new GlobeCuts(iConfig);
}

GlobeMuons::~GlobeMuons() {
	if (theAssociatorParameters) delete theAssociatorParameters;
	if (theAssociator) delete theAssociator;
}

void GlobeMuons::defineBranch(TTree* tree) {

  mu_p4 = new TClonesArray("TLorentzVector", MAX_MUONS);
  mu_momvtx = new TClonesArray("TVector3", MAX_MUONS);
  mu_posvtx = new TClonesArray("TVector3", MAX_MUONS);
  mu_posecal = new TClonesArray("TVector3", MAX_MUONS);
  mu_poshcal = new TClonesArray("TVector3", MAX_MUONS);

  tree->Branch("mu_glo_n", &mu_n, "mu_glo_n/I");
  tree->Branch("mu_glo_p4", "TClonesArray", &mu_p4, 32000, 0);
  tree->Branch("mu_glo_momvtx", "TClonesArray", &mu_momvtx, 32000, 0);
  tree->Branch("mu_glo_posvtx", "TClonesArray", &mu_posvtx, 32000, 0);
  tree->Branch("mu_glo_posecal", "TClonesArray", &mu_posecal, 32000, 0);
  tree->Branch("mu_glo_poshcal", "TClonesArray", &mu_poshcal, 32000, 0);

  tree->Branch("mu_glo_nmatches", &mu_nmatches, "mu_glo_nmatches[mu_glo_n]/I");
  tree->Branch("mu_glo_em", &mu_em, "mu_glo_em[mu_glo_n]/F");
  tree->Branch("mu_glo_had", &mu_had, "mu_glo_had[mu_glo_n]/F");
  tree->Branch("mu_glo_ho", &mu_ho, "mu_glo_ho[mu_glo_n]/F");
  tree->Branch("mu_glo_emS9", &mu_emS9, "mu_glo_emS9[mu_glo_n]/F");
  tree->Branch("mu_glo_hadS9", &mu_hadS9, "mu_glo_hadS9[mu_glo_n]/F");
  tree->Branch("mu_glo_hoS9", &mu_hoS9, "mu_glo_hoS9[mu_glo_n]/F");
  tree->Branch("mu_glo_chi2", &mu_chi2, "mu_glo_chi2[mu_glo_n]/F");
  tree->Branch("mu_glo_dof", &mu_dof, "mu_glo_dof[mu_glo_n]/F");
  tree->Branch("mu_glo_tkind", &mu_tkind,"mu_glo_tkind[mu_glo_n]/I");
  tree->Branch("mu_glo_staind", &mu_staind,"mu_glo_staind[mu_glo_n]/I");
  tree->Branch("mu_glo_z0", &mu_z0, "mu_glo_z0[mu_glo_n]/F");
  tree->Branch("mu_glo_d0", &mu_d0, "mu_glo_d0[mu_glo_n]/F");
  tree->Branch("mu_glo_z0err", &mu_z0err, "mu_glo__z0err[mu_glo_n]/F");
  tree->Branch("mu_glo_d0err", &mu_d0err, "mu_glo_d0err[mu_glo_n]/F");
  tree->Branch("mu_glo_charge", &mu_charge, "mu_glo_charge[mu_glo_n]/I");
  tree->Branch("mu_glo_losthits", &mu_losthits, "mu_glo_losthits[mu_glo_n]/I");
  tree->Branch("mu_glo_validhits", &mu_validhits, "mu_glo_validhits[mu_glo_n]/I");
  tree->Branch("mu_glo_type", &mu_type, "mu_glo_type[mu_glo_n]/I"); 

  tree->Branch("mu_glo_iso", &mu_iso, "mu_glo_iso[mu_glo_n]/F");
  tree->Branch("mu_glo_3dip_valid", &mu_3dip_valid, "mu_glo_3dip_valid[mu_glo_n]/O");
}

bool GlobeMuons::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  
  if (debug_level > 99)
    std::cout << "GlobeMuons: Start "<< std::endl;
  // take collection
  edm::Handle<reco::MuonCollection> muH;
  iEvent.getByLabel(muonColl, muH);
  
  edm::ESHandle<MagneticField> bField;
  iSetup.get<IdealMagneticFieldRecord>().get(bField);
  
  edm::Handle<reco::TrackCollection> tkH; 
  iEvent.getByLabel(trackColl, tkH);
  
  edm::Handle<reco::VertexCollection> vtxH;
  iEvent.getByLabel(vertexColl, vtxH);
  
  mu_p4->Clear(); 
  mu_momvtx->Clear();
  mu_posvtx->Clear();
  mu_posecal->Clear();
  mu_poshcal->Clear();
  mu_n = 0;
  
  if (debug_level > 9)
    std::cout << "GlobeMuons: Muon " << nome << " Collection Size: " << muH->size() << std::endl;
  
  for(unsigned int i=0; i<muH->size(); i++) {
    reco::MuonRef m(muH, i);
    if(gCUT->cut(*m))
      continue;
    
    if (mu_n >= MAX_MUONS) {
      std::cout << "GlobeMuons: WARNING TOO MANY " << nome << " MUONS: " << muH->size() << " (allowed " << MAX_MUONS << ")" << std::endl;
      break;
    }
    
    if (debug_level > 99)
      std::cout << "GlobeMuons: Start 1"<< std::endl;
    
    new ((*mu_p4)[mu_n]) TLorentzVector();
    ((TLorentzVector *)mu_p4->At(mu_n))->SetXYZT(m->px(), m->py(), m->pz(), m->energy()); 
    
    //marco added
    new ((*mu_posvtx)[mu_n]) TVector3();
    new ((*mu_momvtx)[mu_n]) TVector3();
    
    if (m->isTrackerMuon()||m->isGlobalMuon()) {
      
      if (debug_level > 9) {
        std::cout<<"mu posvtx "<<m->track()->vx()<<" "<<m->track()->vy()<<" "<<m->track()->vz()<<" "<<std::endl;
        std::cout<<"mu momvtx "<<m->track()->px()<<" "<<m->track()->py()<<" "<<m->track()->pz()<<" "<<std::endl;
      }
      ((TVector3 *)mu_posvtx->At(mu_n))->SetXYZ(m->track()->vx(), m->track()->vy(), m->track()->vz());
      ((TVector3 *)mu_momvtx->At(mu_n))->SetXYZ(m->track()->px(), m->track()->py(), m->track()->pz());
      
    }
    else {
      //otherwise set 0,0,0 and the mu momentum
      ((TVector3 *)mu_momvtx->At(mu_n))->SetXYZ(m->px(), m->py(), m->pz());
      ((TVector3 *)mu_posvtx->At(mu_n))->SetXYZ(0., 0., 0.);
    }
    
    if (debug_level > 99)
      std::cout << "GlobeMuons: Start 2"<< std::endl;
    
    mu_charge[mu_n] = m->charge();      
    
    
    mu_type[mu_n] = 0;
    
    if (m->isGlobalMuon()) {
      mu_type[mu_n] +=1000; //was 0 but wrong because no else if
    }
    if (m->isTrackerMuon()) {
      mu_type[mu_n] +=100; //was 1 but wrong because no else if
    }
    if (m->isStandAloneMuon()) {
      mu_type[mu_n] +=10;  //was 2 but wrong because no else if
    }
    if (m->isCaloMuon()) {
      mu_type[mu_n] +=1;   //was 3 but wrong because no else if
    }
    
    if (m->isMatchesValid()) 
      mu_nmatches[mu_n] = m->numberOfMatches();
    else
      mu_nmatches[mu_n] = -1;
    
    if (debug_level > 99)
      std::cout << "GlobeMuons: Start 3"<< std::endl;
    
    //CHECK THIS ONES
    if (!m->isStandAloneMuon()) {
      mu_em[mu_n] = m->calEnergy().em;
      mu_had[mu_n] = m->calEnergy().had;
      mu_ho[mu_n] = m->calEnergy().ho;
      mu_emS9[mu_n] = m->calEnergy().emS9;
      mu_hadS9[mu_n] = m->calEnergy().hadS9;
      mu_hoS9[mu_n] = m->calEnergy().hoS9; 
    } else {
      mu_em[mu_n] = 0;
      mu_had[mu_n] = 0;
      mu_ho[mu_n] = 0;
      mu_emS9[mu_n] = 0;
      mu_hadS9[mu_n] = 0;
      mu_hoS9[mu_n] = 0;
    }
    
    if (debug_level > 99)
      std::cout << "GlobeMuons: Start 4"<< std::endl;
    
    if (m->isGlobalMuon()) { 
      mu_d0[mu_n] = m->combinedMuon()->d0();
      mu_z0[mu_n] = m->combinedMuon()->dz();
      mu_d0err[mu_n] = m->combinedMuon()->d0Error();
      mu_z0err[mu_n] = m->combinedMuon()->dzError();
      mu_chi2[mu_n] = m->combinedMuon()->chi2();
      mu_dof[mu_n] = m->combinedMuon()->ndof();
      mu_validhits[mu_n] = m->combinedMuon()->numberOfValidHits();
      mu_losthits[mu_n] = m->combinedMuon()->numberOfLostHits();    
    } else {
      mu_d0[mu_n] = -1;
      mu_z0[mu_n] = -1;
      mu_d0err[mu_n] = -1;
      mu_z0err[mu_n] = -1;
      mu_chi2[mu_n] = -1;
      mu_dof[mu_n] = -1;
      mu_validhits[mu_n] = -1;
      mu_losthits[mu_n] = -1;
    }
    
    if (debug_level > 99)
      std::cout << "GlobeMuons: Start 5"<< std::endl;
    
    if (m->isTrackerMuon()||m->isGlobalMuon()) {
      edm::Handle<reco::TrackCollection> tkH;
      iEvent.getByLabel(trackColl, tkH);
      for(unsigned int k=0; k<tkH->size(); ++k) {
        reco::TrackRef tk(tkH, k);
        if (&(*(m->track())) == (&(*tk))) {
          mu_tkind[mu_n] = k;
          break;
        }
      }
    } else {
      mu_tkind[mu_n] = -1;
    }
    
    //Isolation
    mu_iso[mu_n]=0;
    
    if (m->isTrackerMuon()||m->isGlobalMuon()) {
      for(unsigned int j=0; j<tkH->size(); ++j) {
        
        reco::TrackRef tk(tkH, j);
        
        //Doesn't pass the track quality cuts
        if (!(doAodSim || doEgammaSummer09Skim)) {
          if(vtxH->size())
            {
              reco::VertexRef vtx(vtxH, 0);
              if (debug_level > 99)
                std::cout << "GlobeMuons: Start 91"<< std::endl;
              
              if(!gCUT->isocut(*tk,*m->track(),*vtx)) continue; //is primary vertex
              
              if (debug_level > 99)
                std::cout << "GlobeMuons: Start 10"<< std::endl;
            }
          else 
            if( !gCUT->isocut(*tk,*m->track())) 
              continue; //is no primary vertex
        }
	
        //Add the pt
        if (debug_level > 99)
          std::cout << "GlobeMuons: Start 11"<< std::endl;
        mu_iso[mu_n] += tk->pt();
      }
      
      reco::TrackRef tk(tkH, mu_tkind[mu_n]);

      mu_3dip_valid[mu_n] = false;
      mu_3dip_x[mu_n]     = -9999;
      mu_3dip_y[mu_n]     = -9999;
      mu_3dip_z[mu_n]     = -9999;
      mu_3dip_xerr[mu_n]  = -9999;
      mu_3dip_yerr[mu_n]  = -9999;
      mu_3dip_zerr[mu_n]  = -9999;
    } //end if isTrackerMuon
    else {
      
      //simple straight line with vertex 000 for ecal and hcal position
      float eta=m->eta();
      float phi=m->phi();
      float the=2*atan(exp(-eta));
      the=m->theta();
      
      float ecal_r2d0 = 130.;
      float ecal_z0 = 321.;
      float ecal_eta0 = 1.6;
      
      float hcal_r2d0 = 195.;
      float hcal_z0 = 393.;
      float hcal_eta0 = 1.45;
      
      float ecal_x = 0.;
      float ecal_y = 0.;
      float ecal_z = 0.;
      float hcal_x = 0.;
      float hcal_y = 0.;
      float hcal_z = 0.;
      
      if(fabs(eta)<ecal_eta0) {
        ecal_x=ecal_r2d0*cos(phi);
        ecal_y=ecal_r2d0*sin(phi);
        if(fabs(eta)<0.00000001) 
          ecal_z=0.;
        else 
          ecal_z=ecal_r2d0/tan(the);
      }
      else {
        if(eta>0) 
          ecal_z=ecal_z0;
        else 
          ecal_z=-ecal_z0;
        ecal_x=ecal_z*tan(the)*cos(phi);
        ecal_y=ecal_z*tan(the)*sin(phi);
      }
      
      if(fabs(eta)<hcal_eta0) {
        hcal_x=hcal_r2d0*cos(phi);
        hcal_y=hcal_r2d0*sin(phi);
        if(fabs(eta)<0.00000001) 
          hcal_z=0.;
        else 
          hcal_z=hcal_r2d0/tan(the);
      }
      else {
        if(eta>0) 
          hcal_z=hcal_z0;
        else 
          hcal_z=-hcal_z0;
        hcal_x=hcal_z*tan(the)*cos(phi);
        hcal_y=hcal_z*tan(the)*sin(phi);
      }
      
      //std::cout<<"ECAL "<<mInfo.trkGlobPosAtEcal.x()<<"-"<<ecal_x<<" "<<mInfo.trkGlobPosAtEcal.y()<<"-"<<ecal_y<<" "<<mInfo.trkGlobPosAtEcal.z()<<"-"<<ecal_z<<" "<<std::endl;
      //std::cout<<"HCAL "<<mInfo.trkGlobPosAtHcal.x()<<"-"<<hcal_x<<" "<<mInfo.trkGlobPosAtHcal.y()<<"-"<<hcal_y<<" "<<mInfo.trkGlobPosAtHcal.z()<<"-"<<hcal_z<<" "<<std::endl;
      
      new ((*mu_poshcal)[mu_n]) TVector3();
      ((TVector3 *)mu_poshcal->At(mu_n))->SetXYZ(hcal_x,
                                                 hcal_y,
                                                 hcal_z);
      
      if (debug_level > 99)
        std::cout << "GlobeMuons: Start 20"<< std::endl;
      new ((*mu_posecal)[mu_n]) TVector3();
      ((TVector3 *)mu_posecal->At(mu_n))->SetXYZ(ecal_x,
                                                 ecal_y,
                                                 ecal_z);
      
    }
    
    mu_n++;
    if (debug_level > 99)
      std::cout << "GlobeMuons: End 21"<< std::endl;


  }  
  
  if (debug_level > 99)
    std::cout << "GlobeMuons: End"<< std::endl;
  return true; 
  
}


