#pragma config(Sensor, S1,     colorA,         sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S2,     i2c,            sensorEV3_GenericI2C)
#pragma config(Sensor, S3,     infraR,         sensorEV3_IRSensor)
#pragma config(Sensor, S4,     colorB,         sensorEV3_Color, modeEV3Color_Color)
/** CONFIGURA��O DOS SENSORES
 * S1 => Sensor de Cor Direito
 * S2 => Sensor do I2C
 * S3 => Sensor Infravermelho
 * S4 => Sensor de Cor Esquerdo
 */

// CONSTANTES GLOBAIS
#define ARDUINO_ADDRESS 0x08 // Endere�o do Arduino
#define ARDUINO_PORT S2 // Port do sensor do Arduino
#define KP 0.85 // Constante Proporcional do PID
#define KI 0.0 // Constante de Integral do PID
#define KD 0.2 // Constante Derivativa do PID
#define SET_POINT 65 // Ponto intermedi�rio das leituras do sensor QTR8
#define OFFSET -18 // Offset de Velocidade do PID
#define TURN_TIME_90 60 // Tempo da virada na curva de 90�
#define TURN_SPEED_90 30 // Velocidade da virada na curva de 90�
#define TURN_ERRO_K 8 // Erro permitido na virada da curva de 90�
#define IMAGE_KP 0.2 // Constante proporcional da busca no resgate
#define IMAGE_SETPOINT 47 // Ponto intermedi�rio da busca no resgate
#define COLOR_ERRO 6 // Erro permitido da cor durante a calibra��o
#define INT_COUNT_MAX 20 // M�ximo de itera��es da saida de estado
#define GYRO_THRESH_MAX 55 // Limiar m�ximo do girosc�pio
#define GYRO_THRESH_MIN 40 // Limiar m�nimo do girosc�pio
#define SMOOTH_K 0.2 // Costante do Exponential Smoothing, usado no girosc�pio
#define A_MOTOR_OFFSET 1 // Ajuste do offset no motor A
#define B_MOTOR_OFFSET 1 // Ajuste do offset no motor B

bool resgate = false; // Armazena o estado do resgate
bool corrigido = false; // Armazena o estado da corre��o
bool offRoad = false; // Armazena o estado do rob� em rela��o ao terreno
int limiarWhite[2][3]; // Armazena os limiares da cor branca
int linha; // Armazena posi��o do sensor QTR8-A | Lim: 0 - 127
int estado; // Armazena estado do sensor | {1, 2, 3, 4}
int gyro = 1; // Armazena a angula��o do rob�
int resgateCount = 0; // Armazena o contador para entrar no resgate
long coresA[3]; // Armazena as cores do sensor de cor direito
long coresB[3]; // Armazena as cores do sensor de cor esquerdo
TI2CStatus mI2CStatus; // Armazena o status do sensor
byte replyMsg[10]; // Armazena a resposta do I2C
byte sendMsg[10]; // Armazena a mensagem a ser enviada


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
 * Realiza a convers�o entre magnitudes diferentes
 * --------------------------------------------------------------------
 * @param | [long] x       | Valor a ser convertido
 * @param | [long] in_min  | Valor m�nimo do tipo de unidade de entrada
 * @param | [long] in_max  | Valor m�ximo do tipo de unidade de entrada
 * @param | [long] out_min | Valor m�nimo do tipo de unidade de sa�da
 * @param | [long] out_max | Valor m�ximo do tipo de unidade de sa�da
 * --------------------------------------------------------------------
 * @return [long] O valor j� convertido
 */
long map( long x, long in_min, long in_max, long out_min, long out_max){
	return (x - in_min) * ( out_max - out_min) / ( in_max - in_min ) + out_min ;
}


/**
 * Envia e recebe dados dos componentes via protocolo
 * I2C, e permite o controle dos mesmos atrav�s de
 * seletos valores
 * ---------------------------------------------------------------------------------
 * @param | [int] reply_size                  | Tamanho da resposta em bytes
 * @param | [int] message_size                | Tamanho da mensagem a ser enviada
 * @param | [byte] byte1, byte2, byte3, byte4 | Bytes a serem enviados como mensagem
 */
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
	// Checa por erro de skip na transmiss�o do I2C
	bool skip = replyMsg[1] == 0;
	// Resposta, analisando o erro de skip
	linha = skip ? linha : replyMsg[0];
	estado = skip ? estado : replyMsg[1];
	// Aplica um Exponential Smoothing, caso n�o d� erro de skip
	gyro = skip ? gyro : (SMOOTH_K * replyMsg[2]) + ((1-SMOOTH_K) * gyro);
	// Espera a sincroniza��o
	wait1Msec(35);
}

#include "movement.c"
#include "sensors.c"
#include "heuristica.c"

// ESCOPO PRINCIPAL
task main
{
	// Manda mensagem para o Arduino sair do modo de resgate
	i2c_msg(2, 8, -1, 0, 0, 0);
	// Calibra o limiar de branco
	calibrateThresh();
	// Loop principal
	while(1){
			// Executa a fun��o de seguir linhas
			lineFollowing();
	}
}
