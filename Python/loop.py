import librosa
import librosa.display
import matplotlib.pyplot as plt
import numpy as np

import serial
import time
import os


import sounddevice as sd
from scipy.io.wavfile import write
import random

#def get_audio():
#    r = sr.Recognizer()
#    with sr.Microphone() as source:
#        audio = r.listen(source)
#        said = ""
#        try:
#            said = r.recognize_google(audio)
#            print(said)
#        except Exception as e:
#            print("Exception: " + str(e))
#    return said

arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)

def write_freqamp(gamp, gfreq, bamp, bfreq, gbias, bbias, pshift):
    print("write freqamp")
    ###### AMP ---->  (1,250)
    ###### FEQ ----> (10,150)
    ###### BIAS ---> (1,1000 - 4*AMP)
    ###### SHIFT --> (0,255)

    arduino.write(gamp)
    arduino.write(gfreq)
    arduino.write(bamp)
    arduino.write(bfreq)
    arduino.write(gbias)
    arduino.write(bbias)
    arduino.write(pshift)
    #time.sleep(0.05)
    #data = arduino.readline()
    #while True:
    #    if arduino.in_waiting!=0:
#            break
    #data = arduino.readline()
    #return data



gamp = 120
gfreq = 40
bamp = 120
bfreq = 40
gbias = 1
bbias = 1
pshift = 128

gamp1 = int(gamp).to_bytes(1, byteorder='big',signed=False)
gfreq1 = int(gfreq).to_bytes(1, byteorder='big',signed=False)
bamp1 = int(bamp).to_bytes(1, byteorder='big',signed=False)
bfreq1 = int(bfreq).to_bytes(1, byteorder='big',signed=False)
gbias1 = int(gbias).to_bytes(1, byteorder='big',signed=False)
bbias1 = int(bbias).to_bytes(1, byteorder='big',signed=False)
pshift1 = int(pshift).to_bytes(1, byteorder='big',signed=False)



write_freqamp(gamp1, gfreq1, bamp1, bfreq1, gbias1, bbias1, pshift1)
time.sleep(3)

rep = 90

while rep>1:



    tempof = random.randint(13, 16)
    seconds = 10

    if rep<61:
        tempof = random.randint(9, 12)
        seconds = 13
    if rep<30:
        tempof = random.randint(5, 8)
        seconds = 15

    gfreq = tempof * random.randint(1, 3)
    bfreq = tempof * random.randint(2, 5)
    gamp = random.randint(100, 240)

    bamp = random.randint(55, 150)
    gbias = int(min((random.randint(1, 1000-4*gamp))*100/(1000-4*gamp),100))
    bbias = int(min((random.randint(1, 1000-4*bamp))*100/(1000-4*bamp),100))

    #gbias = 1
    #bbias = 1

    pshift = random.randint(0, 255)
    #pshift = 128

    #pshift = 0

    filename ="t{}t{}t{}t{}t{}t{}t{}.wav".format(gamp, gfreq, bamp, bfreq, gbias, bbias, pshift)
    fs = 44100  # Sample rate
    #seconds = 15  # Duration of recording

    print('rep=',rep)
    print(gamp)
    print(gfreq)
    print(bamp)
    print(bfreq)
    print(gbias)
    print(bbias)
    print(pshift)

    gamp1 = int(gamp).to_bytes(1, byteorder='big')
    gfreq1 = int(gfreq).to_bytes(1, byteorder='big')
    bamp1 = int(bamp).to_bytes(1, byteorder='big')
    bfreq1 = int(bfreq).to_bytes(1, byteorder='big')
    gbias1 = int(gbias).to_bytes(1, byteorder='big')
    bbias1 = int(bbias).to_bytes(1, byteorder='big')
    pshift1 = int(pshift).to_bytes(1, byteorder='big')

    write_freqamp(gamp1, gfreq1, bamp1, bfreq1, gbias1, bbias1, pshift1)




    myrecording = sd.rec(int(seconds * fs), samplerate=fs, channels=2)
    sd.wait()  # Wait until recording is finished
    write(filename, fs, myrecording)  # Save as WAV file


    y, sr = librosa.load(filename, offset=3.1, duration=seconds-3.1)

    D = librosa.stft(y)
    D_harmonic, D_percussive = librosa.decompose.hpss(D)
    y_percussive = librosa.istft(D_percussive, length=len(y))

    y = y_percussive

    hop_length = 128
    oenv = librosa.onset.onset_strength(y=y, sr=sr, hop_length=hop_length)
    tempogram = librosa.feature.tempogram(onset_envelope=oenv, sr=sr, hop_length=hop_length)
    tdet = librosa.onset.onset_detect(y=y, sr=sr, units='time')
    #print(tdet)


    ac_global = librosa.autocorrelate(oenv, max_size=tempogram.shape[0])
    ac_global = librosa.util.normalize(ac_global)
    # Estimate the global tempo for display purposes
    tempo = librosa.beat.tempo(onset_envelope=oenv, sr=sr, hop_length=hop_length)[0]

    fig, ax = plt.subplots(nrows=2, figsize=(10, 5))
    times = librosa.times_like(oenv, sr=sr, hop_length=hop_length)
    ax[0].plot(times, oenv, label='Onset strength')
    ax[0].label_outer()
    ax[0].legend(frameon=True)
    librosa.display.specshow(tempogram, sr=sr, hop_length=hop_length, x_axis='time', y_axis='tempo', cmap='magma',ax=ax[1])
    ax[1].axhline(tempo, color='w', linestyle='--', alpha=1,label='Estimated tempo={:g}'.format(tempo))
    ax[1].legend(loc='upper right')
    ax[1].set(title='Tempogram')
    #x = np.linspace(0, tempogram.shape[0] * float(hop_length) / sr,num=tempogram.shape[0])
    #ax[2].plot(x, np.mean(tempogram, axis=1), label='Mean local autocorrelation')
    #ax[2].plot(x, ac_global, '--', alpha=0.75, label='Global autocorrelation')
    #ax[2].set(xlabel='Lag (seconds)')
    #ax[2].legend(frameon=True)
    #freqs = librosa.tempo_frequencies(tempogram.shape[0], hop_length=hop_length, sr=sr)
    #ax[3].semilogx(freqs[1:], np.mean(tempogram[1:], axis=1),label='Mean local autocorrelation', base=2)
    #ax[3].semilogx(freqs[1:], ac_global[1:], '--', alpha=0.75, label='Global autocorrelation', base=2)
    #ax[3].axvline(tempo, color='black', linestyle='--', alpha=.8,label='Estimated tempo={:g}'.format(tempo))
    #ax[3].legend(frameon=True)
    #ax[3].set(xlabel='BPM')
    #ax[3].grid(True)
    filename2 ="p{}p{}p{}p{}p{}p{}p{}.pdf".format(gamp, gfreq, bamp, bfreq, gbias, bbias, pshift)
    plt.savefig(filename2)





    rep = rep - 1



        #filename ="t{}{}.wav".format(f, a)
        #text = get_audio()
        #if "run" in text:
        #write_freqamp(freq,amp,bias)
        #print(f)
        #print(a)

        #fs = 44100  # Sample rate
        #seconds = 8  # Duration of recording

        #myrecording = sd.rec(int(seconds * fs), samplerate=fs, channels=2)
        #sd.wait()  # Wait until recording is finished
        #write(filename, fs, myrecording)  # Save as WAV file

        #a=a-10
    #f=f+5
#    if value=='1':
#        num2=input("Emter amp (0,5000): ")
#        value2 = write_read(num2)
    #    print(value2)
    #else:
    #    print(value)
