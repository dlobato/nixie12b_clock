#include "NixieDisplay.h"
#include <SPI.h>

NixieDisplay::NixieDisplay(){
  //set SPI
  pinMode(SS,OUTPUT);
  digitalWrite(SS,HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  
  blinkDelayMS = BLINKSLOW_DELAYMS;
  
  for (int i=0; i<ELEMENUMMAX; i++){
    elemValues[i] = 0;
    elemState[i] = OFF;
  }
}

// HV5812 has a 20 bit shift register. Arduino shitfout sends 1byte (8bits) at a time
// So 3bytes (24bits) where the 4MSBs are ignored. XXXX(0_20)(O_19)....(0_1)
// The wiring O_15 = dp, 0_14..O_11 = controls anodes, 0_10..0_1 = control digits
// 0=on 1=off
void NixieDisplay::display(const ElemEnum elem){
  word digit = (1<<elemValues[elem]);
  byte shiftregData[3];//data[0]=XXXXO_20..O_17, data[1]=O_16..O_9, data[2]=O_8..O_1
  //set elem value
  shiftregData[0] = 0;
  shiftregData[1] = (byte)(digit >> 8);
  shiftregData[2] = (byte)digit;
  
  //check elem state
  switch(elemState[elem]){
  case OFF:
    break;
  case ON:
    shiftregData[1] |= (byte) (1<<elem+2);
    break;
  case BLINK:
    if (elemBlinkState[elem])
      shiftregData[1] |= (byte) (1<<elem+2);
    
    if (sinceBlinkChange[elem] >= blinkDelayMS){
      elemBlinkState[elem] = !elemBlinkState[elem];
      sinceBlinkChange[elem] -= blinkDelayMS;
    }
    break;
  default:
    elemState[elem] = OFF;
    break;
  }
   
  //shift out data 
  digitalWrite(SS, LOW);
  SPI.transfer(~shiftregData[0]);//high byte not used
  SPI.transfer(~shiftregData[1]);
  SPI.transfer(~shiftregData[2]);
  digitalWrite(SS, HIGH);
}

void NixieDisplay::setElem(ElemEnum elem, char value, ElemState state){
  if (value>=0 && value<10)
    elemValues[elem] = value;
    
  if (state == BLINK && elemState[elem] != BLINK){//reset state and sinceBlinkChange
      elemBlinkState[elem] = false;
      sinceBlinkChange[elem] = 0;
  }  
  elemState[elem] = state;
}

void NixieDisplay::setBlinkDelayMS(unsigned long delayms){
  blinkDelayMS = delayms;
}

//void NixieDisplay::setAllElems(char d0, ElemState sd0, char d1, ElemState sd1, char d2, ElemState sd2, char d3, ElemState sd3, char dp, ElemState sdp){
  
//}
