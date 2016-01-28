/*
 * statemachine.h
 *
 *  Created on: 25/gen/2016
 *      Author: Matteo
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#define CLASS(class_) typedef struct class_ class_;\
	struct class_ {
#define METHODS };
#define END_CLASS



typedef enum State_Control {
	timemode, sethoursTimer, setminutesTimer,
	setsecondsTimer, timer_start, timer_finish,
	stop, start, sethoursAlarm, setminutesAlarm,
	sethoursTimeset, setminutesTimeset
}State_C ;

typedef enum Event_Control {
	Bplus, Bminus, Btime,
	Btimer, Balarm, Bswatch,
	Bstart, Bstop, ptick} Event_C;

	/* Control StateMachine */
	CLASS(Control)
		State_C state_;

	METHODS
		extern void control_smInit(Control *me);
		extern void control_smDispatch(Control *me, Event_C e);
		extern void control_smTran_(Control *me, State_C dest);

	END_CLASS



extern	int hours, minutes, seconds, tenths,
	Ahours,	Aminutes, Aseconds,	Atenths
	, Alarm_h, Alarm_m, Alarm_state, mutex;

	/* Output variables */

extern	int Dhours, Dminutes, Dseconds, Dtenths,
	mode, Alarm_signal, Timer_exp;

#endif /* STATEMACHINE_H_ */
