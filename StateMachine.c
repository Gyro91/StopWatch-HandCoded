#include "StateMachine.h"

/* Local variables */

int hours, minutes, seconds, tenths,
Ahours,	Aminutes, Aseconds,	Atenths
, Alarm_h, Alarm_m, Alarm_state, mutex;

/* Output variables */

int Dhours, Dminutes, Dseconds, Dtenths,
mode, Alarm_signal, Timer_exp;

void control_smInit(Control *me)
{
	Ahours = Aminutes = Aseconds = Atenths = 0;
	me->state_ = timemode;
}


void control_smTran_(Control *me, State_C dest)
{
	me->state_ = dest;
}

/* Dispatching Entry actions */

void dispatch_entryAction(State_C state)
{
	switch(state) {
	case timemode:
		mode = 0;
		break;
	case sethoursTimer:
		Dhours = Ahours;
		break;
	case setminutesTimer:
		Dminutes = Aminutes;
		break;
	case setsecondsTimer:
		Dseconds = Aseconds;
		break;
	case timer_finish:
		Timer_exp = 1;
		break;
	case stop:
		Dhours = Ahours;
		Dminutes = Aminutes;
		Dseconds = Aseconds;
		Dtenths = Atenths;
		break;

	case start:
		Dhours = Ahours;
		Dminutes = Aminutes;
		Dseconds = Aseconds;
		Dtenths = Atenths;
		break;
	case sethoursAlarm:
		Dhours=Ahours;
		break;
	case setminutesAlarm:
		Dminutes=Aminutes;
		break;
	case sethoursTimeset:
		Dhours = hours;
		mutex = 1;
		break;
	case setminutesTimeset:
		Dminutes = minutes;
	break;
	default:
		break;
	}
}


void dispatch_initialAction(State_C state)
{
	switch(state) {
	case sethoursTimer:
		mode = 1;
		Dhours = Ahours = 0;
		Dminutes  = Aminutes  = 0;
		Dseconds = Aseconds = 0;

		break;
	case stop:
		mode = 3;
		Ahours = 0;
		Aminutes = 0;
		Aseconds = 0;
		break;
	case sethoursAlarm:
		mode = 2;
		Dhours = Ahours = 0;
		Dminutes  = Aminutes  = 0;
		Dseconds = Aseconds = 0;
		break;
	case sethoursTimeset:
		hours = 0;
		minutes = 0;
		seconds = 0;
		tenths = 0;
		mutex = 1;
		break;
	default:
		break;
	}
}

/* Dispatching Events */

