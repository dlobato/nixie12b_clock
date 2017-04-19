#ifndef CLOCK_STATE_H
#define CLOCK_STATE_H

#include "NixieDisplay.h"
#include "Time.h"

//year range 2016..2050 (values are offset from 1970)
#define YEAR_START 46
#define YEAR_END 80

#define DEBUG 0

class ClockState{
public:
	enum State{
		INIT,
		HOURMINUTE_DISPLAY,
		DAYMONTH_DISPLAY,
		YEAR_DISPLAY,
    INIT_SET,
		MINUTE_SET,
		HOUR_SET,
		DAY_SET,
		MONTH_SET,
		YEAR_SET,
		CLOCKSTATE_COUNT
	};

	enum Event{
		NOEVENT,
		TIMER_DISPLAY_NEXT,
		SET_BUTTON_UP,
		SET_BUTTON_HOLD,
		MODE_BUTTON_UP,
		CLOCKEVENT_COUNT
	};

	typedef void (*Action)(ClockState& s);

	struct StateElement{
		State nextState;
		Action action;
	};

	ClockState(const State initialState = INIT);
	void processEvent(Event e);
	char getElemValue(const NixieDisplay::ElemEnum i) const;
	NixieDisplay::ElemState getElemState(const NixieDisplay::ElemEnum i) const;

private:
	//actions
	static void initTime(ClockState& s);
	static void initSetTime(ClockState& s);
	static void exitSetTime(ClockState& s);
	static void displayHourMinute(ClockState& s);
	static void displayDayMonth(ClockState& s);
	static void displayYear(ClockState& s);
	static void displaySetMinute(ClockState& s);
	static void displaySetHour(ClockState& s);
	static void displaySetDay(ClockState& s);
	static void displaySetMonth(ClockState& s);
	static void displaySetYear(ClockState& s);
	static void incrementMinute(ClockState& s);
	static void incrementHour(ClockState& s);
	static void incrementDay(ClockState& s);
	static void incrementMonth(ClockState& s);
	static void incrementYear(ClockState& s);

	State currentState;
	//status variables
	char elemValues[NixieDisplay::ELEMENUMMAX];
  	NixieDisplay::ElemState elemState[NixieDisplay::ELEMENUMMAX];
	tmElements_t newTimeElements;

	//transition table
	const static StateElement stateTransitionTable[CLOCKSTATE_COUNT][CLOCKEVENT_COUNT];
};

#endif //CLOCK_STATE_H
