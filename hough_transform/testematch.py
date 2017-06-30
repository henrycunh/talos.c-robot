# encoding: utf-8
import cv2
import time
import numpy as np
import os
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
THRESHOLD_DETECT = 0.82
NUM_TPL = 0
tpl = [] # Array contendo os templates, utilizados no matching
tplSz = [] # Array contendo os dados do template

# Carregando templates
for filename in os.listdir("myTempl"):
	fn = "myTempl/%s" % filename
	NUM_TPL += 1
	print(fn)
	img = cv2.imread(fn, 0)
	tpl.append(img)
	tplSz.append(img.shape[::-1])

# LOOP DE CAPTURA E PROCESSAMENTO
for i in range(1, 436):
	# Carregando imagem
	fn = "record/frame (%d).jpg" % i
	image = cv2.imread(fn)
	# Converte de colorido para GS
	gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
	# Preenche o topo da imagem
	cv2.rectangle(gray,(0,0),(width, int(height/2.1)),(FG,FG,FG),-1)
    # Preenche borda ao redor da imagem
	border = cv2.copyMakeBorder(gray,5,5,5,5,cv2.BORDER_CONSTANT,value=[FG,FG,FG])
	display = border.copy()
	display = cv2.cvtColor(display, cv2.COLOR_GRAY2BGR)
	"""
	IDENTIFICAÇÃO DA VÍTIMA
	"""
	# Variável que vai armazenar as locs compativeis
	matches = []
	# Testando templates com a imagem
	for i in range (0, NUM_TPL):
		res = cv2.matchTemplate(border, tpl[i], cv2.TM_CCOEFF_NORMED)
		loc = np.where(res >= THRESHOLD_DETECT)
		matches.append(loc)
		found = zip(*loc[::-1])
		flag = False
		if(len(found) > 0):
		 	print("Template [%d]: %d" % (i+1, len(found)))
			size = tplSz[i]
			point = found[0]
			cv2.rectangle(display, point, (point[0] + size[0], point[1] + size[1]), (0,255,255), 2)
			cx = (point[0] + point[0] + size[0]) / 2
			cy = (point[1] + point[1] + size[1]) / 2
			cv2.circle(display, (cx, cy), 8, (255,0,255), 1)
			flag = True
			break
		if flag:
			break

	"""
	IDENTIFICAÇÃO DE RECEPTÁCULO
		A identificação do objeto em que se entrega a vítima é feito através
	da extração do nível de cinza em que o receptaculo se encontra por meio
	do threshold, e a partir do resultado, aplicar Canny para revelar os
	limites da imagem, dilatando o resultado para aproximar as linhas, por fim,
	obtendo contornos do resultado final.
	"""
    # Extrai threshhold da imagem
	_, threshEdge = cv2.threshold(border, 50, 255, cv2.THRESH_BINARY)
    # Extrai os limites da imagem
	edgesOne = cv2.Canny(threshEdge, EDGE_MIN, EDGE_MAX)
    # Dilatando os edges
	edges = cv2.dilate(edgesOne, getKernel(3), iterations = 1)
    # PEGAR CONTOURS
	_, contours,_ = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    # Colorindo imagem
	edges = cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)



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
			cv2.drawContours(edges, contours, -1, (255,0,0), 1)
	        # Extrai os moments do contorno
			M = cv2.moments(cnt)
	        # Calcula o centroide
			cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 5
			cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
	        # Desenha um circulo no centroide
			cv2.circle(edges, (cx, cy),8, (255,0,255), 1)
			cv2.circle(edges, (cx, cy), 1, (255,255,255), 1)
	        # Dispõe um texto com as coordenadas do centroide
			cv2.putText(edges, "CX: %d  CY: %d" % (cx,cy), (40,40), cv2.FONT_HERSHEY_PLAIN, 1, (0,0,0))
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

	cv2.imshow('feed', display)
	cv2.imshow('edges', edges)

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
