/**
* Arquivo com as funções de fluxo heurístico, que
* nos ajuda a fazer soluções com 'educated guesses'
* ------------------------------------------------
* @author Iago Elias
* @author Henrique Cunha
* @version 1.0
*/

/**
* Corrige o robô, dado um limiar, através do
* PID com um offset menor
* -----------------------------------------------------------
* @param | [int] limiar | Limiar a ser atingido pela correção
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
* Realiza o Gap, ao analisar se já fora corrigido
* o erro da posição do robô na linha, e anda para
* frente até que o estado mude para o que representa
* uma linha dentro dos limites do sensor
*/
int gap(){
	if(garantiaRampa > 100){
		auxiliar = 1;
		return 0;
	}
	// Se não foi, anda um pouco para trás (VEJA ISSO AGORA)
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
		// Para o robô
		setSpeed(0,0);
		// Testa a direção do robô
		bool turning = (( linha - IMAGE_SETPOINT ) > 0 ) ? true : false;
		int erroDis = linha - IMAGE_SETPOINT;
		if (erroDis < 0){
			erroDis = erroDis * -1;
		}
		// Vira em função da distância da bolinha
		turn((erroDis*KPHOUGH), !turning);
		// Para o robÔ
		setSpeed(0,0);
		// Vai um pouco para trás
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
		// Caso a bolinha não seja detectada ele vira para o ultimo lado que estava (verificar)
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
// Procura o receptáculo
int searchRecipe(){
	// Verifica se o receptáculo está na posição certa
	if ((replyMsg[5] < 50 + IMAGE_ERRO) && (replyMsg[5] >= 50 - IMAGE_ERRO)){
		// Para o robô
		setSpeed(0,0);
		int distanceInf = 0;
		// Se aproxima do receptáculo via PID
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
		// Caso o receptáculo não esteja no lugar certo ele se move na direção correta
		} else if (replyMsg[5] > 50){
		setSpeed(IMAGE_OFFSET * 2, -IMAGE_OFFSET * 2);
		} else {
		setSpeed(-IMAGE_OFFSET * 2, IMAGE_OFFSET * 2);
	}
	return 0;
}

/**
* Movimenta o robô para a frente até
* sair de cima de uma área verde
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
* Faz a virada de 90° utilizada ao
* detectar uma área verde, dado o lado
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
* Faz a virada de 90°
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
* e uma direção - frente ou ré
* ------------------------------------------
* @param | [int] mult | Direção do movimento
* @param | [int] std  | Estado a ser evitado
*/
int sairEstado(int mult, int std){
	// Contador de interações
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
		// Adiciona à quantidade de iterações
		intCount++;
		if (estado == 4)
			return 4;
		// Caso ultrapasse a quantidade máxima de iterações
		if(intCount > INT_COUNT_MAX){
			// Corrige
			corrigir(8);
			return 0;
		}
	} while(estado != std);
	return 0;
}


/**
* Faz a virada de 90° dado uma direção
* -------------------------------------
* @param | [bool] dir | Lado da curva
*/
int grade90(bool dir){
	// Garante que não está na rampa
	if (gyro > gyroV[0])
		return 0;
	if (garantiaRampa > 100)
		return 0;
	print("GRADE 90");
	// Garante que a detecção de 90º não é um falso positivo e se não foi uma flutuação
	if(estado != 4){
		// Entra na área branca para testar se não é uma encruzilhada ou qualquer outra coisa
		if (sairEstado(-1, 3) == 4){
			// Caso ele encontre uma linha, ele continua reto (cuidado!!!!!!!!!!!!!!) (verificar em caso de erro)
			return 0;
		}
	}
	// Quando estiver fora da linha
	if(estado == 3){
		read_line_sensor(1);
		int erro;
		// Vira um pouco para o lado contrário, para talvez aumentar o erro (testar sem)
		//turn(15, dir);
		// Verifica se o valor da linha que ele seguiria via PID realmente corresponde ao lado que ele deve virar
		bool testeL = dir? ( linha > SET_POINT? false : true ) : ( linha < SET_POINT? false : true);
		if (testeL) {
			// Caso seja o lado certo, ele corrige via PID
			do {
				print("VIRADA 90");
				// Corrige até que o erro esteja dentro do definido
				erro = PID(read_line_sensor(1), OFFSET/2, KP, SET_POINT);
			}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);

			}else{
			// Caso o erro esteja do lado contrário, ele corrige com turn;
			do {
				print("VIRADA 90");
				// Corrige até que o erro esteja dentro do definido
				turn(1, !dir);
			}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);
		}
	}
	read_line_sensor(1);
	int erro = read_line_sensor(1) - SET_POINT;
	// Eu não sei porque ele entra aqui, mas é relevante
	if(erro > 20 || erro < -20){
		int erro;
		// Caso ele esteja parcialmente na linha
		if(estado != 3){
			do {
				print("CORRIGINDO PARA TRÁS");
				// Corrige até que o erro esteja dentro do definido
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
						// Corrige até que o erro esteja dentro do definido
						erro = PID(read_line_sensor(1), OFFSET/2, KP, SET_POINT);
					}	while (erro > TURN_ERRO_K || erro < -TURN_ERRO_K);

					}else{
					// Substitua o marcador com o conteudo abaixo se der tudo errado

					print("BACKWARDS");
					read_line_sensor(1);
					// Anda para trás
					setSpeed(20, 20);
					// Corrige novamente
				}
			}
		}
	}
	return 0;
}

