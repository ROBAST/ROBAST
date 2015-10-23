// define useful unit
static const Double_t  m = AOpticsManager::m();
static const Double_t nm = AOpticsManager::nm();

Double_t multithread(Int_t nthreads)
{
  TThread::Initialize();
  AOpticsManager* manager = new AOpticsManager("manager", "multithread");
  manager->SetLimit(1000); // rays will have long track histories without being suspended

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", 10*m, 10*m, 10*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  manager->SetTopVolume(world);

  // photons are propageted inside this spherical mirror
  TGeoSphere* sphere = new TGeoSphere("sphere", 0.9*m, 1*m);
  AMirror* mirror = new AMirror("mirror", sphere);
  world->AddNode(mirror, 1);

  manager->CloseGeometry();
  // two lines are needed for multi-thread support, call after CloseGeometry
  manager->SetMultiThread(kTRUE);
  manager->SetMaxThreads(nthreads);

  ARayArray* rays = ARayShooter::RandomSphere(400*nm, 10000);

  TStopwatch watch;
  watch.Start();
  manager->TraceNonSequential(rays);
  watch.Stop();

  delete manager;
  delete rays;

  return watch.RealTime();
}

void check_scalability()
{
  TGraph* graph = new TGraph;
  TGraph* graphIdeal = new TGraph;

  for(Int_t i = 0; i < 8; i++){
    Double_t t = multithread(i + 1);
    graph->SetPoint(i, i + 1, t);
    if(i == 0){
      graphIdeal->SetPoint(i, i + 1, t);
    } else {
      graphIdeal->SetPoint(i, i + 1, graphIdeal->GetY()[0]/(i + 1));
    } // if
  } // i

  graph->Draw("a*");
  graph->GetXaxis()->SetTitle("Number of Threads");
  graph->GetYaxis()->SetTitle("Total Time (s)");
  graph->GetYaxis()->SetRangeUser(0, 10);

  graphIdeal->Draw("* same");
  graphIdeal->SetMarkerColor(2);

  TLegend* leg = new TLegend(0.5, 0.6, 0.8, 0.8);
  leg->AddEntry(graph, "Measured Time", "p");
  leg->AddEntry(graphIdeal, "Ideal Time", "p");
  leg->SetFillStyle(0);
  leg->Draw();
}
