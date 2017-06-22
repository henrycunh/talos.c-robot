#include <Wire.h>
#include <SoftwareSerial.h>
#include <Ultrasonic.h>
#include <TimerOne.h>

#define SLAVE_ADDRESS 0x04
//define o endereço I2C escravo do arduino como 4 em hexadecimal

SoftwareSerial mySerial(7, 6);
//define que as portas RX, TX de software serão, respectivamente 7 e 6
Ultrasonic ultrasonic1(11, 8, 4000UL);
Ultrasonic ultrasonic2(10, 9, 4000UL);

byte val = 0;
bool resgate = false;
bool flag = false;
bool ultra = false;
byte trans = 200;
byte aux;
byte estado;
uint8_t linha[] = {0, 0, 0, 0, 0, 0, 0, 0};
int buffer1[] = {0, 0, 0, 0, 0, 0, 0, 0};
float distancia1;
float distancia2;
const float smooth = 0.4;

void setup() {
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS); //Inicia a comunicação I2C
  Wire.onReceive(receiveData); //Define a função que irá receber os dados do EV3
  Wire.onRequest(sendData); //Define a função que enviará os dados sob requisito do EV3
  Timer1.initialize(500); // Inicializa o Timer1 e configura para um período de 0,5 segundos
  Timer1.attachInterrupt(callback); // Configura a função callback() como a função para ser chamada a cada interrupção do Timer1
  mySerial.begin(115200); //Inicia a porta do tipo SoftwareSerial
  mySerial.setTimeout(500);
  //pinMode(13, OUTPUT);
  //digitalWrite(13, LOW);

}


int callback() {
  if (resgate) {

    buffer1[0] = Serial.read();
    linha[0] = buffer1[0];
    linha[0] = constrain(linha[0], 0, 127);
    buffer1[1] = Serial.read();
    linha[1] = buffer1[1];
    linha[1] = constrain(linha[1], 0, 127);
  } else if (ultra) {
    //Captura o valor do sensor ultrasonico esquerdo
    distancia1 = ultrasonic1.distanceRead() * smooth + distancia1 * (1 - smooth);
    //Captura o valor do sensor ultrasonico direito
    distancia2 = ultrasonic2.distanceRead() * smooth + distancia2 * (1 - smooth);
    linha[0] = constrain((int)distancia1, 0, 127);
    linha[1] = constrain((int)distancia2, 0, 127);
    Serial.println(linha[4]);
    return 0;
    ultra = false;
  } else {
    if (mySerial.available()) {
      byte leitura = mySerial.read();
      if ((leitura > 127) && (leitura < 132)) {
        linha[1] = leitura - 127;
      } else if (leitura < 132) {
        linha[0] = leitura;
      }
      //Serial.println(linha[0]);
      linha[2] = map(analogRead(3), 440, 330, 0, 127);
      linha[0] = constrain(linha[0], 0, 127);
      //Serial.println(linha[0]);
      //Serial.println(linha[0]);
      //Serial.println(linha[0]);
      //Serial.println(linha[1]);
      //  linha[0] = mySerial.read();
      //}
    }

  }
  return 0;
}

void loop() {
  //aux = mySerial.read();
  //delay(2);
  //mySerial.flush();
  flag = !flag;
}

void receiveData(int byteCount) {
  //função que recebe os dados
  while (Wire.available() > 0) {

    val = Wire.read();
    Serial.println(val);
    if (val == 13) {
      //digitalWrite(13, HIGH);
      resgate = true;
    }
    if (val == 1) {
      resgate = false;
    }
    if (val == 10) {
      ultra = true;
    }
    //Serial.println(val);
    flag = true;
  }
}

/*byte getValue() {
  int lineBrute;
  int b = 1;
  int c = -8;
  byte line[] = {0, 0, 0, 0, 0, 0, 0, 0};
  int linha[] = {0, 0, 0, 0, 0, 0, 0, 0};
  if (mySerial.available()) {
    for (int a = 0; a < 8; a++) {
    Serial.println(a);
      linha[a] = mySerial.read();
      Serial.println(linha[a]);
    }
  }
  for (int a = 0; a < 8; a++) {
    if (a < 4) {
      line[a] = linha[a] * c;
      c = c / 2;
    } else {
      line[a] = linha[a] * b;
      b = b * 2;
    }
    lineBrute = lineBrute + line[a];
  }
  estado = encruzilhada(linha);
  Serial.println(lineBrute);
  byte value = (int)map(lineBrute, -2500, 2500, 0, 127); //Realiza a conversão do valor unitário da soma analógica para um valor transferível via I2C, ou seja, um valor entre 0 e 127
  value = constrain(value, 0, 127); // Garante que o valor enviado para o UNO estará entre 0 e 127
  return value;
  }*/
void sendData() {
  //Serial.println(linha[val]);
  //Serial.println("q");
  //linha[0] = constrain(buffer[0],0,127);
  //Serial.print("Wire:");
  //Serial.println(linha[0]);
  //linha[0] = buffer1[0];
  //Captura o valor do sensor ultrasonico esquerdo
  /*distancia1 = ultrasonic1.distanceRead() * smooth + distancia1 * (1 - smooth);
  //Captura o valor do sensor ultrasonico direito
  distancia2 = ultrasonic2.distanceRead() * smooth + distancia2 * (1 - smooth);
  linha[0] = constrain((int)distancia1, 0, 127);
  linha[1] = constrain((int)distancia2, 0, 127);
  Serial.println(linha[1]);*/
  Wire.write(linha, 8);

  //
  //mySerial.flush();


}

/*byte getData() {
  //mySerial.flush();
  if (mySerial.available()) {
    linha[0] = mySerial.read();
    while (linha[0] > 127) {
      linha[0] = mySerial.read();
    }
    linha[0] = constrain(linha[0], 0, 127);
  }
  return linha[0];
  }*/

