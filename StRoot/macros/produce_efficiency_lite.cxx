 /* STAR Collaboration - Nick Elsey
  
    Example of how to use StEfficiencyMaker to produce
    efficiency measurements from miniMC files
  
    Requires the STAR libraries, miniMC and  muDST files. Defaults
    are set to a test production using the test.list file
    which should be filled with a few miniMC files before
    running :)
  
    arguments -- 
    nEvents:       number of events to reproduce
    muFileList:    list of filenames & paths to muDSTs
    mcFileList:    list of filenames & paths to corresponding miniMCs
    nametag:       identifier used in output file name
    nFiles:        number of files to accept from the file list
*/

void produce_efficiency_lite(const char* mcFileList = "mctest.list",
                             const char* nametag  = "StEfficiencyMakerLite_example",
                             int geantId = 8, // 8 = pi+, 9 = pi-
                             double dcaMax = 3.0,
                             int minFitPoints = 20,
                             double fitPtFrac = 0.52,
                             int nFiles = 5)
{
  // load STAR libraries
  gROOT->Macro("LoadLogger.C");
  gROOT->Macro("loadMuDst.C");
  gSystem->Load("StMiniMcEvent");
  gSystem->Load("libStEfficiencyMakerLite.so");

  StChain* chain = new StChain("StChain");

  // make StEfficiencyMaker
  std::ifstream file(mcFileList);
  TChain* mcChain = new TChain("StMiniMcTree");
  std::string line;
  while (std::getline(file, line)) {
    mcChain->Add(line.c_str());
  }
  std::string outname = std::string(nametag) + ".root";
  StEfficiencyMakerLite* eff_maker = new StEfficiencyMakerLite(mcChain, outname);
  eff_maker->AddGeantId(geantId);
  eff_maker->SetDCAMax(dcaMax);
  eff_maker->SetMinFitPoints(minFitPoints);
  eff_maker->SetMinFitPointFraction(fitPtFrac);

  // for each event, print the memory usage
  // helpful for debugging
  StMemStat memory;
  memory.PrintMem(NULL);
	
  if (chain->Init()) { cout<<"StChain failed init: exiting"<<endl; return;}
  cout << "chain initialized" << endl;
	
  TStopwatch total;
  TStopwatch timer;
	
  int i=0;
  while ( i < mcChain->GetEntries() && chain->Make() == kStOk ) {
    if ( i % 500 == 0 ) {
      cout<<"done with event "<<i;
      cout<<"\tcpu: "<<timer.CpuTime()<<"\treal: "<<timer.RealTime()<<"\tratio: "<<timer.CpuTime()/timer.RealTime();//<<endl;
      timer.Start();
      memory.PrintMem(NULL);
    }
    i++;
    chain->Clear();
  }
  
  chain->ls(3);
  chain->Finish();
  printf("my macro processed %i events in %s", i, nametag);
  cout << "\tcpu: " << total.CpuTime() << "\treal: " << total.RealTime() << "\tratio: " << total.CpuTime() / total.RealTime() << endl;

  cout << endl;
  cout << "--------------" << endl;
  cout << " (-: Done :-) " << endl;
  cout << "--------------" << endl;
  cout << endl;
}
