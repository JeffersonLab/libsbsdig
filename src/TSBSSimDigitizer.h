#ifndef _TSBSSIMDIGITIZER_H
#define _TSBSSIMDIGITIZER_H

#include "THaAnalysisObject.h"
#include <vector>

class TSBSGeant4File;
class TSBSSimDetector;
class TSBSSimEvent;
class TFile;
class TTree;
class THaAnalysisObject;

class TSBSSimDigitizer : public THaAnalysisObject {
public:
  TSBSSimDigitizer();
  virtual ~TSBSSimDigitizer();

  // Parsses the passed file event by event and digitizes
  int Process(TSBSGeant4File *file, int max_events = 0);
  // File superposition: Procession of a stack of files ?
  //Option 1: use std::vector< std::pair<TSBSGeant4File, int> >: 
  // advantage: forces to declare a weight drawback: a bit cumbersome to setup ?
  //int Process(std::vector< std::pair<TSBSGeant4File *file, uint w> filestack, int max_events = 0); 
  //Option 2 ? just use std::vector<TSBSGeant4File>, an optional std::vector<int> for relative weight, and leave responsibility to user to set his weights correctly ?
  //int Process(std::vector< TSBSGeant4File *file > filestack, std::vector*<uint w> weightstack=0, int max_events = 0); 
  // option 3: make the file stack a member of TSBSSimDigitizer, and just "Process" it
  int Process(int max_events = 0);// Process the mmeber file stack
  
  // Add a new detector to the list
  int AddDetector(TSBSSimDetector* detector);

  // Add a new file to the file stack
  int AddInputFile(TSBSGeant4File* file, UInt_t weight = 1);
private:
  std::vector<TSBSSimDetector*> fDetectors;
  TSBSSimEvent *fEvent;
  TFile *fOutFile;
  TTree *fOutTree;
  
  //std::vector< std::pair<TSBSGeant4File*, UInt_t> > fG4FileStack;
  //Files and weights are added at the same time, and cannot be accessed from the outside... 
  // No need to inforce them to be bound together by a quirky object
  std::vector< TSBSGeant4File* > fG4FileStack_;
  // vector of vector if strings: 
  // the global vector contains the type of file, the inner vector contains the list of names of files.
  std::vector< std::vector<TString> > fG4FileStack;
  std::vector< UInt_t > fG4FileWeights;
  
  
  ClassDef(TSBSSimDigitizer,1)
};

#endif // _TSBSSIMDIGITIZER_H

