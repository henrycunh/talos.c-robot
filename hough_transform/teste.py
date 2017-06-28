# encoding: utf-8
import cv2
import time
import numpy as np

# Converte a magnitude de um número
def mapVal(x, inMin, inMax, outMin, outMax):
	val = (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin
	return int(val);

# Limita o valor máximo de um número
def constrain(x, maxVal):
	return x if x <= maxVal else maxVal;

def getKernel(val):
    val = val if val % 2 == 1 else val + 1
    return np.ones((val, val),np.uint8);

def toByte(val):
    return bytes([val]);

# RESOLUÇÃO DA CAPUTRA
width = 160
height = 128

# CONSTANTES / VARIAVEIS GLOBAIS
FG = 255
CLAHE_CLIP = 3 # Clip Limit do CLAHE
CLAHE_GRID_SIZE = 16 # Grid Size do CLAHE
MAX_VAL = 255 # Kernel do Closing
THRESH_VAL = 220 # Median Blur
AREA_MIN = 1000 # Area minima de um receptaculo
EDGE_MIN = 110 # Valor minimo para detecção de edges
EDGE_MAX = 255 # Valor máximo para detecção de edges
MEDIAN_BLUR = 11
BALL_DETECTED = False
RECEPT_DETECTED = False



# LOOP DE CAPTURA E PROCESSAMENTO
for i in range(1, 436):
    fn = "record/frame (%d).jpg" % i
    image = cv2.imread(fn)

    # Converte de colorido para GS
    gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)

    # Aplica CLAHE
    clahe = cv2.createCLAHE(clipLimit=CLAHE_CLIP, tileGridSize=(CLAHE_GRID_SIZE,CLAHE_GRID_SIZE))
    imageB = clahe.apply(gray)
    imageC = gray

    # Preenche o topo da imagem
    cv2.rectangle(imageC,(0,0),(width, int(height/2.1)),(FG,FG,FG),-1)
    cv2.rectangle(imageB,(0,0),(width, int(height/2.1)),(0,0,0),-1)

    # Preenche borda ao redor da imagem
    imageC = cv2.copyMakeBorder(imageC,5,5,5,5,cv2.BORDER_CONSTANT,value=[FG,FG,FG])
    imageB = cv2.copyMakeBorder(imageB,5,5,5,5,cv2.BORDER_CONSTANT,value=[0,0,0])

    # Extrai threshhold da imagem
    _, threshOne = cv2.threshold(imageB, 232, 255, cv2.THRESH_BINARY)
    _, threshEdge = cv2.threshold(imageC, 50, 255, cv2.THRESH_BINARY)

    # Filtro bilateral
    image = cv2.bilateralFilter(image,10,80,40)

    # Extrai os limites da imagem
    edgesOne = cv2.Canny(threshEdge, EDGE_MIN, EDGE_MAX)

    # Dilatando os edges
    edges = cv2.dilate(edgesOne, getKernel(3), iterations = 1)
    # Erodindo o threshold da bolinha
    thresh = cv2.dilate(threshOne, getKernel(5), iterations = 3)
    # Expandindo o threshold da bolinha
    thresh = cv2.medianBlur(thresh, MEDIAN_BLUR)

    # PEGAR CONTOURS
    _, cntBall,_ = cv2.findContours(thresh, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    _, contours,_ = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    # Colorindo imagem
    thresh = cv2.cvtColor(thresh, cv2.COLOR_GRAY2BGR)
    imageC = cv2.cvtColor(imageC, cv2.COLOR_GRAY2BGR)

    # Verifica se existe contornos
    if len(contours) > 0:
        area = 0
        # Se existir mais de um contorno na hierarquia
        if len(contours) > 1:
            # Pega a área dos dois primeiros
            area0 = cv2.contourArea(contours[0])
            area1 = cv2.contourArea(contours[1])
            # Mantem a área do maior
            area = area0 if area0 > area1 else area1
            cnt = contours[0] if area0 > area1 else contours[1]

        # Se a área for maior que o mínimo
        if area > AREA_MIN:
            # Desenha o contorno
            cv2.drawContours(imageC, contours, -1, (255,0,0), 1)
            # Extrai os moments do contorno
            M = cv2.moments(cnt)
            # Calcula o centroide
            cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 5
            cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
            # Desenha um circulo no centroide
            cv2.circle(imageC, (cx, cy),8, (255,0,255), 1)
            cv2.circle(imageC, (cx, cy), 1, (255,255,255), 1)
            # Dispõe um texto com as coordenadas do centroide
            cv2.putText(imageC, "CX: %d  CY: %d" % (cx,cy), (40,40), cv2.FONT_HERSHEY_PLAIN, 1, (0,0,0))
            # Mapeia o valor
            cxRecept = mapVal(cx, 5, 165, 128, 255)
            # Converte para byte
            byte = toByte(cxRecept)
            # Envia por serial
            # ser.write(byte)
            print("Recept. Center\t%d" % (cxRecept))
            RECEPT_DETECTED = True
        else:
            RECEPT_DETECTED = False
    else:
        RECEPT_DETECTED = False



    # Verifica se existe contornos para a bola
    if len(cntBall) > 0:
        area = 0

        # Se existir mais de um contorno na hierarquia
        if len(cntBall) > 1:
            # Pega a área dos dois primeiros
            area0 = cv2.contourArea(cntBall[0])
            area1 = cv2.contourArea(cntBall[1])
            # Mantem a área do maior
            area = area0 if area0 > area1 else area1
            cnt = cntBall[0] if area0 > area1 else cntBall[1]
        else:
            # Se só existir um, mantém o único contorno e pega sua área
            cnt = cntBall[0]
            area = cv2.contourArea(cntBall[0])

        # Se a área for maior que o mínimo
        if area > 170:
            # Desenha o contorno
            cv2.drawContours(thresh, cntBall, -1, (255,0,0), 2)
            # Extrai os moments do contorno
            M = cv2.moments(cnt)
            # Calcula o centroide
            cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 5
            cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
            # Desenha um circulo no centroide
            cv2.circle(thresh, (cx, cy),8, (255,0,255), 1)
            cv2.circle(thresh, (cx, cy), 1, (255,0,255), 1)
            # Dispõe um texto com as coordenadas do centroide
            cv2.putText(thresh, "CX: %d  CY: %d" % (cx,cy), (40,40), cv2.FONT_HERSHEY_PLAIN, 1, (255,255,255))
            # Mapeia o valor
            cxBall = mapVal(cx, 5, 165, 128, 255)
            # Converte para byte
            byte = toByte(cxBall)
            # Envia por serial
            # ser.write(byte)
            print("Circle Center\t%d" % (cxBall))
            BALL_DETECTED = True
        else:
            BALL_DETECTED = False
    else:
        BALL_DETECTED = False

    cv2.imshow('feed', imageC)
    cv2.imshow('edges', thresh)

    # Visualização no Terminal
    if BALL_DETECTED or RECEPT_DETECTED:
        print(u'\u2500'*20)
    else:
        print("Nada a ser encontrado\n"+u'\u2500'*20)
    
    time.sleep(0.05)

    key = cv2.waitKey(1) & 0xFF

    if key == ord('q'):
        cv2.destroyAllWindows()
        break
