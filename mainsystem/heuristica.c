/**
* Arquivo com as fun��es de fluxo heur�stico, que
* nos ajuda a fazer solu��es com 'educated guesses'
* ------------------------------------------------
* @author Iago Elias
* @author Henrique Cunha
* @version 1.0
*/

/**
* Corrige o rob�, dado um limiar, atrav�s do
* PID com um offset menor
* -----------------------------------------------------------
* @param | [int] limiar | Limiar a ser atingido pela corre��o
*/
void corrigir(int limiar){
	print("CORRIGINDO");
	int erro = read_line_sensor(1) - SET_POINT;
	while(erro > limiar){
		erro = PID(read_line_sensor(1), OFFSET/3, KP*2, SET_POINT);
	}
	corrigido = true;
}


/**
* Realiza o Gap, ao analisar se j� fora corrigido
* o erro da posi��o do rob� na linha, e anda para
* frente at� que o estado mude para o que representa
* uma linha dentro dos limites do sensor
*/
void gap(){
	// Se n�o foi, anda um pouco para tr�s
	walk(-20, 80);
	// E corrige
	corrigir(6);
	if(estado != 4) return;
	walk(20,80);
	// E enquanto estiver fora da linha
	read_line_sensor(1);
	while(estado == 3){
		// Anda para frente
		setSpeed(-20, -20);
		read_line_sensor(1);
		print("G A P");
	}
}

int searchBall(){
	//PID(linha, 0, IMAGE_KP, IMAGE_SETPOINT);
	long tAtual = getMicroTimer();
	if (linha == 0) {
		if ((tAtual - timer) >= TIMER_ESPERA){

			} else {
			setSpeed(0, 0);
		}
		} else {
		timer = tAtual;
	}
	if ((linha < IMAGE_SETPOINT + IMAGE_ERRO) && (linha >= IMAGE_SETPOINT - IMAGE_ERRO)){
		setSpeed(0,0);
		closeG();
		cDown();
		openG();
		PIDAprox();
		closeG();
		back();
		parseUP();
		parseDW();
		parseUP();
		return 1;
		} else if (linha > IMAGE_SETPOINT){
		setSpeed(IMAGE_OFFSET, -IMAGE_OFFSET);
		} else {
		setSpeed(-IMAGE_OFFSET, IMAGE_OFFSET);
	}
	return 0;
}

int searchRecipe(){
	//PID(linha, 0, IMAGE_KP, IMAGE_SETPOINT);
	long tAtual = getMicroTimer();
	if (replyMsg[5] == 0) {
		if ((tAtual - timer) >= TIMER_ESPERA){

			} else {
			setSpeed(0, 0);
		}
		} else {
		timer = tAtual;
	}
	if ((replyMsg[5] < 77 + IMAGE_ERRO) && (replyMsg[5] >= 77 - IMAGE_ERRO)){
		setSpeed(0,0);
		for(int a = 0; a < 30000; a++){
			setSpeed(-30, -30);
		}
		openG();
		return 1;
		} else if (replyMsg[5] > 77){
		setSpeed(IMAGE_OFFSET, -IMAGE_OFFSET);
		} else {
		setSpeed(-IMAGE_OFFSET, IMAGE_OFFSET);
	}
	return 0;
}

/**
* Movimenta o rob� para a frente at�
* sair de cima de uma �rea verde
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
* Faz a virada de 90� utilizada ao
* detectar uma �rea verde, dado o lado
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
* Faz a virada de 90�
* -------------------------------------
* @param | [bool] side | Lado da curva
*/
void turning(bool side){
	gExit();
	print("GREEN TURN");
	walk(TURN_SPEED_90, TURN_TIME_90);
	turn(50, side);
}

