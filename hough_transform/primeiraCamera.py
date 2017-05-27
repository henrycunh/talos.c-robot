from picamera.array import PiRGBArray
from picamera import PiCamera
import cv2
import time
import serial
import numpy as np

ser = serial.Serial('/dev/ttyACM1', 9600)
camera = PiCamera()
camera.resolution = (640, 480)
height = 640
width = 480
camera.framerate = 32

minim = 0
inic = 300
teste = True

rawCapture = PiRGBArray(camera, size=(640, 480))

time.sleep(0.1)

for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        image = frame.array

        cv2.imshow("Frame", image)
        key = cv2.waitKey(1) & 0xFF

        rawCapture.truncate(0)


        lower_blue = np.array([0, 0, 0])
        upper_blue = np.array([255,255,255]) 
    
        hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        mask = cv2.inRange(hsv, lower_blue, upper_blue)
        res = cv2.bitwise_and(image, image, mask = mask)   
        img = cv2.medianBlur(res,5)
        imgg = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
        cimg = cv2.cvtColor(imgg,cv2.COLOR_GRAY2BGR)
        circles = cv2.HoughCircles(imgg,cv2.HOUGH_GRADIENT,1,10,param1=100,param2=30,minRadius=inic - 15,maxRadius=inic + 15)
    
        #print height, width

        
        #print 1
        if circles is None: 
            teste = False
            hlimitM = 640
            hlimitm = 0
            wlimitM = 480
            wlimitm = 0   
            cv2.imshow('res', res)
            if inic < minim:
                inic = 100
         #       print 4
                continue
            else:
                inic = inic -12
      #         print 3
                continue
        #circles = np.uint16(np.around(circles))
        #print inic
        for i in circles[0,:]:
            if teste is False:
               lh = (127*i[0])/height
               lw = (127*i[1])/width
               #print 'lh = '
               #print lh
               #print 'lw = '
               #print lw
               hlimitM = lh + 5 
               hlimitm = lh - 5
               wlimitM = lw + 5 
               wlimitm = lw - 5
               cv2.circle(res,(i[0],i[1]),i[2],(0,255,0),1) 
               cv2.circle(res,(i[0],i[1]),2,(0,0,255),3) 
               try:
                   print (lw)
                   ser.write(lw)
                   #ser.write(i[0])
                   time.sleep(0.05)
               except ser.SerialTimeoutException:
                   print('Data could not be read')
               time.sleep(0.05)
            else:
               lh = (127*i[0])/height
               lw = (127*i[1])/width
               #print 'lh = '
               #print lh
               #print 'lw = '
               #print lw
               if lh < hlimitM and lh > hlimitm and lw < wlimitM and lw > wlimitm:
                   cv2.circle(res,(i[0],i[1]),i[2],(0,255,0),1) 
                   cv2.circle(res,(i[0],i[1]),2,(0,0,255),3) 
                   try:
                       ser.write(lw)
                       print (lw)
                       #ser.write(i[0])
                       time.sleep(1)
                   except ser.SerialTimeoutException:
                       print('Data could not be read')
                   time.sleep(1)

        cv2.imshow('res', res)
        if key == ord("q"):
            break
        cv2.waitKey(5)

cv2.destroyAllWindows()