/**
* Controla o comportamento de heurística
* como curvas de 90° e detecção de áreas verde
* --------------------------------------------
* @param | [int] cor | Cor a ser analisada
*/
int heuristica(int cor){
	read_line_sensor(1);
	// Confere se o resgate foi ativado
	if (resgate)
		return 0;
	switch(estado){
	case 1: // Curva de 90° para a esquerda
		print("CURVA: ESQUERDA");
		grade90(true);
		break;
	case 2: // Curva de 90° para a direita
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
* Realiza a detecção e desvio de obstáculo
* através de uma análise sensível dos arredores
* e movimentação guiada por heurística
* ----------------------------------------------------------
* @param | [int] range | Alcançe mínimo para ativar o desvio
*/
int obstaculo(int range){
	// Checa se algum obstáculo já foi superado
	if (obst)
		return 0;
	// Verifica se está na rampa
	if (garantiaRampa > 100)
		return 0;
	// Checa pela colisão
	if(getIRDistance(infraR) < range){
		//Anda até ficar na distância certa em relação ao obstáculo
		int distanceInf = getIRDistance(infraR);
		while(distanceInf - SET_POINT_INFRA > 1 || distanceInf - SET_POINT_INFRA < -1){
			print("AJUSTANDO AO OBSTÁCULO");
			motor[motorA] = - ((distanceInf	- SET_POINT_INFRA) * KP * 2);
			motor[motorB] = - ((distanceInf	- SET_POINT_INFRA) * KP * 2);
			distanceInf = getIRDistance(infraR);
		}
		read_line_sensor(1);

		// Verifica se o obstáculo não foi encontrado no momento errado
		if (estado == 3){
			walk(-TURN_SPEED_90, TURN_TIME_90*4);
			turn(15, true);
			return 0;
			}else{
			walk(-TURN_SPEED_90, TURN_TIME_90*2);
		}
		// Corrige
		corrigir(5);
		// Vira 90° para direita
		turning(false);
		// Anda para frente
		walk(TURN_SPEED_90, TURN_TIME_90*10);
		// Vira 90° para esquerda
		turning(true);
		// Anda para frente
		walk(TURN_SPEED_90, TURN_TIME_90*18);
		// Vira 90° para esquerda
		turning(true);
		// Anda para frente até achar a linha
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
		// Anda para trás enquanto estiver fora
		read_line_sensor(1);
		if (estado == 3) {
			while	(estado == 3){
				read_line_sensor(1);
				setSpeed(30, 30);
			}
			} else {
			// Ou só anda para trás mesmo
			walk(-TURN_SPEED_90, TURN_TIME_90*4);
		}
		// Fica indo para trás e para frente para garantir que ele não fique fora da linha
		while (estado == 3){
			if (estado == 3) {
				int c = 0;
				// Vai pra trás procurando a linha até um contador superar 1000
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
				// Vai pra frente procurando a linha até um contador superar 2000
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
		// Diz que o obstáculo já foi superado
		obst = true;
		return 1;
	}
	return 0;
}
/**
* Realiza as funções de Line Following
* que são interpretadas na função principal
*/
int lineFollowing(){
	// Faz a leitura da linha e do verde
	int sensor = read_line_sensor(1);
	int cor = read_color_sensor();
	// Checa pelo resgate
	if (resgate)
		return 0;
	// Checa por um obstáculo
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
* Checa se o robô está na rampa, e define sua
* movimentação na mesma, além de se preparar para
* detectar o cinza, que indica para entrar no modo
* de resgate
* ------------------------------------------------------
* @param | [int] gyroRange | Limite mínimo do giroscópio
*/
int checkRampa(void){
	// Checa se o robô está no limite
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
	// Variável que armazena tanto se a bola foi capturada quanto se o receptáculo foi encontrado
	bool search = false;
	// Loop que executará até a finalização do código
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
			// Verifica se a bolinha foi encontrada e trata a informação
			search = searchBall();
		}
		parseUP();
		ajuste();
		// Procura pelo receptáculo
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
		// Vai para trás para se afastar da parede
		parseUP();
		ajuste();
	}
}
