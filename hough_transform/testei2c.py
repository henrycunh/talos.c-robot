import time
import serial
import serial.tools.list_ports
import numpy as np
from random import randint
# PORTA SERIAL
arduino_ports = [
    p.device
    for p in serial.tools.list_ports.comports()
    if 'ACM' in p.description
]

while not arduino_ports:
    arduino_ports = [
        p.device
        for p in serial.tools.list_ports.comports()
        if 'ACM' in p.description
    ]
    print ("No Arduino found")

ser = serial.Serial(arduino_ports[0], 9600)

time.sleep(0.1)

while 1:
	val = randint(0,9)
	val = bytes([int(val)])
	print(val)	
	ser.write(val)
	time.sleep(0.3)
