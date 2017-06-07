#include <AltSoftSerial.h>

int media[] = {0, 0, 0, 0, 0, 0, 0, 0};
AltSoftSerial mySerial;
//RX TX 

void setup() {
  Serial.begin(9660);
  delay(500);
  mySerial.begin(57600);
  pinMode(13, OUTPUT);

}


void loop() {
  Serial.println(0x01);
  mySerial.write(0x01);

}

