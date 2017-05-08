    #include <QTRSensors.h>
#include <SoftwareSerial.h>

#define NUM_SENSORS             8  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  2  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2

// sensors 0 through 5 are connected to analog inputs 0 through 5, respectively
QTRSensorsAnalog qtra((unsigned char[]) {
  0, 1, 2, 3, 4, 5, 6, 7
},
NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];
int media[] = {0, 0, 0, 0, 0, 0, 0, 0};
SoftwareSerial mySerial(10, 11);

void setup() {
  delay(500);
  mySerial.begin(115200);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);    // turn on Arduino's LED to indicate we are in calibration mode
  Serial.begin(9600);
  for (int i = 0; i < 400; i++) { // make the calibration take about 10 seconds
    qtra.calibrate();       // reads all sensors 10 times at 2.5 ms per six sensors (i.e. ~25 ms per call)
    Serial.println("aqui");
  }
  digitalWrite(13, LOW);     // turn off Arduino's LED to indicate we are through with calibration

  // print the calibration minimum values measured when emitters were on
  //for (int i = 0; i < NUM_SENSORS; i++) {
  //  Serial.print(qtra.calibratedMinimumOn[i]);
  //  Serial.print(' ');
  //}
  //Serial.println();

  // print the calibration maximum values measured when emitters were on
  //for (int i = 0; i < NUM_SENSORS; i++) {
  //Serial.print(qtra.calibratedMaximumOn[i]);
  //Serial.print(' ');
  //}
  for (int i = 0; i < 8; i++) {
    Serial.println("estou aqui");
    media[i] = (qtra.calibratedMinimumOn[i] + qtra.calibratedMaximumOn[i]) / 2;
  }
  //Serial.println();
  //Serial.println();

}


void loop() {
  // read calibrated sensor values and obtain a measure of the line position from 0 to 5000
  // To get raw sensor values, call:
  //  qtra.read(sensorValues); instead of unsigned int position = qtra.readLine(sensorValues);
  unsigned int position = qtra.readLine(sensorValues);
  byte linha[] = {map(position, 0, 7000, 0, 127), getEstado()};
  //Serial.println(linha[0]);
  //Serial.println(linha[1]);
  mySerial.write(linha[0]);
  //Serial.println(linha[0]);
  /*for (unsigned char i = 0; i < NUM_SENSORS; i++) {
    Serial.print(sensorValues[i]);
    Serial.print('\t');
    }*/
  //Serial.println(); // uncomment this line if you are using raw values
  //Serial.println(position); // comment this line out if you are using raw values
}

byte getEstado() {
  if (((sensorValues[0] > media[0]) && (sensorValues[2] > media[2])) && ((sensorValues[3] > media[3]) && (sensorValues[4] > media[4])) && ((sensorValues[5] < media[5]) && (sensorValues[7] < media[7]))) {
    return 128;
  } else if (((sensorValues[0] < media[0]) && (sensorValues[2] < media[2])) && ((sensorValues[3] > media[0]) && (sensorValues[2] > media[2])) && ((sensorValues[5] > media[5]) && (sensorValues[7] > media[7]))) {
    return 129;
  } else if (((sensorValues[0] < media[0]) && (sensorValues[2] < media[2])) && ((sensorValues[3] < media[3]) && (sensorValues[4] < media[4])) && ((sensorValues[5] < media[5]) && (sensorValues[7] < media[7]))) {
    return 130;
  } else {
    return 131;
  };
}

