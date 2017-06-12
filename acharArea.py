# from picamera.array import PiRGBArray
# from picamera import PiCamera
import cv2
import time
#import serial
import numpy as np

def thresh(x):
    pass

cap = cv2.VideoCapture(0)
cv2.createTrackbar('R','frame',0,255,thresh)
while(True):
    # Capture
    ret, frame = cap.read()
    # To Gray
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    # Gaussian blur
    gray = cv2.GaussianBlur(gray, (5,5), 0)
    # Threshold
    thresh = cv2.threshold(gray,70,255,cv2.THRESH_BINARY_INV)[1]

    # Display
    cv2.imshow("frame", thresh)
    # Wait for key
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# End program
cap.release()
cv2.destroyAllWindows()