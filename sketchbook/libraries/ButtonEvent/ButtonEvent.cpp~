/*
  ButtonEvent.cpp - Event-Based Library for Arduino.
  Copyright (c) 2011, Renato A. Ferreira
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of the author nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ButtonEvent.h"

Button::Button(int pin_,
	       unsigned long debounceIntervalMillis_,
	       void (*onDown_)(Button* Sender),
	       void (*onUp_)(Button* Sender),
	       void (*onHold_)(Button* Sender),
	       unsigned long holdMillisWait_)
  :pin(pin_), onDown(onDown_), onUp(onUp_), onHold(onHold_), holdMillisWait(holdMillisWait_), 
   buttonB(pin_,debounceIntervalMillis_), buttonState(BUTTON_INIT){
}

ButtonEventClass::ButtonEventClass(const unsigned int maxNButtons_)
  :maxNButtons(maxNButtons_), nButtons(0), buttons(new Button*[maxNButtons])  {}

void ButtonEventClass::addButton(int pin,
				 unsigned long debounceIntervalMillis,
				 void (*onDown)(Button* Sender),
				 void (*onUp)(Button* Sender), 
				 void (*onHold)(Button* Sender), unsigned long holdMillisWait) {
  if (this->nButtons < maxNButtons){
    this->buttons[this->nButtons] = new Button(pin, debounceIntervalMillis,
					       onDown, onUp, onHold, holdMillisWait);
    this->nButtons++;
  }
}

void ButtonEventClass::spinOnce() {
  Button *button_p;
  for (int b = 0; b < this->nButtons; b++){
    button_p = buttons[b];
    switch(button_p->buttonState){
    case BUTTON_INIT:
      if (button_p->buttonB.update()){ //button changed
        if (button_p->buttonB.risingEdge()){//down event
          button_p->buttonState = BUTTON_PRESSED;
	  button_p->startMillis = millis();
	  if (button_p->onDown) button_p->onDown(button_p);
        }
      }
      break;
    case BUTTON_PRESSED:
      if (button_p->buttonB.update()){ //button changed
        if (button_p->buttonB.fallingEdge()){//up event
          button_p->buttonState = BUTTON_INIT;
	  if (button_p->onUp) button_p->onUp(button_p);
        }
      }else{
        if ((millis() - button_p->startMillis) >= button_p->holdMillisWait){//hold event
          button_p->buttonState = BUTTON_HOLD;
	  if (button_p->onHold) button_p->onHold(button_p);
        }
      }
      break;
    case BUTTON_HOLD:
      if (button_p->buttonB.update()){ //button changed
        if (button_p->buttonB.fallingEdge()){//up event
          button_p->buttonState = BUTTON_INIT;
	  if (button_p->onUp) button_p->onUp(button_p);
        }
      }
      break;
    default:
      button_p->buttonState = BUTTON_INIT;
      break;
    }
  }
}
