#!/bin/bash

OBJDIR=objs
if [ ! -d $OBJDIR ]; then mkdir $OBJDIR; fi 

cd resources	# bad, but needed (otherwise alarmwav.o will have badly named symbols)
objcopy -I binary -O elf64-x86-64 --binary-architecture i386 alarm.wav ../$OBJDIR/alarmwav.o
cd ..
gcc -Wall -O2 src/playsound.c src/timer.c $OBJDIR/alarmwav.o -lopenal -o timer
#gcc -Wall -O2 src/playsound.c src/timer.c $OBJDIR/alarmwav.o -DMEASURE_LOOP_OVERHEAD -lrt -lopenal -o timer
