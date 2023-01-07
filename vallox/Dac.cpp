/* 
Simple 10 bit DAC for the Arduino
Version 21-3-2013
1. changed constructor

Copyright (C) 2012  Albert van Dalen http://www.avdweb.nl
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License at http://www.gnu.org/licenses .

write(): Write a value to the DAC. 0 = 0V, 1023 = 10V.
read():  Read the actual output voltage.
refresh(): Because of leakage current, refresh the DAC periodically (10 sec. for 1 LSB error).
The settling time is max. 20ms.
Don't touch C1 / R1 during run.

               C1 100nF 10% MKT  
                _____||____
               |     ||    |  
           R1  |  |\       | LMC6482
   I/O 2--56k-----|-\      |
                  |  \ ____|____ DAC out 0 ... 10V
           R2     |  /    |     
     5V- -10k-----|+/     |
               |  |/  R4 10k
               |          |_____
           R3 10k         |     |
               |      R5 10k    |
               |          |     |
              GND        GND    |
    ADC 0-----------------------
         
            5V |      _       _
               |     | |     | |
  I/O 2        |     | |     | |
          2.5V |_____| |_____| |______
               |
               |
            0V |______________________
          
               |______
               |      \_______
 DAC out       |              \_______ 
 (not to scale)| 
               |______________________
*/

#include <Arduino.h>
#include "Dac.h"

Dac::Dac(int dacUpdownPin, int UDacPin):
dacUpdownPin(dacUpdownPin), UDacPin(UDacPin), overshoot(5)
{ write(512); // set to 2,5V 
  write(512); // the first conversion can be wrong 
}

bool Dac::write(int val)
{ targetVal = val;
  if(targetVal > 1023) targetVal = 1023;
  if(targetVal < 0) targetVal = 0;   
  
  if(abs(read() - targetVal) > overshoot) // avoid overshoot from setDac() for small value changes
    if(!setDac()) return false; 
  if(!fineTune()) return false;
  if(abs(read() - targetVal) > 1) return false; // final error check
  return true;
} 

bool Dac::refresh()
{ if(!fineTune()) return false;
  return true;
} 

int Dac::read() const// not inline
{ return analogRead(UDacPin);
}

inline int Dac::fastRead() const
{ return analogRead(UDacPin);
}
  
/* To increase the DAC speed, the overshoot is reduced by an overshoot value (5). 
   However, this mechanism has a small influence on the accuracy. The DAC error is 0 or 1. 
   When the overshoot value is changed to 0 the DAC error is mostly 0. This mechanism can perhaps be improved.
*/

bool Dac::setDac()  
{ const byte timeout1 (255); // maxSettlingTime1 = 195
  int targetCorr; 

  if(read() == targetVal) return true; 
  if(read() < targetVal) 
  { targetCorr = targetVal - overshoot; // reduce overshoot caused by adc delay
    dacUp();
    for(settlingTime1=0; settlingTime1 < timeout1; settlingTime1++) 
    { if(fastRead() >= targetCorr) 
      { dacHold(); 
        break;
      }
    }
  } else  
  { targetCorr = targetVal + overshoot;
    dacDown();
    for(settlingTime1=0; settlingTime1 < timeout1; settlingTime1++)
    { if(fastRead() <= targetCorr)  
      { dacHold(); 
        break;
      }
    }
  }
  dacHold(); // end always with hold, in case of timeout
  if(settlingTime1 >= timeout1) return false;
  else return true;
}

bool Dac::fineTune() // produces no overshoot 
{ const byte timeout2 (80); // maxSettlingTime2 ~ 20
  const byte halfLsbCorrection (1);
  
  if(read() == targetVal) return true; // avoid ripple at refresh()
  if(read() < targetVal) 
  { for(settlingTime2=0; settlingTime2 < timeout2; settlingTime2++)
    { dacUp(); dacHold(); // finetuning with short pulse 
      if(fastRead() >= targetVal) 
      { for(int i=0; i<halfLsbCorrection; i++) dacUp(); // reduce error to 0
        break;
      }
    }     
  } else  
  { for(settlingTime2=0; settlingTime2 < timeout2; settlingTime2++)
    { dacDown(); dacHold(); // finetuning with short pulse 
      if(fastRead() <= targetVal) 
      { for(int i=0; i<halfLsbCorrection; i++) dacDown(); // reduce error to 0
        break;
      }
    }
  }
  dacHold(); // end always with hold, in case of timeout
  if(settlingTime2 >= timeout2) return false;
  else return true;
}

void Dac::dacUp() const
{ digitalWrite(dacUpdownPin, LOW);
  pinMode(dacUpdownPin, OUTPUT); 
}
void Dac::dacDown() const
{ digitalWrite(dacUpdownPin, HIGH);
  pinMode(dacUpdownPin, OUTPUT);
}

void Dac::dacHold() const
{ pinMode(dacUpdownPin, INPUT); // high impedance tristate 
  digitalWrite(dacUpdownPin, LOW); // disable pull up resistor 1*)
}
