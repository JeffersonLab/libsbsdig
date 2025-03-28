#include "SBSDigPMTSignal.h"
#include "TMath.h"
#include "TFormula.h"
#include "g4sbs_types.h"

using namespace std;

//
// Class SPEModel
//
SPEModel::SPEModel()
{
  fPulseHisto = new TH1D("fPulseHisto", "", 1000, -50, 50);
}

SPEModel::SPEModel(UShort_t uniqueid, double sigma, 
		   double t0, double tmin, double tmax)
{
  //TF1 fFunc("fFunc", "landaun", tmin, tmax);//garbage (sorry)
  // power law x exp decay...
  TF1 fFunc("fFunc", 
	    "TMath::Max(0., [0]*((x-[1]+[2]*0.4)/([2]*[2]*0.16))*TMath::Exp(-(x-[1]+[2]*0.4)/([2]*0.4)) )", 
	    tmin, tmax); 
  
  fFunc.SetParameters(1., t0, sigma);
  const int NbinsTotal = int(tmax-tmin)*20;// 20 bins/ns should do... since we will extrapolate after...
  //let's try 20...
  fPulseHisto = new TH1D(Form("fPulseHisto_%d", uniqueid), "", NbinsTotal, tmin, tmax);
  double t_i;//, t_j;
  double ps_i;//, g_j;
  for(int i = 1; i<=NbinsTotal; i++){
    t_i = fPulseHisto->GetBinCenter(i+1);
    ps_i = fFunc.Eval(t_i);
    fPulseHisto->Fill(t_i, ps_i);
    //cout << fPulseHisto->GetBinContent(i) << " ";
  }
  //cout << endl;
}

SPEModel::~SPEModel()
{
  fPulseHisto->Delete();
}

double SPEModel::Integral(int binmin, int binmax)
{
  binmax = max(binmax, 0);
  return fPulseHisto->Integral(binmin, binmax, "width");
}

bool SPEModel::PulseOverThr(double charge, double thr)
{
  if(fPulseHisto->GetMaximum()<thr/charge){
    return false;
  }else{
    return true;
  }
};
 
bool SPEModel::FindLeadTrailTime(double charge, double thr, double &t_lead, double &t_trail)
{
  //cout << charge << " " << thr << endl;  
  if(!PulseOverThr(charge, thr)){
    t_lead = 1.0e38;
    t_trail = 1.0e38;
    return false;
  }else{
    double xmax = fPulseHisto->GetBinCenter(fPulseHisto->GetMaximumBin());
    //cout << fPulseHisto->GetBinContent(1)<< " " << thr/charge << " " << fPulseHisto->GetBinContent(fPulseHisto->GetNbinsX())<< endl; 
    if(fPulseHisto->GetBinContent(1)<thr/charge){
      t_lead = GetHistoX(thr/charge, fPulseHisto->GetBinLowEdge(1), xmax);
    }else{
      t_lead = 1.0e38;
      return false;
    }
    if(fPulseHisto->GetBinContent(fPulseHisto->GetNbinsX())<thr/charge){
      t_trail = GetHistoX(thr/charge, xmax, fPulseHisto->GetBinLowEdge(fPulseHisto->GetNbinsX()+1));
    }else{
      t_trail = 1.0e38;
      return false;
    }
    //why does this function seem to oblitarate fMCHit containers when t_trail is calculated to be 1.e38... GetHistoX?
    if(t_trail>1.e30) cout << "thr/charge" << thr/charge << " >? " <<  fPulseHisto->GetBinContent(fPulseHisto->GetNbinsX()) << " or " << fPulseHisto->GetBinContent(fPulseHisto->GetNbinsX()+1) << endl;
    return true;
  }
}

bool SPEModel::FindPeakTimeAmp(double charge, double thr, double &amp_peak, double &t_peak)
{
  if(!PulseOverThr(charge, thr)){
    t_peak = 1.0e38;
    amp_peak = 1.0e38;
    return false;
  }else{
    amp_peak = fPulseHisto->GetMaximum()*charge;
    //Really necessary??? the time peak is at zero by definition of the reference histogram...
    /*
    int binmax = fPulseHisto->GetMaximumBin();
    t_peak = fPulseHisto->GetBinCenter(binmax-1)*fPulseHisto->GetBinContent(binmax-1)+
      amp_peak*fPulseHisto->GetBinCenter(binmax)+
      fPulseHisto->GetBinCenter(binmax+1)*fPulseHisto->GetBinContent(binmax+1);
    t_peak/= (fPulseHisto->GetBinContent(binmax-1)+
	      amp_peak+
	      fPulseHisto->GetBinContent(binmax+1));
    */
    t_peak = 0.0;//...
  }
}

double SPEModel::GetHistoX(double y, double x1, double x2)
{
  double splineslope;
  for(int k = fPulseHisto->FindBin(x1); k<=fPulseHisto->FindBin(x2); k++){
    if(  ( (fPulseHisto->GetBinContent(k+1)-y)*(fPulseHisto->GetBinContent(k)-y) )<0 ){
      // threshold crossed if diff(TH1::GetBinContent-y) changes sign
      splineslope = (fPulseHisto->GetBinContent(k+1)-fPulseHisto->GetBinContent(k))/(fPulseHisto->GetBinCenter(k+1)-fPulseHisto->GetBinCenter(k));
      
      return fPulseHisto->GetBinCenter(k)+(y-fPulseHisto->GetBinContent(k))/splineslope;
    }
  }
  return 1.0e38;
}

//
// Class PMTSignal
//
PMTSignal::PMTSignal()
  : fSumEdep(0), fNpe(0), fNpeChargeConv(1.0), fADC(0), fEventTime(0), fNorm(0), ft0(0), ftau(0), fNADCSamps(0), fNSamps(0)
{ 
  fLeadTimes.clear();
  fTrailTimes.clear();
  fTDCs.clear();
  
  fPeakAmps.clear();
  //f1 = 0;
  //R = TRndmManager::GetInstance();
}

PMTSignal::PMTSignal(double npechargeconv)
  : fSumEdep(0), fNpe(0), fNpeChargeConv(npechargeconv), fADC(0), fEventTime(0), fNorm(0), ft0(0), ftau(0), fNADCSamps(0), fNSamps(0)
{ 
  fLeadTimes.clear();
  fTrailTimes.clear();
  fTDCs.clear();
  
  fPeakAmps.clear();
  //f1 = 0;
}

