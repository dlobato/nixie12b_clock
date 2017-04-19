#include "ClockState.h"
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

#if DEBUG
static const char *STATE_STRING[] = {
    "INIT",
    "HOURMINUTE_DISPLAY",
    "DAYMONTH_DISPLAY",
    "YEAR_DISPLAY",
    "INIT_SET",
    "MINUTE_SET",
    "HOUR_SET",
    "DAY_SET",
    "MONTH_SET",
    "YEAR_SET"
};

static const char *EVENT_STRING[] = {
    "NOEVENT",
    "TIMER_DISPLAY_NEXT",
    "SET_BUTTON_UP",
    "SET_BUTTON_HOLD",
    "MODE_BUTTON_UP"
};
#endif

int numberOfDaysMonth(tmElements_t &tm){
	int i = monthDays[tm.Month-1];
	if (tm.Month == 2 && LEAP_YEAR(tm.Year)){
		i++;
	}
	return i;
}

const ClockState::StateElement ClockState::stateTransitionTable[ClockState::CLOCKSTATE_COUNT][ClockState::CLOCKEVENT_COUNT] = {
	{
		{ClockState::HOURMINUTE_DISPLAY, &ClockState::initTime} /*ClockState::NOEVENT*/, {ClockState::HOURMINUTE_DISPLAY, 0} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::HOURMINUTE_DISPLAY, 0} /*ClockState::SET_BUTTON_UP*/,{ClockState::HOURMINUTE_DISPLAY, 0} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::HOURMINUTE_DISPLAY, 0} /*ClockState::MODE_BUTTON_UP*/
	}, //ClockState::INIT

	{
		{ClockState::HOURMINUTE_DISPLAY, &ClockState::displayHourMinute} /*ClockState::NOEVENT*/, {ClockState::DAYMONTH_DISPLAY, &ClockState::displayDayMonth} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::HOURMINUTE_DISPLAY, 0} /*ClockState::SET_BUTTON_UP*/,{ClockState::INIT_SET, &ClockState::initSetTime} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::DAYMONTH_DISPLAY, &ClockState::displayDayMonth} /*ClockState::MODE_BUTTON_UP*/
	}, //ClockState::HOURMINUTE_DISPLAY

	{
		{ClockState::DAYMONTH_DISPLAY, &ClockState::displayDayMonth} /*ClockState::NOEVENT*/, {ClockState::YEAR_DISPLAY, &ClockState::displayYear} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::DAYMONTH_DISPLAY, 0} /*ClockState::SET_BUTTON_UP*/,{ClockState::INIT_SET, &ClockState::initSetTime} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::YEAR_DISPLAY, &ClockState::displayYear} /*ClockState::MODE_BUTTON_UP*/
	}, //ClockState::DAYMONTH_DISPLAY

	{
		{ClockState::YEAR_DISPLAY, &ClockState::displayYear} /*ClockState::NOEVENT*/, {ClockState::HOURMINUTE_DISPLAY, &ClockState::displayHourMinute} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::YEAR_DISPLAY, 0} /*ClockState::SET_BUTTON_UP*/,{ClockState::INIT_SET, &ClockState::initSetTime} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::HOURMINUTE_DISPLAY, &ClockState::displayHourMinute}
	}, //ClockState::YEAR_DISPLAY

  {
    {ClockState::INIT_SET, 0} /*ClockState::NOEVENT*/, {ClockState::INIT_SET, 0} /*ClockState::TIMER_DISPLAY_NEXT*/,
    {ClockState::MINUTE_SET, &ClockState::displaySetMinute} /*ClockState::SET_BUTTON_UP*/,{ClockState::INIT_SET, 0} /*ClockState::SET_BUTTON_HOLD*/,
    {ClockState::INIT_SET, 0} /*ClockState::MODE_BUTTON_UP*/
  }, //ClockState::INIT_SET

	{
		{ClockState::MINUTE_SET, &ClockState::displaySetMinute} /*ClockState::NOEVENT*/, {ClockState::MINUTE_SET, 0} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::HOUR_SET, &ClockState::displaySetHour} /*ClockState::SET_BUTTON_UP*/, {ClockState::HOURMINUTE_DISPLAY, &ClockState::displayHourMinute} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::MINUTE_SET, &ClockState::incrementMinute} /*ClockState::MODE_BUTTON_UP*/
	}, //ClockState::MINUTE_SET

	{
		{ClockState::HOUR_SET, &ClockState::displaySetHour} /*ClockState::NOEVENT*/, {ClockState::HOUR_SET, 0} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::DAY_SET, &ClockState::displaySetDay} /*ClockState::SET_BUTTON_UP*/, {ClockState::HOURMINUTE_DISPLAY, &ClockState::displayHourMinute} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::HOUR_SET, &ClockState::incrementHour} /*ClockState::MODE_BUTTON_UP*/
	}, //ClockState::HOUR_SET

	{
		{ClockState::DAY_SET, &ClockState::displaySetDay} /*ClockState::NOEVENT*/, {ClockState::DAY_SET, 0} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::MONTH_SET, &ClockState::displaySetMonth} /*ClockState::SET_BUTTON_UP*/, {ClockState::HOURMINUTE_DISPLAY, &ClockState::displayHourMinute} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::DAY_SET, &ClockState::incrementDay} /*ClockState::MODE_BUTTON_UP*/
	}, //ClockState::DAY_SET

	{
		{ClockState::MONTH_SET, &ClockState::displaySetMonth} /*ClockState::NOEVENT*/, {ClockState::MONTH_SET, 0} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::YEAR_SET, &ClockState::displaySetYear} /*ClockState::SET_BUTTON_UP*/, {ClockState::HOURMINUTE_DISPLAY, &ClockState::displayHourMinute} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::MONTH_SET, &ClockState::incrementMonth} /*ClockState::MODE_BUTTON_UP*/
	}, //ClockState::MONTH_SET

	{
		{ClockState::YEAR_SET, &ClockState::displaySetYear} /*ClockState::NOEVENT*/, {ClockState::YEAR_SET, 0} /*ClockState::TIMER_DISPLAY_NEXT*/,
		{ClockState::HOURMINUTE_DISPLAY, &ClockState::exitSetTime} /*ClockState::SET_BUTTON_UP*/, {ClockState::HOURMINUTE_DISPLAY, &ClockState::displayHourMinute} /*ClockState::SET_BUTTON_HOLD*/,
		{ClockState::YEAR_SET, &ClockState::incrementYear} /*ClockState::MODE_BUTTON_UP*/
	}, //ClockState::YEAR_SET
};

