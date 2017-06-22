from picamera.array import PiRGBArray
from picamera import PiCamera
import cv2
import time
import serial
import serial.tools.list_ports
import numpy as np

# RESOLUCAO DA CAPUTRA
width = 320
height = 240
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
# INICIALIZACAO DA CAMERA
camera = PiCamera()
camera.resolution = (width, height)
camera.framerate = 32

minim = 0
inic = 300
valAnterior = 0
teste = True

rawCapture = PiRGBArray(camera, size=(width, height))

time.sleep(0.1)

# LOOP DE CAPTURA E PROCESSAMENTO
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        #ser.write(3)
        image = frame.array

        key = cv2.waitKey(1) & 0xFF

        rawCapture.truncate(0)

        # RANGE DE CORES
        lower_blue = np.array([0, 0, 0])
        upper_blue = np.array([255,255,255]) 

        # CONVERSAO DE RGB PARA HSV
        hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        # MASCARA BASEADA NO RANGE
        mask = cv2.inRange(hsv, lower_blue, upper_blue)
        # RESULTADO A PARTIR DA APLICACAO DA MASCARA
        res = cv2.bitwise_and(image, image, mask = mask)
        # BORRANDO UM POUCO A IMAGEM
        # TIREI O BLUR PORQUE ELE DEIXAVA A BOLINHA MUITO POUCO NITIDA
        img = res
        
        # APLICACAO DE FILTRAGEM DE CORES
        imgg = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
        clahe = cv2.createCLAHE(clipLimit=5.0, tileGridSize=(8,8))
        img2 = clahe.apply(imgg)
        img2 = cv2.medianBlur(img2,1)
        cimg = cv2.cvtColor(imgg,cv2.COLOR_GRAY2BGR)
        # HOUGH TRANSFORM [PARAMS]
        circles = cv2.HoughCircles(img2, cv2.HOUGH_GRADIENT, 1, 10, param1=200, param2=30, minRadius=0, maxRadius=100)
                

        # SE NENHUM CIRCULO FOR ENCONTRADO
        if circles is None: 
                teste = False
                hlimitM = height
                hlimitm = 0
                wlimitM = width
                wlimitm = 0
                try:
                        valx = bytes([int(0)])
                        ser.write(valx)
                        print (int(0))
                        #ser.write(i[0])
                        time.sleep(0.05)
                except ser.SerialTimeoutException:
                        print('Data could not be read')
                        time.sleep(0.05)
                if inic < minim:
                        inic = 100
                        continue
                else:
                        inic -= 12
                        continue
                
        lhf = 1000
        # CASO ENCONTRE CIRCULOS
        a = 0
        for i in circles[0,:]:
               lh = (127*i[0])/height
               lw = (127*i[1])/width
               if lh < lhf :
                       i0 = i[0]
                       i1 = i[1]
                       i2 = i[2]
                       lhf = lh
               
               if a == 1 :
                       break
               a += 1
               try:
                        valx = bytes([int(round(lh))])
                        ser.write(valx)
                        print (int(round(lh)))
                        
                        time.sleep(0.05)
               except ser.SerialTimeoutException:
                        print('Data could not be read')
                        time.sleep(0.05)
               cv2.circle(img2,(i0,i1),i2,(0,255,0),1) 
               cv2.circle(img2,(i0,i1),2,(0,0,255),3)    
               cv2.imshow('resultstream', img2)
               cv2.imshow('resultstream1', imgg)
        key = cv2.waitKey(5)
        if key == ord('q'):
            break
        

cv2.destroyAllWindows()
