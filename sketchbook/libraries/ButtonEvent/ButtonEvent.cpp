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

Button::Button(const int pin_,
	       const unsigned long debounceIntervalMillis_,
	       void (*onDown_)(Button* Sender),
	       void (*onUp_)(Button* Sender),
	       void (*onHold_)(Button* Sender),
	       const unsigned long holdMillisWait_,
	       const ButtonConfig cfg_)
  :pin(pin_), onDown(onDown_), onUp(onUp_), onHold(onHold_), 
   holdMillisWait(holdMillisWait_), 
   buttonB(pin_,debounceIntervalMillis_), cfg(cfg_), 
   buttonState(BUTTON_INIT) {}

ButtonEventClass::ButtonEventClass(const unsigned int maxNButtons_)
  :maxNButtons(maxNButtons_), nButtons(0), buttons(new Button*[maxNButtons])  {}

ButtonEventClass& ButtonEventClass::addButton(Button* button_p) {
  if (this->nButtons < maxNButtons){
    this->buttons[this->nButtons] = button_p;
    this->nButtons++;
  }
  return *this;
}

void ButtonEventClass::spinOnce() {
  Button *button_p;
  bool changed, up, down;
  for (int b = 0; b < this->nButtons; b++){
    button_p = buttons[b];
    changed = button_p->buttonB.update();
    if (button_p->cfg & Button::ACTIVE_HIGH){
      down = button_p->buttonB.risingEdge();
      up = button_p->buttonB.fallingEdge();
    }else{
      down = button_p->buttonB.fallingEdge();
      up = button_p->buttonB.risingEdge();
    }

    switch(button_p->buttonState){
    case Button::BUTTON_INIT:
      if (changed && down){
	button_p->buttonState = Button::BUTTON_PRESSED;
	button_p->startMillis = millis();
	if (button_p->onDown) button_p->onDown(button_p);
      }
      break;
    case Button::BUTTON_PRESSED:
      if (changed && up){
	button_p->buttonState = Button::BUTTON_INIT;
	if (button_p->onUp) button_p->onUp(button_p);
      }else{
        if ((millis() - button_p->startMillis) >= button_p->holdMillisWait){//hold event
          button_p->buttonState = Button::BUTTON_HOLD;
	  if (button_p->onHold) button_p->onHold(button_p);
        }
      }
      break;
    case Button::BUTTON_HOLD:
      if (changed && up){
	button_p->buttonState = Button::BUTTON_INIT;
	if (button_p->onUp) button_p->onUp(button_p);
      }
      break;
    default:
      button_p->buttonState = Button::BUTTON_INIT;
      break;
    }
  }
}
