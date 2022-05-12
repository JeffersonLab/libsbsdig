#ifndef SBSDIGBKGDGEN_H
#define SBSDIGBKGDGEN_H

#include <iostream>
#include <vector>
#include <map>
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TRandom3.h"
#include "SBSDigPMTDet.h"
#include "SBSDigGEMDet.h"


//________________________________
class SBSDigBkgdGen {
  // --------------------------------
  // this class generates beam induced background hits from sampling histograms of hit energy deposit and position distributions.
  // the hit multipicity histograms are not sampled, but are fitted with a gaussian distribution.
  // the number obtained from this fit is scaled by the detector time window size divided by the equivalent time window size of the sample (provided by the input file).
 public:
  SBSDigBkgdGen();
  SBSDigBkgdGen(TFile* f_bkgd, std::vector<TString> det_list, double timewindow, bool pmtbkgddig);
  ~SBSDigBkgdGen();
  void Initialize(TFile* f_bkgd, std::vector<TString> det_list);
  
  // this function performs basically the same action as SBSDigAuxi::UnfoldData, except that instead of taking hits from an input tree, it generates hits from the histograms
  void GenerateBkgd(TRandom3* R, 
		    std::vector<SBSDigPMTDet*> pmtdets,
		    std::vector<int> detmap, 
		    std::vector<SBSDigGEMDet*> gemdets, 
		    std::vector<int> gemmap, 
		    double lumifrac);
  
  void WriteXCHistos();
  
 private:
  double fTimeWindow;
  // necessary histograms for GEMs detectors: 
  // * hit multiplicity
  // * hit energy deposit
  // * hit position (x and y): one per GEM layer
  // * hit spread (x and y): one per GEM layer
  Double_t* NhitsBBGEMs;// one number per GEM layer
  TH1D* h_EdephitBBGEMs;// distribution is assumed to be the same for all layers
  TH1D** h_xhitBBGEMs;// one hisogram per GEM layer
  TH1D** h_yhitBBGEMs;// one hisogram per GEM layer
  TH1D** h_dxhitBBGEMs;// one hisogram per GEM layer
  TH1D** h_dyhitBBGEMs;// one hisogram per GEM layer

  Double_t* NhitsSBSGEMs;// one number per GEM layer
  TH1D* h_EdephitSBSGEMs;// distribution is assumed to be the same for all layers
  TH1D** h_xhitSBSGEMs;// one hisogram per GEM layer
  TH1D** h_yhitSBSGEMs;// one hisogram per GEM layer
  TH1D** h_dxhitSBSGEMs;// one hisogram per GEM layer
  TH1D** h_dyhitSBSGEMs;// one hisogram per GEM layer
  
///CEPOL_GEMFRONT
  Double_t* NhitsCEPOL_GEMFRONTs;// one number per GEM layer
  TH1D* h_EdephitCEPOL_GEMFRONTs;// distribution is assumed to be the same for all layers
  TH1D** h_xhitCEPOL_GEMFRONTs;// one hisogram per GEM layer
  TH1D** h_yhitCEPOL_GEMFRONTs;// one hisogram per GEM layer
  TH1D** h_dxhitCEPOL_GEMFRONTs;// one hisogram per GEM layer
  TH1D** h_dyhitCEPOL_GEMFRONTs;// one hisogram per GEM layer

///CEPOL_GEMFRONT
  Double_t* NhitsCEPOL_GEMREARs;// one number per GEM layer
  TH1D* h_EdephitCEPOL_GEMREARs;// distribution is assumed to be the same for all layers
  TH1D** h_xhitCEPOL_GEMREARs;// one hisogram per GEM layer
  TH1D** h_yhitCEPOL_GEMREARs;// one hisogram per GEM layer
  TH1D** h_dxhitCEPOL_GEMREARs;// one hisogram per GEM layer
  TH1D** h_dyhitCEPOL_GEMREARs;// one hisogram per GEM layer

///PRPOLBS_GEM
  Double_t* NhitsPRPOLBS_GEMs;// one number per GEM layer
  TH1D* h_EdephitPRPOLBS_GEMs;// distribution is assumed to be the same for all layers
  TH1D** h_xhitPRPOLBS_GEMs;// one hisogram per GEM layer
  TH1D** h_yhitPRPOLBS_GEMs;// one hisogram per GEM layer
  TH1D** h_dxhitPRPOLBS_GEMs;// one hisogram per GEM layer
  TH1D** h_dyhitPRPOLBS_GEMs;// one hisogram per GEM layer

///PRPOLFS_GEM
  Double_t* NhitsPRPOLFS_GEMs;// one number per GEM layer
  TH1D* h_EdephitPRPOLFS_GEMs;// distribution is assumed to be the same for all layers
  TH1D** h_xhitPRPOLFS_GEMs;// one hisogram per GEM layer
  TH1D** h_yhitPRPOLFS_GEMs;// one hisogram per GEM layer
  TH1D** h_dxhitPRPOLFS_GEMs;// one hisogram per GEM layer
  TH1D** h_dyhitPRPOLFS_GEMs;// one hisogram per GEM layer


