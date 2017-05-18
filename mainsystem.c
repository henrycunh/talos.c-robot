#pragma config(Sensor, S1,     colorA,         sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S2,     i2c,            sensorEV3_GenericI2C)
#pragma config(Sensor, S4,     colorB,         sensorEV3_Color, modeEV3Color_Color)
// Definindo endereços do Arduino
#define ARDUINO_ADDRESS 0x08
#define ARDUINO_PORT S2
#define KP 1.8
#define KI 0.0
#define KD 0.2
#define SET_POINT 65
#define OFFSET -15
#define TURN_RATE_90 1700
#define TURN_TIME_90 4
#define TURN_SPEED_90 30

//Mudan�a de magnitude para diferentes unidades

long map( long x, long in_min, long in_max, long out_min, long out_max){
	return (x - in_min) * ( out_max - out_min) / ( in_max - in_min ) + out_min ;
}
/* ---------------------------------
||							I2C								||
--------------------------------- */
//Variavel que armazena a posição do sensor de 0 a 127
int linha;
//Variavel que armazena o estado especial do sensor de linha
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
// Faz o robô andar para frente
void walk(int value, float duration){
	int a = getMotorEncoder(motorA);
	while(getMotorEncoder(motorA) + duration > a){
		motor[motorA] = -value;
		motor[motorB] = -value;
	}
}

int read_line_sensor(){
	i2c_msg(2,1,1,0,0,0);
	int value = linha;
	return value;
}

int read_color_sensor(bool right_sensor){
	int value = (right_sensor );
	return 0;
}

// Faz o robô girar em graus
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

	/*degrees = (465 * degrees) / 90;
	setMotorTarget(motorA, degrees, 50);
	setMotorTarget(motorB, degrees, -50);
	waitUntilMotorStop(motorB);*/
}
// PID
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
	while(estado == 3){
		motor[motorA] = 20;
		motor[motorB] = 20;
		read_line_sensor();
		displayBigTextLine(3, "%d", estado);
	}
	//int erro = 20;
	/*while(erro > 10){
			sensor = read_line_sensor();
			erro = PID(sensor, 0);
			displayBigTextLine(3, "%d", estado);
		}*/
	walk(20, 300);
	read_line_sensor();
	while(estado == 3){
		motor[motorA] = -20;
		motor[motorB] = -20;
		read_line_sensor();
		displayBigTextLine(3, "%d", estado);
	}
}

// Corrigir
void corrigir(int limiar){
	int erro = read_line_sensor() - SET_POINT;
	while(erro > limiar){
		erro = PID(read_line_sensor(), 0);
	}
}
// Função principal
task main()
{

	//walk(100, 20);
	while(1){
		int sensor = read_line_sensor();
		if((sensor != 127) && (sensor != 0)){
			writeDebugStreamLine("CARAIO: %d", sensor);
			PID(sensor, -20);
			displayBigTextLine(3, "%d", estado);
		} else if(estado == 3){
			// GAP
			GAP();
		}
	}
}


/*
long cores[3];
getColorRGB(colorA, cores[0], cores[1], cores[2]);

//Write the values to the Debug Stream
writeDebugStreamLine("Colour detected: %d, %d, %d", cores[0], cores[1], cores[2]);
sleep(100);*/