ClockState::ClockState(const ClockState::State initialState)
: currentState(initialState){}

void ClockState::processEvent(Event e){
#if DEBUG
    Serial.print("ClockState::processEvent ");
    Serial.println(EVENT_STRING[e]);
#endif
  
	const StateElement se = ClockState::stateTransitionTable[this->currentState][e];

#if DEBUG
    Serial.print("currentState=");
    Serial.print(STATE_STRING[this->currentState]);
    Serial.print(" nextState=");
    Serial.println(STATE_STRING[se.nextState]);
#endif
  
	this->currentState = se.nextState;
	if (se.action != 0){
#if DEBUG
    Serial.println("executing action...");
#endif
		se.action(*this);
	}
}

char ClockState::getElemValue(const NixieDisplay::ElemEnum i) const{
	if (i < NixieDisplay::ELEMENUMMAX){
		return this->elemValues[i];
	}
	return 0;
}

NixieDisplay::ElemState ClockState::getElemState(const NixieDisplay::ElemEnum i) const{
	if (i < NixieDisplay::ELEMENUMMAX){
		return this->elemState[i];
	}
	return NixieDisplay::OFF;
}

void ClockState::initTime(ClockState& s){
	for(unsigned int i = 0; i < NixieDisplay::ELEMENUMMAX; i++){
		s.elemValues[i] = 0;
		s.elemState[i] = NixieDisplay::BLINK;
	}
}

void ClockState::initSetTime(ClockState& s){
	breakTime(now(), s.newTimeElements);
	if (s.newTimeElements.Year < YEAR_START){//start year on 2015
		s.newTimeElements.Year = YEAR_START;
	}
}

void ClockState::exitSetTime(ClockState& s){
	setTime(makeTime(s.newTimeElements));
  RTC.write(s.newTimeElements);
}

void ClockState::displayHourMinute(ClockState& s){
	time_t t = now();

	s.elemValues[NixieDisplay::DIGIT0] = minute(t)%10; (second(t)<55)? s.elemState[NixieDisplay::DIGIT0] = NixieDisplay::ON: s.elemState[NixieDisplay::DIGIT0] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DIGIT1] = minute(t)/10; s.elemState[NixieDisplay::DIGIT1] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT2] = hour(t)%10; s.elemState[NixieDisplay::DIGIT2] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT3] = hour(t)/10; s.elemState[NixieDisplay::DIGIT3] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DP] = 0; s.elemState[NixieDisplay::DP] = NixieDisplay::BLINK;
}

void ClockState::displayDayMonth(ClockState& s){
	time_t t = now();

	s.elemValues[NixieDisplay::DIGIT0] = month(t)%10; s.elemState[NixieDisplay::DIGIT0] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT1] = month(t)/10; s.elemState[NixieDisplay::DIGIT1] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT2] = day(t)%10; s.elemState[NixieDisplay::DIGIT2] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT3] = day(t)/10; s.elemState[NixieDisplay::DIGIT3] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DP] = 0; s.elemState[NixieDisplay::DP] = NixieDisplay::OFF;
}

