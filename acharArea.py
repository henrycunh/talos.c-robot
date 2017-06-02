from picamera.array import PiRGBArray
from picamera import PiCamera
import cv2
import time
import serial
import numpy as np

# RESOLUÇÃO DA CAPUTRA
width = 320
height = 240
# INICIALIZAÇÃO DA CAMERA
camera = PiCamera()
camera.resolution = (width, height)
camera.framerate = 32

rawCapture = PiRGBArray(camera, size=(width, height))

time.sleep(0.1)

# LOOP DE CAPTURA E PROCESSAMENTO
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        image = frame.array
        orig = image.copy()
        key = cv2.waitKey(1) & 0xFF

        rawCapture.truncate(0)

        # RANGE DE CORES
        lower = np.array([0, 0, 0])
        upper = np.array([255,255,255]) 

        # CONVERSÃO DE RGB PARA HSV
        hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        # MÁSCARA BASEADA NO RANGE
        mask = cv2.inRange(hsv, lower, upper)
        # RESULTADO A PARTIR DA APLICAÇÃO DA MASCARA
        res = cv2.bitwise_and(image, image, mask = mask)
        img = res
        # APLICAÇÃO DE FILTRAGEM DE CORES
        imgGrey = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
        clahe = cv2.createCLAHE(clipLimit=5.0, tileGridSize=(8,8))
        imgCts = clahe.apply(imgGrey)
        imgCts = cv2.medianBlur(imgCts,1)

        thresh, res= cv2.threshold(imgCts, 63, 255, cv2.THRESH_BINARY)

        res = cv2.GaussianBlur(imgCts, (9,9), 5, 0)
        (minVal, maxVal, minLoc, maxLoc) = cv2.minMaxLoc(res)
        cv2.circle(res, minLoc, 24, (255,0,255), 2)
        print(minLoc)
        cv2.imshow('resultstream', res)
cv2.destroyAllWindows()
