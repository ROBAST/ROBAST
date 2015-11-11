#!/usr/bin/env python

import unittest
import ROOT
import array
import time

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

cm = ROOT.AOpticsManager.cm()
mm = ROOT.AOpticsManager.mm()
um = ROOT.AOpticsManager.um()
nm = ROOT.AOpticsManager.nm()
m  = ROOT.AOpticsManager.m()
r2d = ROOT.TMath.RadToDeg()
d2r = ROOT.TMath.DegToRad()

def makeTheWorld():
    manager = ROOT.AOpticsManager("manager", "manager")
    worldbox = ROOT.TGeoBBox("worldbox", 1*m, 1*m, 1*m)
    world = ROOT.AOpticalComponent("world", worldbox)
    manager.SetTopVolume(world)
    
    return manager

class TestROBAST(unittest.TestCase):
    """
    Unit test for ROBAST
    """
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def testAbsorptionLength(self):
        manager = makeTheWorld()

        lensbox = ROOT.TGeoBBox("lensbox", 0.5*m, 0.5*m, 0.5*m)
        lens = ROOT.ALens("lens", lensbox)

        manager.GetTopVolume().AddNode(lens, 1)
        manager.CloseGeometry()
        manager.SetMultiThread(True);
        manager.SetMaxThreads(4)

        for j in range(2):
            if j == 0:
                # test a constant absorption length
                lens.SetConstantAbsorptionLength(1*mm)
            else:
                # test absorption length evaluated from a TGraph
                graph = ROOT.TGraph()
                graph.SetPoint(0, 300*nm, 0.5*mm) # 1 mm at 400 nm
                graph.SetPoint(1, 500*nm, 1.5*mm)
                lens.SetAbsorptionLength(graph)

            rays = ROOT.ARayShooter.RandomSphere(400*nm, 10000)
            manager.TraceNonSequential(rays)
            
            h = ROOT.TH1D("h", "h", 1000, 0, 10)
            
            absorbed = rays.GetAbsorbed()
            
            for i in range(absorbed.GetLast() + 1):
                ray = absorbed.At(i)
                p = array.array("d", [0, 0, 0])
                ray.GetLastPoint(p)
                d = (p[0]*p[0] + p[1]*p[1] + p[2]*p[2])**0.5
                h.Fill(d/mm)
                
            h.Draw()
            h.Fit("expo", "l")
            ROOT.gPad.Update()

            expo = h.GetFunction("expo")
            p = -expo.GetParameter(1)
            e = expo.GetParError(1)

            self.assertGreater(1, p - 3*e)
            self.assertLess(1, p + 3*e)

    def testFresnelReflection(self):
        manager = makeTheWorld()
        manager.DisableFresnelReflection(False) # enable

        lensbox = ROOT.TGeoBBox("lensbox", 0.5*m, 0.5*m, 0.5*m)
        lens = ROOT.ALens("lens", lensbox)
        lens.SetConstantAbsorptionLength(1*um)

        idx = 3.
        lens.SetConstantRefractiveIndex(idx)

        manager.GetTopVolume().AddNode(lens, 1)
        manager.CloseGeometry()
        manager.SetMultiThread(True);
        manager.SetMaxThreads(4)

        N = 10000

        rays = ROOT.ARayArray()
        for i in range(N):
            ray = ROOT.ARay(i, 400*nm, 0, 0, 0.8*m, 0, 0, 0, -1)
            rays.Add(ray)

        manager.TraceNonSequential(rays)

        n = rays.GetExited().GetLast() + 1
        ref = (idx - 1)**2/(idx + 1)**2

        self.assertGreater(ref, (n - n**0.5*3)/N)
        self.assertLess(ref, (n + n**0.5*3)/N)

    def testMirrorReflection(self):
        manager = makeTheWorld()

        mirrorbox = ROOT.TGeoBBox("mirrorbox", 0.5*m, 0.5*m, 0.5*m)
        mirror = ROOT.AMirror("mirror", mirrorbox)
        manager.GetTopVolume().AddNode(mirror, 1)
        manager.CloseGeometry()
        manager.SetMultiThread(True);
        manager.SetMaxThreads(4)
        
        graph = ROOT.TGraph()
        graph.SetPoint(0, 300*nm, 0.)
        graph.SetPoint(1, 500*nm, .5) # 0.25 at 400 nm
        mirror.SetReflectivity(graph)
        self.assertAlmostEqual(mirror.GetReflectivity(400*nm, 0), 0.25, 6)

        N = 10000

        rays = ROOT.ARayArray()
        for i in range(N):
            ray = ROOT.ARay(i, 400*nm, 0, 0, 0.8*m, 0, 0, 0, -1)
            rays.Add(ray)

        manager.TraceNonSequential(rays)

        n = rays.GetExited().GetLast() + 1
        ref = 0.25

        self.assertGreater(ref, (n - n**0.5*3)/N)
        self.assertLess(ref, (n + n**0.5*3)/N)
        
        # Test of a 2D reflectance graph
        graph = ROOT.TGraph2D()
        deg = ROOT.TMath.DegToRad()

        # This should be 0.5 at (400 nm, 45 deg)
        graph.SetPoint(0, 300*nm,  0*deg, 0.0)
        graph.SetPoint(1, 300*nm, 90*deg, 0.3)
        graph.SetPoint(2, 500*nm,  0*deg, 0.7)
        graph.SetPoint(3, 500*nm, 90*deg, 1.0)
        mirror.SetReflectivity(graph)
        self.assertAlmostEqual(mirror.GetReflectivity(400*nm, 45*deg), 0.5, 3)

        rays = ROOT.ARayArray()
        for i in range(N):
            x, y, z, t = 0, 0, 0.51*m, 0
            px, py, pz = ROOT.TMath.Sqrt2(), 0, -ROOT.TMath.Sqrt2()
            ray = ROOT.ARay(i, 400*nm, x, y, z, t, px, py, pz)
            rays.Add(ray)

        manager.TraceNonSequential(rays)

        n = rays.GetExited().GetLast() + 1
        ref = 0.5

        self.assertGreater(ref, (n - n**0.5*3)/N)
        self.assertLess(ref, (n + n**0.5*3)/N)

    def testMirrorScattaring(self):
        manager = makeTheWorld()

        mirrorbox = ROOT.TGeoBBox("mirrorbox", 0.5*m, 0.5*m, 0.5*m)
        mirror = ROOT.AMirror("mirror", mirrorbox)

        condition = ROOT.ABorderSurfaceCondition(manager.GetTopVolume(), mirror )
        sigma = 1
        condition.SetGaussianRoughness(sigma*d2r)

        manager.GetTopVolume().AddNode(mirror, 1)
        manager.CloseGeometry()
        manager.SetMultiThread(True);
        manager.SetMaxThreads(4)

        N = 10000

        rays = ROOT.ARayArray()
        for i in range(N):
            ray = ROOT.ARay(i, 400*nm, 0, 0, 0.8*m, 0, 0, 0, -1)
            rays.Add(ray)

        manager.TraceNonSequential(rays)

        exited = rays.GetExited()

        h2 = ROOT.TH2D("h2", "h2", 40, -10*sigma, 10*sigma, 40, -10*sigma, 10*sigma)

        for i in range(N):
            ray = exited.At(i)
            p = array.array("d", [0, 0, 0])
            ray.GetDirection(p)
            px = p[0]
            py = p[1]
            pz = p[2]
            h2.Fill(px*r2d, py*r2d)

        f2 = ROOT.TF2("f2", "[0]*exp(-(x*x + y*y)/(2*[1]*[1]))", -10*sigma, 10*sigma, -10*sigma, 10*sigma)
        f2.SetParameter(0, 1000)
        f2.SetParLimits(1, 0, 10)
        f2.SetParameter(1, sigma)
        h2.Draw("lego")
        ROOT.gPad.Update()
        h2.Fit("f2", "l")
        p = f2.GetParameter(1)
        e = f2.GetParError(1)

        self.assertGreater(2*sigma, p - 3*e) # reflected angle is 2 times larger
        self.assertLess(2*sigma, p + 3*e)

    def testLimitForSuspended(self):
        manager = makeTheWorld()
        manager.SetLimit(1000)

        mirrorsphere = ROOT.TGeoSphere("mirrorsphere", 0.1*m, 0.2*m)
        mirror = ROOT.AMirror("mirror", mirrorsphere)

        manager.GetTopVolume().AddNode(mirror, 1)
        manager.CloseGeometry()
        manager.SetMultiThread(True);
        manager.SetMaxThreads(4)

        ray = ROOT.ARay(0, 400*nm, 0, 0, 0, 0, 0, 0, -1)

        manager.TraceNonSequential(ray)

        n = ray.GetNpoints()
        self.assertEqual(n, 1000)

    def testSnellsLaw(self):
        manager = makeTheWorld()
        manager.SetLimit(1000)

        lensbox = ROOT.TGeoBBox("lensbox", 0.5*m, 0.5*m, 1*mm)
        lens = ROOT.ALens("lens", lensbox)

        idx = 1.5
        lens.SetConstantRefractiveIndex(idx)

        manager.GetTopVolume().AddNode(lens, 1)

        focalbox = ROOT.TGeoBBox("focalbox", 0.5*m, 0.5*m, 0.1*mm)
        focal = ROOT.AFocalSurface("focal", focalbox)
        lens.AddNode(focal, 1)

        manager.CloseGeometry()
        manager.SetMultiThread(True);
        manager.SetMaxThreads(4)
        manager.DisableFresnelReflection(True)

        theta = 30*d2r
        sint = ROOT.TMath.Sin(theta)
        cost = ROOT.TMath.Cos(theta)
        ray = ROOT.ARay(0, 400*nm, 0*m, 0*m, 2*mm, 0, sint, 0, -cost)
        arr = ROOT.ARayArray()
        arr.Add(ray)

        # calling TraceNonSequential(ARay*) causes a seg fault...
        manager.TraceNonSequential(arr)

        p = array.array("d", [0, 0, 0])
        ray.GetDirection(p)
        px = p[0]
        py = p[1]
        pz = p[2]

        self.assertAlmostEqual(px, sint/idx)
        self.assertAlmostEqual(py, 0)

    def testQE(self):
        manager = makeTheWorld()

        focalbox = ROOT.TGeoBBox("focalbox", 0.5*m, 0.5*m, 1*mm)
        focal = ROOT.AFocalSurface("focal", focalbox)

        qe_lambda = ROOT.TGraph()
        qe_lambda.SetPoint(0, 300*nm, 0.0)
        qe_lambda.SetPoint(1, 500*nm, 1.0)

        qe_angle = ROOT.TGraph()
        qe_angle.SetPoint(0,  0*d2r, 1.) # QE = 100% for on-axis photons
        qe_angle.SetPoint(1, 90*d2r, 0.)

        manager.GetTopVolume().AddNode(focal, 1)
        manager.CloseGeometry()
        manager.SetMultiThread(True)
        manager.SetMaxThreads(4)

        for i in range(3):
            if i == 1:
                focal.SetQuantumEfficiency(qe_lambda)
            elif i == 2:
                focal.SetQuantumEfficiencyAngle(qe_angle)

            array = ROOT.ARayArray()

            N = 1000**2
            raytr = ROOT.TGeoTranslation("raytr", 0, 0, 2*mm)
            direction = ROOT.TVector3(ROOT.TMath.Cos(45*d2r), 0, -ROOT.TMath.Sin(45*d2r))
            array = ROOT.ARayShooter.Square(400*nm, 1*mm, 1000, 0, raytr, direction)
            manager.TraceNonSequential(array)
        
            nfocused = array.GetFocused().GetLast() + 1
            nstopped = array.GetStopped().GetLast() + 1

            self.assertEqual(nfocused + nstopped, N)
            if i == 0:
                self.assertEqual(nfocused, N)
            elif i == 1:
                sigma = (N*(1 - 0.5)*0.5)**0.5
                self.assertLess(abs(nfocused - N/2.), 3*sigma)
            else:
                sigma = (N*(1 - 0.25)*0.25)**0.5
                self.assertLess(abs(nfocused - N/4.), 3*sigma)

    def testSellmeierFormula(self):
        # N-BK7 from a SCHOTT catalog
        nbk7 = ROOT.ASellmeierFormula(1.03961212, 0.231792344, 1.01046945,
                                      0.00600069867, 0.0200179144, 103.560653)
       
        self.assertAlmostEqual(nbk7.GetIndex( 312.6*nm), 1.548620, 4) # n312.6
        self.assertAlmostEqual(nbk7.GetIndex( 589.3*nm), 1.516730, 4) # nD
        self.assertAlmostEqual(nbk7.GetIndex(1014.0*nm), 1.507310, 4) # nt
        self.assertAlmostEqual(nbk7.GetIndex(2325.4*nm), 1.489210, 4) # n2325.4

        self.assertAlmostEqual(nbk7.GetAbbeNumber(), 64.17, 1) # vD, vd = 64.17

        data = ((2325.4*nm, 1.489210),
                (1970.1*nm, 1.494950),
                (1529.6*nm, 1.500910),
                (1060.0*nm, 1.506690),
                (1014.0*nm, 1.507310),
                ( 852.1*nm, 1.509800),
                ( 706.5*nm, 1.512890),
                ( 656.3*nm, 1.514320),
                ( 643.8*nm, 1.514720),
                ( 632.8*nm, 1.515090),
                ( 589.3*nm, 1.516730),
                ( 587.6*nm, 1.516800),
                ( 546.1*nm, 1.518720),
                ( 486.1*nm, 1.522380),
                ( 480.0*nm, 1.522830),
                ( 435.8*nm, 1.526680),
                ( 404.7*nm, 1.530240),
                ( 365.0*nm, 1.536270),
                ( 334.1*nm, 1.542720),
                ( 312.6*nm, 1.548620))

        graph = ROOT.TGraph()
        for i in range(len(data)):
            graph.SetPoint(i, data[i][0], data[i][1])

        nbk7 = ROOT.ASellmeierFormula(1.03961212*0.95, 0.231792344*0.95, 1.01046945*0.95,
                                      0.00600069867*0.95, 0.0200179144*0.95, 103.560653*0.95)

        # These comparisons should fail because B1 to C3 are scaled by 0.95
        self.assertNotAlmostEqual(nbk7.GetIndex( 312.6*nm), 1.548620, 3) # n312.6
        self.assertNotAlmostEqual(nbk7.GetIndex( 589.3*nm), 1.516730, 3) # nD
        self.assertNotAlmostEqual(nbk7.GetIndex(1014.0*nm), 1.507310, 3) # nt
        self.assertNotAlmostEqual(nbk7.GetIndex(2325.4*nm), 1.489210, 3) # n2325.4

        f = nbk7.FitData(graph, "N-BK7", "")
        graph.Draw("a*")

        # Will get an almost correct answers. "3" is due to an inperfect fitting result.
        self.assertAlmostEqual(nbk7.GetIndex( 312.6*nm), 1.548620, 3) # n312.6
        self.assertAlmostEqual(nbk7.GetIndex( 589.3*nm), 1.516730, 3) # nD
        self.assertAlmostEqual(nbk7.GetIndex(1014.0*nm), 1.507310, 3) # nt
        self.assertAlmostEqual(nbk7.GetIndex(2325.4*nm), 1.489210, 3) # n2325.4

        self.assertAlmostEqual(nbk7.GetIndex( 312.6*nm), f.Eval( 312.6*nm), 6) # n312.6
        self.assertAlmostEqual(nbk7.GetIndex( 589.3*nm), f.Eval( 589.3*nm), 6) # nD
        self.assertAlmostEqual(nbk7.GetIndex(1014.0*nm), f.Eval(1014.0*nm), 6) # nt
        self.assertAlmostEqual(nbk7.GetIndex(2325.4*nm), f.Eval(2325.4*nm), 6) # n2325.4

if __name__=="__main__":
    ROOT.gRandom.SetSeed(int(time.time()))
    suite = unittest.TestLoader().loadTestsFromTestCase(TestROBAST)
    unittest.TextTestRunner(verbosity=2).run(suite)
