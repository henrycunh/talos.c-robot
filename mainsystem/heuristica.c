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
int corrigir(int limiar){
	print("CORRIGINDO");
	int erro = read_line_sensor(1) - SET_POINT;
	while(erro > limiar){
		erro = PID(read_line_sensor(1), OFFSET/3, KP*2, SET_POINT);
	}
	corrigido = true;
	return 0;
}


/**
* Realiza o Gap, ao analisar se j� fora corrigido
* o erro da posi��o do rob� na linha, e anda para
* frente at� que o estado mude para o que representa
* uma linha dentro dos limites do sensor
*/
int gap(){
	if(garantiaRampa > 100){
		auxiliar = 1;
		return 0;
	}
	// Se n�o foi, anda um pouco para tr�s (VEJA ISSO AGORA)
	while(estado == 3){
		read_line_sensor(1);
		setSpeed(20, 20);
	}
	if(estado != 4)
		return 0;
	// E corrige
	if(estado == 4){
		corrigir(6);
	}
	if(estado != 4) return 0;
	while(estado != 3){
		if((estado == 1) || (estado == 2)){
			return 0;
		}
		if ((linha > SET_POINT + 6) || (linha < SET_POINT - 6)){
			return 0;
		}
		read_line_sensor(1);
		setSpeed(-20, -20);
	}
	//walk(20,80);
	// E enquanto estiver fora da linha
	read_line_sensor(1);
	while(estado == 3){
		// Anda para frente
		setSpeed(-20, -20);
		read_line_sensor(1);
		print("G A P");
	}
	return 0;
}
// Procura a bolinha
int searchBall(){
	static bool lado = true;
	// Caso a bolinha seja detectada pela camera
	if ((linha < 127) && (linha > 0)){
		// Para o rob�
		setSpeed(0,0);
		// Testa a dire��o do rob�
		bool turning = (( linha - IMAGE_SETPOINT ) > 0 ) ? true : false;
		int erroDis = linha - IMAGE_SETPOINT;
		if (erroDis < 0){
			erroDis = erroDis * -1;
		}
		// Vira em fun��o da dist�ncia da bolinha
		turn((erroDis*KPHOUGH), !turning);
		// Para o rob�
		setSpeed(0,0);
		// Vai um pouco para tr�s
		backin(4000);
		// Fecha a garra
		closeG();
		// Levanta a garra
		cDown();
		cDown();
		// Fecha a garra
		openG();
		// Se aproxima da parede
		PIDAprox();
		// Fecha a garra
		closeG();
		// Se afasta enquanto fecha a garra
		backin(4000);
		// Levanta a bolinha
		parseUP();
		parseUP();
		ball = true;
		return 1;
		// Caso a bolinha n�o seja detectada ele vira para o ultimo lado que estava (verificar)
		} else if (linha == 0){
		int dist = getIRDistance(infraR);
		int erroDi = - dist + 22;
		setSpeed(-IMAGE_OFFSET, IMAGE_OFFSET);
		}else if (linha == 127){
		int dist = getIRDistance(infraR);
		int erroDi = - dist + 22;
		setSpeed(IMAGE_OFFSET, -IMAGE_OFFSET);
	}
	return 0;
}
// Ajuste
bool ajuste(){
	turn(TURN_TIME_90/4, false);
	int dist = getIRDistance(infraR);
	turn(TURN_TIME_90/4, false);
	if((getIRDistance(infraR) - dist) > 0){
		do {
			dist = getIRDistance(infraR);
			turn(TURN_TIME_90/16, false);
			if (!ball){
				searchBall();
			}
		} while((getIRDistance(infraR) - dist) > 0);
		} else {
		do {
			dist = getIRDistance(infraR);
			turn(TURN_TIME_90/16, false);
			if (!ball){
				searchBall();
			}
		} while((getIRDistance(infraR) - dist) < 0);
	}
	if((getIRDistance(infraR) - dist) > 0){
		do {
			dist = getIRDistance(infraR);
			turn(TURN_TIME_90/16, false);
			if (!ball){
				searchBall();
			}
		} while((getIRDistance(infraR) - dist) > 0);
		} else {
		do {
			dist = getIRDistance(infraR);
			turn(TURN_TIME_90/16, false);
			if (!ball){
				searchBall();
			}
		} while((getIRDistance(infraR) - dist) < 0);
	}
	for(int a = 0; a < 10000; a++){
		motor[motorA] = - ((dist	- 25) * KP * 2);
		motor[motorB] = - ((dist	- 25) * KP * 2);
		dist = getIRDistance(infraR);
		if (!ball){
			searchBall();
		}
	}
	if((getIRDistance(infraR) - dist) > 0){
		do {
			dist = getIRDistance(infraR);
			turn(TURN_TIME_90/16, true);
			if (!ball){
				searchBall();
			}
		} while((getIRDistance(infraR) - dist) > 0);
		} else {
		do {
			dist = getIRDistance(infraR);
			turn(TURN_TIME_90/16, true);
			if (!ball){
				searchBall();
			}
		} while((getIRDistance(infraR) - dist) < 0);
	}
	if((getIRDistance(infraR) - dist) > 0){
		do {
			dist = getIRDistance(infraR);
			turn(TURN_TIME_90/16, true);
			if (!ball){
				searchBall();
			}
		} while((getIRDistance(infraR) - dist) > 0);
		} else {
		do {
			dist = getIRDistance(infraR);
			turn(TURN_TIME_90/16, true);
			if (!ball){
				searchBall();
			}
		} while((getIRDistance(infraR) - dist) < 0);
	}
	for(int a = 0; a < 10000; a++){
		motor[motorA] = - ((dist	- 25) * KP * 2);
		motor[motorB] = - ((dist	- 25) * KP * 2);
		if (!ball){
			searchBall();
		}
		dist = getIRDistance(infraR);
	}
	return true;
}
// Procura o recept�culo
int searchRecipe(){
	// Verifica se o recept�culo est� na posi��o certa
	if ((replyMsg[5] < 50 + IMAGE_ERRO) && (replyMsg[5] >= 50 - IMAGE_ERRO)){
		// Para o rob�
		setSpeed(0,0);
		int distanceInf = 0;
		// Se aproxima do recept�culo via PID
		for(int a = 0; a < 5000; a++){
			distanceInf = getIRDistance(infraR);
			if(distanceInf < 10) {
				for(int a = 0; a < 5000; a++){
					motor[motorA] = -30;
					motor[motorB] = -30;
				}
				} else {
				motor[motorA] = - ((distanceInf	- 20) * KP * 4);
				motor[motorB] = - ((distanceInf	- 20) * KP * 4);
			}
		}
		openG();
		// Desce a garra
		cDown();
		// Abre a garra
		openG();
		back();
		front(40);
		back();
		//rebolation();
		// Fecha a garra
		closeG();
		// Sobe a garra
		parseUP();
		return 1;
		// Caso o recept�culo n�o esteja no lugar certo ele se move na dire��o correta
		} else if (replyMsg[5] > 50){
		setSpeed(IMAGE_OFFSET * 2, -IMAGE_OFFSET * 2);
		} else {
		setSpeed(-IMAGE_OFFSET * 2, IMAGE_OFFSET * 2);
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
	int cor = read_color_sensor();
	walk(TURN_SPEED_90, TURN_TIME_90/2);
	read_line_sensor(1);
	if(estado != 3){
		corrigir(8);
		}else{
		turn(17, !side);
	}
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
			return 4;
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
	// Garante que n�o est� na rampa
	if (gyro > gyroV[0])
		return 0;
	if (garantiaRampa > 100)
		return 0;
	print("GRADE 90");
	// Garante que a detec��o de 90� n�o � um falso positivo e se n�o foi uma flutua��o
	if(estado != 4){
		// Entra na �rea branca para testar se n�o � uma encruzilhada ou qualquer outra coisa
		if (sairEstado(-1, 3) == 4){
			// Caso ele encontre uma linha, ele continua reto (cuidado!!!!!!!!!!!!!!) (verificar em caso de erro)
			return 0;
		}
	}
	// Quando estiver fora da linha
	if(estado == 3){
		read_line_sensor(1);
		int erro;
		// Vira um pouco para o lado contr�rio, para talvez aumentar o erro (testar sem)
		//turn(15, dir);
		// Verifica se o valor da linha que ele seguiria via PID realmente corresponde ao lado que ele deve virar
		bool testeL = dir? ( linha > SET_POINT? false : true ) : ( linha < SET_POINT? false : true);
		if (testeL) {
			// Caso seja o lado certo, ele corrige via PID
			do {
				print("VIRADA 90");
				// Corrige at� que o erro esteja dentro do definido
				erro = PID(read_line_sensor(1), OFFSET/2, KP, SET_POINT);
			}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);

			}else{
			// Caso o erro esteja do lado contr�rio, ele corrige com turn;
			do {
				print("VIRADA 90");
				// Corrige at� que o erro esteja dentro do definido
				turn(1, !dir);
			}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);
		}
	}
	read_line_sensor(1);
	int erro = read_line_sensor(1) - SET_POINT;
	// Eu n�o sei porque ele entra aqui, mas � relevante
	if(erro > 20 || erro < -20){
		int erro;
		// Caso ele esteja parcialmente na linha
		if(estado != 3){
			do {
				print("CORRIGINDO PARA TR�S");
				// Corrige at� que o erro esteja dentro do definido
				erro = PID(read_line_sensor(1), -OFFSET/2, KP, SET_POINT);
			}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);
		}
		else{
			while(estado == 3){
				// Marcador
				// Verifica se o valor da linha que ele seguiria via PID realmente corresponde ao lado que ele deve virar
				bool testeL = dir? ( linha > SET_POINT? false : true ) : ( linha < SET_POINT? false : true);
				if (testeL) {
					// Caso seja o lado certo, ele corrige via PID
					do {
						print("VIRADA 90");
						// Corrige at� que o erro esteja dentro do definido
						erro = PID(read_line_sensor(1), OFFSET/2, KP, SET_POINT);
					}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);

					}else{
					// Substitua o marcador com o conteudo abaixo se der tudo errado

					print("BACKWARDS");
					read_line_sensor(1);
					// Anda para tr�s
					setSpeed(20, 20);
					// Corrige novamente
				}
			}
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
	read_line_sensor(1);
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
	// Checa se algum obst�culo j� foi superado
	if (obst)
		return 0;
	// Verifica se est� na rampa
	if (garantiaRampa > 100)
		return 0;
	// Checa pela colis�o
	if(getIRDistance(infraR) < range){
		//Anda at� ficar na dist�ncia certa em rela��o ao obst�culo
		int distanceInf = getIRDistance(infraR);
		while(distanceInf - SET_POINT_INFRA > 1 || distanceInf - SET_POINT_INFRA < -1){
			print("AJUSTANDO AO OBST�CULO");
			motor[motorA] = - ((distanceInf	- SET_POINT_INFRA) * KP * 2);
			motor[motorB] = - ((distanceInf	- SET_POINT_INFRA) * KP * 2);
			distanceInf = getIRDistance(infraR);
		}
		read_line_sensor(1);

		// Verifica se o obst�culo n�o foi encontrado no momento errado
		if (estado == 3){
			walk(-TURN_SPEED_90, TURN_TIME_90*4);
			turn(15, true);
			return 0;
			}else{
			walk(-TURN_SPEED_90, TURN_TIME_90*2);
		}
		// Corrige
		corrigir(5);
		// Vira 90� para direita
		turning(false);
		// Anda para frente
		walk(TURN_SPEED_90, TURN_TIME_90*10);
		// Vira 90� para esquerda
		turning(true);
		// Anda para frente
		walk(TURN_SPEED_90, TURN_TIME_90*18);
		// Vira 90� para esquerda
		turning(true);
		// Anda para frente at� achar a linha
		read_line_sensor(1);
		while(estado == 3){
			read_line_sensor(1);
			motor[motorA] = - 20;
			motor[motorB] = - 20;
			displayCenteredBigTextLine(1, "PROCURANDO A LINHA");
		}
		// Anda um pouco mais pra frente
		walk(TURN_SPEED_90, TURN_TIME_90*2);
		// Vira para direita
		turning(false);
		// Anda para tr�s enquanto estiver fora
		read_line_sensor(1);
		if (estado == 3) {
			while	(estado == 3){
				read_line_sensor(1);
				setSpeed(30, 30);
			}
			} else {
			// Ou s� anda para tr�s mesmo
			walk(-TURN_SPEED_90, TURN_TIME_90*4);
		}
		// Fica indo para tr�s e para frente para garantir que ele n�o fique fora da linha
		while (estado == 3){
			if (estado == 3) {
				int c = 0;
				// Vai pra tr�s procurando a linha at� um contador superar 1000
				while	(estado == 3){
					c++;
					read_line_sensor(1);
					setSpeed(30, 30);
					if (c > 1000)
						break;
				}
			}
			if (estado == 3) {
				int c = 0;
				// Vai pra frente procurando a linha at� um contador superar 2000
				while	(estado == 3){
					c++;
					read_line_sensor(1);
					setSpeed(-30, -30);
					if (c > 2000)
						break;
				}
			}
		}
		corrigir(5);
		// Diz que o obst�culo j� foi superado
		obst = true;
		return 1;
	}
	return 0;
}
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
	if(gyro > gyroV[0]){// || gyro < gyroV[1]){
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

// Ciclo de resgate final

void cicloResgate(){
	int count = 0;
	// Vari�vel que armazena tanto se a bola foi capturada quanto se o recept�culo foi encontrado
	bool search = false;
	// Loop que executar� at� a finaliza��o do c�digo
	while(1){
		// Loop de procura
		parseUP();
		while((!search)){
			count++;
			if(count > 50){
				for(int a = 0; a < 60; a++){
					setSpeed(-20, -20);
					if(searchBall()) break;
				}
				for(int a = 0; a < 60; a++){
					setSpeed(20, 20);
					if(searchBall()) break;
				}
				count = 0;
			}
			i2c_msg(8, 8, 13, 0, 0, 0, 300);
			displayCenteredBigTextLine(1, "RESGATE");
			displayCenteredBigTextLine(5, "%d | %d", estado, linha);
			// Verifica se a bolinha foi encontrada e trata a informa��o
			search = searchBall();
		}
		parseUP();
		ajuste();
		// Procura pelo recept�culo
		if(search){
			search = false;
			while(!search){
				i2c_msg(8, 8, 13, 0, 0, 0, 300);
				displayCenteredBigTextLine(1, "RECEPT");
				displayCenteredBigTextLine(5, "%d", replyMsg[5]);
				search = searchRecipe();
			}
		}
		search = false;
		// Vai para tr�s para se afastar da parede
		parseUP();
		ajuste();
	}
}
