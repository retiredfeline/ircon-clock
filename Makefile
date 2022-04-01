# Compiler
CC=sdcc
CFLAGS=-mstm8
INCLUDES=-Iinclude -Ipt-1.4
LIBS=spl.lib
DDS_ADJ=0ul

# Add DS3231=[is] to use rtc module instead of tod module
# Default is to use tod (internal oscillator) for testing
ifdef DS3231
CFLAGS+=-DDS3231
# if set to i, use harware I2C implementation
ifeq "$(DS3231)" "i"
TIMEBASE_OBJ=rtc.rel
else
TIMEBASE_OBJ=rtcsoft.rel
endif
else
TIMEBASE_OBJ=tod.rel
endif

ifdef TM1637
CFLAGS+=-DTM1637
endif

clock.ihx:	clock.rel mcu.rel tick.rel display.rel button.rel $(TIMEBASE_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Remake tod.rel if DDS_ADJ changed in Makefile
tod.rel:	tod.c tod.h Makefile
	$(CC) -c $(CFLAGS) -DDDS_ADJ=$(DDS_ADJ) $(INCLUDES) $< -o $(<:.c=.rel)

rtcsoft.rel:	rtcsoft.c rtc.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $(<:.c=.rel)

%.rel:		%.c %.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $(<:.c=.rel)

%.flash:
	test -r $(@:.flash=.ihx) && stm8flash -cstlinkv2 -pstm8s103f3 -w$(@:.flash=.ihx)

flash:	clock.flash

clean:
	rm -f *.asm *.cdb *.lk *.lst *.map *.mem *.rel *.rst *.sym
