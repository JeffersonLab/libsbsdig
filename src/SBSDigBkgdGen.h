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
  Double_t* NhitsBBGEM;// one number per GEM layer
  TH1D* h_EdephitBBGEM;// distribution is assumed to be the same for all layers
  TH1D** h_xhitBBGEM;// one hisogram per GEM layer
  TH1D** h_yhitBBGEM;// one hisogram per GEM layer
  TH1D** h_dxhitBBGEM;// one hisogram per GEM layer
  TH1D** h_dyhitBBGEM;// one hisogram per GEM layer

  Double_t* NhitsSBSGEM;// one number per GEM layer
  TH1D* h_EdephitSBSGEM;// distribution is assumed to be the same for all layers
  TH1D** h_xhitSBSGEM;// one hisogram per GEM layer
  TH1D** h_yhitSBSGEM;// one hisogram per GEM layer
  TH1D** h_dxhitSBSGEM;// one hisogram per GEM layer
  TH1D** h_dyhitSBSGEM;// one hisogram per GEM layer
  
  ///CEPol_GEMFront
  Double_t* NhitsCEPol_GEMFront;// one number per GEM layer
  TH1D* h_EdephitCEPol_GEMFront;// distribution is assumed to be the same for all layers
  TH1D** h_xhitCEPol_GEMFront;// one hisogram per GEM layer
  TH1D** h_yhitCEPol_GEMFront;// one hisogram per GEM layer
  TH1D** h_dxhitCEPol_GEMFront;// one hisogram per GEM layer
  TH1D** h_dyhitCEPol_GEMFront;// one hisogram per GEM layer

  ///CEPol_GEMRear
  Double_t* NhitsCEPol_GEMRear;// one number per GEM layer
  TH1D* h_EdephitCEPol_GEMRear;// distribution is assumed to be the same for all layers
  TH1D** h_xhitCEPol_GEMRear;// one hisogram per GEM layer
  TH1D** h_yhitCEPol_GEMRear;// one hisogram per GEM layer
  TH1D** h_dxhitCEPol_GEMRear;// one hisogram per GEM layer
  TH1D** h_dyhitCEPol_GEMRear;// one hisogram per GEM layer

  ///PRPOLBS_GEM
  Double_t* NhitsPrPolBS_GEM;// one number per GEM layer
  TH1D* h_EdephitPrPolBS_GEM;// distribution is assumed to be the same for all layers
  TH1D** h_xhitPrPolBS_GEM;// one hisogram per GEM layer
  TH1D** h_yhitPrPolBS_GEM;// one hisogram per GEM layer
  TH1D** h_dxhitPrPolBS_GEM;// one hisogram per GEM layer
  TH1D** h_dyhitPrPolBS_GEM;// one hisogram per GEM layer

  ///PRPOLFS_GEM
  Double_t* NhitsPrPolFS_GEM;// one number per GEM layer
  TH1D* h_EdephitPrPolFS_GEM;// distribution is assumed to be the same for all layers
  TH1D** h_xhitPrPolFS_GEM;// one hisogram per GEM layer
  TH1D** h_yhitPrPolFS_GEM;// one hisogram per GEM layer
  TH1D** h_dxhitPrPolFS_GEM;// one hisogram per GEM layer
  TH1D** h_dyhitPrPolFS_GEM;// one hisogram per GEM layer


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
  
  Double_t* NhitsPrPolBS_Scint;
  TH1D* h_EdephitPrPolBS_Scint;
  TH1D* h_xhitPrPolBS_Scint;
  
  Double_t* NhitsPrPolFS_Scint;
  TH1D* h_EdephitPrPolFS_Scint;
  TH1D* h_xhitPrPolFS_Scint;
  
  Double_t* NhitsActiveAna;
  TH1D* h_EdephitActiveAna;
  TH1D* h_xhitActiveAna;
  
  
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
  TH1D** h_NhitsBBGEM_XC;
  TH1D* h_EdephitBBGEM_XC;
  TH1D** h_xhitBBGEM_XC;
  TH1D** h_yhitBBGEM_XC;
  TH1D* h_modBBGEM_XC;
  
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

