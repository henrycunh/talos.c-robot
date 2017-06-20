/**
 * Arquivo com as fun��es de movimenta��o do rob�
 * ----------------------------------------------
 * @author Iago Elias
 * @author Henrique Cunha
 * @version 1.0
 */

/**
 * Define a velocidade dos motores do rob�,
 * aliado ao offset definido nas constantes
 * para cada um dos motores
 * -------------------------------------------
 * @param | [int] a | Velocidade do motor A
 * @param | [int] b | Velocidade do motor B
 */
void setSpeed(int a, int b){
	motor[motorA] = a * A_MOTOR_OFFSET;
	motor[motorB] = b * B_MOTOR_OFFSET;
}

/**
 * Faz com que o rob� pare indefinitivamente
 */
void stopUs(){
	while (1){
		setSpeed(0, 0);
	}
}

/**
 * Faz com o que o rob� ande para frente
 * uma determinada quantidade de rota��es
 * ---------------------------------------------------
 * @param | [int] value      | Valor para a velocidade
 * @param | [float] duration | Dura��o da opera��o
 */
void walk(int value, float duration){
	print("WALK");
	resetMotorEncoder(motorA);
	int a = getMotorEncoder(motorA);
	if(value > 0){
		while(getMotorEncoder(motorA) + duration > a){
			setSpeed(-value, -value);
		}
	} else {
		while(getMotorEncoder(motorA) - duration < a){
			setSpeed(-value, -value);
		}
	}
}

/**
 * Faz com que o rob� vire uma determinada quantidade
 * de unidades, em uma dire��o
 * --------------------------------------------------------------------
 * @param | [float] value    | Valor adimensional da angula��o da curva
 * @param | [bool] direction | Dire��o da voltar
 */
void turn(float value, bool direction){
	print("TURN");
	value = map(value, 0, 180, 0, 1320);
	int a = getMotorEncoder(motorA);
	if(direction){
		while(getMotorEncoder(motorA) < a + value){
			setSpeed(30, -30);
		}
	}	else {
		while(getMotorEncoder(motorA) + value > a ){
			setSpeed(-30, 30);
		}
	}
}


/**
 * Fun��o que executa o ajuste PID
 * -------------------------------------------------------------------------
 * @param | [int] input      | Entrada de dados para o ajuste
 * @param | [int] offset     | Velocidade utilizada nos motores
 * @param | [float] KP1      | Constante proporcional utilizada no ajuste
 * @param | [int] SET_POINT1 | Constante que determina o ponto intermedi�rio
 */
int PID(int input, int offset, float KP1, int SET_POINT1){
	// Ultimo input
	static int lstinput;
	// Erro acumulado
	static float erroAc;
	int erro = input - SET_POINT1;
	displayBigTextLine(1, "GENERIC PID: %d", erro);
	float valor = (erro * KP1) + (input - lstinput) * KD + (erroAc) * KI;
	setSpeed(valor + offset, -valor + offset);
	lstinput = input;
	erroAc += erro;
	if(erro < 8 && erro > -8)
		corrigido = true;
	return erro;
}
