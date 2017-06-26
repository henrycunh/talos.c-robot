/**
 * Arquivo com as funções de movimentação do robô
 * ----------------------------------------------
 * @author Iago Elias
 * @author Henrique Cunha
 * @version 1.0
 */

/**
 * Define a velocidade dos motores do robô,
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

void moveY(int count, int vel){

	for(int a = 0; a < count; a++){
		motor[motorD] = vel;
	}
	motor[motorD] = 0;
}

void front(float vel){
	motor[motorA] = -vel;
	motor[motorB] = -vel;
}

void cDown(){
	moveY(30000, DWVEL)
}

void wB(){
	while(getButtonPress(2) == 0){}
}

void parseUP(){
	moveY(25000, UPVEL)
}

void parseDW(){
	moveY(24000, DWVEL)
}

void closeG(){
	for(int a = 0; a < 5000; a++){
		motor[motorC] = -60;
	}
}

void openG(){
	for(int a = 0; a < 5000; a++){
		motor[motorC] = 60;
	}
}
void back(){
	for(int a = 0; a < 5000; a++){
		motor[motorA] = 30;
		motor[motorB] = 30;
	}
	motor[motorA] = 0;
	motor[motorB] = 0;
}

void stopM(){
	motor[motorA] = 0;
	motor[motorB] = 0;
}

void PIDaprox(){
	int erro = getIRDistance(S3) - SETPOINTIR;
	while(erro > 4 || erro < -4){
		displayCenteredBigTextLine(1,"%d | %d",erro, getIRDistance(S3));
		front(erro*KPIR);
		erro = getIRDistance(S3) - SETPOINTIR;
	}

	stopM();
}

/**
 * Faz com que o robô pare indefinitivamente
 */
void stopUs(){
	while (1){
		setSpeed(0, 0);
	}
}

void gotcha(){
	closeG();
	cDown();
	openG();
	PIDAprox();
	closeG();
	back();
	parseUP();
}
/**
 * Faz com o que o robô ande para frente
 * uma determinada quantidade de rotações
 * ---------------------------------------------------
 * @param | [int] value      | Valor para a velocidade
 * @param | [float] duration | Duração da operação
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
 * Faz com que o robô vire uma determinada quantidade
 * de unidades, em uma direção
 * --------------------------------------------------------------------
 * @param | [float] value    | Valor adimensional da angulação da curva
 * @param | [bool] direction | Direção da voltar
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
 * Função que executa o ajuste PID
 * -------------------------------------------------------------------------
 * @param | [int] input      | Entrada de dados para o ajuste
 * @param | [int] offset     | Velocidade utilizada nos motores
 * @param | [float] KP1      | Constante proporcional utilizada no ajuste
 * @param | [int] SET_POINT1 | Constante que determina o ponto intermediário
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
