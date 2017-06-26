from picamera.array import PiRGBArray
from picamera import PiCamera
import cv2
import time
import serial
import serial.tools.list_ports
import numpy as np

<<<<<<< HEAD
# RESOLUCAO DA CAPUTRA
width = 160 
height = 128 
=======
# RESOLUÇÃO DA CAPUTRA
width = 160
height = 128
>>>>>>> 99b926e4aa0ab99b03d3aa153dd6187b15dee067
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
cont = 0
CLAHE_CLIP = 3 # Clip Limit do CLAHE
CLAHE_GRID_SIZE = 16 # Grid Size do CLAHE
BF_D = 10 # Diametro do Pixel Neighborhood do BF (Bilateral Filter)
BF_SIGMA_COLOR = 115 # Filtro Sigma do Color Space do BF
BF_SIGMA_SPACE = 30 # Filtro Sigma do Coordinate Space do BF
MAX_VAL = 255 # Kernel do Closing
THRESH_VAL = 104 # Median Blur
AREA_MIN = 2500


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

        # CONVERSÃO DE RGB PARA HSV
        hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        # MÁSCARA BASEADA NO RANGE
        mask = cv2.inRange(hsv, lower_blue, upper_blue)
        # RESULTADO A PARTIR DA APLICAÇÃO DA MASCARA
        res = cv2.bitwise_and(image, image, mask = mask)
        # BORRANDO UM POUCO A IMAGEM
        img = res
        
        # APLICAÇÃO DE FILTRAGEM DE CORES
        imgg = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
        clahe = cv2.createCLAHE(clipLimit=CLAHE_CLIP, tileGridSize=(CLAHE_GRID_SIZE,CLAHE_GRID_SIZE))
        img2 = clahe.apply(imgg)
        img2 = cv2.medianBlur(img2,1)
        cimg = cv2.cvtColor(imgg,cv2.COLOR_GRAY2BGR)
        # Borda
        border = cv2.copyMakeBorder(img2,10,10,10,10,cv2.BORDER_CONSTANT,value=[192,192,192])
        cv2.rectangle(image,(0,0),(width, int(height/3)),(192,192,192),-1)
        blur = cv2.bilateralFilter(border,BF_D,BF_SIGMA_COLOR,BF_SIGMA_SPACE)
        # ret, thresh = cv2.threshold(blur, THRESH_VAL, MAX_VAL, cv2.THRESH_BINARY_INV)
        edges = cv2.Canny(blur, 100, 255)

        # HOUGH TRANSFORM [PARAMS]
        circles = cv2.HoughCircles(img2, cv2.HOUGH_GRADIENT, 1, 10, param1=200, param2=30, minRadius=0, maxRadius=100)
        # ret, thresh = cv2.threshold(img2, THRESH_VAL, MAX_VAL, cv2.THRESH_BINARY_INV)
        # PEGAR CONTOURS
        _, contours,_ = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        if len(contours) > 0:
                area = 0
                if len(contours) > 1:
                        area0 = cv2.contourArea(contours[0])
                        area1 = cv2.contourArea(contours[1])
                        area = area0 if area0 > area1 else area1
                        cnt = contours[0] if area0 > area1 else contours[1]
                
                cv2.putText(edges, "%d AREA: %d" % (i,area), (40,40), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255))
                
                if area > AREA_MIN:
                        cv2.drawContours(image, contours, -1, (255,0,0), 2)
                        M = cv2.moments(cnt)
                        cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 0
                        cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
                        cv2.circle(image, (cx, cy),8, (255,0,255), 1)
                        cv2.circle(image, (cx, cy), 1, (255,255,255), 1)
                        cv2.putText(image, "CX: %d  CY: %d" % (cx,cy), (40,40), cv2.FONT_HERSHEY_PLAIN, 2, (0,0,0))

        # SE NENHUM CIRCULO FOR ENCONTRADO
        if circles is None: 
                teste = False
                hlimitM = height
                hlimitm = 0
                wlimitM = width
                wlimitm = 0
                cv2.imshow('resultstream', img2)
                cv2.imshow('resultstream1', thresh)
                try:
                        valx = bytes([int(0)])
                        ser.write(valx)
                        print (valx)
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
               a = a + 1
               if a > 3 :
                       break
               try:
                        valx = bytes([int(round(lh))])
                        ser.write(valx)
                        print (valx)
                        #ser.write(i[0])
                        time.sleep(0.05)
               except ser.SerialTimeoutException:
                        print('Data could not be read')
                        time.sleep(0.05)
               cv2.circle(img2,(i0,i1),i2,(0,255,0),1) 
               cv2.circle(img2,(i0,i1),2,(0,0,255),3)    
               cv2.imshow('resultstream', img2)
               cv2.imshow('resultstream1', imgg)
        if key == ord('q'):
            break
        cv2.waitKey(5)

cv2.destroyAllWindows()
