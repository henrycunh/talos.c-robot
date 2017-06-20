/**
 * Arquivo com as funções de captação dos sensores
 * -----------------------------------------------
 * @author Iago Elias
 * @author Henrique Cunha
 * @version 1.0
 */

/**
 * Lê os valores dos sensores ligados ao Arduino
 */
int read_line_sensor(byte byte1){
	i2c_msg(8,3,byte1,0,0,0);
	int value = linha;
	return value;
}


/**
 * Define os limiares em RGB para cada um dos
 * sensores, a modo de definir para cor branca
 * um limiar unificado no ínicio da competição
 */
void calibrateThresh(){
	while(!getButtonPress(buttonEnter)){
		// Pegando cores do sensor esquerdo
		long coresB[3];
		getColorRGB(colorB, coresB[0], coresB[1], coresB[2]);
		// Pegando cores do sensor direito
		long coresA[3];
		getColorRGB(colorA, coresA[0], coresA[1], coresA[2]);

		// Display visual
		print("CALIBRANDO O BRANCO");
		displayTextLine(4, "E => (R:%d, G:%d, B:%d)", coresB[0], coresB[1], coresB[2]);
		displayTextLine(7, "D => (R:%d, G:%d, B:%d)", coresA[0], coresA[1], coresA[2]);

		// Armazenando na variável global
		for(int a = 0; a < 2; a++){
			for(int b = 0; b < 3; b++){
				limiarWhite[a][b] = (a == 0 ? coresA[b] - COLOR_ERRO : coresB[b] - COLOR_ERRO);
				limiarWhite[a][b] = map(limiarWhite[a][b], 0, 90, 0, 255);
			}
		}

	}
}


/**
 * Lê os sensores de cor, e analisa a partir
 * de escopos espectométricos se a cor atual
 * se encaixa na faixa do verde
 * -------------------------------------
 * 0 | Nenhum dos sensores está em verde
 * 1 | O sensor direito está em verde
 * 2 | O sensor esquerdo está em verde
 * -----------------------------------
 * @return [int] O estado dos sensores
 */
int read_color_sensor(){
	// Pegando cores do sensor esquerdo
	getColorRGB(colorB, coresB[0], coresB[1], coresB[2]);
	// Pegando cores do sensor direito
	getColorRGB(colorA, coresA[0], coresA[1], coresA[2]);
	// Mapeia os limites das cores
	for(int a = 0; a < 3; a++){
		coresA[a] = map(coresA[a], 0, 90, 0, 255);
	}
	for(int a = 0; a < 3; a++){
		coresB[a] = map(coresB[a], 0, 90, 0, 255);
	}
	// Esquerda
	if ((coresA[1] >= 40) && ((sqrt(pow(coresA[0], 2) + pow(coresA[2], 2)) - (coresA[1] - 20)) <= 0)){
			print("GREEN TURN ESQUERDO");
			return 2;
	}
	// Direita
	if ((coresB[1] >= 40) && ((sqrt(pow(coresB[0], 2) + pow(coresB[2], 2)) - (coresB[1] - 20)) <= 0)){
			displayCenteredBigTextLine(1,"GREEN TURN DIREITO");
			return 1;
	}
	return 0;
}