void PMTSignal::Fill(SPEModel *model, int npe, double thr, double evttime, int signal)
{
  if(signal==0)fEventTime = evttime;
  fNpe+= npe;
  //if(model->PulseOverThr(fCharge, thr))fNpe//fADC = model->GetCharge()*model->GetADCconversion();
    
  //determine lead and trail times
  double t_lead, t_trail;
  // find the lead and trail time for *this* pulse, not the total pulse
  // the following line slows the digitization in the case of full background. 
  // Needs improvement ASAP!
  bool goodtime = 
    model->FindLeadTrailTime(npe*fNpeChargeConv, thr, t_lead, t_trail);
  
  //if(goodtime)cout << evttime << " " << t_lead << " " << t_trail << endl;
  
  t_lead+=evttime;
  t_trail+=evttime;
  
  if(goodtime){
    //Filter here the lead and trail times
    if(fLeadTimes.size()>0){
      // Check if the lead and trail times are inside an existing lead time- trail time pair
      // we assume here that fLeadTimes and fTrailTimes are same size 
      // *if we do things correctly, that should be the case*
      // we shall keep lead-trail times pair in timing order
      // we neglect pulse overlaps ftm.
      if(fLeadTimes.size()!=fTrailTimes.size()){
	cout << " B - Warning: size of lead times container: " << fLeadTimes.size() 
	     << " != size of trail times container: " << fTrailTimes.size() << endl;
      }
      
      for(size_t i = 0; i<fLeadTimes.size(); i++){
	// possibility of the current pair straddling with others.... :/
	// treat those separately to simplify...
	// tL < tT_i-1 < tL_i < tT
	if(i>0){
	  if(t_lead < fTrailTimes.at(i-1) && fLeadTimes.at(i) < t_trail){
	    //do necessary substitutions first, then erase
	    // tL < tL_i-1 => tL *replaces* tL_i-1
	    if(t_lead < fLeadTimes.at(i-1)){
	      fLeadTimes.erase(fLeadTimes.begin()+i-1);
	      fLeadTimes.insert(fLeadTimes.begin()+i-1, t_lead);
	    }
	    // tT_i < tT => tT *replaces* tT_i
	    if(fTrailTimes.at(i) < t_trail){
	      fTrailTimes.erase(fTrailTimes.begin()+i);
	      fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	    }
	    fLeadTimes.erase(fLeadTimes.begin()+i);
	    fTrailTimes.erase(fTrailTimes.begin()+i-1);
	    break;
	  }
	}//end if(i>0)
	// tL < tT_i < tL_i+1 < tT
	if(i<fLeadTimes.size()-1){
	  if(t_lead < fTrailTimes.at(i) && fLeadTimes.at(i+1) < t_trail){
	    //do necessary substitutions first, then erase
	    // tL < tL_i => tL *replaces* tL_i
	    if(t_lead < fLeadTimes.at(i)){
	      fLeadTimes.erase(fLeadTimes.begin()+i);
	      fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	    }
	    // tT_i+1 < tT => tT *replaces* tT_i+1
	    if(fTrailTimes.at(i+1) < t_trail){
	      fTrailTimes.erase(fTrailTimes.begin()+i+1);
	      fTrailTimes.insert(fTrailTimes.begin()+i+1, t_trail);
	    }
	    fLeadTimes.erase(fLeadTimes.begin()+i+1);
	    fTrailTimes.erase(fTrailTimes.begin()+i);
	    break;
	  }
	}//end if(i<fLeadTimes.size()-1)
	
	// if not, 6 cases to consider:
	// tL < tT < tL_i < tT_i => both inserted *before* existing pair 
	if(t_trail < fLeadTimes.at(i)){
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail); 
	  break;
	}
	// tL < tL_i < tT < tT_i => tL *replaces* tL_i
	if(t_lead < fLeadTimes.at(i) && fLeadTimes.at(i) < t_trail && t_trail < fTrailTimes.at(i)){
	  fLeadTimes.erase(fLeadTimes.begin()+i);
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  break;
	}
	// tL_i < tL < tT < tT_i => tL *replaces* tL_i AND tT *replaces* tT_i
	if(t_lead < fLeadTimes.at(i) && fTrailTimes.at(i) < t_trail){
	  fLeadTimes.erase(fLeadTimes.begin()+i);
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  fTrailTimes.erase(fTrailTimes.begin()+i);
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	  break;
	}
	if(fLeadTimes.at(i) < t_lead && t_trail < fTrailTimes.at(i)){
	  break;
	}
	if(fLeadTimes.at(i) < t_lead && t_lead < fTrailTimes.at(i) && fTrailTimes.at(i) < t_trail){
	  fTrailTimes.erase(fTrailTimes.begin()+i);
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	}
	if(fTrailTimes.at(i) < t_lead){
	  fLeadTimes.insert(fLeadTimes.begin()+i+1, t_lead);
	  fTrailTimes.insert(fTrailTimes.begin()+i+1, t_trail);
	  break;
	}
	if(fLeadTimes.size()!=fTrailTimes.size()){
	  cout << " A - Warning: size of lead times container: " << fLeadTimes.size() 
	       << " != size of trail times container: " << fTrailTimes.size() << endl;
	}
	if(i>=fLeadTimes.size())cout << "Warning: i = " << i << " >= size of containers = " << fLeadTimes.size() << endl;
      }
    }else{
      //of course, if initial size was 0, just psuh it back
      //hopefully it will be the case most of the time
      fLeadTimes.push_back(t_lead);
      fTrailTimes.push_back(t_trail);
    }
    if(fLeadTimes.size()!=fTrailTimes.size()){
      cout << " A - Warning: size of lead times container: " << fLeadTimes.size() 
	   << " != size of trail times container: " << fTrailTimes.size() << endl;
    }
  }//end if(t_lead && t_trail<30)
}

