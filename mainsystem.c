#pragma config(Sensor, S1,     colorA,         sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S2,     i2c,            sensorEV3_GenericI2C)
#pragma config(Sensor, S4,     colorB,         sensorEV3_Color, modeEV3Color_Color)
// Definindo endereços do Arduino
#define ARDUINO_ADDRESS 0x08
#define ARDUINO_PORT S2
#define KP 1.2
#define SET_POINT 73
#define OFFSET -20

/* ---------------------------------
||							I2C								||
--------------------------------- */
// Armazena o status do sensor
TI2CStatus mI2CStatus;
// Armazena a resposta
byte replyMsg[10];
// Armazena a mensagem a ser enviada
byte sendMsg[10];

// Mandar e receber mensagens para o I2C
int i2c_msg(int reply_size, int message_size, byte byte1, byte byte2, byte byte3, byte byte4){
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
	sendI2CMsg(i2c, &sendMsg[0], 1);
	// Esperar 30ms
	wait1Msec(30);

	// Ler resposta
	readI2CReply(i2c, &replyMsg[0], reply_size);
	// Resposta
	int resp = replyMsg[0];
	wait1Msec(35);
	return resp;
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
	int value = i2c_msg(1,1,1,0,0,0);
	return value;
}

int read_color_sensor(bool right_sensor){
	int value = (right_sensor );
}

// Faz o robô girar em graus
void turn(float degrees){
	degrees = (465 * degrees) / 90;
	setMotorTarget(motorA, degrees, 50);
	setMotorTarget(motorB, degrees, -50);
	waitUntilMotorStop(motorB);
}
void PID(int input){
	int erro = input - SET_POINT;
	motor[motorA] = (erro * KP) + OFFSET;
	motor[motorB] = -(erro * KP) + OFFSET;
}
// Função principal
task main()
{

	//turn(90);
	while(1){
		int sensor = read_line_sensor();
		if(sensor != 127){
			writeDebugStreamLine("CARAIO: %d", sensor);
			PID(sensor);
		}
	}
}


/*
long cores[3];
getColorRGB(colorA, cores[0], cores[1], cores[2]);

//Write the values to the Debug Stream
writeDebugStreamLine("Colour detected: %d, %d, %d", cores[0], cores[1], cores[2]);
sleep(100);*/
