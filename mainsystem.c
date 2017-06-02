#pragma config(Sensor, S1,     colorA,         sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S2,     i2c,            sensorEV3_GenericI2C)
#pragma config(Sensor, S4,     colorB,         sensorEV3_Color, modeEV3Color_Color)
// Definindo endere�os do Arduino
#define ARDUINO_ADDRESS 0x08
#define ARDUINO_PORT S2
#define KP 1.8
#define KI 0.0
#define KD 0.2
#define SET_POINT 65
#define OFFSET -22
#define TURN_RATE_90 70
#define TURN_TIME_90 60
#define TURN_SPEED_90 30
#define G_THRESH 2.6
#define TURN_ERRO_K 4
#define COLOR_ERRO 6

bool resgate = false;
int WHITE_THRESH = 30;

/* ---------------------------------
||						UTILS								||
--------------------------------- */
// Realiza mudan�a na magnitude
long map( long x, long in_min, long in_max, long out_min, long out_max){
	return (x - in_min) * ( out_max - out_min) / ( in_max - in_min ) + out_min ;
}
// Pega o maior de tr�s n�meros
int max(int a, int b, int c){
     int m = a;
     (m < b) && (m = b);
     (m < c) && (m = c);
     return m;
}
// Pega o menor de tr�s n�meros
int min(int a, int b, int c){
     int m = a;
     (m > b) && (m = b);
     (m > c) && (m = c);
     return m;
}

/* ---------------------------------
||							I2C								||
--------------------------------- */
// Vari�vel que armazena a posi��o do sensor de 0 a 127
int linha;
// Vari�vel que armazena o estado especial do sensor de linha
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
	// Reservando espa�o na mem�ria para a resposta
	memset(replyMsg, 0, sizeof(replyMsg));
	// Reservando espa�o no tamanho da mensagem
	message_size += 3;
	// Atribuindo o tamanho da mensagem e o endere�o
	sendMsg[0] = message_size;
	sendMsg[1] = ARDUINO_ADDRESS;
	// Atribuindo os bytes da mensagem
	sendMsg[2] = byte1;
	sendMsg[3] = byte2;
	sendMsg[4] = byte3;
	sendMsg[5] = byte4;

	// Enviando mensagem
	sendI2CMsg(i2c, &sendMsg[0], 2);
	// Esperar 30ms
	wait1Msec(30);

	// Ler resposta
	readI2CReply(i2c, &replyMsg[0], reply_size);

	// Resposta
	linha = replyMsg[0];
	estado = replyMsg[1];
	wait1Msec(35);
}


/* ---------------------------------
||						MOVIMENTOS					||
--------------------------------- */
// Faz o rob� andar para frente
void walk(int value, float duration){
	int a = getMotorEncoder(motorA);
	while(getMotorEncoder(motorA) + duration > a){
		motor[motorA] = -value;
		motor[motorB] = -value;
	}
}

// L� os valores e estado do QTR8-A
int read_line_sensor(){
	i2c_msg(2,1,1,0,0,0);
	int value = linha;
	return value;
}


// Calibra o threshold de branco e preto
void calibrateThresh(){
	// Calibra��o do branco
	while(!getButtonPress(buttonEnter)){
		// Pegando cores do sensor esquerdo
		long coresB[3];
		getColorRGB(colorB, coresB[0], coresB[1], coresB[2]);
		// Pegando cores do sensor direito
		long coresA[3];
		getColorRGB(colorA, coresA[0], coresA[1], coresA[2]);

		displayTextLine(1, "CALIBRANDO O BRANCO");
		displayTextLine(4, "E => (R:%d, G:%d, B:%d)", coresB[0], coresB[1], coresB[2]);
		displayTextLine(7, "D => (R:%d, G:%d, B:%d)", coresA[0], coresA[1], coresA[2]);

		int lowestA = min(coresA[0],coresA[1],coresA[2]);
		int lowestB = min(coresB[0],coresB[1],coresB[2]);
		WHITE_THRESH = (lowestA < lowestB ? lowestA : lowestB);
	}
}