void ClockState::displayYear(ClockState& s){
	time_t t = now();

	int y = year(t);
	for (int i=0; i<4; i++){
		s.elemValues[i] = y % 10; s.elemState[i] = NixieDisplay::ON;
		y /= 10;
	}
	s.elemValues[NixieDisplay::DP] = 0; s.elemState[NixieDisplay::DP] = NixieDisplay::OFF;
}

void ClockState::displaySetMinute(ClockState& s){
	s.elemValues[NixieDisplay::DIGIT0] = s.newTimeElements.Minute%10; s.elemState[NixieDisplay::DIGIT0] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DIGIT1] = s.newTimeElements.Minute/10; s.elemState[NixieDisplay::DIGIT1] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DIGIT2] = s.newTimeElements.Hour%10; s.elemState[NixieDisplay::DIGIT2] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT3] = s.newTimeElements.Hour/10; s.elemState[NixieDisplay::DIGIT3] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DP] = 0; s.elemState[NixieDisplay::DP] = NixieDisplay::BLINK;
}

void ClockState::displaySetHour(ClockState& s){
	s.elemValues[NixieDisplay::DIGIT0] = s.newTimeElements.Minute%10; s.elemState[NixieDisplay::DIGIT0] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT1] = s.newTimeElements.Minute/10; s.elemState[NixieDisplay::DIGIT1] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT2] = s.newTimeElements.Hour%10; s.elemState[NixieDisplay::DIGIT2] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DIGIT3] = s.newTimeElements.Hour/10; s.elemState[NixieDisplay::DIGIT3] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DP] = 0; s.elemState[NixieDisplay::DP] = NixieDisplay::BLINK;
}

void ClockState::displaySetDay(ClockState& s){
	s.elemValues[NixieDisplay::DIGIT0] = s.newTimeElements.Month%10; s.elemState[NixieDisplay::DIGIT0] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT1] = s.newTimeElements.Month/10; s.elemState[NixieDisplay::DIGIT1] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT2] = s.newTimeElements.Day%10; s.elemState[NixieDisplay::DIGIT2] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DIGIT3] = s.newTimeElements.Day/10; s.elemState[NixieDisplay::DIGIT3] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DP] = 0; s.elemState[NixieDisplay::DP] = NixieDisplay::OFF;
}

void ClockState::displaySetMonth(ClockState& s){
	s.elemValues[NixieDisplay::DIGIT0] = s.newTimeElements.Month%10; s.elemState[NixieDisplay::DIGIT0] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DIGIT1] = s.newTimeElements.Month/10; s.elemState[NixieDisplay::DIGIT1] = NixieDisplay::BLINK;
	s.elemValues[NixieDisplay::DIGIT2] = s.newTimeElements.Day%10; s.elemState[NixieDisplay::DIGIT2] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DIGIT3] = s.newTimeElements.Day/10; s.elemState[NixieDisplay::DIGIT3] = NixieDisplay::ON;
	s.elemValues[NixieDisplay::DP] = 0; s.elemState[NixieDisplay::DP] = NixieDisplay::OFF;
}

void ClockState::displaySetYear(ClockState& s){
	int y = s.newTimeElements.Year + 1970;// tmElements_t.Year is offset from 1970;
	for (int i=0; i<4; i++){
		s.elemValues[i] = y % 10; s.elemState[i] = NixieDisplay::BLINK;
		y /= 10;
	}
	s.elemValues[NixieDisplay::DP] = 0; s.elemState[NixieDisplay::DP] = NixieDisplay::OFF;
}

void ClockState::incrementMinute(ClockState& s){
	s.newTimeElements.Minute++;

	//valid range 0..59
	if (s.newTimeElements.Minute > 59){
		s.newTimeElements.Minute = 0;
	}
}

void ClockState::incrementHour(ClockState& s){
	s.newTimeElements.Hour++;

	//valid range 0..23
	if (s.newTimeElements.Hour > 23){
		s.newTimeElements.Hour = 0;
	}
}

void ClockState::incrementDay(ClockState& s){
	s.newTimeElements.Day++;

	//valid range 1..numberOfDaysMonth()
	if (s.newTimeElements.Day > numberOfDaysMonth(s.newTimeElements)){//go back one month
		s.newTimeElements.Day = 1;
	}
}

void ClockState::incrementMonth(ClockState& s){
	s.newTimeElements.Month++;

	//valid range 1..12
	if (s.newTimeElements.Month > 12){
		s.newTimeElements.Month = 1;
	}
}

void ClockState::incrementYear(ClockState& s){
	s.newTimeElements.Year++;

	//valid range YEAR_START..YEAR_END
	if (s.newTimeElements.Year > YEAR_END){
		s.newTimeElements.Year = YEAR_START;
	}
}
