/**
 * Arquivo com as funções de comunicação com o sensor I2C
 * -------------------------------------------------------
 * @author Iago Elias
 * @author Henrique Cunha
 * @version 1.0
 */

/**
 * Envia e recebe dados dos componentes via protocolo
 * I2C, e permite o controle dos mesmos através de
 * seletos valores
 * ---------------------------------------------------------------------------------
 * @param | [int] reply_size                  | Tamanho da resposta em bytes
 * @param | [int] message_size                | Tamanho da mensagem a ser enviada
 * @param | [byte] byte1, byte2, byte3, byte4 | Bytes a serem enviados como mensagem
 */
void i2c_msg(int reply_size, int message_size, byte byte1, byte byte2, byte byte3, byte byte4, byte timeout){
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
	// Esperar um tempo
	delay(timeout);

	// Ler resposta
	readI2CReply(i2c, &replyMsg[0], reply_size);
	// Checa por erro de skip na transmissão do I2C
	bool skip = replyMsg[1] == 0;
	// Resposta, analisando o erro de skip
	linha = skip ? linha : replyMsg[0];
	estado = skip ? estado : replyMsg[1];
	ultra1 = replyMsg[3];
	ultra2 = replyMsg[4];
	// Aplica um Exponential Smoothing, caso não dê erro de skip
	gyro = skip ? gyro : (SMOOTH_K * replyMsg[2]) + ((1-SMOOTH_K) * gyro);
	// Espera a sincronização
	wait1Msec(35);
}
