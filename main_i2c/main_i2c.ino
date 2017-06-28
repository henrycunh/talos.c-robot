
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>
#include <SimpleTimer.h>
#include <Ultrasonic.h>

#define SLAVE_ADDRESS 0x04
//define o endereço I2C escravo do arduino como 4 em hexadecimal

SoftwareSerial mySerial(7, 6);
//define que as portas RX, TX de software serão, respectivamente 7 e 6

SimpleTimer timer;
//Timer para o raspberry

Ultrasonic ultrasonic1(11, 8, 4000UL);
Ultrasonic ultrasonic2(10, 9, 4000UL);

byte val = 1;
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
  Timer1.initialize(500); // Inicializa o Timer1 e configura para um período de 0,5 milisegundos
  Timer1.attachInterrupt(callback); // Configura a função callback() como a função para ser chamada a cada interrupção do Timer1
  mySerial.begin(115200); //Inicia a porta do tipo SoftwareSerial
  mySerial.setTimeout(500);
  timer.setInterval(150, raspiData);
  //pinMode(13, OUTPUT);
  //digitalWrite(13, LOW);

}

void raspiData(){
  if (resgate) {
    atualizaResg();
  } 
  if (ultra) {
    atualizaUltra();
  }
}

int callback() {
  if (!ultra && !resgate){
    if (mySerial.available()) {
      atualizaLinha();
    }
  }
  //Serial.flush();
  return 0;
}

void loop() {
  timer.run();
}

void atualizaLinha(){
  byte leitura = mySerial.read();
  if ((leitura > 127) && (leitura < 132)) {
    linha[1] = leitura - 127;
  } else if (leitura < 132) {
    linha[0] = leitura;
  }
  linha[2] = map(analogRead(3), 440, 330, 0, 127);
  linha[0] = constrain(linha[0], 0, 127);
}

void atualizaResg(){
  if(Serial.available()){
    int b = Serial.available();
    for (int a = 0; a < b; a++){
      buffer1[0] = Serial.read();
      buffer1[1] = Serial.read();
      if(buffer1[0] > 127){
        linha[5] = constrain((buffer1[0] - 127), 0, 127);
      }
      if(buffer1[1] > 127){
        linha[5] = constrain((buffer1[1] - 127), 0, 127);
      }
    }
    if((buffer1[0] != -1) && (buffer1[0] < 128)){
      linha[0] = buffer1[0];
      linha[0] = constrain(linha[0], 0, 127);
      Serial.print("valor\t");
      Serial.print(linha[0]);
      Serial.print("\n");
    }
    if((buffer1[1] != -1) && (buffer1[1] < 128)){
      linha[1] = buffer1[1];
      linha[1] = constrain(linha[1], 0, 127);
    }
  }
}

void atualizaUltra(){
  distancia1 = ultrasonic1.distanceRead() * smooth + distancia1 * (1 - smooth);
  //Captura o valor do sensor ultrasonico direito
  distancia2 = ultrasonic2.distanceRead() * smooth + distancia2 * (1 - smooth);
  linha[3] = constrain((uint8_t)distancia1, 0, 127);
  linha[4] = constrain((uint8_t)distancia2, 0, 127);
  Serial.print(linha[3]);
  Serial.print("\t");
  Serial.print(linha[4]);
  Serial.print("\n");
}

void receiveData(int byteCount) {
  //função que recebe os dados
  while (Wire.available() > 0) {

    int value = Wire.read();
    bool skip = value == 0;
    val = skip ? val : value;
    //Serial.println(val);
    if (val == 13) {
      //digitalWrite(13, HIGH);
      resgate = true;
    }
    if (val == 1) {
      resgate = false;
      ultra = false;
    }
    if (val == 10) {
      ultra = true;
    }
    // valor para procurar o receptaculo
    //Serial.println(val);
    flag = true;
    Wire.flush();
  }
}
void sendData() {
  //Serial.println("enviando");
  Wire.write(linha, 8);

  //
  //mySerial.flush();
}
