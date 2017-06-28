from picamera.array import PiRGBArray
from picamera import PiCamera
import cv2
import time
import serial
import serial.tools.list_ports
import numpy as np


def mapVal(x, inMin, inMax, outMin, outMax):
	val = (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin
	return int(val);

def constrain(x, maxVal):
	return x if x <= maxVal else maxVal;

# RESOLUÇÃO DA CAPUTRA
width = 160
height = 128
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
GB = 7
FG = 255
ALPHA = 0.2
CLAHE_CLIP = 3 # Clip Limit do CLAHE
CLAHE_GRID_SIZE = 16 # Grid Size do CLAHE
MAX_VAL = 255 # Kernel do Closing
THRESH_VAL = 220 # Median Blur
AREA_MIN = 1000

rawCapture = PiRGBArray(camera, size=(width, height))

time.sleep(0.1)

# LOOP DE CAPTURA E PROCESSAMENTO
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        #ser.write(3)
        image = frame.array
        fn = "record/frame_%d.jpg" % cont
        cont = cont + 1
        cv2.imwrite(fn, image)        
        
        key = cv2.waitKey(1) & 0xFF

        rawCapture.truncate(0)
        
        # APLICAÇÃO DE FILTRAGEM DE CORES
        gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
        clahe = cv2.createCLAHE(clipLimit=CLAHE_CLIP, tileGridSize=(CLAHE_GRID_SIZE,CLAHE_GRID_SIZE))
        imageC = clahe.apply(gray)
        color = cv2.cvtColor(gray,cv2.COLOR_GRAY2BGR)
        cv2.rectangle(imageC,(0,0),(width, int(height/2.1)),(FG,FG,FG),-1)
        # Borda
        imageC = cv2.copyMakeBorder(imageC,5,5,5,5,cv2.BORDER_CONSTANT,value=[FG,FG,FG])
        #img2 = cv2.bilateralFilter(img2,BF_D,BF_SIGMA_COLOR,BF_SIGMA_SPACE)
        ret, othresh = cv2.threshold(imageC, 200, 255, cv2.THRESH_BINARY)    
        edges = cv2.Canny(imageC, 180, 255)
        kernel = np.ones((3,3),np.uint8)
        ballK = np.ones((5,5),np.uint8)
        edges = cv2.dilate(edges, kernel, iterations =1)
        thresh = cv2.erode(othresh, ballK, iterations =1)
        bthresh = cv2.dilate(thresh, ballK, iterations =3)
        athresh = cv2.erode(bthresh, kernel, iterations =7)
        athresh = cv2.medianBlur(athresh, 11)
        thresh = athresh
        #thresh = cv2.addWeighted(imageC, ALPHA, thresh, 1 - ALPHA, 0)
        #thresh = clahe.apply(thresh)
        #ret, thresh = cv2.threshold(thresh, THRESH_VAL, 255, cv2.THRESH_BINARY)
        _, cntBall,_ = cv2.findContours(athresh, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        # HOUGH TRANSFORM [PARAMS]
        # circles = cv2.HoughCircles(thresh, cv2.HOUGH_GRADIENT, 1, 10, param1=200, param2=30, minRadius=0, maxRadius=100)
        # ret, thresh = cv2.threshold(img2, THRESH_VAL, MAX_VAL, cv2.THRESH_BINARY_INV)
        # PEGAR CONTOURS
        _, contours,_ = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        #thresh = cv2.cvtColor(thresh, cv2.COLOR_GRAY2BGR)
        imageC = cv2.cvtColor(imageC, cv2.COLOR_GRAY2BGR)
        if len(contours) > 0:
                area = 0
                if len(contours) > 1:
                        area0 = cv2.contourArea(contours[0])
                        area1 = cv2.contourArea(contours[1])
                        area = area0 if area0 > area1 else area1
                        cnt = contours[0] if area0 > area1 else contours[1]
                else:
                        cnt = contours[0]
                        area = cv2.contourArea(contours[0])
                if area > AREA_MIN:
                        cv2.drawContours(imageC, contours, -1, (255,0,0), 1)
                        M = cv2.moments(cnt)
                        cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 0
                        cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
                        cv2.circle(imageC, (cx, cy),8, (255,0,255), 1)
                        cv2.circle(imageC, (cx, cy), 1, (255,255,255), 1)
                        cv2.putText(imageC, "CX: %d  CY: %d" % (cx,cy), (40,40), cv2.FONT_HERSHEY_PLAIN, 1, (0,0,0))
                        cxT = mapVal(cx, 5, 165, 128, 255)
                        ser.write(bytes([cxT]))
                        print("CX: %d" % (cxT))
        cv2.waitKey(5)

        if len(cntBall) > 0:
                area = 0
                if len(cntBall) > 1:
                        area0 = cv2.contourArea(cntBall[0])
                        area1 = cv2.contourArea(cntBall[1])
                        area = area0 if area0 > area1 else area1
                        cnt = cntBall[0] if area0 > area1 else cntBall[1]
                else:
                        cnt = cntBall[0]
                        area = cv2.contourArea(cntBall[0])
                #cv2.putText(edges, "AREA: %d" % area, (40,40), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,255))  
                if area > 200:
                        cv2.drawContours(thresh, cntBall, -1, (255,0,0), 2)
                        M = cv2.moments(cnt)
                        cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 0
                        cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
                        cv2.circle(athresh, (cx, cy),8, (255,0,255), 1)
                        cv2.circle(athresh, (cx, cy), 1, (255,0,255), 1)
                        cv2.putText(athresh, "CX: %d  CY: %d" % (cx,cy), (40,40), cv2.FONT_HERSHEY_PLAIN, 1, (255,255,255))
                        cxT = mapVal(cx, 5, 165, 128, 255)
                        ser.write(bytes([cxT]))
                        print("CIRCLE CX: %d" % (cxT))

        cv2.waitKey(5)
        cv2.imshow('feed', imageC)
        cv2.imshow('edges', thresh)

"""
        # SE NENHUM CIRCULO FOR ENCONTRADO
        if circles is None: 
                teste = False
                hlimitM = height
                hlimitm = 0
                wlimitM = width
                wlimitm = 0
                cv2.imshow('feed', imageC)
                cv2.imshow('edges', thresh)
                try:
                        valx = bytes([int(0)])
                        ser.write(valx)
                        print (0)
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
                        valx = constrain(int(round(lh)), 127)
                        bit = bytes([valx])
                        ser.write(bit)
                        print (valx)
                        #ser.write(i[0])
                        time.sleep(0.05)
               except ser.SerialTimeoutException:
                        print('Data could not be read')
                        time.sleep(0.05)
               cv2.circle(imageC,(i0,i1),i2,(0,255,0),1) 
               cv2.circle(imageC,(i0,i1),2,(0,0,255),3)    
               cv2.imshow('feed', imageC)
               cv2.imshow('edges', thresh)
"""
      #  if key == ord('q'):
      #     break


