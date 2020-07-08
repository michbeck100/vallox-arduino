#ifndef DAC_H
#define DAC_H

/* 
Simple 10 bit DAC for the Arduino

Copyright (C) 2012  Albert van Dalen http://www.avdweb.nl
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License at http://www.gnu.org/licenses .
*/

class Dac
{ 
public:
  Dac(int dacUpdownPin, int UDacPin);
  bool write(int val); 
  bool refresh();
  int read() const;
  
  int targetVal;
  byte settlingTime1, settlingTime2;
  
private:
  inline int fastRead() const; 
  inline bool setDac();
  inline bool fineTune();
  inline void dacUp() const; 
  inline void dacDown() const;
  inline void dacHold() const;
  
  int overshoot;
  int dacUpdownPin; // Digital
  int UDacPin; // Analog in
};

#endif
