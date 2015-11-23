////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// AGeoUtil                                                                   //
//                                                                            //
// Utility functions to build complex geometries easily                       //
////////////////////////////////////////////////////////////////////////////////

#include "TGeoTube.h"
#include "TMath.h"

#include "AGeoUtil.h"

#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(AGeoUtil)
#endif

namespace AGeoUtil {

//______________________________________________________________________________
void MakePointToPointBBox(const char* name, TVector3& v1, TVector3& v2,
                          Double_t dx, Double_t dy,
                          TGeoBBox** box, TGeoCombiTrans** combi)
{
  TVector3 v3 = v1 + v2;
  v3 *= 0.5; // center of the gravity
  TVector3 v4 = v3 - v1;

  Double_t theta = v4.Theta()*TMath::RadToDeg();
  Double_t phi   = v4.Phi()*TMath::RadToDeg();

  *box = new TGeoBBox(Form("%sbox", name), dx, dy, v4.Mag());
  *combi = new TGeoCombiTrans(TGeoTranslation(v3.X(), v3.Y(), v3.Z()), TGeoRotation("", phi + 90, theta, 0));
  (*combi)->SetName(Form("%scombi", name));
  (*combi)->RegisterYourself();
}

//______________________________________________________________________________
void MakePointToPointTube(const char* name, TVector3& v1, TVector3& v2,
                          Double_t radius, TGeoTube** tube,
                          TGeoCombiTrans** combi)
{
  // Create a TGeoTube whose both ends are rotated to locate at v1 or v2 by
  // using combi
  /*
  Begin_Macro(gui, source)
  {
    Double_t m = AOpticsManager::m();
    AOpticsManager manager("manager", "manager");
    TGeoBBox* worldbox = new TGeoBBox("worldbox", 10*m, 10*m, 10*m);
    AOpticalComponent* world = new AOpticalComponent("world", worldbox);
    manager.SetTopVolume(world);

    TGeoTube* tube;
    TGeoCombiTrans* combi;
    TVector3 v1(-1*m, -2*m, 3*m);
    TVector3 v2(4*m, 5*m, -1*m);
    AGeoUtil::MakePointToPointTube("tube", v1, v2, 0.5*m, &tube, &combi);
    ALens* lens = new ALens("lens", tube);
    world->AddNode(lens, 1, combi);
    manager.CloseGeometry();

    world->Draw("ogl");
    TGLViewer* gl = (TGLViewer*)gPad->GetViewer3D();

    return gl;
  }
  End_Macro
  */
  TVector3 v3 = v1 + v2;
  v3 *= 0.5; // center of the gravity
  TVector3 v4 = v3 - v1;

  Double_t theta = v4.Theta()*TMath::RadToDeg();
  Double_t phi   = v4.Phi()*TMath::RadToDeg();

  *tube = new TGeoTube(Form("%stube", name), 0., radius, v4.Mag());
  *combi = new TGeoCombiTrans(TGeoTranslation(v3.X(), v3.Y(), v3.Z()), TGeoRotation("", phi + 90, theta, 0));
  (*combi)->SetName(Form("%scombi", name));
  (*combi)->RegisterYourself();
}

} // AGeoUtil
