#include <Arduino.h>
#include <SPI.h>
#include <EthernetENC.h>
#include <PubSubClient.h>

#include "Dac.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// mqtt client
EthernetClient net;
PubSubClient mqttClient(net);

const char broker[] = "mqtt.nas.fritz.box";
int        port     = 1883;

const int dacUpdownPin = 7;
const int UDacPin = 0;
const int AnalogSwitchPin = 3;
Dac dac(dacUpdownPin, UDacPin);

char switchPosition = '?';

void setVentilation(int value) {
  Serial.print("Setting value to ");
  Serial.println(value);
  if (value < 560 || value > 1023) {
    Serial.println("Invalid value, must be between 560 and 1023");
    return;
  }
  dac.write(value);
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

void callback(char* topic, byte* payload, unsigned int length) {
  char format[16];
  snprintf(format, sizeof format, "%%%ud", length);

  // Convert the payload
  int payload_value = 0;
  if (sscanf((const char *) payload, format, &payload_value) == 1)
    setVentilation(payload_value);
  else
    Serial.println("error converting payload to int");
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("vallox-arduino")) {
      Serial.println("connected");
      mqttClient.subscribe("vallox/ventilation/set");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // connect to mqtt broker 
  mqttClient.setServer(broker, port);
  mqttClient.setCallback(callback);

  // start the Ethernet connection and the server:
  Ethernet.init(10);
  Ethernet.begin(mac);
  
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  Serial.print("ip address is ");
  Serial.println(Ethernet.localIP());

  setVentilation(700);
  switchPosition = readAnalogSwitch();
}

void loop() {
  static int counter = 0;
  char current;

  delay(100);

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

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
  
  counter++;
  /* every 5 sec */
  if ( counter % 50 == 0) {
    //stabilize DAC output
    dac.refresh();

    // send message, the Print interface can be used to set the message contents
    char payload[16];
    itoa(dac.read(), payload, 10);
    mqttClient.publish("vallox/ventilation", payload);
  }

}
