#!/bin/bash

OBJDIR=objs

mkdir -p $OBJDIR

ld -r -b binary -o $OBJDIR/alarmwav.o resources/alarm.wav

gcc -Wall -O2 src/playsound.c src/timer.c $OBJDIR/alarmwav.o -lopenal -o timer
