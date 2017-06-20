#pragma config(Sensor, S1,     colorA,         sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S2,     i2c,            sensorEV3_GenericI2C)
#pragma config(Sensor, S3,     infraR,         sensorEV3_IRSensor)
#pragma config(Sensor, S4,     colorB,         sensorEV3_Color, modeEV3Color_Color)
/** CONFIGURAÇÃO DOS SENSORES
 * S1 => Sensor de Cor Direito
 * S2 => Sensor do I2C
 * S3 => Sensor Infravermelho
 * S4 => Sensor de Cor Esquerdo
 */

// CONSTANTES GLOBAIS
#define ARDUINO_ADDRESS 0x08 // Endereço do Arduino
#define ARDUINO_PORT S2 // Port do sensor do Arduino
#define KP 0.85 // Constante Proporcional do PID
#define KI 0.0 // Constante de Integral do PID
#define KD 0.2 // Constante Derivativa do PID
#define SET_POINT 65 // Ponto intermediário das leituras do sensor QTR8
#define OFFSET -18 // Offset de Velocidade do PID
#define TURN_TIME_90 60 // Tempo da virada na curva de 90°
#define TURN_SPEED_90 30 // Velocidade da virada na curva de 90°
#define TURN_ERRO_K 8 // Erro permitido na virada da curva de 90°
#define IMAGE_KP 0.2 // Constante proporcional da busca no resgate
#define IMAGE_SETPOINT 47 // Ponto intermediário da busca no resgate
#define COLOR_ERRO 6 // Erro permitido da cor durante a calibração
#define INT_COUNT_MAX 20 // Máximo de iterações da saida de estado
#define GYRO_THRESH_MAX 55 // Limiar máximo do giroscópio
#define GYRO_THRESH_MIN 40 // Limiar mínimo do giroscópio
#define SMOOTH_K 0.2 // Costante do Exponential Smoothing, usado no giroscópio
#define A_MOTOR_OFFSET 1 // Ajuste do offset no motor A
#define B_MOTOR_OFFSET 1 // Ajuste do offset no motor B

bool resgate = false; // Armazena o estado do resgate
bool corrigido = false; // Armazena o estado da correção
int limiarWhite[2][3]; // Armazena os limiares da cor branca
int linha; // Armazena posição do sensor QTR8-A | Lim: 0 - 127
int estado; // Armazena estado do sensor | {1, 2, 3, 4}
int gyro = 1; // Armazena a angulação do robô
long coresA[3]; // Armazena as cores do sensor de cor direito
long coresB[3]; // Armazena as cores do sensor de cor esquerdo
TI2CStatus mI2CStatus; // Armazena o status do sensor
byte replyMsg[10]; // Armazena a resposta do I2C
byte sendMsg[10]; // Armazena a mensagem a ser enviada

/**
 * Envia e recebe dados dos componentes via protocolo
 * I2C, e permite o controle dos mesmos através de
 * seletos valores
 * ---------------------------------------------------------------------------------
 * @param | [int] reply_size                  | Tamanho da resposta em bytes
 * @param | [int] message_size                | Tamanho da mensagem a ser enviada
 * @param | [byte] byte1, byte2, byte3, byte4 | Bytes a serem enviados como mensagem
 */
void i2c_msg(int reply_size, int message_size, byte byte1, byte byte2, byte byte3, byte byte4){
	// Pegando o status do sensor I2C
	mI2CStatus = nI2CStatus[i2c];
	// Reservando espaÃ§o na memÃ³ria para a resposta
	memset(replyMsg, 0, sizeof(replyMsg));
	// Reservando espaÃ§o no tamanho da mensagem
	message_size += 3;
	// Atribuindo o tamanho da mensagem e o endereÃ§o
	sendMsg[0] = message_size;
	sendMsg[1] = ARDUINO_ADDRESS;
	// Atribuindo os bytes da mensagem
	sendMsg[2] = byte1;
	sendMsg[3] = byte2;
	sendMsg[4] = byte3;
	sendMsg[5] = byte4;

	// Enviando mensagem
	sendI2CMsg(i2c, &sendMsg[0], 8);
	// Esperar 30ms
	wait1Msec(30);

	// Ler resposta
	readI2CReply(i2c, &replyMsg[0], reply_size);
	// Checa por erro de skip na transmissão do I2C
	bool skip = replyMsg[1] == 0;
	// Resposta, analisando o erro de skip
	linha = skip ? linha : replyMsg[0];
	estado = skip ? estado : replyMsg[1];
	// Aplica um Exponential Smoothing, caso não dê erro de skip
	gyro = skip ? gyro : (SMOOTH_K * replyMsg[2]) + ((1-SMOOTH_K) * gyro);
	// Espera a sincronização
	wait1Msec(35);
}

/**
 * Apaga a tela e imprime uma string no display do EV3
 * --------------------------------------------------------
 * @param | [char[]] str | String a ser impressa no display
 */
