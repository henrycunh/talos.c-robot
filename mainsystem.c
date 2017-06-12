#pragma config(Sensor, S1,     colorA,         sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S2,     i2c,            sensorEV3_GenericI2C)
#pragma config(Sensor, S3,     infraR,         sensorEV3_IRSensor)
#pragma config(Sensor, S4,     colorB,         sensorEV3_Color, modeEV3Color_Color)
//S1 = Direita || S4 = Esquerda

// Definindo endereços do Arduino
#define ARDUINO_ADDRESS 0x08
#define ARDUINO_PORT S2
#define KP 0.85
#define KI 0.0
#define KD 0.2
#define SET_POINT 65
#define OFFSET -18
#define TURN_RATE_90 70
#define TURN_TIME_90 60
#define TURN_SPEED_90 30
#define G_THRESH 2.6
#define TURN_ERRO_K 8
#define IMAGE_KP 0.2
#define IMAGE_SETPOINT 47
#define COLOR_ERRO 6
#define INT_COUNT_MAX 20

bool resgate = false;
int limiarWhite[2][3];

// Limpar tela
void print(char *str){
	eraseDisplay();
	displayCenteredBigTextLine(1,"%s",str);
}

// Realiza mudança na magnitude
long map( long x, long in_min, long in_max, long out_min, long out_max){
	return (x - in_min) * ( out_max - out_min) / ( in_max - in_min ) + out_min ;
}

void stopUs(){
	while (1){
		motor[motorA] = 0;
		motor[motorB] = 0;
	}
}


/* ---------------------------------
||							I2C								||
--------------------------------- */
// Variável que armazena a posição do sensor de 0 a 127
int linha;
// Variável que armazena o estado especial do sensor de linha
int estado;
// Armazena o status do sensor
TI2CStatus mI2CStatus;
// Armazena a resposta
byte replyMsg[10];
// Armazena a mensagem a ser enviada
byte sendMsg[10];

// Mandar e receber mensagens para o I2C
void i2c_msg(int reply_size, int message_size, byte byte1, byte byte2, byte byte3, byte byte4){
	// Pegando o status do sensor I2C
	mI2CStatus = nI2CStatus[i2c];
	// Reservando espaço na memória para a resposta
	memset(replyMsg, 0, sizeof(replyMsg));
	// Reservando espaço no tamanho da mensagem
	message_size += 3;
	// Atribuindo o tamanho da mensagem e o endereço
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

	// Resposta
	linha = replyMsg[0];
	estado = replyMsg[1];
	wait1Msec(35);
}


// Faz o robô andar para frente
void walk(int value, float duration){
	print("WALK");
	resetMotorEncoder(motorA);
	int a = getMotorEncoder(motorA);
	if(value > 0){
		while(getMotorEncoder(motorA) + duration > a){
			motor[motorA] = -value;
			motor[motorB] = -value;
		}
	}else{
		while(getMotorEncoder(motorA) - duration < a){
			motor[motorA] = -value;
			motor[motorB] = -value;
		}
	}
}
// Faz o robô girar em graus
void turn(float value, bool direction){
	print("TURN");
	value = map(value, 0, 180, 0, 1320);
	int a = getMotorEncoder(motorA);
	if(direction){
		while(getMotorEncoder(motorA) < a + value){
			motor[motorA] = 30;
			motor[motorB] = -30;
		}
		}	else{
		while(getMotorEncoder(motorA) + value > a ){
			motor[motorA] = -30;
			motor[motorB] = 30;
		}
	}
}

// Lê os valores e estado do QTR8-A
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


// Calibra o threshold de branco e preto
void calibrateThresh(){
	// Calibração do branco
	while(!getButtonPress(buttonEnter)){
		// Pegando cores do sensor esquerdo
		long coresB[3];
		getColorRGB(colorB, coresB[0], coresB[1], coresB[2]);
		// Pegando cores do sensor direito
		long coresA[3];
		getColorRGB(colorA, coresA[0], coresA[1], coresA[2]);

		print("CALIBRANDO O BRANCO");
		displayTextLine(4, "E => (R:%d, G:%d, B:%d)", coresB[0], coresB[1], coresB[2]);
		displayTextLine(7, "D => (R:%d, G:%d, B:%d)", coresA[0], coresA[1], coresA[2]);

		for(int a = 0; a < 2; a++){
			for(int b = 0; b < 3; b++){
				limiarWhite[a][b] = (a == 0 ? coresA[b] - COLOR_ERRO : coresB[b] - COLOR_ERRO);
				limiarWhite[a][b] = map(limiarWhite[a][b], 0, 90, 0, 255);
			}
		}
	}
}

