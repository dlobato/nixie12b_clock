/*
  David Lobato Based on:
  ButtonEvent.h - Event-Based Library for Arduino.
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

#ifndef ButtonEvent_h
#define ButtonEvent_h

#include <stdlib.h>
#include <Bounce.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

enum ButtonState {
  BUTTON_INIT,
  BUTTON_PRESSED,
  BUTTON_HOLD,
  BUTTON_MAXSTATE,
};
  

struct Button {
  Button(int pin_,
	 unsigned long debounceIntervalMillis_,
	 void (*onDown_)(Button* Sender),
	 void (*onUp_)(Button* Sender),
	 void (*onHold_)(Button* Sender),
	 unsigned long holdMillisWait_);
  const int pin;
  void (* const onDown)(Button* Sender);
  void (* const onUp)(Button* Sender);
  void (* const onHold)(Button* Sender);
  const unsigned long holdMillisWait;
  //button state
  Bounce buttonB;
  ButtonState buttonState;
  unsigned long startMillis;
};

class ButtonEventClass
{
public:
  ButtonEventClass(const unsigned int maxNButtons_ = 8);
  void addButton(int pin, unsigned long debounceIntervalMillis,
		 void (*onDown)(Button* Sender),
		 void (*onUp)(Button* Sender),
		 void (*onHold)(Button* Sender), unsigned long holdMillisWait);
  void spinOnce();
	
private:
  const unsigned int maxNButtons;
  unsigned int nButtons;
  Button** buttons;
};



#endif