  // necessary histograms for calorimeter/scintillator type detectors:
  // * hit multiplicity
  // * hit energy deposit
  // * optional hit position (x or y or z)
  Double_t* NhitsHCal;// one number per block
  TH1D* h_EdephitHCal;// distribution is assumed to be the same for all blocks
  TH1D* h_zhitHCal;// distribution is assumed to be the same for all blocks
  
  Double_t* NhitsBBPS;
  TH1D* h_EdephitBBPS;
  
  Double_t* NhitsBBSH;
  TH1D* h_EdephitBBSH;
  
  Double_t* NhitsBBHodo;
  TH1D* h_EdephitBBHodo;
  TH1D* h_xhitBBHodo;
  
  Double_t* NhitsPRPOLBS_SCINT;
  TH1D* h_EdephitPRPOLBS_SCINT;
  TH1D* h_xhitPRPOLBS_SCINT;
  
  Double_t* NhitsPRPOLFS_SCINT;
  TH1D* h_EdephitPRPOLFS_SCINT;
  TH1D* h_xhitPRPOLFS_SCINT;
  
  Double_t* NhitsACTIVEANA;
  TH1D* h_EdephitACTIVEANA;
  TH1D* h_xhitACTIVEANA;
  
  
  // necessary histograms for cherenkov type detectors:
  // * probability for 1 hit
  // * probability for 2 hits (presumably more that 2 hits is very unlikely)
  // * number of photoelectrons
  Double_t* P1hitGRINCH;// one number per PMT
  Double_t* P2hitsGRINCH;// one number per PMT
  TH1D* h_NpeGRINCH;// distribution is assumed to be the same for all PMTs
  
  Double_t* NhitsECal;
  TH1D* h_EdephitECal;
  
  Double_t* NhitsCDet;
  TH1D* h_EdephitCDet;
  TH1D* h_xhitCDet;
  
  Double_t* NhitsFT;
  TH1D* h_EdephitFT;
  TH1D** h_xhitFT;
  TH1D** h_yhitFT;
  TH1D** h_dxhitFT;
  TH1D** h_dyhitFT;
  
  Double_t* NhitsFPP1;
  TH1D* h_EdephitFPP1;
  TH1D** h_xhitFPP1;
  TH1D** h_yhitFPP1;
  TH1D** h_dxhitFPP1;
  TH1D** h_dyhitFPP1;
  
  Double_t* NhitsFPP2;
  TH1D* h_EdephitFPP2;
  TH1D** h_xhitFPP2;
  TH1D** h_yhitFPP2;
  TH1D** h_dxhitFPP2;
  TH1D** h_dyhitFPP2;
  
  /*  
  //cross-check histos
  TH1D** h_NhitsBBGEMs_XC;
  TH1D* h_EdephitBBGEMs_XC;
  TH1D** h_xhitBBGEMs_XC;
  TH1D** h_yhitBBGEMs_XC;
  TH1D* h_modBBGEMs_XC;
  
  TH2D* h_NhitsHCal_XC;
  TH1D* h_EdephitHCal_XC;
  TH1D* h_zhitHCal_XC;
  
  TH2D* h_NhitsBBPS_XC;
  TH1D* h_EdephitBBPS_XC;
  
  TH2D* h_NhitsBBSH_XC;
  TH1D* h_EdephitBBSH_XC;
  
  TH2D* h_NhitsBBHodo_XC;
  TH1D* h_EdephitBBHodo_XC;
  TH1D* h_xhitBBHodo_XC;
  
  TH2D* h_NhitsGRINCH_XC;
  TH1D* h_NpeGRINCH_XC;
  */
  
  bool fPMTBkgdDig;
  
};

#endif // SBSDIGAUXI_H

