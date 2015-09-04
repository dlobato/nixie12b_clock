#include <Bounce.h>
#include <ButtonEvent.h>

//PINS
const int modeButtonPin = 3;
const int setButtonPin = 4;

//event handlers
void onDown(Button* Sender){
  Serial.print("onDown: pin=");
  Serial.print(Sender->pin);
  Serial.print(", state=");
  Serial.println(Sender->buttonState);
}

void onUp(Button* Sender){
  Serial.print("onUp: pin=");
  Serial.print(Sender->pin);
  Serial.print(", state=");
  Serial.println(Sender->buttonState);
}

void onHold(Button* Sender){
  Serial.print("onHold: pin=");
  Serial.print(Sender->pin);
  Serial.print(", state=");
  Serial.println(Sender->buttonState);
}

//buttons and event manager
Button modeButton(modeButtonPin, 10, onDown, onUp, onHold, 1000, Button::ACTIVE_LOW);
Button setButton(setButtonPin, 10, onDown, onUp, onHold, 2000, Button::ACTIVE_LOW);
ButtonEventClass buttonEvent(2);//2 buttons

void setup(){
  //set pin modes
  pinMode(modeButtonPin, INPUT_PULLUP);
  pinMode(setButtonPin, INPUT_PULLUP);

  //setup buttons
  buttonEvent
    .addButton(&modeButton)
    .addButton(&setButton);
  Serial.begin(9600);
  while (!Serial) ; // wait until Arduino Serial Monitor opens
  Serial.println("Setup done");
}

void loop(){
  buttonEvent.spinOnce();
  delay(100);
}
