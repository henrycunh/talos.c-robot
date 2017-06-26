from picamera.array import PiRGBArray
from picamera import PiCamera
import cv2
import time
import serial
import numpy as np

# RESOLUÇÃO DA CAPUTRA
width = 320
height = 240
# PORTA SERIAL
ser = serial.Serial('/dev/ttyACM2', 9600)
# INICIALIZAÇÃO DA CAMERA
camera = PiCamera()
camera.resolution = (width, height)
camera.framerate = 32

minim = 0
inic = 300
teste = True
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
        # HOUGH TRANSFORM [PARAMS]
        circles = cv2.HoughCircles(img2, cv2.HOUGH_GRADIENT, 1, 10, param1=200, param2=30, minRadius=0, maxRadius=100)
        ret, thresh = cv2.threshold(img2, THRESH_VAL, MAX_VAL, cv2.THRESH_BINARY_INV)
        # PEGAR CONTOURS
        _, contours,_ = cv2.findContours(thresh, 1, 2)

        cx = 0
        cy = 0
        # Detectando area
        if len(contours) > 0:
                cnt = contours[0]
                area = cv2.contourArea(cnt)
                if area > AREA_MIN:
                        cv2.drawContours(img2, contours, -1, (255,0,0), 2)
                        M = cv2.moments(cnt)
                        cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 0
                        cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
                        cv2.circle(img2, (cx, cy), 8, (255,0,255), 1)
                        cv2.circle(img2, (cx, cy), 1, (255,255,255), 1)
        
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

def detectRecept(image):
        # RESIZE --------------------------
        height, width = image.shape[:2]
        height, width = (height/SAMPLE_SIZE, width/SAMPLE_SIZE)
        image = cv2.resize(image, (width, height), interpolation=cv2.INTER_CUBIC)
        
        # GRAY ----------------------------
        image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

        # CLAHE ---------------------------
        clahe = cv2.createCLAHE(clipLimit=CLAHE_CLIP, tileGridSize=(CLAHE_GRID_SIZE,CLAHE_GRID_SIZE))
        image = clahe.apply(image)

        # FILLING TOP --------------------
        cv2.rectangle(image,(0,0),(width, int(height/3)),(192,192,192),-1)

        # BLUR SELETIVO DE EDGES ---------
        image = cv2.bilateralFilter(image,BF_D,BF_SIGMA_COLOR,BF_SIGMA_SPACE)

        # THRESHOLD ----------------------
        ret, thresh = cv2.threshold(image, THRESH_VAL, MAX_VAL, cv2.THRESH_BINARY_INV)

        # CONTOURS -----------------------
        _, contours,_ = cv2.findContours(thresh, 1, 2)
        cx = 0
        cy = 0
        if len(contours) > 0:
                cnt = contours[0]
                area = cv2.contourArea(cnt)
                if area > AREA_MIN:
                #cv2.drawContours(image, contours, -1, (255,0,0), 2)
                M = cv2.moments(cnt)
                cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 0
                cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
                # cv2.circle(image, (cx, cy),8, (255,0,255), 1)
                # cv2.circle(image, (cx, cy), 1, (255,255,255), 1)
        
        pass (cx, cy)