void PMTSignal::Fill_FADCmode1(int npe, double thr, double evttime, double sigmatime, int signal)
{
  if(signal==0)fEventTime = evttime;
  fNpe+= npe;
  //if(model->PulseOverThr(fCharge, thr))fNpe//fADC = model->GetCharge()*model->GetADCconversion();
  
  //cout << evttime << " <? " << fTmin << "+" << fNSamps << "*" << fSampSize << " = " << fTmin+fNSamps*fSampSize << endl;

  if(evttime>=fTmin+fNSamps*fSampSize)return;
  
  //cout << "fillfadcmode1 " << sigmatime << endl;
  
  SetPulseParam(npe*fNpeChargeConv, evttime, sigmatime);
  
  //cout << npe*fNpeChargeConv << " " << evttime << " " << sigmatime << endl;
  
  //f1->SetParameters(npe*fNpeChargeConv, evttime, sigmatime);
  //determine lead and trail times
  double t_lead, t_trail;
  bool goodtime = false;//model->FindLeadTrailTime(npe*fNpeChargeConv, thr, t_lead, t_trail);
  // the following block slows the digitization in the case of full background. 
  // Needs improvement ASAP!
  if(fNSamps){
    fSamples[0]+= Eval(fTmin+(0.5)*fSampSize);//f1->Eval(fTmin+(0.5)*fSampSize);//*fSampSize;
    //cout << Eval(fTmin+(0.5)*fSampSize) << endl;
    //Evaluate this function might be a bit of a time drain!
    //cout << fSamples[0] << " ";
    for(int i = 1; i<fNSamps; i++){
      fSamples[i]+= Eval(fTmin+(i+0.5)*fSampSize);//f1->Eval(fTmin+(i+0.5)*fSampSize);//*fSampSize;
      //if(i>0){
      //cout << Eval(fTmin+(i+0.5)*fSampSize) << endl;
      //cout << fSamples[i] << " ";
      if(fSamples[i-1]<=thr && thr<fSamples[i]){
	t_lead = fTmin+(i-0.5)*fSampSize+fSampSize*(thr-fSamples[i-1])/(fSamples[i]-fSamples[i-1]);
	goodtime = true;
      }
      if(fSamples[i-1]>=thr && thr>fSamples[i]){
	t_trail = fTmin+(i-0.5)*fSampSize+fSampSize*(thr-fSamples[i-1])/(fSamples[i]-fSamples[i-1]);
	goodtime = true;
      }
    }
    //cout << endl;
  }
    
  // find the lead and trail time for *this* pulse, not the total pulse
  //t_lead+=evttime;
  //t_trail+=evttime;
  //if(goodtime)cout << evttime << " " << t_lead << " " << t_trail << endl;
  
  if(goodtime && signal==0){
    //Filter here the lead and trail times
    if(fLeadTimes.size()>0){
      // Check if the lead and trail times are inside an existing lead time- trail time pair
      // we assume here that fLeadTimes and fTrailTimes are same size 
      // *if we do things correctly, that should be the case*
      // we shall keep lead-trail times pair in timing order
      // we neglect pulse overlaps ftm.
      if(fLeadTimes.size()!=fTrailTimes.size()){
	cout << " B - Warning: size of lead times container: " << fLeadTimes.size() 
	     << " != size of trail times container: " << fTrailTimes.size() << endl;
      }
      
      for(size_t i = 0; i<fLeadTimes.size(); i++){
	// possibility of the current pair straddling with others.... :/
	// treat those separately to simplify...
	// tL < tT_i-1 < tL_i < tT
	if(i>0){
	  if(t_lead < fTrailTimes.at(i-1) && fLeadTimes.at(i) < t_trail){
	    //do necessary substitutions first, then erase
	    // tL < tL_i-1 => tL *replaces* tL_i-1
	    if(t_lead < fLeadTimes.at(i-1)){
	      fLeadTimes.erase(fLeadTimes.begin()+i-1);
	      fLeadTimes.insert(fLeadTimes.begin()+i-1, t_lead);
	    }
	    // tT_i < tT => tT *replaces* tT_i
	    if(fTrailTimes.at(i) < t_trail){
	      fTrailTimes.erase(fTrailTimes.begin()+i);
	      fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	    }
	    fLeadTimes.erase(fLeadTimes.begin()+i);
	    fTrailTimes.erase(fTrailTimes.begin()+i-1);
	    break;
	  }
	}//end if(i>0)
	// tL < tT_i < tL_i+1 < tT
	if(i<fLeadTimes.size()-1){
	  if(t_lead < fTrailTimes.at(i) && fLeadTimes.at(i+1) < t_trail){
	    //do necessary substitutions first, then erase
	    // tL < tL_i => tL *replaces* tL_i
	    if(t_lead < fLeadTimes.at(i)){
	      fLeadTimes.erase(fLeadTimes.begin()+i);
	      fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	    }
	    // tT_i+1 < tT => tT *replaces* tT_i+1
	    if(fTrailTimes.at(i+1) < t_trail){
	      fTrailTimes.erase(fTrailTimes.begin()+i+1);
	      fTrailTimes.insert(fTrailTimes.begin()+i+1, t_trail);
	    }
	    fLeadTimes.erase(fLeadTimes.begin()+i+1);
	    fTrailTimes.erase(fTrailTimes.begin()+i);
	    break;
	  }
	}//end if(i<fLeadTimes.size()-1)
	
	// if not, 6 cases to consider:
	// tL < tT < tL_i < tT_i => both inserted *before* existing pair 
	if(t_trail < fLeadTimes.at(i)){
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail); 
	  break;
	}
	// tL < tL_i < tT < tT_i => tL *replaces* tL_i
	if(t_lead < fLeadTimes.at(i) && fLeadTimes.at(i) < t_trail && t_trail < fTrailTimes.at(i)){
	  fLeadTimes.erase(fLeadTimes.begin()+i);
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  break;
	}
	// tL_i < tL < tT < tT_i => tL *replaces* tL_i AND tT *replaces* tT_i
	if(t_lead < fLeadTimes.at(i) && fTrailTimes.at(i) < t_trail){
	  fLeadTimes.erase(fLeadTimes.begin()+i);
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  fTrailTimes.erase(fTrailTimes.begin()+i);
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	  break;
	}
	if(fLeadTimes.at(i) < t_lead && t_trail < fTrailTimes.at(i)){
	  break;
	}
	if(fLeadTimes.at(i) < t_lead && t_lead < fTrailTimes.at(i) && fTrailTimes.at(i) < t_trail){
	  fTrailTimes.erase(fTrailTimes.begin()+i);
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	}
	if(fTrailTimes.at(i) < t_lead){
	  fLeadTimes.insert(fLeadTimes.begin()+i+1, t_lead);
	  fTrailTimes.insert(fTrailTimes.begin()+i+1, t_trail);
	  break;
	}
	if(fLeadTimes.size()!=fTrailTimes.size()){
	  cout << " A - Warning: size of lead times container: " << fLeadTimes.size() 
	       << " != size of trail times container: " << fTrailTimes.size() << endl;
	}
	if(i>=fLeadTimes.size())cout << "Warning: i = " << i << " >= size of containers = " << fLeadTimes.size() << endl;
      }
    }else{
      //of course, if initial size was 0, just psuh it back
      //hopefully it will be the case most of the time
      fLeadTimes.push_back(t_lead);
      fTrailTimes.push_back(t_trail);
    }
    if(fLeadTimes.size()!=fTrailTimes.size()){
      cout << " A - Warning: size of lead times container: " << fLeadTimes.size() 
	   << " != size of trail times container: " << fTrailTimes.size() << endl;
    }
  }//end if(t_lead && t_trail<30)
}

