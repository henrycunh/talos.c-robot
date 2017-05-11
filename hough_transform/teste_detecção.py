import cv2
import numpy as np

def nothing(x):
    pass

cv2.namedWindow('image')

cap = cv2.VideoCapture(0)
cv2.createTrackbar('R','image',0,255,nothing)
cv2.createTrackbar('G','image',0,255,nothing)
cv2.createTrackbar('B','image',0,255,nothing)

cv2.createTrackbar('R1','image',0,255,nothing)
cv2.createTrackbar('G1','image',0,255,nothing)
cv2.createTrackbar('B1','image',0,255,nothing)


while(1):
    _, frame = cap.read()

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    r = cv2.getTrackbarPos('R','image')
    g = cv2.getTrackbarPos('G','image')
    b = cv2.getTrackbarPos('B','image')
    r1 = cv2.getTrackbarPos('R1','image')
    g1 = cv2.getTrackbarPos('G1','image')
    b1 = cv2.getTrackbarPos('B1','image')
    lower_blue = np.array([r, g, b])
    upper_blue = np.array([r1,g1,b1])

    mask = cv2.inRange(hsv, lower_blue, upper_blue)

    res = cv2.bitwise_and(frame, frame, mask = mask)

    img = cv2.medianBlur(mask,5)


    cv2.imshow('frame', frame)
    cv2.imshow('mask', mask)
    cv2.imshow('res', res)
    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break
cv2.destroyAllWindows()
