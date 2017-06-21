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
bool offRoad = false; // Armazena o estado do robô em relação ao terreno
int limiarWhite[2][3]; // Armazena os limiares da cor branca
int linha; // Armazena posição do sensor QTR8-A | Lim: 0 - 127
int estado; // Armazena estado do sensor | {1, 2, 3, 4}
int gyro = 1; // Armazena a angulação do robô
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
			// Executa a função de seguir linhas
			lineFollowing();
	}
}
