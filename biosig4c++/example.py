import biosig
import scipy as S



def load(fname):
    HDR = biosig.constructHDR(0, 0)
    HDR = biosig.sopen(fname, "r", HDR);

    for i in range(HDR.NS):
        HDR.CHANNEL[i].OnOff = 0

    HDR.CHANNEL[0].OnOff = 1
    HDR.CHANNEL[1].OnOff = 1
    HDR.CHANNEL[HDR.NS-1].OnOff = 1

    data = biosig.sread(0, HDR.NRec, HDR)

    biosig.sclose(HDR)
    biosig.destructHDR(HDR)
    
    return data


def testsin(sig, sr, freq):
    quartper = int(sr / freq * .25)
    x = S.sqrt(sig[:-quartper] ** 2 + sig[quartper:] ** 2)
    ampl = x.mean()
    err = x.std()
    if err > ampl/3:
        return 0
    return ampl

import sys

if len(sys.argv) < 3:
    print "usage:\n%s filename.bdf freq" % sys.argv[0]
    sys.exit(1)

fname = sys.argv[1]
sr = int(sys.argv[2])
print "\nexample.py\nchecking file %s" % fname
sig = load(fname)

freq = 3
print "Looking for sinusoidal signals at %f Hz" % freq
sig = sig - S.mean(sig, axis=1)[:, S.newaxis]
a1 = testsin(sig[0], sr, freq)
print "channel A1, peak to peak amplitude of sin at %f Hz: %f (should be %f)" % (freq, 2*a1, 200)
a2 = testsin(sig[1], sr, freq)
print "channel A2, peak to peak amplitude of sin at %f Hz: %f (should be %f)" % (freq, 2*a2, 100)
a3 = testsin(sig[0] + 2*sig[1], sr, freq)
print "channel A1 + 2*A2, peak to peak amplitude of sin at %f Hz: %f (should be %f)" % (freq, 2*a3, 0)
a4 = testsin(sig[2], sr, freq)
print "status channel, peak to peak amplitude of sin at %f Hz: %f (should be %f)" % (freq, 2*a4, 0)
