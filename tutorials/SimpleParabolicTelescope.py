import array
import ROOT

# The following lines are only needed in PyROOT
ROOT.PyConfig.StartGuiThread = 'inputhook' # for the OpenGL viewer thread
ROOT.gSystem.Load("libGeom")
ROOT.gSystem.Load("libROBAST")

# Hack to avoid a seg fault in ROOT6. To be removed at some point.
# See https://groups.cern.ch/group/roottalk/Lists/Archive/Flat.aspx?RootFolder=%2Fgroup%2Froottalk%2FLists%2FArchive%2FPyROOT%20Seg%20fault%20with%20a%20custom%20class%20%28only%20in%20ROOT6%29&FolderCTID=0x01200200A201AF59FD011C4E9284C43BF0CDA2A4
ROOT.ARefractiveIndex

for i in xrange(ROOT.gClassTable.Classes()):
    cname = ROOT.gClassTable.At(i)
    if cname[:4] == 'TGeo' or (cname[:4] == 'AGeo' and cname != 'AGeoUtil') or \
       cname in ('AOpticsManager', 'ALens', 'AMirror', 'AFocalSurface', 'AObscuration'):
        getattr(getattr(ROOT, cname), '__init__')._creates = False
# PyROOT hack ends

# define useful units
cm = ROOT.AOpticsManager.cm()
mm = ROOT.AOpticsManager.mm()
um = ROOT.AOpticsManager.um()
nm = ROOT.AOpticsManager.nm()
m  = ROOT.AOpticsManager.m()

kMirrorRad = 1.5*m
kFocalLength = 3*m
kMirrorSag = kMirrorRad*kMirrorRad/4./kFocalLength
kFocalRad = 20*cm

def MakeGeometry():
    # Make the geometry of a simple parabolic telescope
    manager = ROOT.AOpticsManager("manager", "SimpleParabolicTelescope")
    manager.SetNsegments(100) # Set the smoothness of surface drawing. Display use only.

    # Make the world
    worldbox = ROOT.TGeoBBox("worldbox", 10*m, 10*m, 10*m)
    world = ROOT.AOpticalComponent("world", worldbox)
    manager.SetTopVolume(world)

    # Define a paraboloid for the mirror
    para = ROOT.TGeoParaboloid("mirror_para", 0, kMirrorRad, kMirrorSag/2.)

    mirror_tr1 = ROOT.TGeoTranslation("mirror_tr1", 0, 0, kMirrorSag/2.)
    mirror_tr1.RegisterYourself()
    mirror_tr2 = ROOT.TGeoTranslation("mirror_tr2", 0, 0, kMirrorSag/2. - 1*um)
    mirror_tr2.RegisterYourself()

    # Composite two TGeoParaboloid to make a thin parabolic surface
    mirror_comp = ROOT.TGeoCompositeShape("mirror_comp", "mirror_para:mirror_tr2 - mirror_para:mirror_tr1")

    # Make a parabolic mirror
    mirror = ROOT.AMirror("mirror", mirror_comp)
    world.AddNode(mirror, 1)

    # Define a tube for the forcal surface
    focal_tube = ROOT.TGeoTube("focal_tube", 0, kFocalRad, 10*um)

    focal_tr = ROOT.TGeoTranslation("focal_tr", 0, 0, kFocalLength + 10*um)
    focal_tr.RegisterYourself()

    # Make a focal surface
    focal = ROOT.AFocalSurface("focal", focal_tube)
    world.AddNode(focal, 1, focal_tr)

    obs_tube1 = ROOT.TGeoTube("obs_tube1", 0, kFocalRad + 10*um, 10*um)
    obs_tr1 = ROOT.TGeoTranslation("obs_tr1", 0, 0, kFocalLength + 30*um)
    obs_tr1.RegisterYourself()

    # Make a dummy obscuration behind the focal plane
    obs1 = ROOT.AObscuration("obs1", obs_tube1)
    world.AddNode(obs1, 1, obs_tr1)

    obs_tube2 = ROOT.TGeoTube("obs_tube2", kFocalRad, kFocalRad + 10*um, 10*um)
    obs_tr2 = ROOT.TGeoTranslation("obs_tr2", 0, 0, kFocalLength + 10*um)
    obs_tr2.RegisterYourself()

    # Make one more obscuration surrounding the focal plane
    obs2 = ROOT.AObscuration("obs2", obs_tube2)
    world.AddNode(obs2, 1, obs_tr2)

    manager.CloseGeometry()

    world.Draw("ogl") # GL View does not work in PyROOT

    return manager

