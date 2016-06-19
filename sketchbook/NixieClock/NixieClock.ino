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
#include "TimedAction.h"
#include "elapsedMillis.h"

#define DEBUG 1

#define DISPLAY_UPDATE_F 2
//compare match register count = [ F_CPU / (prescaler * desired interrupt frequency) ] - 1
// c = F_CPU / (8*DISPLAY_UPDATE_F) = 4000
#define TIMER1_COMPARE_COUNT 4000

//PINS
const unsigned int BUTTON0_PIN = 3;
const unsigned int BUTTON1_PIN = 4;

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

ClockState cstate = HOURMINUTE_DISPLAY;
FreqState fstate = FREQ_250HZ;
NixieDisplay nx;

const unsigned long showTimeDelayMS = 30000;
const unsigned long showDayMonthDelayMS = 5000;
const unsigned long showYearDelayMS = 5000;
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
    fstate = (FreqState)((fstate+1) % FREQ_MAXSTATE);
    set_timer1(FREQS[fstate]);
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
    cstate = MINUTE_SET;
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
  serial_task.check();
  input_task.check();
}

void display_task_f(){
  time_t t = now();
  char elemValues[ELEMENUMMAX];
  ElemState elemState[ELEMENUMMAX];

  if (cstate == HOURMINUTE_DISPLAY){
    elemValues[DIGIT0] = minute(t)%10; (second(t)<55)?elemState[DIGIT0] = ON: elemState[DIGIT0] = BLINK;
    elemValues[DIGIT1] = minute(t)/10; elemState[DIGIT1] = ON;
    elemValues[DIGIT2] = hour(t)%10; elemState[DIGIT2] = ON;
    elemValues[DIGIT3] = hour(t)/10; elemState[DIGIT3] = ON;
    elemValues[DP] = 0; elemState[DP] = BLINK;
    if (sinceDisplayChange >= showTimeDelayMS){
      cstate = DAYMONTH_DISPLAY;
      sinceDisplayChange = 0;
    }
  }else if (cstate == DAYMONTH_DISPLAY){
    elemValues[DIGIT0] = month(t)%10; elemState[DIGIT0] = ON;
    elemValues[DIGIT1] = month(t)/10; elemState[DIGIT1] = ON;
    elemValues[DIGIT2] = day(t)%10; elemState[DIGIT2] = ON;
    elemValues[DIGIT3] = day(t)/10; elemState[DIGIT3] = ON;
    elemValues[DP] = 0; elemState[DP] = OFF;
    if (sinceDisplayChange >= showDayMonthDelayMS){
      cstate = YEAR_DISPLAY;
      sinceDisplayChange = 0;
    }
  }else if (cstate == YEAR_DISPLAY){
    int y = year(t);
    for (int i=0; i<4; i++){
      elemValues[i] = y % 10; elemState[i] = ON;
      y /= 10;
    }
    elemValues[DP] = 0; elemState[DP] = OFF;
    if (sinceDisplayChange >= showYearDelayMS){
      cstate = HOURMINUTE_DISPLAY;
      sinceDisplayChange = 0;
    }
  }

  for (unsigned int i=0; i<ELEMENUMMAX; i++)
    nx.setElem(static_cast<ElemEnum>(i), elemValues[i], elemState[i]);

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
  nx.display(static_cast<ElemEnum>(currElem));
  //next elem
  currElem++;
  if (currElem>=ELEMENUMMAX) currElem = 0;
}
