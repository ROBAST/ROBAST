#!/usr/bin/env python

import unittest
import ROOT
import array
import time

# The following lines are only needed in PyROOT
ROOT.PyConfig.StartGuiThread = 'inputhook' # for the OpenGL viewer thread
ROOT.gSystem.Load("libGeom")
ROOT.gSystem.Load("libROBAST")
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
        graph = ROOT.TGraph()
        graph.SetPoint(0, 300*nm, 0.)
        graph.SetPoint(1, 500*nm, .5) # 0.25 at 400 nm
        mirror.SetReflectivity(graph)

        manager.GetTopVolume().AddNode(mirror, 1)
        manager.CloseGeometry()

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

    def testMirrorScattaring(self):
        manager = makeTheWorld()

        mirrorbox = ROOT.TGeoBBox("mirrorbox", 0.5*m, 0.5*m, 0.5*m)
        mirror = ROOT.AMirror("mirror", mirrorbox)

        condition = ROOT.ABorderSurfaceCondition(manager.GetTopVolume(), mirror )
        sigma = 1
        condition.SetGaussianRoughness(sigma*d2r)

        manager.GetTopVolume().AddNode(mirror, 1)
        manager.CloseGeometry()

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
        ray = ROOT.ARay(i, 400*nm, 0, 0, 0, 0, 0, 0, -1)

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
        manager.DisableFresnelReflection(True)

        theta = 30*d2r
        sint = ROOT.TMath.Sin(theta)
        cost = ROOT.TMath.Cos(theta)
        ray = ROOT.ARay(i, 400*nm, 0*m, 0*m, 2*mm, 0, sint, 0, -cost)

        manager.TraceNonSequential(ray)
        
        p = array.array("d", [0, 0, 0])
        ray.GetDirection(p)
        px = p[0]
        py = p[1]
        pz = p[2]

        self.assertAlmostEqual(px, sint/idx)
        self.assertAlmostEqual(py, 0)

if __name__=="__main__":
    ROOT.gRandom.SetSeed(int(time.time()))
    suite = unittest.TestLoader().loadTestsFromTestCase(TestROBAST)
    unittest.TextTestRunner(verbosity=2).run(suite)
