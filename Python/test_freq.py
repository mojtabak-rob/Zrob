import serial
import time
import os
import time
import playsound
import speech_recognition as sr
from gtts import gTTS

def get_audio():
    r = sr.Recognizer()
    with sr.Microphone() as source:
        audio = r.listen(source)
        said = ""
        try:
            said = r.recognize_google(audio)
            print(said)
        except Exception as e:
            print("Exception: " + str(e))
    return said

arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)

def write_freqamp(x,y):
    print("write freqamp")
    arduino.write(x)
    arduino.write(y)
    #time.sleep(0.05)
    #data = arduino.readline()
    #while True:
    #    if arduino.in_waiting!=0:
#            break
    #data = arduino.readline()
    #return data

while True:
    freq = int(input("Enter freq (10,40): ")).to_bytes(1, byteorder='big')
    amp = int(input("Emter amp (0,200): ")).to_bytes(1, byteorder='big')
    #text = get_audio()
    #if "run" in text:
    write_freqamp(freq,amp)
#    if value=='1':
#        num2=input("Emter amp (0,5000): ")
#        value2 = write_read(num2)
    #    print(value2)
    #else:
    #    print(value)
