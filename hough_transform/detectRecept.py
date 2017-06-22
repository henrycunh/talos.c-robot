from picamera.array import PiRGBArray
from picamera import PiCamera
import imutils
import cv2
import time
import serial
import serial.tools.list_ports
import numpy as np

# RESOLUCAO DA CAPUTRA
width = 320
height = 240

# INICIALIZACAO DA CAMERA
camera = PiCamera()
camera.resolution = (width, height)
camera.framerate = 32

# CONSTANTES
MIN_THRESH = 30

rawCapture = PiRGBArray(camera, size=(width, height))

time.sleep(0.1)

# LOOP DE CAPTURA E PROCESSAMENTO

for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        image = frame.array
        rawCapture.truncate(0)
        # Apply Gray filter
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        # Apply Gaussian blur
        blurred = cv2.GaussianBlur(gray, (5,5), 0)
        # Apply Thresh
        thresh = cv2.threshold(blurred, 55, 255, cv2.THRESH_BINARY)[1]
        
        # Contours
        cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
                                cv2.CHAIN_APPROX_SIMPLE)
        cnts = cnts[0] if imutils.is_cv2 else cnts [1]

        # Iterate on contours
        for c in cnts:
            M = cv2.moments(c)
            if M["m00"] != 0:
                # Center of contour
                cX = int(M["m10"] / M["m00"])
                cY = int(M["m01"] / M["m00"])

                # Draw the contour
                cv2.drawContours(image, [c], -1, (0,255,0) ,2)
                cv2.circle(image, (cX, cY), 7, (255,255,255), -1)
            else:
                cX,cY = 0,0
        
        # Exibir
        cv2.imshow("frame", image)
        
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        

cv2.destroyAllWindows()