/**
* Sai do estado atual, dado o estado
* e uma dire��o - frente ou r�
* ------------------------------------------
* @param | [int] mult | Dire��o do movimento
* @param | [int] std  | Estado a ser evitado
*/
int sairEstado(int mult, int std){
	// Contador de intera��es
	int intCount = 0;
	do{
		print("SAIR DE ESTADO");
		read_line_sensor(1);
		// Anda para frente, dado o multiplicador
		setSpeed(20 * mult, 20 * mult);
		// Detectando cor
		int cor = read_color_sensor();
		if(cor == 1){
			greenTurn(false);
			gExit();
			return 0;
			}else if(cor == 2){
			greenTurn(true);
			gExit();
			return 0;
		}
		// Adiciona � quantidade de itera��es
		intCount++;
		if (estado == 4)
			return 0;
		// Caso ultrapasse a quantidade m�xima de itera��es
		if(intCount > INT_COUNT_MAX){
			// Corrige
			corrigir(8);
			return 0;
		}
	} while(estado != std);
	return 0;
}


/**
* Faz a virada de 90� dado uma dire��o
* -------------------------------------
* @param | [bool] dir | Lado da curva
*/
int grade90(bool dir){
	if (gyro > gyroV[0] - 10)
		return 0;
	print("GRADE 90");
	read_line_sensor(1);
	// Sai do estado atual
	// Garante que a detec��o de 90� n�o � um falso positivo
	if(estado != 4){
		if (gyro > gyroV[0])
			return 0;
		sairEstado(-1, 3);
	}
	// Se estiver fora da linha
	if(estado == 3){
		read_line_sensor(1);
		int erro;
		// Vira um pouco para o lado contr�rio
		turn(15, dir);
		bool testeL = dir? ( linha > SET_POINT? false : true ) : ( linha < SET_POINT? false : true);
		if (testeL) {
			do {
				print("GRADE 90");
				// Corrige at� que o erro esteja dentro do definido
				erro = PID(read_line_sensor(1), OFFSET/2, KP, SET_POINT);
			}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);
			}else{
			do {
				print("GRADE 90");
				// Corrige at� que o erro esteja dentro do definido
				turn(1, !dir);
			}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);
		}
	}
	read_line_sensor(1);
	int erro = read_line_sensor(1) - SET_POINT;
	if(erro > 20 || erro < -20){
		int erro;
		// Anda para tr�s
		//walk(-15,22);
		if(estado != 3){
			do {
				print("CORRIGINDO PARA TR�S");
				// Corrige at� que o erro esteja dentro do definido
				erro = PID(read_line_sensor(1), -OFFSET/2, KP, SET_POINT);
			}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);
		}
		else{
			while(estado == 3){
				print("BACKWARDS");
				read_line_sensor(1);
				// Anda para tr�s
				// Anda para frente, dado o multiplicador
				setSpeed(20, 20);
				// Corrige novamente
			}
			corrigir(5);
		}
	}
	return 0;
}

/**
* Controla o comportamento de heur�stica
* como curvas de 90� e detec��o de �reas verde
* --------------------------------------------
* @param | [int] cor | Cor a ser analisada
*/
int heuristica(int cor){
	// Confere se o resgate foi ativado
	if (resgate)
		return 0;
	switch(estado){
	case 1: // Curva de 90� para a esquerda
		print("CURVA: ESQUERDA");
		grade90(true);
		break;
	case 2: // Curva de 90� para a direita
		print("CURVA: DIREITA");
		grade90(false);
		break;
	case 3: // Realizar o Gap
		gap();
		break;
	}
	return 0;
}