void print(char *str){
	eraseDisplay();
	displayCenteredBigTextLine(1,"%s",str);
	displayCenteredBigTextLine(10,"%d",gyro);
}

/**
 * Realiza a conversão entre magnitudes diferentes
 * --------------------------------------------------------------------
 * @param | [long] x       | Valor a ser convertido
 * @param | [long] in_min  | Valor mínimo do tipo de unidade de entrada
 * @param | [long] in_max  | Valor máximo do tipo de unidade de entrada
 * @param | [long] out_min | Valor mínimo do tipo de unidade de saída
 * @param | [long] out_max | Valor máximo do tipo de unidade de saída
 * --------------------------------------------------------------------
 * @return [long] O valor já convertido
 */
long map( long x, long in_min, long in_max, long out_min, long out_max){
	return (x - in_min) * ( out_max - out_min) / ( in_max - in_min ) + out_min ;
}

/**
 * Define a velocidade dos motores do robô,
 * aliado ao offset definido nas constantes
 * para cada um dos motores
 * -------------------------------------------
 * @param | [int] a | Velocidade do motor A
 * @param | [int] b | Velocidade do motor B
 */
void setSpeed(int a, int b){
	motor[motorA] = a * A_MOTOR_OFFSET;
	motor[motorB] = b * B_MOTOR_OFFSET;
}

/**
 * Faz com que o robô pare indefinitivamente
 */
void stopUs(){
	while (1){
		setSpeed(0, 0);
	}
}

/**
 * Faz com o que o robô ande para frente
 * uma determinada quantidade de rotações
 * ---------------------------------------------------
 * @param | [int] value      | Valor para a velocidade
 * @param | [float] duration | Duração da operação
 */
void walk(int value, float duration){
	print("WALK");
	resetMotorEncoder(motorA);
	int a = getMotorEncoder(motorA);
	if(value > 0){
		while(getMotorEncoder(motorA) + duration > a){
			setSpeed(-value, -value);
		}
	} else {
		while(getMotorEncoder(motorA) - duration < a){
			setSpeed(-value, -value);
		}
	}
}

/**
 * Faz com que o robô vire uma determinada quantidade
 * de unidades, em uma direção
 * --------------------------------------------------------------------
 * @param | [float] value    | Valor adimensional da angulação da curva
 * @param | [bool] direction | Direção da voltar
 */
void turn(float value, bool direction){
	print("TURN");
	value = map(value, 0, 180, 0, 1320);
	int a = getMotorEncoder(motorA);
	if(direction){
		while(getMotorEncoder(motorA) < a + value){
			setSpeed(30, -30);
		}
	}	else {
		while(getMotorEncoder(motorA) + value > a ){
			setSpeed(-30, 30);
		}
	}
}

/**
 * Lê os valores dos sensores ligados ao Arduino
 */
int read_line_sensor(){
	int byte1;
	if(resgate){
		byte1 = 13;
	}else{
		byte1 = 1;
	}
	i2c_msg(8,3,byte1,0,0,0);
	int value = linha;
	return value;
}

/**
 * Calibra os limiares da cor branca
 */
