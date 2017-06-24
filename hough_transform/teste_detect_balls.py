import cv2
import numpy as np
import time

def deliver(x):
    pass

# Captura de Video
cap = cv2.VideoCapture(0)

# VARIAVEIS GLOBAIS
SAMPLE_SIZE = 5 # Tamanho da reducao da imagem
CLAHE_CLIP = 4 # Clip Limit do CLAHE
CLAHE_GRID_SIZE = 56 # Grid Size do CLAHE
BF_D = 43 # Diametro do Pixel Neighborhood do BF (Bilateral Filter)
BF_SIGMA_COLOR = 115 # Filtro Sigma do Color Space do BF
BF_SIGMA_SPACE = 57 # Filtro Sigma do Coordinate Space do BF
KERNEL = 5 # Kernel do Closing
M_BLUR = 9 # Median Blur

# # CRIANDO JANELA
# cv2.namedWindow("DisplayC")
# # CRIANDO TRACKBARS
# cv2.createTrackbar('CLAHE_CLIP','DisplayC',1,10,deliver)
# cv2.setTrackbarPos('CLAHE_CLIP','DisplayC', CLAHE_CLIP)

# cv2.createTrackbar('CLAHE_GRID_SIZE','DisplayC',1,64,deliver)
# cv2.setTrackbarPos('CLAHE_GRID_SIZE','DisplayC', CLAHE_GRID_SIZE)

# cv2.createTrackbar('BF_D','DisplayC',1,64,deliver)
# cv2.setTrackbarPos('BF_D','DisplayC', BF_D)

# cv2.createTrackbar('BF_SIGMA_COLOR','DisplayC',1,255,deliver)
# cv2.setTrackbarPos('BF_SIGMA_COLOR','DisplayC', BF_SIGMA_COLOR)

# cv2.createTrackbar('BF_SIGMA_SPACE','DisplayC',1,255,deliver)
# cv2.setTrackbarPos('BF_SIGMA_SPACE','DisplayC', BF_SIGMA_SPACE)

# cv2.createTrackbar('KERNEL','DisplayC',1,10,deliver)
# cv2.setTrackbarPos('KERNEL','DisplayC', KERNEL)

# cv2.createTrackbar('M_BLUR','DisplayC',1,10,deliver)
# cv2.setTrackbarPos('M_BLUR','DisplayC', M_BLUR)

for i in range(1, 14): 
    image = cv2.imread("sample/sample (%d).jpg" % (i))
# while True:
    # image = cv2.imread("sample/sample (3).jpg")
    # CLAHE_CLIP = cv2.getTrackbarPos('CLAHE_CLIP','DisplayC')
    # CLAHE_GRID_SIZE = cv2.getTrackbarPos('CLAHE_GRID_SIZE','DisplayC')
    # BF_D = cv2.getTrackbarPos('BF_D','DisplayC')
    # BF_SIGMA_COLOR = cv2.getTrackbarPos('BF_SIGMA_COLOR','DisplayC')
    # BF_SIGMA_SPACE = cv2.getTrackbarPos('BF_SIGMA_SPACE','DisplayC')
    # KERNEL = cv2.getTrackbarPos('KERNEL','DisplayC')
    # M_BLUR = cv2.getTrackbarPos('M_BLUR','DisplayC')
    # M_BLUR = M_BLUR if M_BLUR % 2 == 1 else M_BLUR + 1
#     _, image = cap.read();    
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

    # Aplicando Contraste adaptivo
    image = cv2.bilateralFilter(image,BF_D,BF_SIGMA_COLOR,BF_SIGMA_SPACE)

    # Aplicando threshold
    thresh = cv2.adaptiveThreshold(image, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 11, 4)
    
    kernel = np.ones((KERNEL,KERNEL),np.uint8)
    # thresh = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel)
    thresh = cv2.medianBlur(thresh, M_BLUR)
    
    color = cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)
    # Pegando contornos
    dst = cv2.addWeighted(thresh,0.5,image,0.5,0)
    
    circles = cv2.HoughCircles(dst,cv2.HOUGH_GRADIENT,1,15,param1=200,param2=40,minRadius=15,maxRadius=0)

    cv2.imshow("Display", thresh)
    cv2.imshow("DisplayC", dst)
    cv2.waitKey()
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    # circles = np.uint16(np.around(circles))
    if circles is None: 
        print "none"
        continue
    for i in circles[0,:]:
        print("%d" + str(i))
        # draw the outer circle
        cv2.circle(color,(i[0],i[1]),i[2],(0,255,0),2)
        # draw the center of the circle
        cv2.circle(color,(i[0],i[1]),2,(0,0,255),3)
        cv2.imshow("DisplayCX", color)
        
    # Desenhando contours
    
    # DISPOSICAO
    #cv2.imshow("Display", thresh)
    
cap.release()
cv2.destroyAllWindows()