void PMTSignal::Fill_FADCmode7(SPEModel *model, int npe, double thr, double evttime, int signal)
{
  if(signal==0)fEventTime = evttime;
  fNpe+= npe;
  
  //Mode7 (?): pedestal, amplitude, integral, peak time (I assume there is a threshold?) 
  int evttime_offset = int(evttime/80.);
  for(int k = 0; k<fNADCSamps; k++){
    //if evttime = 0, no offset
    // fPulseHisto bears 20 bins/ns, each FADC sample is 4ns 
    // => 1 fADC sample = 80 ns
    fADCSamples[k] = 0;//model->Integral(evttime_offset+k*80, evttime_offset+(k+1)*80-1);//fPulseHisto->Integral(evttime_offset+k*80, evttime_offset+(k+1)*80, "width");
    
  }
    
  /*
  double amp_peak, t_peak;
  
  if(model->FindPeakTimeAmp(npe*fNpeChargeConv, thr, amp_peak, t_peak)){
    fLeadTimes.push_back(t_peak+evttime);
    fPeakAmps.push_back(amp_peak);
  }
  */
}


void PMTSignal::Digitize(int chan, int detid, g4sbs_tree* T, //gmn_tree* T, 
			 TRandom3* R, double ped, double ped_noise, double ADCconv, double ADCbits, double TDCconv, double TDCbits, int thr_adc)
{
  if(fNpe<=0){
    //fADC = R->Gaus(ped, ped_noise);
    return;
  }

  fADC = TMath::Nint(Charge()*1.0e15/ADCconv+R->Gaus(ped, ped_noise));

  if( fADC>UInt_t(TMath::Nint( TMath::Power(2, ADCbits) )) ){
    fADC = TMath::Nint( TMath::Power(2, ADCbits) );
  }
  
  //if(fPeakAmps.size()){
  //for(int i = 0; i<fPeakAmps.size(); i++){
  //}
  //}
  
  Int_t tdc_value;
  if(fLeadTimes.size()){
    for(size_t i = 0; i<fLeadTimes.size(); i++){
      //cout << "detid " << detid << " fLeadTimes.at(" << i << ") " << fLeadTimes.at(i) << " fTrailTimes.at(" << i << ") " << fTrailTimes.at(i) << endl;
      // trim "all" bits that are above the number of TDC bits - a couple to speed it up
      // (since TDC have a revolving clock, as far as I understand)
      // let's use an arbitrary reference time offset of 1000 (?) TDC chans before the trigger
      tdc_value = TMath::Nint((fLeadTimes.at(i))/TDCconv)+1000;
      for(int j = 30; j>=TDCbits; j--){
	tdc_value ^= ( -0 ^ tdc_value) & ( 1 << (j) );
      }
      tdc_value ^= ( -0 ^ tdc_value) & ( 1 << (31) );
      //fTDCs.insert(fTDCs.begin()+0, TMath::Nint(fLeadTimes.at(0)*diginfo.TDCConversion()));//bug!!!!
      fTDCs.push_back(tdc_value);//they're already sorted in order, presumably
      // also mark the traling time with setting bin 31 to 1 // need to reconvert then
      if(fTrailTimes.size()){
	tdc_value = TMath::Nint((fTrailTimes.at(i))/TDCconv)+1000;
	for(int j = 30; j>=TDCbits; j--){
	  tdc_value ^= ( -0 ^ tdc_value) & ( 1 << (j) );
	}
	tdc_value ^= ( -1 ^ tdc_value) & ( 1 << (31) );
	fTDCs.push_back(tdc_value);
      }
    }
  }
  //cout << "detid " << detid << " TDC size " << fTDCs.size() << endl;
  
  int i_tc = -1;
  int i_max = -1;
  double vpeak = thr_adc;//to ensure we only look for the max if the threshold is crossed
  double vmin = 0;
  
  if(fNSamps){
    fADC = 0;
    Int_t Nconv = fNSamps/fNADCSamps;
    for(int i = 0; i<fNADCSamps; i++){
      for(int j = 0; j<Nconv; j++){

	
	fADCSamples[i]+=fSamples[i*Nconv+j]*fSampSize;//renormalize the sample for the integration;
	//The purpose of the multiplication by fSampSize here is to integrate the charge within a single FADC sample. fSampSize is 0.125 ns (4.0 ns/32)
	//if(detid==ACTIVEANA_UNIQUE_DETID && TMath::IsNaN(fADCSamples[i]) )cout << " i " << i << " j " << j << ", idx " << i*Nconv+j << " size " << sizeof(fSamples) << " samp " << fSamples[i*Nconv+j] << "; ";
      }
      //Now fADCSamples[i] contains the integral charge of the sample in Coulombs
      // ADC conv = 40 fC/LSB, so FADC in LSB is sample charge (C) * 1e15 fC/C / 40 fC/LSB
      fADCSamples[i]*=1.0e15/ADCconv;
      fADCSamples[i]+=R->Gaus(ped, ped_noise);
      
      if(fADCSamples[i]>4095)fADCSamples[i] = 4095;
      
      fADC+=fADCSamples[i];
      /*
      if(i_tc<0 && fADCSamples[i]>thr_adc)i_tc = i;
      if(fADCSamples[i]>vpeak){
	vpeak = fADCSamples[i];
	i_max = i;
      }
      if(i<4)vmin+= fADCSamples[i]/4;
      */
    }
    //if(detid==ACTIVEANA_UNIQUE_DETID)cout << endl;
  }
  //fSumEdep*=1.0e9;// store in eV.
  
  //Fill in directly (hoping it takes less time...)
  
  if(detid==BBPS_UNIQUE_DETID){
    // T->Earm_BBPS_dighit_nchan++;
    // T->Earm_BBPS_dighit_chan->push_back(chan);
    // T->Earm_BBPS_dighit_adc->push_back(fADC);
    for(int i = 0; i<fNADCSamps; i++){
      T->Earm_BBPS_Dig.nchan++;
      T->Earm_BBPS_Dig.chan->push_back(chan);
      T->Earm_BBPS_Dig.adc->push_back(fADCSamples[i]);
      T->Earm_BBPS_Dig.samp->push_back(i);
      T->Earm_BBPS_Dig.tdc->push_back(-1000000);
    }
    /*
    if(fTDCs.size()){
      for(int j = 0;j<fTDCs.size(); j++){
	T->Earm_BBPS_Dig.tdc->push_back(fTDCs[j]-1000);
	if(j>1){ 
	  T->Earm_BBPS_Dig.nchan++;
	  T->Earm_BBPS_Dig.chan->push_back(chan);
	  T->Earm_BBPS_Dig.adc->push_back(-1000000);
	  T->Earm_BBPS_Dig.samp->push_back(-1000000);
	}
      }
    }else{
    */
    //}
    
    /*
    double vmid = (vpeak+vmin)/2;
    double integral = 0;
    double tf;
    int n1 = -1;
    int imin = max(i_tc-3, 0);
    int imax = min(i_tc+12, fNADCSamps-1);
    for(int i = imin; i<imax; i++){
      integral+= fADCSamples[i];
      if(n1==-1 && i<imax-1 && fADCSamples[i]<vmid && vmid<fADCSamples[i+1])n1 = i;
    }
    if(n1>=0)tf = 64*(vmid-fADCSamples[n1])/(fADCSamples[n1+1]-fADCSamples[n1]);
    
    T->Earm_BBPS_Dig.adc->push_back(integral);
    T->Earm_BBPS_Dig.ped->push_back(TMath::Nint(vmin));
    T->Earm_BBPS_Dig.amp->push_back(TMath::Nint(vpeak));
    T->Earm_BBPS_Dig.tdc->push_back(tf*TDCconv);
    */
    /*
    T->Earm_BBPS_Dig.adc->push_back(fADC);
    //if(fPeakAmps.size()!=fTDCs.size()){
    //cout << fPeakAmps.size() << " " << fTDCs.size() << endl;
    //}else 
    if(fPeakAmps.size()){
      double ped_amp;
      for(int i = 0; i<fPeakAmps.size(); i++){
	if(i>0){
	  T->Earm_BBPS_Dig.nchan++;
	  T->Earm_BBPS_Dig.chan->push_back(chan);
	  T->Earm_BBPS_Dig.adc->push_back(-1000000);
	}
	T->Earm_BBPS_Dig.tdc->push_back(fTDCs[i]-1000);
	ped_amp = R->Gaus(ped, ped_noise);
	T->Earm_BBPS_Dig.ped->push_back(TMath::Nint(ped_amp));
	T->Earm_BBPS_Dig.amp->push_back(TMath::Nint(ped_amp+fPeakAmps[i]*1.0e15/ADCconv));
      }
    }else{
      T->Earm_BBPS_Dig.tdc->push_back(-1000000);
      T->Earm_BBPS_Dig.ped->push_back(-1000000);
      T->Earm_BBPS_Dig.amp->push_back(-1000000);
    }
    */
  }
  
  if(detid==BBSH_UNIQUE_DETID){
    // T->Earm_BBSH_dighit_nchan++;
    // T->Earm_BBSH_dighit_chan->push_back(chan);
    // T->Earm_BBSH_dighit_adc->push_back(fADC);
    for(int i = 0; i<fNADCSamps; i++){
      T->Earm_BBSH_Dig.nchan++;
      T->Earm_BBSH_Dig.chan->push_back(chan);
      T->Earm_BBSH_Dig.adc->push_back(fADCSamples[i]);
      T->Earm_BBSH_Dig.samp->push_back(i);
      T->Earm_BBSH_Dig.tdc->push_back(-1000000);
    }
    /*
    if(fTDCs.size()){
      for(int j = 0;j<fTDCs.size(); j++){
	T->Earm_BBSH_Dig.tdc->push_back(fTDCs[j]-1000);
	if(j>1){ 
	  T->Earm_BBSH_Dig.nchan++;
	  T->Earm_BBSH_Dig.chan->push_back(chan);
	  T->Earm_BBSH_Dig.adc->push_back(-1000000);
	  T->Earm_BBSH_Dig.samp->push_back(-1000000);
	}
      }
    }else{
    */
    //}
    
    /*
    double vmid = (vpeak+vmin)/2;
    double integral = 0;
    double tf;
    int n1 = -1;
    int imin = max(i_tc-3, 0);
    int imax = min(i_tc+12, fNADCSamps-1);
    for(int i = imin; i<imax; i++){
      integral+= fADCSamples[i];
      if(n1==-1 && i<imax-1 && fADCSamples[i]<vmid && vmid<fADCSamples[i+1])n1 = i;
    }
    if(n1>=0)tf = 64*(vmid-fADCSamples[n1])/(fADCSamples[n1+1]-fADCSamples[n1]);
    
    T->Earm_BBSH_Dig.adc->push_back(integral);
    T->Earm_BBSH_Dig.ped->push_back(TMath::Nint(vmin));
    T->Earm_BBSH_Dig.amp->push_back(TMath::Nint(vpeak));
    T->Earm_BBSH_Dig.tdc->push_back(tf*TDCconv);
    */
    /*
    T->Earm_BBSH_Dig.adc->push_back(fADC);
    //if(fPeakAmps.size()!=fTDCs.size()){
    //cout << fPeakAmps.size() << " " << fTDCs.size() << endl;
    //}else 
    if(fPeakAmps.size()){
      double ped_amp;
      for(int i = 0; i<fPeakAmps.size(); i++){
	if(i>0){
	  T->Earm_BBSH_Dig.nchan++;
	  T->Earm_BBSH_Dig.chan->push_back(chan);
	  T->Earm_BBSH_Dig.adc->push_back(-1000000);
	}
	T->Earm_BBSH_Dig.tdc->push_back(fTDCs[i]-1000);
	ped_amp = R->Gaus(ped, ped_noise);
	T->Earm_BBSH_Dig.ped->push_back(TMath::Nint(ped_amp));
	T->Earm_BBSH_Dig.amp->push_back(TMath::Nint(ped_amp+fPeakAmps[i]*1.0e15/ADCconv));
	
      }
    }else{
      T->Earm_BBSH_Dig.tdc->push_back(-1000000);
      T->Earm_BBSH_Dig.ped->push_back(-1000000);
      T->Earm_BBSH_Dig.amp->push_back(-1000000);
    }
    */
  }
  
  if(detid==ECAL_UNIQUE_DETID){
    /*
    T->Earm_ECal_Dig.nchan++;
    T->Earm_ECal_Dig.chan->push_back(chan);
    T->Earm_ECal_Dig.adc->push_back(fADC);
    */
    for(int i = 0; i<fNADCSamps; i++){
      T->Earm_ECal_Dig.nchan++;
      T->Earm_ECal_Dig.chan->push_back(chan);
      T->Earm_ECal_Dig.adc->push_back(fADCSamples[i]);
      T->Earm_ECal_Dig.samp->push_back(i);
      T->Earm_ECal_Dig.tdc->push_back(-1000000);
    }
  }
  
  if(detid==HODO_UNIQUE_DETID){
    // T->Earm_BBHodo_dighit_nchan++;
    // T->Earm_BBHodo_dighit_chan->push_back(chan);
    // T->Earm_BBHodo_dighit_adc->push_back(fADC);
    T->Earm_BBHodo_Dig.nchan++;
    T->Earm_BBHodo_Dig.chan->push_back(chan);
    T->Earm_BBHodo_Dig.adc->push_back(fADC);
    if(fTDCs.size()){
      for(int j = 0;j<fTDCs.size(); j++){
	if(fTDCs[j] & ( 1 << (31) )){
	  fTDCs[j] ^= ( -0 ^ fTDCs[j] ) & ( 1 << (31) );
	  //T->Earm_BBHodo_dighit_tdc_t->push_back(fTDCs[j]-1000);
	  T->Earm_BBHodo_Dig.tdc_t->push_back(fTDCs[j]);
	}else{
	  //T->Earm_BBHodo_dighit_tdc_l->push_back(fTDCs[j]-1000);
	  T->Earm_BBHodo_Dig.tdc_l->push_back(fTDCs[j]);
	}
      }
      // equalize the hits:
      int max_size = max(T->Earm_BBHodo_Dig.tdc_l->size(), T->Earm_BBHodo_Dig.tdc_t->size());
      max_size = max(max_size, T->Earm_BBHodo_Dig.nchan);
      while(T->Earm_BBHodo_Dig.nchan<max_size){
	T->Earm_BBHodo_Dig.nchan++;
	T->Earm_BBHodo_Dig.chan->push_back(chan);
	T->Earm_BBHodo_Dig.adc->push_back(-1000000);
      }
      while(T->Earm_BBHodo_Dig.tdc_l->size()<max_size){
	T->Earm_BBHodo_Dig.tdc_l->push_back(-1000000);
      }
      while(T->Earm_BBHodo_Dig.tdc_t->size()<max_size){
	T->Earm_BBHodo_Dig.tdc_t->push_back(-1000000);
      }
      
    }else{
      T->Earm_BBHodo_Dig.tdc_l->push_back(-1000000);
      T->Earm_BBHodo_Dig.tdc_t->push_back(-1000000);
    }
  }
  
  if(detid==CDET_UNIQUE_DETID){
    // T->Earm_CDet_dighit_nchan++;
    // T->Earm_CDet_dighit_chan->push_back(chan);
    // T->Earm_CDet_dighit_adc->push_back(fADC);
    T->CDET_Dig.nchan++;
    T->CDET_Dig.chan->push_back(chan);
    T->CDET_Dig.adc->push_back(fADC);
    if(fTDCs.size()){
      for(int j = 0;j<fTDCs.size(); j++){
	if(fTDCs[j] & ( 1 << (31) )){
	  fTDCs[j] ^= ( -0 ^ fTDCs[j] ) & ( 1 << (31) );
	  //T->CDET_dighit_tdc_t->push_back(fTDCs[j]-1000);
	  T->CDET_Dig.tdc_t->push_back(fTDCs[j]);
	}else{
	  //T->CDET_dighit_tdc_l->push_back(fTDCs[j]-1000);
	  T->CDET_Dig.tdc_l->push_back(fTDCs[j]);
	}
      }
      // equalize the hits:
      int max_size = max(T->CDET_Dig.tdc_l->size(), T->CDET_Dig.tdc_t->size());
      max_size = max(max_size, T->CDET_Dig.nchan);
      while(T->CDET_Dig.nchan<max_size){
	T->CDET_Dig.nchan++;
	T->CDET_Dig.chan->push_back(chan);
	T->CDET_Dig.adc->push_back(-1000000);
      }
      while(T->CDET_Dig.tdc_l->size()<max_size){
	T->CDET_Dig.tdc_l->push_back(-1000000);
      }
      while(T->CDET_Dig.tdc_t->size()<max_size){
	T->CDET_Dig.tdc_t->push_back(-1000000);
      }
      
    }else{
      T->CDET_Dig.tdc_l->push_back(-1000000);
      T->CDET_Dig.tdc_t->push_back(-1000000);
    }
  }
  
  if(detid==GRINCH_UNIQUE_DETID){
    // T->Earm_GRINCH_dighit_nchan++;
    // T->Earm_GRINCH_dighit_chan->push_back(chan);
    // T->Earm_GRINCH_dighit_adc->push_back(fADC);
    T->Earm_GRINCH_Dig.nchan++;
    T->Earm_GRINCH_Dig.chan->push_back(chan);
    T->Earm_GRINCH_Dig.adc->push_back(fADC);
    if(fTDCs.size()){
      for(int j = 0;j<fTDCs.size(); j++){
	if(fTDCs[j] & ( 1 << (31) )){
	  fTDCs[j] ^= ( -0 ^ fTDCs[j] ) & ( 1 << (31) );
	  //T->Earm_GRINCH_dighit_tdc_t->push_back(fTDCs[j]-1000);
	  T->Earm_GRINCH_Dig.tdc_t->push_back(fTDCs[j]);
	}else{
	  //T->Earm_GRINCH_Dig.tdc_l->push_back(fTDCs[j]-1000);
	  T->Earm_GRINCH_Dig.tdc_l->push_back(fTDCs[j]);
	}
	// equalize the hits:
	int max_size = max(T->Earm_GRINCH_Dig.tdc_l->size(), T->Earm_GRINCH_Dig.tdc_t->size());
	max_size = max(max_size, T->Earm_GRINCH_Dig.nchan);
	while(T->Earm_GRINCH_Dig.nchan<max_size){
	  T->Earm_GRINCH_Dig.nchan++;
	  T->Earm_GRINCH_Dig.chan->push_back(chan);
	  T->Earm_GRINCH_Dig.adc->push_back(-1000000);
	}
	while(T->Earm_GRINCH_Dig.tdc_l->size()<max_size){
	  T->Earm_GRINCH_Dig.tdc_l->push_back(-1000000);
	}
	while(T->Earm_GRINCH_Dig.tdc_t->size()<max_size){
	  T->Earm_GRINCH_Dig.tdc_t->push_back(-1000000);
	}
	
      }
    }else{
      T->Earm_GRINCH_Dig.tdc_l->push_back(-1000000);
      T->Earm_GRINCH_Dig.tdc_t->push_back(-1000000);
    }
  }
  
  if(detid==HCAL_UNIQUE_DETID){
    for(int i = 0; i<fNADCSamps; i++){
      T->Harm_HCal_Dig.nchan++;
      T->Harm_HCal_Dig.chan->push_back(chan);
      T->Harm_HCal_Dig.adc->push_back(fADCSamples[i]);
      T->Harm_HCal_Dig.samp->push_back(i);
      T->Harm_HCal_Dig.tdc->push_back(-1000000);
    }
    /*
    T->Harm_HCal_Dig.adc_0->push_back(fADCSamples[0]);
    T->Harm_HCal_Dig.adc_1->push_back(fADCSamples[1]);
    T->Harm_HCal_Dig.adc_2->push_back(fADCSamples[2]);
    T->Harm_HCal_Dig.adc_3->push_back(fADCSamples[3]);
    T->Harm_HCal_Dig.adc_4->push_back(fADCSamples[4]);
    T->Harm_HCal_Dig.adc_5->push_back(fADCSamples[5]);
    T->Harm_HCal_Dig.adc_6->push_back(fADCSamples[6]);
    T->Harm_HCal_Dig.adc_7->push_back(fADCSamples[7]);
    T->Harm_HCal_Dig.adc_8->push_back(fADCSamples[8]);
    T->Harm_HCal_Dig.adc_9->push_back(fADCSamples[9]);
    T->Harm_HCal_Dig.adc_10->push_back(fADCSamples[10]);
    T->Harm_HCal_Dig.adc_11->push_back(fADCSamples[11]);
    T->Harm_HCal_Dig.adc_12->push_back(fADCSamples[12]);
    T->Harm_HCal_Dig.adc_13->push_back(fADCSamples[13]);
    T->Harm_HCal_Dig.adc_14->push_back(fADCSamples[14]);
    T->Harm_HCal_Dig.adc_15->push_back(fADCSamples[15]);
    T->Harm_HCal_Dig.adc_16->push_back(fADCSamples[16]);
    T->Harm_HCal_Dig.adc_17->push_back(fADCSamples[17]);
    T->Harm_HCal_Dig.adc_18->push_back(fADCSamples[18]);
    T->Harm_HCal_Dig.adc_19->push_back(fADCSamples[19]);
    */
    if(fTDCs.size()){
      for(int j = 0;j<fTDCs.size(); j++){
	T->Harm_HCal_Dig.tdc->push_back(fTDCs[j]);
	//if(j>1){ 
	T->Harm_HCal_Dig.nchan++;
	T->Harm_HCal_Dig.chan->push_back(chan);
	T->Harm_HCal_Dig.adc->push_back(-1000000);
	T->Harm_HCal_Dig.samp->push_back(-1000000);
	  /*
	  T->Harm_HCal_Dig.adc_0->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_1->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_2->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_3->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_4->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_5->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_6->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_7->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_8->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_9->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_10->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_11->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_12->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_13->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_14->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_15->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_16->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_17->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_18->push_back(-1000000);
	  T->Harm_HCal_Dig.adc_19->push_back(-1000000);	  
	  */
	//}
      }
    }//else{
    //T->Harm_HCal_Dig.tdc->push_back(-1000000);
    //}
  }
  
  // ** How to add a new subsystem **
  // fill the new detector output here
  //genrp Hoda Harm_PRPolScintBeamSide
  if(detid==PRPOLBS_SCINT_UNIQUE_DETID){
    // T->Harm_PRPolScintBeamSide_Dighit_nchan++;
    // T->Harm_PRPolScintBeamSide_Dighit_chan->push_back(chan);
    // T->Harm_PRPolScintBeamSide_Dighit_adc->push_back(fADC);
    T->Harm_PRPolScintBeamSide_Dig.nchan++;
    T->Harm_PRPolScintBeamSide_Dig.chan->push_back(chan);
    T->Harm_PRPolScintBeamSide_Dig.adc->push_back(fADC);
    if(fTDCs.size()){
      for(int j = 0;j<fTDCs.size(); j++){
	if(fTDCs[j] & ( 1 << (31) )){
	  fTDCs[j] ^= ( -0 ^ fTDCs[j] ) & ( 1 << (31) );
	  //T->Harm_PRPolScintBeamSide_Dighit_tdc_t->push_back(fTDCs[j]-1000);
	  T->Harm_PRPolScintBeamSide_Dig.tdc_t->push_back(fTDCs[j]);
	}else{
	  //T->Harm_PRPolScintBeamSide_Dighit_tdc_l->push_back(fTDCs[j]-1000);
	  T->Harm_PRPolScintBeamSide_Dig.tdc_l->push_back(fTDCs[j]);
	}
	// equalize the hits:
	int max_size = max(T->Harm_PRPolScintBeamSide_Dig.tdc_l->size(), T->Harm_PRPolScintBeamSide_Dig.tdc_t->size());
	max_size = max(max_size, T->Harm_PRPolScintBeamSide_Dig.nchan);
	while(T->Harm_PRPolScintBeamSide_Dig.nchan<max_size){
	  T->Harm_PRPolScintBeamSide_Dig.nchan++;
	  T->Harm_PRPolScintBeamSide_Dig.chan->push_back(chan);
	  T->Harm_PRPolScintBeamSide_Dig.adc->push_back(-1000000);
	}
	while(T->Harm_PRPolScintBeamSide_Dig.tdc_l->size()<max_size){
	  T->Harm_PRPolScintBeamSide_Dig.tdc_l->push_back(-1000000);
	}
	while(T->Harm_PRPolScintBeamSide_Dig.tdc_t->size()<max_size){
	  T->Harm_PRPolScintBeamSide_Dig.tdc_t->push_back(-1000000);
	}
	
      }
    }else{
      T->Harm_PRPolScintBeamSide_Dig.tdc_l->push_back(-1000000);
      T->Harm_PRPolScintBeamSide_Dig.tdc_t->push_back(-1000000);
    }
  }

  if(detid==PRPOLFS_SCINT_UNIQUE_DETID){
    T->Harm_PRPolScintFarSide_Dig.nchan++;
    T->Harm_PRPolScintFarSide_Dig.chan->push_back(chan);
    T->Harm_PRPolScintFarSide_Dig.adc->push_back(fADC);
    
    if(fTDCs.size()==2){
      for(int j = 0;j<fTDCs.size(); j++){
	if(fTDCs[j] & ( 1 << (31) )){
	  fTDCs[j] ^= ( -0 ^ fTDCs[j] ) & ( 1 << (31) );
	  T->Harm_PRPolScintFarSide_Dig.tdc_t->push_back(fTDCs[j]);
	}else{
	  T->Harm_PRPolScintFarSide_Dig.tdc_l->push_back(fTDCs[j]);
	}
      }
    }else{
      T->Harm_PRPolScintFarSide_Dig.tdc_l->push_back(-1000000);
      T->Harm_PRPolScintFarSide_Dig.tdc_t->push_back(-1000000);
    }

  }
 
  //genrp ActiveAna
  if(detid==ACTIVEANA_UNIQUE_DETID){
    //cout << " detector " << detid << " number of ADC samples " << fNADCSamps << endl;
    for(int i = 0; i<fNADCSamps; i++){
      T->Harm_ActAn_Dig.nchan++;
      T->Harm_ActAn_Dig.chan->push_back(chan);
      T->Harm_ActAn_Dig.adc->push_back(fADCSamples[i]);
      T->Harm_ActAn_Dig.samp->push_back(i);
      T->Harm_ActAn_Dig.tdc->push_back(-1000000);
    }
  //   T->Harm_ActAn_Dig.nchan++;
  //   T->Harm_ActAn_Dig.chan->push_back(chan);
  //   T->Harm_ActAn_Dig.adc->push_back(fADC);
  //   if(fTDCs.size()==2){
  //     for(int j = 0;j<fTDCs.size(); j++){
  // 	if(fTDCs[j] & ( 1 << (31) )){
  // 	  fTDCs[j] ^= ( -0 ^ fTDCs[j] ) & ( 1 << (31) );
  // 	  T->Harm_ActAn_Dig.tdc_t->push_back(fTDCs[j]);
  // 	}else{
  // 	  T->Harm_ActAn_Dig.tdc_l->push_back(fTDCs[j]);
  // 	}
  //     }
  //   }else{
  //     T->Harm_ActAn_Dig.tdc_l->push_back(-1000000);
  //     T->Harm_ActAn_Dig.tdc_t->push_back(-1000000);
  //   }
  }
}//

