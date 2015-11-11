#include "HESS1.C"

void CORSIKA()
{
  AOpticsManager* manager = new AOpticsManager("manager", "HESS CT3 System");
  TGeoBBox* boxWorld = new TGeoBBox("boxWorld", 20*m, 20*m, 20*m);
  AOpticalComponent* world = new AOpticalComponent("world", boxWorld);
  manager->SetTopVolume(world);

  AddMirrors(world);
  AddCamera(world);
  AddMasts(world);
  manager->CloseGeometry(); // finalize the geometry construction

  ACorsikaIACTFile f;
  f.Open("muon_ring4.corsika.gz");
  ACorsikaIACTRunHeader* runheader = f.GetRunHeader();
  std::cout << "Run number: " << runheader->GetRunNumber() << std::endl;
  std::cout << "Date of run: " << runheader->GetDateOfBeginRun().AsString() << std::endl;
  std::cout << "CORSIKA version: " << runheader->GetVersionOfProgram() << std::endl;
  int nObsLevels = runheader->GetNumberOfObservationLevels();
  std::cout << "Number of observation levels: " << nObsLevels << std::endl;
  for(int i = 1; i < nObsLevels + 1; i++){
    std::cout << "Observation Level: " << runheader->GetHeightOfLevel(i)/m << std::endl;
  } // i

  f.ReadEvent(1); // event number starts from 1
  TTree* tree = f.GetBunches();

  int telescope_number = 0;
  int array_number = 0;
  double zoffset = 30*m;
  double refractive_index = 1.;
  ARayArray* array = f.GetRayArray(telescope_number, array_number, zoffset, refractive_index);

  manager->TraceNonSequential(array);
  TObjArray* focused = array->GetFocused();
  TH2D* h = new TH2D("h", ";Camera X (cm);Camera Y (cm)", 200, -50, 50, 200, -50, 50);
  
  for(int i = 0; i <= focused->GetLast(); i++){
    ARay* ray = (ARay*)(*focused)[i];
    double p[4];
    ray->GetLastPoint(p);
    h->Fill(p[0]/cm, p[1]/cm);
  } // i

  h->Draw("colz");

  f.Close();
}
