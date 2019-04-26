#ifndef clock_mode_h
#define clock_mode_h

#include <arduino.h>
#include <TimeLib.h>


extern int8_t timeoffset;

void calculate_offset();
void set_timeoffset(int8_t offset);
void open_NTP_Port(void);
void run_clock();

#endif