// Lê o valor dos sensores de cor
long coresA[3];
long coresB[3];
int read_color_sensor(){
	// Pegando cores do sensor esquerdo
	getColorRGB(colorB, coresB[0], coresB[1], coresB[2]);
	// Pegando cores do sensor direito
	getColorRGB(colorA, coresA[0], coresA[1], coresA[2]);

	for(int a = 0; a < 3; a++){
		coresA[a] = map(coresA[a], 0, 90, 0, 255);
	}
	for(int a = 0; a < 3; a++){
		coresB[a] = map(coresB[a], 0, 90, 0, 255);
	}
	// Detecta se o valor do verde passa de certo limiar
	//Cinza
	//stopUs();
	read_line_sensor();
	if ((coresA[2] > limiarWhite[0][2] / 4) && (coresB[2] > limiarWhite[1][2] / 4) && (coresA[2] < (3*limiarWhite[0][2])/5) && (coresB[2] < (3*limiarWhite[1][2])/5) && (linha < 70) && (linha > 50)){
			displayCenteredBigTextLine(1,"CINZA");
			setMotorTarget(motorA, 60, -20);
			setMotorTarget(motorB, 60, -20);
			if ((coresA[2] > limiarWhite[0][2] / 4) && (coresB[2] > limiarWhite[1][2] / 4) && (coresA[2] < (3*limiarWhite[0][2])/5) && (coresB[2] < (3*limiarWhite[1][2])/5)){
				return 3;
			}else{
				setMotorTarget(motorA, 50, 20);
				setMotorTarget(motorB, 50, 20);
			}
	}

	// Esquerda
	if ((coresA[1] >= 40) && ((sqrt(pow(coresA[0], 2) + pow(coresA[2], 2)) - (coresA[1] - 20)) <= 0)){
			print("GREEN TURN ESQUERDO")
			return 2;
	}
	// Direita
	if ((coresB[1] >= 40) && ((sqrt(pow(coresB[0], 2) + pow(coresB[2], 2)) - (coresB[1] - 20)) <= 0)){
			displayCenteredBigTextLine(1,"GREEN TURN DIREITO");
			return 1;
	}

	return 0;
}


/* ---------------------------------
||						   PID    					||
--------------------------------- */
int PID(int input, int offset, float KP1, int SET_POINT1){
	// Ultimo input
	static int lstinput;
	// Erro acumulado
	static float erroAc;
	int erro = input - SET_POINT1;
	displayBigTextLine(1, "GENERIC PID: %d", erro);
	float valor = (erro * KP1) + (input - lstinput) * KD + (erroAc) * KI;
	motor[motorA] = valor + offset;
	motor[motorB] = -valor + offset;
	lstinput = input;
	erroAc += erro;
	return erro;
}

bool corrigido = false;
// Corrigir
void corrigir(int limiar){
	print("CORRIGINDO")
	int erro = read_line_sensor() - SET_POINT;
	while(erro > limiar){
		erro = PID(read_line_sensor(), OFFSET/3, KP, SET_POINT);
	}
	corrigido = true;
}

// GAP
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
		motor[motorA] = -20;
		motor[motorB] = -20;
		read_line_sensor();
		print("G A P")
	}
}

//Saída do Verde
void gExit(){
	int cor;
	do {
		print("G EXIT");
		cor = read_color_sensor();
		motor[motorA] = -20;
		motor[motorB] = -20;
	} while(cor != 0);
}
//Virada do Verde
void greenTurn(bool side){
	gExit();
	print("GREEN TURN")
	walk(TURN_SPEED_90, TURN_TIME_90);
	turn(50, side);
	walk(TURN_SPEED_90, TURN_TIME_90/2);
	corrigir(8);
}
//Saída de estado
int stdOut(int mult, int std){
	int intCount = 0;
	do{
		print("SAIR DE ESTADO")
		read_line_sensor();
		motor[motorA] = 20 * mult;
		motor[motorB] = 20 * mult;
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

// Tratamento 90°
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
	return 0;
}

/**
* HEURÍSTICA
*/
int heuState(int cor){
	int erro = read_line_sensor() - SET_POINT;
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
* LINE FOLLOWING
*/
int lineFollowing(){
	while(1){
		if (resgate)
			break;
		if(getIRDistance(infraR)<1){
			greenTurn(false);
			walk(TURN_SPEED_90, TURN_TIME_90*4);
			greenTurn(true);
			walk(TURN_SPEED_90, TURN_TIME_90*12);
			greenTurn(true);
			read_line_sensor();
			while(estado == 3){
				motor[motorA] = -30;
				motor[motorB] = -30;
				read_line_sensor();
			}
			greenTurn(false);
			walk(-TURN_SPEED_90, TURN_TIME_90*4);
		}
		int sensor = read_line_sensor();
		int cor = read_color_sensor();

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
			displayCenteredBigTextLine(1, "PID: %d", erro);
			continue;
		}
		else{
			heuState(cor);
		}
	}
	return 0;
}


/* ---------------------------------
||						  MAIN	    				||
--------------------------------- */
task main()
{
	calibrateThresh();
	while(1){
		if(1){ // Checa por resgate
			eraseDisplay();
			lineFollowing();
		}
		else
			resgateMode();
		}
}

// OC142795622BR
