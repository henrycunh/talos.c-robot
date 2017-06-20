/**
 * Arquivo com as funções de utilidade, que são utilizadas
 * amplamente em conjunto com as funções específicas
 * -------------------------------------------------------
 * @author Iago Elias
 * @author Henrique Cunha
 * @version 1.0
 */

/**
 * Apaga a tela e imprime uma string no display do EV3
 * --------------------------------------------------------
 * @param | [char[]] str | String a ser impressa no display
 */
void print(char *str){
	eraseDisplay();
	displayCenteredBigTextLine(1,"%s",str);
	displayCenteredBigTextLine(10,"%s",gyro);
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
