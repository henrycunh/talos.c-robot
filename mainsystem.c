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
#define TURN_TIME_90 65
#define TURN_SPEED_90 30
#define G_THRES 2.6
#define TURN_ERRO_K 3

/* ---------------------------------
||						UTILS								||
--------------------------------- */
// Realiza mudan�a na magnitude
long map( long x, long in_min, long in_max, long out_min, long out_max){
	return (x - in_min) * ( out_max - out_min) / ( in_max - in_min ) + out_min ;
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

// L� o valor dos sensores de cor
int read_color_sensor(){
	long cores[3];
	/*getColorRGB(colorA, cores[0], cores[1], cores[2]);

	//Write the values to the Debug Stream
	displayCenteredBigTextLine(1,"Vermelho: %d", cores[0]);
	displayCenteredBigTextLine(4,"Verde: %d", cores[1]);
	displayCenteredBigTextLine(7,"Azul: %d", cores[2]);*/
	if(getColorName(colorB) == colorGreen){
		eraseDisplay();
		displayCenteredBigTextLine(1, "VIRAR ESQUERDA VERDE");
		return 1;
	}
	if(getColorName(colorA) == colorGreen){
		eraseDisplay();
		displayCenteredBigTextLine(1, "VIRAR DIREITA VERDE");
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
	/*while(estado == 3){
		motor[motorA] = 20;
		motor[motorB] = 20;
		read_line_sensor();
		displayBigTextLine(3, "%d", estado);
	}
	//int erro = 20;
	walk(20, 300);*/
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
	walk(TURN_SPEED_90, TURN_TIME_90);
}
/* ---------------------------------
||						  MAIN	    				||
--------------------------------- */
task main()
{

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
			// Ler estado atual
			sensor = read_line_sensor();
			// Checando por encruzilhadas
			if(estado == 4)
				continue;
			// Checando se � uma curva de 90� ou mais
			else if(estado == 3){
				//Vira 90��� esquerda
				gTurn(true);
				//Anda para frente
				walk(TURN_SPEED_90, TURN_TIME_90/4);
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
			if(estado == 4)
				continue;
			// Checando se � uma curva de 90� ou mais
			else if(estado == 3){
				//Vira 90��� direita
				gTurn(false);
				//Anda para frente
				walk(TURN_SPEED_90, TURN_TIME_90/3);
				corrigir(8);
				continue;
			}
		}
		if(estado == 3){
			GAP();
		}
	}

}
