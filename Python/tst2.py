#matplotlib inline
import matplotlib.pyplot as plt, IPython.display as ipd, numpy
import librosa, librosa.display
import stanford_mir; stanford_mir.init()

x, sr = librosa.load('Take1_amp4.mp3')

print(x.shape, sr)
ipd.Audio(x, rate=sr)

plt.figure(figsize=(14, 5))
librosa.display.waveshow(x, sr=sr)
plt.show()

hop_length = 256
onset_envelope = librosa.onset.onset_strength(x, sr=sr, hop_length=hop_length)

onset_envelope.shape

N = len(x)
T = N/float(sr)
t = numpy.linspace(0, T, len(onset_envelope))

plt.figure(figsize=(14, 5))
plt.plot(t, onset_envelope)
plt.xlabel('Time (sec)')
plt.xlim(xmin=0)
plt.ylim(0)

plt.show()

onset_frames = librosa.util.peak_pick(onset_envelope, 7, 7, 7, 7, 0.5, 5)

onset_frames
plt.figure(figsize=(14, 5))
plt.plot(t, onset_envelope)
plt.grid(False)
plt.vlines(t[onset_frames], 0, onset_envelope.max(), color='r', alpha=0.7)
plt.xlabel('Time (sec)')
plt.xlim(0, T)
plt.ylim(0)
plt.show()
