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
#define OFFSET -20

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
	motor[motorA] = -value;
	motor[motorB] = -value;
	sleep((int)duration*100);
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
void turn(float degrees){
	degrees = (465 * degrees) / 90;
	setMotorTarget(motorA, degrees, 50);
	setMotorTarget(motorB, degrees, -50);
	waitUntilMotorStop(motorB);
}
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
// Função principal
task main()
{

	//turn(90);
	while(1){
		int sensor = read_line_sensor();
		if((sensor != 127) && (sensor != 0) && (estado == 4)){
			writeDebugStreamLine("CARAIO: %d", sensor);
			PID(sensor, -20);
			displayBigTextLine(3, "%d", estado);
		} else if(estado == 1){


		}  else if(estado == 2){


		} else if(estado == 3){
			while(estado == 3){
				motor[motorA] = 20;
				motor[motorB] = 20;
				read_line_sensor();
				displayBigTextLine(3, "%d", estado);
			}
			if((estado == 1) || (estado == 2)){
				continue;
			}
			int erro = 20;
			while(erro > 10){
				sensor = read_line_sensor();
				erro = PID(sensor, 0);
				displayBigTextLine(3, "%d", estado);
			}
			walk(20, 10);
			read_line_sensor();
			while(estado == 3){
				motor[motorA] = -20;
				motor[motorB] = -20;
				read_line_sensor();
				displayBigTextLine(3, "%d", estado);
			}
		}
	}
}


/*
long cores[3];
getColorRGB(colorA, cores[0], cores[1], cores[2]);

//Write the values to the Debug Stream
writeDebugStreamLine("Colour detected: %d, %d, %d", cores[0], cores[1], cores[2]);
sleep(100);*/
