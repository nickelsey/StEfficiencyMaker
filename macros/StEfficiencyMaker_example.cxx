 /* STAR Collaboration - Nick Elsey
  
    Example of how to use StEfficiencyMaker to produce
    efficiency measurements from miniMC files
  
    Requires the STAR libraries, miniMC and  muDST files. Defaults
    are set to a test production using the test.list file
    which should be filled with a few miniMC files before
    running :)
  
    arguments -- 
    nEvents:    number of events to reproduce
    filelist:   list of filenames & paths to muDSTs
    nametag:    identifier used in output file name
    nFiles:     number of files to accept from the file list
*/

void StEfficiencyMaker_example(int nEvents = 1e3,
                              const char* muFileList = "mutest.list",
                              const char* mcFileList = "mctest.list",
                              const char* nametag  = "StEfficiencyMaker_example",
                              int nFiles = 5)
{
  // load STAR libraries
  gROOT->Macro( "LoadLogger.C" );
  gROOT->Macro( "loadMuDst.C" );
  gSystem->Load("StMiniMcEvent");
  gSystem->Load("libStEfficiencyMaker.so");
  
//  gSystem->Load( "StarMagField.so" );
//  gSystem->Load( "StMagF" );
//  gSystem->Load( "StDetectorDbMaker" );
//  gSystem->Load( "StTpcDb" );
//  gSystem->Load( "St_db_Maker" );
//  gSystem->Load( "StDbUtilities" );
//  gSystem->Load( "StMcEvent" );
//  gSystem->Load( "StMcEventMaker" );
//  gSystem->Load( "StDaqLib" );
//  gSystem->Load( "StEmcRawMaker" );
//  gSystem->Load( "StEmcADCtoEMaker" );
//  gSystem->Load( "StEpcMaker" );
//  gSystem->Load( "StTriggerUtilities" );
//  gSystem->Load( "StDbBroker" );
//  gSystem->Load( "libgeometry_Tables" );
//  gSystem->Load( "StEEmcUtil" );
//  gSystem->Load( "StEEmcDbMaker" );
//  gSystem->Load( "StPreEclMaker" );
//  gSystem->Load( "StEpcMaker" );
//  gSystem->Load("StPicoDstMaker.so");
//  gSystem->Load("StPicoEvent.so");
//  // load local StRefMultCorr
//  gSystem->Load( "libStRefMultCorr.so" );
//  // load local TStarJetPico library & its maker
//  gSystem->Load( "libTStarJetPico.so" );
//  gSystem->Load( "libTStarJetPicoMaker.so" );
	
  StChain* chain           = new StChain( "StChain" );
  
  StMuDstMaker* muDstMaker = new StMuDstMaker( 0, 0, "", muFileList, "", nFiles );
//  St_db_Maker *dbMaker     = new St_db_Maker( "StarDb", "MySQL:StarDb" );
//  StEEmcDbMaker* eemcb     = new StEEmcDbMaker( "eemcDb" );
//  StEmcADCtoEMaker *adc    = new StEmcADCtoEMaker();
//  StPreEclMaker *pre_ecl   = new StPreEclMaker();
//  StEpcMaker *epc          = new StEpcMaker();
  
//  // get control table so we can turn off BPRS zero-suppression and save hits from "bad" caps
//  controlADCtoE_st* control_table  = adc->getControlTable();
//  control_table->CutOff[1]         = -1;
//  control_table->CutOffType[1]     = 0;
//  control_table->DeductPedestal[1] = 2;
//  adc->saveAllStEvent( kTRUE );
  
//  // simulates a trigger response based on an ADC value & trigger definitions
//  StTriggerSimuMaker* trigsim = new StTriggerSimuMaker();
//  trigsim->setMC( false );
//  trigsim->useBemc();
//  trigsim->useEemc();
//  trigsim->useBbc();
//  trigsim->useOnlineDB();
//  trigsim->bemc->setConfig( StBemcTriggerSimu::kOffline );
  
  // make StEfficiencyMaker
  std::ifstream file(mcFileList);
  TChain* mcChain = new TChain("StMiniMcTree");
  std::string line;
  while (std::getline(file, line)) {
    mcChain->Add(line.c_str());
  }
  StEfficiencyMaker* eff_maker = new StEfficiencyMaker(mcChain, "test.root");

  // for each event, print the memory usage
  // helpful for debugging
  StMemStat memory;
  memory.PrintMem( NULL );
	
  if ( chain->Init() ) { cout<<"StChain failed init: exiting"<<endl; return;}
  cout << "chain initialized" << endl;
	
  TStopwatch total;
  TStopwatch timer;
	
  int i=0;
  while ( i < nEvents && chain->Make() == kStOk ) {
    if ( i % 500 == 0 ) {
      cout<<"done with event "<<i;
      cout<<"\tcpu: "<<timer.CpuTime()<<"\treal: "<<timer.RealTime()<<"\tratio: "<<timer.CpuTime()/timer.RealTime();//<<endl;
      timer.Start();
      memory.PrintMem( NULL );
    }
    i++;
    chain->Clear();
  }
  
  chain->ls( 3 );
  chain->Finish();
  printf( "my macro processed %i events in %s", i, nametag );
  cout << "\tcpu: " << total.CpuTime() << "\treal: " << total.RealTime() << "\tratio: " << total.CpuTime() / total.RealTime() << endl;

  cout << endl;
  cout << "-------------" << endl;
  cout << "(-: Done :-) " << endl;
  cout << "-------------" << endl;
  cout << endl;
}
