import pyfirmata
import time

board = pyfirmata.Arduino('COM3')
UP=board.digital[2]
UP.mode=pyfirmata.OUTPUT
DOWN=board.digital[4]
DOWN.mode=pyfirmata.OUTPUT

LEFT=board.digital[7]
LEFT.mode=pyfirmata.OUTPUT
RIGHT=board.digital[8]
RIGHT.mode=pyfirmata.OUTPUT

while True:
    UP.write(1)
    time.sleep(.5)
    UP.write(0)
    time.sleep(.5)

    DOWN.write(1)
    time.sleep(.5)
    DOWN.write(0)
    time.sleep(.5)

    #LEFT.write(1)
    #time.sleep(.5)
    #LEFT.write(0)
    #time.sleep(.5)

    #RIGHT.write(1)
    #time.sleep(.5)
    #RIGHT.write(0)
    #time.sleep(.5)