def SimpleParabolicTelescope():
    global hist, can_spot, leg, gra
    manager = MakeGeometry()

    kN = 30
    hist = []

    for i in range(kN):
        deg = i*0.1;
        rad = deg*ROOT.TMath.DegToRad()

        hist.append(ROOT.TH2D("hist%d" % i, "#it{#theta} = %.1f (deg);X (cm);Y (cm)" % deg, 300, -3, 3, 300, -3, 3))

        raytr = ROOT.TGeoTranslation("raytr", -kFocalLength*2*ROOT.TMath.Sin(rad), 0, kFocalLength*2*ROOT.TMath.Cos(rad))
        dir = ROOT.TVector3()
        dir.SetMagThetaPhi(1, ROOT.TMath.Pi() - rad, 0)
        wavelength = 400*nm # does not affect the results because we have no lens
        rays = ROOT.ARayShooter.Square(wavelength, 5*m, 201, 0, raytr, dir)

        manager.TraceNonSequential(rays)
        focused = rays.GetFocused()

        # Get the mean <x> and <y>
        mean = ROOT.TH2D("", "", 1, -10*m, 10*m, 1, -10*m, 10*m)
        for j in range(focused.GetLast() + 1):
            ray = focused.At(j)
            p = array.array("d", [0, 0, 0, 0])
            ray.GetLastPoint(p)
            mean.Fill(p[0], p[1])

        for j in range(focused.GetLast() + 1):
            ray = focused.At(j)
            first = ray.GetFirstPoint()
            last = array.array("d", [0, 0, 0, 0])
            ray.GetLastPoint(last)

            x = deg*10*cm
            hist[i].Fill(last[0] - mean.GetMean(1), last[1] - mean.GetMean(2))

            # Draw only some selected photons in 3D
            if ((i == 0) or (i == kN - 1)) and (ROOT.TMath.Abs(first[0]) < 1*cm or ROOT.TMath.Abs(first[1]) < 1*cm):
                pol = ray.MakePolyLine3D()
                if i == 0:
                    pol.SetLineColor(3)
                pol.Draw()

    can_spot = ROOT.TCanvas("can_spot", "can_spot", 1200, 1000)
    can_spot.Divide(6, 5, 1e-10, 1e-10)

    leg = ROOT.TLegend(0.15, 0.6, 0.5, 0.85)
    leg.SetFillStyle(0)
    title = ["#sigma_{x}", "#sigma_{y}", "#sigma"]

    gra = []
    for i in range(3):
        gra.append(ROOT.TGraph())
        gra[i].SetLineStyle(i + 1)
        gra[i].SetMarkerStyle(24 + i)
        leg.AddEntry(gra[i], title[i], "lp")

    for i in range(kN):
        deg = i*0.1
        can_spot.cd(i + 1)
        hist[i].Draw("colz")
    
        rmsx = hist[i].GetRMS(1)
        rmsy = hist[i].GetRMS(2)

        gra[0].SetPoint(i, deg, rmsx)
        gra[1].SetPoint(i, deg, rmsy)
        gra[2].SetPoint(i, deg, ROOT.TMath.Sqrt(rmsx*rmsx + rmsy*rmsy))

    can_sigma = ROOT.TCanvas("can_sigma", "can_sigma")
    gra[2].Draw("apl")
    gra[2].GetXaxis().SetTitle("Incident Angle (deg)")
    gra[2].GetYaxis().SetTitle("Spot Size (cm)")
    gra[0].Draw("pl same")
    gra[1].Draw("pl same")
    leg.Draw()

def clearCanvases():
    # Call this function to avoid a segmentation fault when exiting from Python
    [c.Close() for c in ROOT.gROOT.GetListOfCanvases()]
