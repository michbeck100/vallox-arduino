#include <Arduino.h>
#include <Dac.h>

const int dacUpdownPin = 7;
const int UDacPin = 0;
const int AnalogSwitchPin = 3;
Dac dac(dacUpdownPin, UDacPin);

char switchPosition = '?';

void setVentilation(int value) {
  dac.write(value);
  Serial.println("Set ventilation to " + value);
}

char readAnalogSwitch() {
  int rawVal = analogRead(AnalogSwitchPin);
  if (rawVal > 286 && rawVal < 347)
    return 'A';
  if (rawVal > 704 && rawVal < 764)
    return '2';
  if (rawVal > 821 && rawVal < 882)
    return '3';
  if (rawVal > 993)
    return '4';
  return '?';
}


void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  setVentilation(700);
  switchPosition = readAnalogSwitch();
}

void loop() {
  static int counter = 0;
  char current;

  delay(100);
  counter++;

  current = readAnalogSwitch();
  if (current != '?' && current != switchPosition) {
    switchPosition = current;
    if (current == '2') {
      setVentilation(560);
    } else if (current == '3') {
      setVentilation(700);
    } else if (current == '4') {
      setVentilation(1023);
    }
  }

  /* every 5 sec: stabilize DAC output */
  if ( counter % 50 == 0) {
    dac.refresh();
  }

}