// L� o valor dos sensores de cor
int read_color_sensor(){
	// Pegando cores do sensor esquerdo
	long coresB[3];
	getColorRGB(colorB, coresB[0], coresB[1], coresB[2]);
	// Pegando cores do sensor direito
	long coresA[3];
	getColorRGB(colorA, coresA[0], coresA[1], coresA[2]);
	// Escreve os valores na tela
	displayCenteredBigTextLine(1,"Vermelho: %d %d", coresA[0], coresB[0]);
	displayCenteredBigTextLine(4,"Verde: %d %d", coresA[1], coresB[1]);
	displayCenteredBigTextLine(7,"Azul: %d %d", coresA[2], coresB[2]);
	// Detecta se o valor do verde passa de certo limiar
	int whiteErro = WHITE_THRESH + COLOR_ERRO;
	// Esquerda
	if(coresB[0] < whiteErro && coresB[1] < whiteErro && coresB[2] < whiteErro &&
		coresB[1] > cores[0] * G_THRESH && coresB[1] > coresB[2] * G_THRESH){
		displayCenteredBigTextLine(10,"GREEN TURN ESQUERDO");
		return 1;
	}
	// Direita
	if(coresA[0] < whiteErro && coresA[1] < whiteErro && coresA[2] < whiteErro &&
		coresA[1] > cores[0] * G_THRESH && coresA[1] > coresA[2] * G_THRESH){
		displayCenteredBigTextLine(10,"GREEN TURN DIREITO");
		return 2;
	}

	return 0;
}

// Faz o rob� girar em graus
void turn(float value, bool direction){
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

/* ---------------------------------
||						   PID    					||
--------------------------------- */
int PID(int input, int offset){
	// Ultimo input
	static int lstinput;
	// Erro acumulado
	static float erroAc;
	int erro = input - SET_POINT;
	float valor = (erro * KP) + (input - lstinput) * KD + (erroAc) * KI;
	motor[motorA] = valor + offset;
	motor[motorB] = -valor + offset;
	lstinput = input;
	erroAc += erro;
	return erro;
}

// GAP
void GAP(){
	read_line_sensor();
	while(estado == 3){
		motor[motorA] = -20;
		motor[motorB] = -20;
		read_line_sensor();
		displayBigTextLine(10, "%d", estado);
	}
}

// Corrigir
void corrigir(int limiar){
	int erro = read_line_sensor() - SET_POINT;
	while(erro > limiar){
		erro = PID(read_line_sensor(), 0);
	}
}

// Virada Gen�rica por erro
void gTurn(bool direction){
	int erro = read_line_sensor() - SET_POINT;
	while((erro > TURN_ERRO_K) || (erro < -TURN_ERRO_K)){
		displayBigTextLine(10, "Erro: %d", erro);
		if(direction){
			motor[motorA] = -30;
			motor[motorB] = 30;
		} else {
			motor[motorA] = 30;
			motor[motorB] = -30;
		}
		erro = read_line_sensor() - SET_POINT;
	}

}

//Virada do Verde
void greenTurn(bool side){
	walk(TURN_SPEED_90, TURN_TIME_90);
	turn(60, side);
	walk(TURN_SPEED_90, TURN_TIME_90/2);
}

/**
* LINE FOLLOWING
*/
void lineFollowing(){
	while(1){
		int sensor = read_line_sensor();
		int cor = read_color_sensor();
		// Detectando cor
		if(cor == 1){
			greenTurn(false);
		}else if(cor == 2){
			greenTurn(true);
		}

		if((sensor != 127) && (sensor != 0)){
			PID(sensor, OFFSET);
			displayBigTextLine(10, "%d", estado);
		}
		if(estado == 1){
			displayBigTextLine(10, "90 ESQUERDA %d", estado);
			// 90� ESQUERDA
			walk(TURN_SPEED_90, TURN_TIME_90);
			// Checando por encruzilhadas
			int erro = read_line_sensor() - SET_POINT;
			if(estado == 4){
				if((erro > 20) || (erro < -20)){
					turn(30, false);
				}
				continue;

			}
			// Checando se � uma curva de 90� ou mais
			else if(estado == 3){
				//Vira 90��� esquerda
				gTurn(true);
				//Anda para frente
				walk(TURN_SPEED_90, TURN_TIME_90/8);
				corrigir(8);
				continue;
			}
		}
		if(estado == 2){
			displayBigTextLine(10, "90 DIREITA %d", estado);
			// 90� DIREITA
			walk(TURN_SPEED_90, TURN_TIME_90);
			// Ler estado atual
			sensor = read_line_sensor();
			// Checando por encruzilhadas
			int erro = read_line_sensor() - SET_POINT;
			if(estado == 4){
				if((erro > 20) || (erro < -20)){
					turn(30, true);
				}
				continue;
			}
			// Checando se � uma curva de 90� ou mais
			else if(estado == 3){
				//Vira 90��� direita
				gTurn(false);
				//Anda para frente
				walk(TURN_SPEED_90, TURN_TIME_90/8);
				continue;
			}
		}
		if(estado == 3){
			GAP();
		}
	}
}

/**
* RESGASTE
*/
void resgateMode(){

}


/* ---------------------------------
||						  MAIN	    				||
--------------------------------- */
task main()
{
	calibrateThresh();
	while(1){
		if(!resgate){
			eraseDisplay();
			lineFollowing();
		}
		else
			resgateMode();
		}
}