void PMTSignal::SetSamples(double tmin, double tmax, double sampsize)
{
  fTmin = tmin;
  fADCSampSize = sampsize; // = 4.0 ns for FADC
  //fSampSize = sampsize/10;//the bin size is too large for the tdc size 
  fSampSize = sampsize/32.0; // = 0.125 ns by default (bin size is similar to TDC bit size)
  fNADCSamps = round((tmax-tmin)/fADCSampSize);
  fNSamps = round((tmax-tmin)/fSampSize);
  fADCSamples = new double[fNADCSamps];
  fSamples = new double[fNSamps];
  
  //cout << fNADCSamps << " " << fNSamps << " " << fADCSampsSize << " " << fSampSize << endl;
  
  memset(fSamples, 0, fNSamps*sizeof(double));
  memset(fADCSamples, 0, fNADCSamps*sizeof(double));
  //f1 = new TF1("f1", "landaun", tmin, tmax);
  //f1 = new TF1("fFunc", 
  //"TMath::Max(0., [0]*((x-[1]+[2]*0.4)/([2]*[2]*0.16))*TMath::Exp(-(x-[1]+[2]*0.4)/([2]*0.4)) )", 
  //tmin, tmax); 
}

void PMTSignal::Clear(bool dosamples)
{
  //cout << " PMTSignal::Clear() " << endl;
  
  fSumEdep = 0;
  fNpe = 0;
  fADC = 0;
  
  fEventTime = 0;
  fLeadTimes.clear();
  fTrailTimes.clear();
  fTDCs.clear();
  
  fPeakAmps.clear();
  
  if(dosamples){
    memset(fSamples, 0, fNSamps*sizeof(double));
    memset(fADCSamples, 0, fNADCSamps*sizeof(double));
  }
}



//ClassImp(SPEModel)

