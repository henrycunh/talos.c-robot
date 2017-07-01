# encoding: utf-8
import cv2
import numpy as np

# Lista referenciando os pontos
refPt = []
# Status do cropping
cropping = False
# Counter
count = 1
countTemp = 1
def click_and_crop(event, x, y, flags, param):
    # Referencia as variáveis globais
    global refPt, cropping

    if event == cv2.EVENT_LBUTTONDOWN:
        refPt = [(x, y)]
        cropping = True

    elif event == cv2.EVENT_LBUTTONUP:
        refPt.append((x, y))
        cropping = False

cv2.namedWindow("Display")
print("R\tReseta a imagem")
print("V\tVisualiza a regiao")
print("Q\tSalva a regiao")
print("D\tPassa para a proxima imagem")
print("A\tVolta a imagem anterior")
print("Esc\tFecha o programa")
img = cv2.imread("record/frame (1).jpg")
clone = img.copy()
cv2.setMouseCallback("Display", click_and_crop)

while True:
    fn = "record/frame (%d).jpg" % count
    # load the image, clone it, and setup the mouse callback function
    img = cv2.imread(fn)
    h, w = img.shape[:2]
    img = cv2.resize(img,(4*w, 4*h), interpolation = cv2.INTER_CUBIC)
    clone = img.copy()

    if len(refPt) == 2:
        cv2.rectangle(img, refPt[0], refPt[1], (255, 0, 255), 1)
        roi = clone[refPt[0][1]:refPt[1][1], refPt[0][0]:refPt[1][0]]

    # display the image and wait for a keypress
    cv2.imshow("Display", img)
    key = cv2.waitKey(1) & 0xFF


    # if the 'r' key is pressed, reset the cropping region
    if key == ord("r"):
    	img = clone.copy()

    if key == ord("v"):
    	cv2.imshow("ROI", roi)

    elif key == ord("q"):
        fn = "myTempl/%d.jpg" % countTemp
        countTemp += 1
        h, w = roi.shape[:2]
        print("W: %d H: %d" % (w, h))
        roi = cv2.resize(roi,(w/4, h/4), interpolation = cv2.INTER_CUBIC)
        cv2.imwrite(fn, roi)

    elif key == ord("d"):
        count += 1

    elif key == ord("a"):
        count = count - 1 if count > 1 else 1

    elif key == 27:
    	break

cv2.destroyAllWindows()
