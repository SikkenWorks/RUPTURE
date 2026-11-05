import math
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from compare_audio import analyze, correlation, aligned

class TestAudioComparison(unittest.TestCase):
    def test_identical(self):
        sr=8000
        samples=[.2 * math.sin(2*math.pi*440*i/sr)+.09*math.sin(2*math.pi*(50+i/sr*120)*i/sr) for i in range(sr)]
        result=analyze(samples,samples,sr,max_lag_ms=250,duration=.8)
        self.assertAlmostEqual(result['correlation'],1,places=4)
        self.assertAlmostEqual(result['normalizedResidualRms'],0,places=3)
        self.assertAlmostEqual(result['emulatorRmsVsPhysicalDb'],0,places=3)
    def test_delay_gain_and_inverted(self):
        sr=8000
        # Non-periodic transient bursts make lag unambiguous.
        samples=[((i * 7411 % 1999)/1999 - .5) * (.25 if i%1300 < 600 else .05) for i in range(sr*2)]
        shifted=[0]*int(.105*sr)+[-.75*x for x in samples]
        result=analyze(samples,shifted,sr,max_lag_ms=200,duration=1)
        self.assertAlmostEqual(result['alignmentLagMs'],105,delta=1)
        self.assertTrue(result['phaseInverted'])
        self.assertAlmostEqual(result['emulatorRmsVsPhysicalDb'],20*math.log10(.75),delta=.03)
        self.assertAlmostEqual(result['normalizedResidualRms'],0,delta=.002)
    def test_silence_fails(self):
        with self.assertRaises(ValueError): analyze([0.0]*8000,[0.0]*8000,8000,duration=.5)
    def test_alignment_sign(self):
        self.assertEqual(aligned([1,2,3],[0,1,2,3],1),([1,2,3],[1,2,3]))
        self.assertEqual(aligned([0,1,2,3],[1,2,3],-1),([1,2,3],[1,2,3]))
        self.assertAlmostEqual(correlation(list(range(32)),list(range(32))),1)

if __name__=='__main__': unittest.main()
