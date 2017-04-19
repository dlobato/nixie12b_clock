/*
 * NixieClock main
 */

#include <Time.h>
#include <Wire.h>
#include <SPI.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <Bounce.h>
#include <ButtonEvent.h>
#include "NixieDisplay.h"
#include "ClockState.h"
#include "TimedAction.h"
#include "elapsedMillis.h"

#define DEBUG 0

#define DISPLAY_UPDATE_F 2
//compare match register count = [ F_CPU / (prescaler * desired interrupt frequency) ] - 1
// c = F_CPU / (8*DISPLAY_UPDATE_F) = 4000
#define TIMER1_COMPARE_COUNT 4000

//PINS
const unsigned int BUTTON0_PIN = 3;
const unsigned int BUTTON1_PIN = 4;

//button handlers forward declarations
void onUp(Button* Sender);
void onHold(Button* Sender);

Button modeButton(BUTTON0_PIN, 10, 0, onUp, 0, 1000, Button::ACTIVE_HIGH);
Button setButton(BUTTON1_PIN, 10, 0, onUp, onHold, 1000, Button::ACTIVE_HIGH);
ButtonEventClass buttonEvent(2);//2 buttons

enum FreqState{
  FREQ_25HZ,
  FREQ_50HZ,
  FREQ_100HZ,
  FREQ_250HZ,
  FREQ_MAXSTATE,
};

const unsigned int FREQS[FREQ_MAXSTATE] = { 25, 50, 100, 250 };

ClockState cstate;
FreqState fstate = FREQ_250HZ;
NixieDisplay nx;

const unsigned long displayChangeTimeDelayMS = 5000;
elapsedMillis sinceDisplayChange;

void display_task_f();
void serial_task_f();
void input_task_f();

TimedAction display_task(1000, display_task_f);
TimedAction serial_task(100, serial_task_f);
TimedAction input_task(100, input_task_f);


void set_timer1(float timer_freq){
  //setup timer1 to call nx.display @ DISPLAY_UPDATE_F Hz
  cli();//stop interrupts
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  OCR1A = ((float)F_CPU/((float)256*timer_freq)-1);
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  //set prescaler to 8
  TCCR1B |= (1 << CS12);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}

//buttons onUp handler
void onUp(Button* Sender){
#if DEBUG
  Serial.print("onUp: pin=");
  Serial.print(Sender->pin);
  Serial.print(", state=");
  Serial.println(Sender->buttonState);
#endif
  if (Sender == &setButton){
    cstate.processEvent(ClockState::SET_BUTTON_UP);
  }else if (Sender == &modeButton){
    cstate.processEvent(ClockState::MODE_BUTTON_UP);
  }
}

void onHold(Button* Sender){
  #if DEBUG
    Serial.print("onHold: pin=");
    Serial.print(Sender->pin);
    Serial.print(", state=");
    Serial.println(Sender->buttonState);
  #endif
  if (Sender == &setButton){
    cstate.processEvent(ClockState::SET_BUTTON_HOLD);
  }
}

void setup()  {
  Serial.begin(9600);
  while (!Serial) ; // wait until Arduino Serial Monitor opens

  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

  set_timer1(FREQS[fstate]);

  //set pin modes
  pinMode(BUTTON0_PIN, INPUT);
  pinMode(BUTTON1_PIN, INPUT);

  //setup buttons
  buttonEvent
    .addButton(&modeButton)
    .addButton(&setButton);

  Serial.println("NixieClock setup done");
}



void loop()
{
  display_task.check();
  //serial_task.check();
  input_task.check();
}

void display_task_f(){
  if (sinceDisplayChange >= displayChangeTimeDelayMS){
    cstate.processEvent(ClockState::TIMER_DISPLAY_NEXT);
    sinceDisplayChange = 0;
  }

  cstate.processEvent(ClockState::NOEVENT);

  for (unsigned int i=0; i<NixieDisplay::ELEMENUMMAX; i++){
    NixieDisplay::ElemEnum e = static_cast<NixieDisplay::ElemEnum>(i);
    nx.setElem(e, cstate.getElemValue(e), cstate.getElemState(e));
  }
}

void serial_task_f(){
  if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      RTC.set(t);   // set the RTC and the system time to the received value
      setTime(t);
    }
  }
}

void input_task_f(){
  buttonEvent.spinOnce();
}


/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}

unsigned int currElem = 0;
ISR(TIMER1_COMPA_vect){
  nx.display(static_cast<NixieDisplay::ElemEnum>(currElem));
  //next elem
  currElem++;
  if (currElem>=NixieDisplay::ELEMENUMMAX) currElem = 0;
}
