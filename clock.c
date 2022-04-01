#define STM8S103

#include "clock.h"

#include "mcu.h"
#include "tick.h"
#include "display.h"
#include "button.h"

#include "pt.h"

#define MSPERTICK	2u
#define	DEPMIN		(100u / MSPERTICK)	// debounce period
#define	RPTTHRESH	(400u / MSPERTICK)	// repeat threshold after debounce
#define	RPTPERIOD	(250u / MSPERTICK)	// repeat period

static uint8_t swstate, swtent, swmin, swrepeat;
static struct pt pt;

static void switchaction()
{
	switch (swstate & B_BOTH) {
	case B_MINS:
		TIME.seconds = 0;
		TIME.minutes++;
		if (TIME.minutes >= 60)
			TIME.minutes = 0;
#ifdef	DS3231
		rtc_update(T_MINUTES);
#endif
		display_update();
		break;
	case B_HOURS:
		TIME.hours++;
		if (TIME.hours >= 24)
			TIME.hours = 0;
#ifdef	DS3231
		rtc_update(T_HOURS);
#endif
		display_update();
		break;
	}
}

static inline void reinitstate()
{
	swmin = DEPMIN;
	swrepeat = RPTTHRESH;
	swtent = swstate = B_NONE;
}

static
PT_THREAD(switchhandler(struct pt *pt))
{
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, swstate != swtent);
	swtent = swstate;
	PT_WAIT_UNTIL(pt, --swmin <= 0 || swstate != swtent);
	if (swstate != swtent) {		// changed, restart
		reinitstate();
		PT_RESTART(pt);
	}
	switchaction();
	PT_WAIT_UNTIL(pt, --swrepeat <= 0 || swstate != swtent);
	if (swstate != swtent) {		// changed, restart
		reinitstate();
		PT_RESTART(pt);
	}
	switchaction();
	for (;;) {
		swrepeat = RPTPERIOD;
		PT_WAIT_UNTIL(pt, --swrepeat <= 0 || swstate == B_NONE);
		if (swstate == B_NONE) {	// released, restart
			reinitstate();
			PT_RESTART(pt);
		}
		switchaction();
	}
	PT_END(pt);
}

int main(void)
{
	mcu_init();
	tick_init();
#ifdef	DS3231
	rtc_init();
#else
	tod_init();
#endif
	display_init();
	reinitstate();
	button_init();
	mcu_enable_interrupts();

	uint8_t counter = 0;
#ifdef	DS3231
	rtc_getnow();
#endif
	display_update();
	for (;;) {
		counter++;
		if (counter >= 125) {	// every quarter second poll RTC
			counter = 0;
#ifdef	DS3231
			rtc_getnow();
#endif
		}
		if (TIME.changed & (T_HOURS | T_MINUTES))
			display_update();
		TIME.changed = T_NONE;
		while (!tick_check())
			;
		swstate = button_state();
		PT_SCHEDULE(switchhandler(&pt));
	}
}
