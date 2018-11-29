#include <SPI.h>

#include <Arduino.h>
#include <Dac.h>

const int dacUpdownPin = 7;
const int UDacPin = 0;
const int SchalterPin = 3;
Dac dac(dacUpdownPin, UDacPin);

char schalter='?', modus='1';

void setLueftung(int newVal)
{
  Serial.write("setLueftung to " + newVal);
  dac.write(newVal);
}

char readSchalter()
{
  int rawVal=analogRead(SchalterPin);
  if (rawVal>286 && rawVal<347)
    return 'A';
  if (rawVal>704 && rawVal<764)
    return '2';
  if (rawVal>821 && rawVal<882)
    return '3';
  if (rawVal>993)
    return '4';
  return '?';  
} 


void setup() 
{
  Serial.begin(115200);

  setLueftung(230);
  schalter=readSchalter();
}

void loop()
{ 
  char buff[23];
  int len = 23;
  static int counter=0;
  char curSchalter;

  delay(100);
  counter++;
  
  curSchalter=readSchalter();
  if (curSchalter!='?' && curSchalter!=schalter) { 
    schalter=curSchalter;
    modus=curSchalter;
    /* using a normal array would consume precious RAM, 
       FLASH_ARRAY means 200 bytes of code increase, 
       so use longish code sequence instead... */
    if (curSchalter=='2')
      setLueftung(560);
    else if (curSchalter=='3')
      setLueftung(700);
    else if (curSchalter=='4')
      setLueftung(1023);
  } 

  /* every 5 sec: stabilize DAC output */
  if ( counter % 50 == 0)
    dac.refresh();
}

