#!/usr/bin/env python

import unittest
import ROOT
import array

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
                lens.SetConstantAbsorptionLength(1*mm)
            else:
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

            self.assertGreater(1, p - 5*e)
            self.assertLess(1, p + 5*e)

if __name__=="__main__":
    suite = unittest.TestLoader().loadTestsFromTestCase(TestROBAST)
    unittest.TextTestRunner(verbosity=2).run(suite)

