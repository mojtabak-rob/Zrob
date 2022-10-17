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

def write_read(x):
    arduino.write(bytes(x,'utf-8'))
    #time.sleep(0.05)
    #data = arduino.readline()
    while True:
        if arduino.in_waiting!=0:
            break
    data = arduino.readline()
    return data

while True:
    num = input("Enter a number: ")
    text = get_audio()
    if "hello" in text:
        value = write_read(num)
        print(value)