/**
* Realiza a detec��o e desvio de obst�culo
* atrav�s de uma an�lise sens�vel dos arredores
* e movimenta��o guiada por heur�stica
* ----------------------------------------------------------
* @param | [int] range | Alcan�e m�nimo para ativar o desvio
*/
int obstaculo(int range){
	if (obst)
		return 0;
	// Checa pela colis�o
	if(getIRDistance(infraR) < range){
		//Anda at� ficar na dist�ncia certa em rela��o ao obst�culo
		int distanceInf = getIRDistance(infraR);
		while(distanceInf - SET_POINT_INFRA > 1 || distanceInf - SET_POINT_INFRA < -1){
			motor[motorA] = - ((distanceInf	- SET_POINT_INFRA) * KP * 2);
			motor[motorB] = - ((distanceInf	- SET_POINT_INFRA) * KP * 2);
			distanceInf = getIRDistance(infraR);
		}
		read_line_sensor(1);
		if (estado == 3){
			walk(-TURN_SPEED_90, TURN_TIME_90*4);
			turn(15, true);
			return 0;
			}else{
			walk(-TURN_SPEED_90, TURN_TIME_90*2);
		}
		corrigir(5);
		// Vira 90� para direita
		turning(false);
		walk(TURN_SPEED_90, TURN_TIME_90*10);
		turning(true);
		walk(TURN_SPEED_90, TURN_TIME_90*16);
		turning(true);
		//stopUs();
		read_line_sensor(1);
		while(estado == 3){
			read_line_sensor(1);
			motor[motorA] = - 20;
			motor[motorB] = - 20;
			displayCenteredBigTextLine(1, "PROCURANDO A LINHA");
		}
		walk(TURN_SPEED_90, TURN_TIME_90*2);
		turning(false);
		//corrigir(5);
		// Anda para tr�s
		walk(-TURN_SPEED_90, TURN_TIME_90*4);
		obst = true;
		return 1;
	}
	return 0;
}
/*int obstaculo(int range){
if (obst)
return 0;
// Checa pela colis�o
if(getIRDistance(infraR) < range){
//Anda at� ficar na dist�ncia certa em rela��o ao obst�culo
int distanceInf = getIRDistance(infraR);
while(distanceInf - SET_POINT_INFRA > 1 || distanceInf - SET_POINT_INFRA < -1){
motor[motorA] = - ((distanceInf	- SET_POINT_INFRA) * KP * 2);
motor[motorB] = - ((distanceInf	- SET_POINT_INFRA) * KP * 2);
distanceInf = getIRDistance(infraR);
}
read_line_sensor(1);
if (estado == 3){
walk(-TURN_SPEED_90, TURN_TIME_90*4);
return 0;
}else{
walk(-TURN_SPEED_90, TURN_TIME_90*2);
}
corrigir(5);
// Vira 90� para direita
turning(false);
//stopUs();
read_line_sensor(1);
int contador = 0;
while(contador < 80){
read_line_sensor(1);
motor[motorA] = - 20;
motor[motorB] = - 45;
contador++;
}
read_line_sensor(1);
while(estado != 4){
read_line_sensor(1);
motor[motorA] = - 20;
motor[motorB] = - 45;
}
turning(false);
//corrigir(5);
// Anda para tr�s
walk(-TURN_SPEED_90, TURN_TIME_90*4);
obst = true;
return 1;
}
return 0;
}
*/
/**
* Realiza as fun��es de Line Following
* que s�o interpretadas na fun��o principal
*/
int lineFollowing(){
	// Faz a leitura da linha e do verde
	int sensor = read_line_sensor(1);
	int cor = read_color_sensor();

	// Checa pelo resgate
	if (resgate)
		return 0;
	// Checa por um obst�culo
	obstaculo(18);
	// Checa pelo verde
	switch(cor){
	case 1:	// Vira para direita
		greenTurn(false);
		gExit();
		break;
	case 2: // Vira para esquerda
		greenTurn(true);
		gExit();
		break;
	}
	// Realiza PID
	if((sensor <= 127) && (sensor >= 0) && (estado == 4)){
		int erro = PID(sensor, OFFSET, KP, SET_POINT);
		eraseDisplay();
		displayCenteredBigTextLine(1, "PID: %d | %d", erro, gyro);
		return 0;
	}
	// Realiza a heuristica
	heuristica(cor);
	return 0;
}

/**
* Checa se o rob� est� na rampa, e define sua
* movimenta��o na mesma, al�m de se preparar para
* detectar o cinza, que indica para entrar no modo
* de resgate
* ------------------------------------------------------
* @param | [int] gyroRange | Limite m�nimo do girosc�pio
*/
int checkRampa(void){
	// Checa se o rob� est� no limite
	if(gyro > gyroV[0] || gyro < gyroV[1]){
		// Executa o PID
		int sensor = read_line_sensor(1);
		int erro = PID(sensor, OFFSET, KP, SET_POINT);
		eraseDisplay();
		displayCenteredBigTextLine(1, "RAMPA: %d | %d", erro, gyro);
		displayCenteredBigTextLine(5, "RAMPA: %d | %d", garantiaRampa, resgateCount);
		return 1;
	}
	return 0;
}
