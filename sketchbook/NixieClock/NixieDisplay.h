/*
  NixieDisplay.h
  Copyright (c) 2013, David Lobato
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

#ifndef NixieDisplay_h
#define NixieDisplay_h

#include "Arduino.h"
#include "elapsedMillis.h"


#define SS 8//latch

enum ElemEnum{
  DIGIT0,
  DIGIT1,
  DIGIT2,
  DIGIT3,
  DP,
  ELEMENUMMAX,
};

enum ElemState{
  OFF,
  ON,
  BLINK,
  ELEMSTATEMAX,
};

#define BLINKFAST_DELAYMS 200
#define BLINKSLOW_DELAYMS 1000

class NixieDisplay {
public:
  NixieDisplay();
  void display(const ElemEnum elem);
  void setElem(const ElemEnum elem, const char value, const ElemState state);
  void setBlinkDelayMS(const unsigned long delayms);
private:
  char elemValues[ELEMENUMMAX];
  ElemState elemState[ELEMENUMMAX];
  bool elemBlinkState[ELEMENUMMAX];
  elapsedMillis sinceBlinkChange[ELEMENUMMAX];
  unsigned long blinkDelayMS;
};

#endif //NixieDisplay_h
