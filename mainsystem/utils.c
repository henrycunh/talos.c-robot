/**
 * Arquivo com as fun??es de utilidade, que s?o utilizadas
 * amplamente em conjunto com as fun??es espec?ficas
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
	displayCenteredBigTextLine(10,"%d",gyro);
}

/**
 * Realiza a convers?o entre magnitudes diferentes
 * --------------------------------------------------------------------
 * @param | [long] x       | Valor a ser convertido
 * @param | [long] in_min  | Valor m?nimo do tipo de unidade de entrada
 * @param | [long] in_max  | Valor m?ximo do tipo de unidade de entrada
 * @param | [long] out_min | Valor m?nimo do tipo de unidade de sa?da
 * @param | [long] out_max | Valor m?ximo do tipo de unidade de sa?da
 * --------------------------------------------------------------------
 * @return [long] O valor j? convertido
 */
long map( long x, long in_min, long in_max, long out_min, long out_max){
	return (x - in_min) * ( out_max - out_min) / ( in_max - in_min ) + out_min ;
}
