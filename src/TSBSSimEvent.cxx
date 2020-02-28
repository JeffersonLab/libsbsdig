//*-- Author :    Ole Hansen (ole@jlab.org)    9-Dec-2011

/////////////////////////////////////////////////////////////////////
//
//   TSBSSimEvent
//
//   Common class definitions for SoLID simulation decoder
//
/////////////////////////////////////////////////////////////////////

#include "TSBSSimEvent.h"
#include "TClonesArray.h"
#include "TString.h"
#include "TMath.h"
#include "TSBSSimAuxi.h"
#include "TSBSDBManager.h"
#include "TGEMSBSDBManager.h"

#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
TSBSSimEvent::TSBSSimEvent()
  : fRunID(0), fEvtID(0), fNSignal(0)//, fMCTracks(0)
{
  cout << "Initializing TSBSSimEvent" << endl;
  fManager = TSBSDBManager::GetInstance();
  
  Clear();
}

//-----------------------------------------------------------------------------
TSBSSimEvent::TSBSSimEvent(int run, int evt, int signal)
  : fRunID(run), fEvtID(evt), fNSignal(signal)//, fMCTracks(0)
{
}

//-----------------------------------------------------------------------------
void TSBSSimEvent::Clear( const Option_t* opt )
{
  // Clear the event in preparation for reading next tree entry

  TString sopt(opt);

  fNSignal = 0;
  fDetectorData.clear();
  fSimDetectorData.clear();

  TGEMSBSDBManager* GEMDBManager;// = DetInfo_i.GetGEMDB();
  std::string fullgemname;
  
  const std::vector<TDetInfo> AllDetInfo = fManager->GetAllDetInfo();
  for(uint i = 0; i<AllDetInfo.size(); i++){
    TDetInfo DetInfo_i = AllDetInfo.at(i);
    std::string fulldetname = DetInfo_i.DetFullName();
    
    NSimDetHits[fulldetname] = 0;
    SimDetChannel[fulldetname].clear();
    SimDetSource[fulldetname].clear();
    SimDetEdep[fulldetname].clear();
    SimDetNpe[fulldetname].clear();
    SimDetTime[fulldetname].clear();
    SimDetLeadTime[fulldetname].clear();
    SimDetTrailTime[fulldetname].clear();
    
    /*
    NDetHits[fulldetname] = 0;
    DetChannel[fulldetname].clear();
    DetDataWord[fulldetname].clear();
    DetADC[fulldetname].clear();
    DetTDC_L[fulldetname].clear();
    DetTDC_T[fulldetname].clear();
    */
    
    switch(DetInfo_i.DetType()){
    case(kGEM):
      fSimGEMHitMCOutData[fulldetname].Clear();
      //fSimGEMDigOutData[fulldetname].Clear();
      GEMDBManager = DetInfo_i.GetGEMDB();
      for(int ipl = 0; ipl<GEMDBManager->GetNGEMPlane(); ipl++){
      	for(int imod = 0; imod<GEMDBManager->GetNModule(ipl); imod++){
      	  for(int ipr = 0; ipr<GEMDBManager->GetNReadOut(); ipr++){
      	    fullgemname = Form("%s.p%d.m%d.%s", 
      			       fulldetname.c_str(), 
      			       ipl+1, imod+1, kProj_str[ipr].c_str());
	    fSimDigSampOutData[fullgemname].Clear();
      	  }
      	}
      }
      break;
    case(kHCal):
      fSimHitMCOutData[fulldetname].Clear();
      fSimDigSampOutData[fulldetname].Clear();
      break;
    default:
      fSimHitMCOutData[fulldetname].Clear();
      fSimDigOutData[fulldetname].Clear();
      break;
    }
    
  }
}

//-----------------------------------------------------------------------------
void TSBSSimEvent::Print( const Option_t* opt ) const
{
  // Print current event info

  cout << ">>>>> =====================================" << endl;
  cout << "Run number:               " << fRunID << endl;
  cout << "Event number:               " << fEvtID << endl;
  cout << "Event weight:               " << fWeight << endl;
  
  // TODO: rewrite this Print function
  // TString sopt(opt);
  // bool do_all   = sopt.Contains("all",   TString::kIgnoreCase);
  // bool do_hit   = sopt.Contains("hit",   TString::kIgnoreCase) || do_all;
  // bool do_clust = sopt.Contains("clust", TString::kIgnoreCase) || do_all;
  // bool do_track = sopt.Contains("track", TString::kIgnoreCase) || do_all;
}

//-----------------------------------------------------------------------------
ClassImp(TSBSSimEvent)
