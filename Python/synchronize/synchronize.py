# -*- coding: utf-8 -*-
"""
Created on Fri Oct  7 14:31:01 2022

@author: alexansz
"""
import numpy as np

def sendcommands(cos1,cos2,sin1,sin2,freq,amp,bias1,bias2,n_cycles):
    #interface function: send commands to Arduino and validate
    #arm1 = bias1 + amp*fourier(cos1,sin1,freq)
    #arm2 = bias2 + amp*fourier(cos2,sin2,freq)
    #note: positive = away from drum
    #note: frequency is in hertz
    #note: amp applies to both, individual amplitudes controlled by fourier coefficients
    
    pass

def getsound():
    #interface function: get input from microphone
    
    #onsets are a 2xn matrix with times in the first row and corresponding amplitudes in the second row

    return onsets



def process_onsets(onsets,cos1,cos2,sin1,sin2,f,curr_t1,curr_t2):
    #separate onsets into arm 1 and arm 2, find double hits
    #NOTE: ASSUMES ARM 1 IS THE FIRST TO HIT
    
    om = f*2*np.pi
    min_t = 0.8 * (1 / f / len(cos1)) #if two hits by same arm are separated by less than this, it is a double

    #subtract first time and align with expected first arm 1 hit
    onsets[0,:] = onsets[0,:] - onsets[0,0] + curr_t1

    t_diff = curr_t2 - curr_t1

    x1 = [sum([sin1[n]*np.sin((n+1)*om*t) + cos1[n]*np.cos((n+1)*om*t) for n in range(len(cos1))]) for t in onsets[0,:]]
    x2 = [sum([sin2[n]*np.sin((n+1)*om*(t-t_diff)) + cos2[n]*np.cos((n+1)*om*(t-t_diff)) for n in range(len(cos2))]) for t in onsets[0,:]]

    inds1 = [0]
    inds2 = []
    lasthit1 = curr_t1
    lasthit2 = -99
    doubles1 = 0
    doubles2 = 0
    
    for i,t in enumerate(onsets[0,1:]):
        if x1[i]<x2[i]:
            if onsets[0,i] - lasthit1 < min_t:
                doubles1 += 1
                continue
            lasthit1 = onsets[0,i]
            inds1.append(i)
        else:
            if onsets[0,i] - lasthit2 < min_t:
                doubles2 += 1
                continue
            lasthit2 = onsets[0,i]
            inds2.append(i)
        
    return (onsets[0,inds1],onsets[0,inds2],onsets[1,inds1],onsets[1,inds2],doubles1,doubles2)

    
def finddelay(f,cos,sin,onsets):
    dtmax = 0.2 * 1/f
    
    om = f*2*np.pi
    #peak should be moved by y*x'/x'' where y is delta function at onset
    xdot = [sum([(n+1)*sin[n]*np.cos((n+1)*om*t) - (n+1)*cos[n]*np.sin((n+1)*om*t) for n in range(len(cos))]) for t in onsets]
    xddot = -[sum([(n+1)^2*sin[n]*np.sin((n+1)*om*t) + (n+1)^2*cos[n]*np.cos((n+1)*om*t) for n in range(len(cos))]) for t in onsets]
    
    shift = np.array(xdot)/np.array(xddot)/om
    
    return np.mean([max(dtmax,dt) for dt in shift])

def findselfdelay(f,cos,sin,onsets):
    #to code: fine tune fourier coefficients for more even hits
    
    pass


def main():
    
    #### parameters
    
    n_iter = 1 #learning iterations
    n_cycles = 10 #number of cycles at fundamental frequency
    freq = 2 #hertz
    bias_init = 0 #degrees
    d_bias = 2 #degrees
    amp = 20 #degrees
    
    cos1_init = np.array([0,0,0])
    sin1_init = np.array([-1,0,0])
    cos2_init = np.array([0,0,0])
    sin2_init = np.array([-1,0,0])
    arm1_firsthit = 0.25 #cycles until first planned hit
    target_delay = 0.5 #cycles between arm1 and arm2
    
    #### initialize
    
    cos1 = cos1_init
    cos2 = cos2_init
    sin1 = sin1_init
    sin2 = sin2_init
    
    curr_t1 = arm1_firsthit / freq
    curr_t2 = (arm1_firsthit + target_delay) / freq
    
    currbias1 = bias_init
    currbias2 = bias_init
    curr_delay = target_delay / freq

    for i in range(n_iter):

        sendcommands(cos1,cos2,sin1,sin2,freq,amp,currbias1,currbias2,n_cycles)
        
        onsets = getsound()

        times1,times2,amps1,amps2,doubles1,doubles2 = process_onsets(onsets,cos1,cos2,sin1,sin2,freq,curr_t1,curr_t2)

        print('total hits:',onsets.shape[1])
        print('double hits:',doubles1,doubles2)

        if doubles1 > 0:
            currbias1 += d_bias
        if doubles2 > 0:
            currbias2 += d_bias

        #get adjustments
        dt1 = finddelay(cos1_init,sin1_init,times1)
        dt2 = finddelay(cos2_init,sin2_init,times2)

        #da1,db1 = findselfdelay(cos1_init,sin1_init,times1)
        #da2,db2 = findselfdelay(cos2_init,sin2_init,times2)
        
        amp1 = np.mean(amps1)
        amp2 = np.mean(amps2)
        d_amp = (amp2-amp1)/(amp2+amp1)

        print('time mismatch:',dt2-dt1,'s')
        print('amp mismatch:',d_amp)
        
        
        #update commands
        cos1 = cos1*(1+d_amp)
        sin1 = sin1*(1+d_amp)
        cos2 = cos2*(1-d_amp)
        sin2 = sin2*(1-d_amp)
        
        curr_t1 = curr_t1 + dt1
        curr_t2 = curr_t2 + dt2
        curr_delay = curr_delay + dt1 - dt2
        
    
if __name__ == '__main__':
    main()
    