// Author: Akira Okumura 2012/11/26

// Examples for simple optical systems which use products by Edmund Optics.

// define useful units
static const Double_t cm = AOpticsManager::cm();
static const Double_t mm = AOpticsManager::mm();
static const Double_t um = AOpticsManager::um();
static const Double_t nm = AOpticsManager::nm();
static const Double_t  m = AOpticsManager::m();

void EdmundOptics()
{
  // Aspherized achromatic lens.
  // http://www.edmundoptics.com/optics/optical-lenses/aspheric-lenses/aspherized-achromatic-lenses/2953

  // ZEMAX data is available at
  // http://www.edmundoptics.jp/techsupport/resource_center/product_docs/zmax_49665.zmx

  AOpticsManager* manager = new AOpticsManager("NT49665", "NT49665");

  // Make the world
  TGeoBBox* box = new TGeoBBox("box", 10*cm, 10*cm, 10*cm);
  AOpticalComponent *top = new AOpticalComponent("top", box);
  manager->SetTopVolume(top);

  Double_t rmax = 12.5*mm;
  Double_t rmin =   0.*mm;

  Double_t d1 = 9.0*mm;
  Double_t d2 = 2.5*mm;
  Double_t d3 = 8e-2*mm;
  Double_t d4 = 4.40605264801e1*mm;

  Double_t z1 = 0*mm;
  Double_t z2 = z1 + d1;
  Double_t z3 = z2 + d2;
  Double_t z4 = z3 + d3;
  Double_t z5 = z4 + d4;

  Double_t curv1 = 1./(28.5*mm);
  Double_t curv2 = 1./(-31.0*mm);
  Double_t curv3 = 1./(-66.0*mm);
  Double_t curv4 = 1./(-63.0*mm);

  AGeoAsphericDisk* disk1
    = new AGeoAsphericDisk("disk1", z1, curv1, z2, curv2, rmax, rmin);
  AGeoAsphericDisk* disk2
    = new AGeoAsphericDisk("disk2", z2, curv2, z3, curv3, rmax, rmin);
  AGeoAsphericDisk* disk3
    = new AGeoAsphericDisk("disk3", z3, curv3, z4, curv4, rmax, rmin);
  Double_t coefficients[3] = {0, 4.66252900195e-6/(mm*mm*mm), -8.02842124899e-9/(mm*mm*mm*mm*mm)};
  disk3->SetPolynomials(0, 0, 3, coefficients);

  // Ohara S-FSL5
  // http://www.ohara-inc.co.jp/jp/product/optical/dl/data/jsfsl05.pdf
  ASellmeierFormula* FSL5
    = new ASellmeierFormula(1.17447043e0,   1.40056154e-2, 1.19272435e0,
                            8.41855181e-3, -5.81790767e-2, 1.29599726e2);
  
  // should be 1.48749
  std::cout << FSL5->GetIndex(0.58756*um) << std::endl;

  // S-TIH13
  // http://www.ohara-inc.co.jp/jp/product/optical/dl/data/jstih13.pdf
  ASellmeierFormula* TIH13
    = new ASellmeierFormula(1.62224674e0, 2.93844589e-1, 1.99225164e0,
                            1.18368386e-2,5.90208025e-2, 1.71959976e2);

  // should be 1.74077
  std::cout << TIH13->GetIndex(0.58756*um) << std::endl;

  ALens* lens1 = new ALens("lens1", disk1);
  lens1->SetRefractiveIndex(FSL5);
  ALens* lens2 = new ALens("lens2", disk2);
  lens2->SetRefractiveIndex(TIH13);
  ALens* lens3 = new ALens("lens3", disk3);
  lens3->SetConstantRefractiveIndex(1.517);

  top->AddNode(lens1, 1);
  top->AddNode(lens2, 1);
  top->AddNode(lens3, 1);

  Double_t origin[3] = {0, 0, z5 + 1*um};
  TGeoBBox* box2 = new TGeoBBox("box2", 1*mm, 1*mm, 1*um, origin);
  AFocalSurface* screen = new AFocalSurface("screen", box2);
  top->AddNode(screen, 1);

  manager->CloseGeometry();

  top->Draw("ogl");

  TGeoRotation* rot = new TGeoRotation;
  rot->SetAngles(180., 0., 0.);

  TGeoTranslation* tr = new TGeoTranslation("tr", 0, 0, -15*mm);

  const Int_t nColor = 3;
  Double_t wavelength[nColor] = {486.1*nm, 587.6*nm, 656.3*nm}; // n_F, n_d, n_C
  TH2D* spot[nColor];

  for(Int_t j = 0; j < nColor; j++){
    ARayArray* array = ARayShooter::Circle(wavelength[j], rmax*1.1, 50, 4, rot, tr);

    manager->TraceNonSequential(*array);

    spot[j] = new TH2D(Form("spot%d", j), Form("Spot Diagram for #it{#lambda} = %5.1f (nm);X (#it{#mu}m);Y (#it{#mu}m)", wavelength[j]/nm), 500, -25, 25, 500, -25, 25);

    TObjArray* focused = array->GetFocused();
    
    for(Int_t i = 0; i <= focused->GetLast(); i++){
      ARay* ray = (ARay*)(*focused)[i];
      if(j == 1 && i%10 == 0){
        ray->MakePolyLine3D()->Draw();
      } // if
      
      Double_t p[4];    
      ray->GetLastPoint(p);
      
      spot[j]->Fill(p[0]/um, p[1]/um);
    } // i
    
    delete array;
  } // j

  TCanvas* can = new TCanvas("can", "can", 1200, 400);
  can->Divide(3, 1, 1e-10, 1e-10);

  for(int i = 0; i < nColor; i++){
    can->cd(i + 1);
    spot[i]->Draw("colz");
  } // i
}