void calibrateThresh(){
	// Calibração do branco
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


/**
 * Função que executa o ajuste PID
 * -------------------------------------------------------------------------
 * @param | [int] input      | Entrada de dados para o ajuste
 * @param | [int] offset     | Velocidade utilizada nos motores
 * @param | [float] KP1      | Constante proporcional utilizada no ajuste
 * @param | [int] SET_POINT1 | Constante que determina o ponto intermediário
 */
int PID(int input, int offset, float KP1, int SET_POINT1){
	// Ultimo input
	static int lstinput;
	// Erro acumulado
	static float erroAc;
	int erro = input - SET_POINT1;
	displayBigTextLine(1, "GENERIC PID: %d", erro);
	float valor = (erro * KP1) + (input - lstinput) * KD + (erroAc) * KI;
	setSpeed(valor + offset, -valor + offset);
	lstinput = input;
	erroAc += erro;
	if(erro < 8 && erro > -8)
		corrigido = true;
	return erro;
}

/**
 * Corrige o robô, dado um limiar, através do
 * PID com um offset menor
 * -----------------------------------------------------------
 * @param | [int] limiar | Limiar a ser atingido pela correção
 */
void corrigir(int limiar){
	print("CORRIGINDO");
	int erro = read_line_sensor() - SET_POINT;
	while(erro > limiar){
		erro = PID(read_line_sensor(), OFFSET/3, KP, SET_POINT);
	}
	corrigido = true;
}

/**
 * Realiza o Gap, ao analisar se já fora corrigido
 * o erro da posição do robô na linha, e anda para
 * frente até que o estado mude para o que representa
 * uma linha dentro dos limites do sensor
 */
void GAP(){
	if(!corrigido){
		walk(-20, 80);
		corrigir(6);
		return;
	} else {
		corrigido = !corrigido;
	}
	read_line_sensor();
	while(estado == 3){
		setSpeed(-20, -20);
		read_line_sensor();
		print("G A P");
	}
}

/**
 * Movimenta o robô para a frente até
 * sair de cima de uma área verde
 */
void gExit(){
	int cor;
	do {
		print("G EXIT");
		cor = read_color_sensor();
		setSpeed(-20, -20);
	} while(cor != 0);
}

/**
 * Faz a virada de 90° utilizada ao
 * detectar uma área verde, dado o lado
 * da curva
 * -------------------------------------
 * @param | [bool] side | Lado da curva
 */
void greenTurn(bool side){
	gExit();
	print("GREEN TURN");
	walk(TURN_SPEED_90, TURN_TIME_90);
	turn(50, side);
	walk(TURN_SPEED_90, TURN_TIME_90/2);
	corrigir(8);
}

/**
 * Sai do estado atual, dado o estado
 * e uma direção - frente ou ré
 * ------------------------------------------
 * @param | [int] mult | Direção do movimento
 * @param | [int] std  | Estado a ser evitado
 */
int stdOut(int mult, int std){
	int intCount = 0;
	do{
		print("SAIR DE ESTADO");
		read_line_sensor();
		setSpeed(20 * mult, 20 * mult);
		int cor = read_color_sensor();
		// Detectando cor
		if(cor == 1){
			greenTurn(false);
			gExit();
			return 0;
		}else if(cor == 2){
			greenTurn(true);
			gExit();
			return 0;
		}
		intCount++;
		if (estado == 4) return 0;
	} while(estado != std && intCount < INT_COUNT_MAX);
	return 0;
}
/**
 * Faz a virada de 90° dado uma direção
 * -------------------------------------
 * @param | [bool] dir | Lado da curva
 */
int grade90(bool dir){
	if(estado != 4) stdOut(-1, 3);
	read_line_sensor();
	if(estado == 3){
		int erro;
		turn(15, dir);
		do {
			print("GRADE 90");
			erro = PID(read_line_sensor(), OFFSET/2, KP, SET_POINT);
		}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);
	}
	walk(-10,45);
	corrigir(8);
	return 0;
}

/**
 * Controla o comportamento de heurística
 * como curvas de 90° e detecção de áreas verde
 * --------------------------------------------
 * @param | [int] cor | Cor a ser analisada
 */
int heuState(int cor){
	if(estado == 1){
		if (resgate)
			return 0;
		print("HEU STATE ESQUERDA");
		grade90(true);
	}
	else if(estado == 2){
		displayBigTextLine(1, "90 DIREITA %d", estado);
		// 90° DIREITA
		// Ler estado atual
		print("HEU STATE 2");
		grade90(false);
	}
	// && (erro < 20) && (erro > -20)
	else if((estado == 3)){
		GAP();
	}
	return 0;
}


/**
 * Realiza as funções de Line Following
 * que são interpretadas na função principal
 */
int lineFollowing(){
		int sensor = read_line_sensor();
		int cor = read_color_sensor();
		if (resgate)
			return 0;
		if(getIRDistance(infraR)<1){
			greenTurn(false);
			walk(TURN_SPEED_90, TURN_TIME_90*4);
			greenTurn(true);
			walk(TURN_SPEED_90, TURN_TIME_90*12);
			greenTurn(true);
			read_line_sensor();
			while(estado == 3){
				setSpeed(-30, -30);
				read_line_sensor();
			}
			greenTurn(false);
			walk(-TURN_SPEED_90, TURN_TIME_90*4);
		}

		// Detectando cor
		if(cor == 1){
			greenTurn(false);
			gExit();
		}else if(cor == 2){
			greenTurn(true);
			gExit();
		}else if(cor == 3){
			//resgate = true;
			//return 0;
		}
		// PID
		if((sensor <= 127) && (sensor >= 0) && (estado == 4)){
			int erro = PID(sensor, OFFSET, KP, SET_POINT);
			eraseDisplay();
			displayCenteredBigTextLine(1, "PID: %d | %d", erro, gyro);
			return 0;
		}
		else{
			heuState(cor);
		}
	return 0;
}



/* ---------------------------------
||						  MAIN	    				||
--------------------------------- */
task main
{
	// Manda mensagem para o Arduino sair do modo de resgate
	i2c_msg(2, 1, -1, 0, 0, 0);
	int resgateCount = 0;
	bool offRoad = false;
	calibrateThresh();
	while(1){
		while(gyro > 8){
			int sensor = read_line_sensor();
			int erro = PID(sensor, OFFSET, KP, SET_POINT);
			eraseDisplay();
			displayCenteredBigTextLine(1, "PID RAMPA: %d | %d", erro, gyro);
			if(resgateCount == 15)
				stopUs();
			if(estado == 3){
				if(offRoad)
					resgateCount++;
				offRoad = true;
			}
			continue;
		}
		if(1){
			offRoad = false;
			resgateCount = 0;
			// Checa por resgate
			eraseDisplay();
			lineFollowing();
		}
	/*	else
			resgateMode(); */
	}
}