void control_smDispatch(Control *me, Event_C e)
{
	switch(me->state_) {
	// TIMEMODE
	case timemode:
		switch(e) {
		case ptick:
			Dhours = hours;
			Dminutes = minutes;
			Dseconds = seconds;
			Dtenths = tenths;
			control_smTran_(me, me->state_);
			dispatch_entryAction(me->state_);
			break;
		case Btimer:
			control_smTran_(me, sethoursTimer);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
			break;
		case Bswatch:
			control_smTran_(me, stop);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
			break;
		case Balarm:
			control_smTran_(me, sethoursAlarm);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
			break;
		case Btime:
			Dhours = hours = 0;
			Dseconds = seconds = 0;
			Dminutes = minutes = 0;
			Dtenths = tenths = 0;
			mutex = 1;
			control_smTran_(me, sethoursTimeset);
			dispatch_entryAction(me->state_);
		}

		break;
		// TIMESET
	case sethoursTimeset:
		switch(e) {
		case Bminus:
			if (hours == 0)
				hours = 23;
			else
				hours--;
			dispatch_entryAction(me->state_);
			break;
		case Bplus:
			hours = (hours + 1) % 24;
			dispatch_entryAction(me->state_);
			break;
		case Btime:
			control_smTran_(me, setminutesTimeset);
			dispatch_entryAction(me->state_);
			break;
		case Bstart:
			mutex = 0;
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
			break;
		}
		break;
	case setminutesTimeset:
		switch(e) {
			case Bminus:
				if (minutes == 0)
					minutes = 23;
				else
					minutes--;
				dispatch_entryAction(me->state_);
				break;
			case Bplus:
				minutes = (minutes + 1) % 60;
				dispatch_entryAction(me->state_);
				break;
			case Btime:
				control_smTran_(me, sethoursTimeset);
				dispatch_entryAction(me->state_);
				break;
			case Bstart:
				mutex = 0;
				control_smTran_(me, timemode);
				dispatch_entryAction(me->state_);
				break;
			}
			break;
		// TIMER
	case sethoursTimer:
		switch(e) {
			case Btimer:
				control_smTran_(me, setminutesTimer);
				dispatch_entryAction(me->state_);
				break;
			case Bplus:
				Ahours = (Ahours + 1) % 24;
				dispatch_entryAction(me->state_);
				break;
			case Bminus:
				if (Ahours == 0)
					Ahours = 23;
				else
					Ahours--;
				dispatch_entryAction(me->state_);
				break;
			case Balarm:
				control_smTran_(me, sethoursAlarm);
				dispatch_initialAction(me->state_);
				dispatch_entryAction(me->state_);
				break;
			case Bswatch:
				control_smTran_(me, stop);
				dispatch_initialAction(me->state_);
				dispatch_entryAction(me->state_);
				break;
			case Btime:
				control_smTran_(me, timemode);
				dispatch_entryAction(me->state_);
				break;
			case Bstart:
				if (Ahours>0 || Aminutes > 0 || Aseconds > 0) {
					control_smTran_(me, timer_start);
					dispatch_initialAction(me->state_);
					dispatch_entryAction(me->state_);
				}
				break;

			case Bstop:
				Dhours = Ahours = 0;
				Dminutes = Aminutes = 0;
				Dseconds = Aseconds = 0;
				Timer_exp = 0;
				control_smTran_(me, sethoursTimer);
				dispatch_entryAction(me->state_);
				break;
			}
			break;

	case setminutesTimer:
		switch(e) {
		case Btimer:
			control_smTran_(me, setsecondsTimer);
			dispatch_entryAction(me->state_);
			break;
		case Bminus:
			if (Aminutes == 0)
				Aminutes = 59;
			else
				Aminutes--;
			Dminutes = Aminutes;
			dispatch_entryAction(me->state_);
		break;
		case Bplus:
			Aminutes = (Aminutes + 1) % 60;
			Dminutes = Aminutes;
			dispatch_entryAction(me->state_);
			break;
		case Balarm:
			control_smTran_(me, sethoursAlarm);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
			break;
		case Bswatch:
			control_smTran_(me, stop);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
			break;
		case Btime:
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
		break;
		case Bstart:
			if (Ahours>0 || Aminutes > 0 || Aseconds > 0) {
				control_smTran_(me, timer_start);
				dispatch_initialAction(me->state_);
				dispatch_entryAction(me->state_);
			}
		break;
		case Bstop:
			Dhours = Ahours = 0;
			Dminutes = Aminutes = 0;
			Dseconds = Aseconds = 0;
			Timer_exp = 0;
			control_smTran_(me, sethoursTimer);
			dispatch_entryAction(me->state_);
			break;
		}
		break;

	case setsecondsTimer:
		switch(e) {
		case Btimer:
			control_smTran_(me, sethoursTimer);
			dispatch_entryAction(me->state_);
		break;
		case Bstart:
			if (Ahours>0 || Aminutes > 0 || Aseconds > 0) {
				control_smTran_(me, timer_start);
				dispatch_initialAction(me->state_);
				dispatch_entryAction(me->state_);
			}
			break;
		case Bminus:
			if (Aseconds == 0)
				Aseconds = 59;
			else
			Aseconds--;
			dispatch_entryAction(me->state_);
		break;
		case Bplus:
			Aseconds = (Aseconds + 1) % 60;
			dispatch_entryAction(me->state_);
		break;
		case Balarm:
			control_smTran_(me, sethoursAlarm);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Bswatch:
			control_smTran_(me, stop);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Btime:
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
		break;
		case Bstop:
			Dhours = Ahours = 0;
			Dminutes = Aminutes = 0;
			Dseconds = Aseconds = 0;
			Timer_exp = 0;
			control_smTran_(me, sethoursTimer);
			dispatch_entryAction(me->state_);
			break;

		}

		break;

	case timer_start:
		switch(e) {
		case(ptick):
		Atenths = (Atenths + 1) % 10;
		if (Atenths == 0) {
			if (Aseconds > 0) {
				Aseconds = Aseconds - 1;
				Dseconds = Aseconds;
			}
			else if (Aminutes > 0 && Aseconds == 0) {
					Aminutes = Aminutes - 1;
					Dminutes = Aminutes;
					Dseconds = Aseconds = 59;
			}
			else if (Ahours > 0 && Aminutes == 0 && Aseconds == 0) {
				Ahours--;
				Dhours = Ahours;
				Dminutes = Aminutes = 59;
				Dseconds = Aseconds = 59;
			}
			else if (Ahours == 0 && Aminutes == 0 && Aseconds == 0) {
					control_smTran_(me, timer_finish);
					dispatch_entryAction(me->state_);
			}
		}
		break;
		case Balarm:
			control_smTran_(me, sethoursAlarm);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
			break;
		case Bswatch:
			control_smTran_(me, stop);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Btime:
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
		break;
		case Bstop:
			Dhours = Ahours = 0;
			Dminutes = Aminutes = 0;
			Dseconds = Aseconds = 0;
			Timer_exp = 0;
			control_smTran_(me, sethoursTimer);
			dispatch_entryAction(me->state_);
		break;
		default: break;
		}
		break;

	case timer_finish:
		switch(e) {
		case Balarm:
			control_smTran_(me, sethoursAlarm);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Bswatch:
			control_smTran_(me, stop);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Btime:
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
		break;
		case Bstop:
			Dhours = Ahours = 0;
			Dminutes = Aminutes = 0;
			Dseconds = Aseconds = 0;
			Timer_exp = 0;
			control_smTran_(me, sethoursTimer);
			dispatch_entryAction(me->state_);
		break;
		}
		break;

	// STOPWATCH

	case stop:
		switch(e) {
		case Bstart:
			control_smTran_(me, start);
			dispatch_entryAction(me->state_);
		break;
		case Bswatch:
			Ahours = 0;
			Aseconds = 0;
			Aminutes = 0;
			Atenths = 0;
		break;
		case Btimer:
			control_smTran_(me, sethoursTimer);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Btime:
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
		break;
		case Balarm:
			control_smTran_(me, sethoursAlarm);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		}
		break;

	case start:
		switch(e) {
		case Bstop:
			control_smTran_(me, stop);
			dispatch_entryAction(me->state_);
			break;
		case Bswatch:
			Ahours = 0;
			Aminutes = 0;
			Aseconds = 0;
			Atenths  = 0;
			break;
		case ptick:
		Atenths = (Atenths + 1) % 10;
		if (Atenths == 0) {
			Aseconds = (Aseconds + 1) % 60;
			if (Aseconds == 0) {
				Aminutes = (Aminutes + 1) % 60;
				if (Aminutes == 0) {
					Ahours = (Ahours + 1) % 24;
				}
			}
		}
		dispatch_entryAction(me->state_);
		break;
		case Btimer:
			control_smTran_(me, sethoursTimer);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Btime:
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
		break;
		case Balarm:
			control_smTran_(me, sethoursAlarm);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
			break;
		}
		break;
		// ALARM
	case sethoursAlarm:
		switch(e) {
		case Btime:
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
		break;
		case Btimer:
			control_smTran_(me, sethoursTimer);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Bswatch:
			control_smTran_(me, stop);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Bminus:
			if (Ahours == 0)
				Ahours = 23;
			else
				Ahours--;
			dispatch_entryAction(me->state_);
			break;
		case Bplus:
			Ahours = (Ahours + 1) % 24;
			dispatch_entryAction(me->state_);
		break;
		case Bstart:
			Alarm_state = 1;
			Alarm_h = Ahours;
			Alarm_m = Aminutes;
		break;
		case Bstop:
			Alarm_signal = 0;
			dispatch_entryAction(me->state_);
		break;
		case Balarm:
			control_smTran_(me, setminutesAlarm);
			dispatch_entryAction(me->state_);
		}
		break;

	case setminutesAlarm:
		switch(e) {
		case Btime:
			control_smTran_(me, timemode);
			dispatch_entryAction(me->state_);
		break;
		case Btimer:
			control_smTran_(me, sethoursTimer);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Bswatch:
			control_smTran_(me, stop);
			dispatch_initialAction(me->state_);
			dispatch_entryAction(me->state_);
		break;
		case Bminus:
			if (Aminutes == 0)
				Aminutes = 59;
			else
				Aminutes--;
			dispatch_entryAction(me->state_);
			break;
		case Bplus:
			Aminutes = (Aminutes + 1) % 60;
			dispatch_entryAction(me->state_);
		break;
		case Bstart:
			Alarm_state = 1;
			Alarm_h = Ahours;
			Alarm_m = Aminutes;
		break;
		case Bstop:
			Alarm_signal = 0;
			dispatch_entryAction(me->state_);
		break;
		case Balarm:
			control_smTran_(me, sethoursAlarm);
			dispatch_entryAction(me->state_);
		}
		break;
	}

}




