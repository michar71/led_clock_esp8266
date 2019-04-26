#ifndef autodim_h
#define autodim_h

#include <arduino.h>
void determineSunriseSunset(int year, int month, int day, int offset);
uint8_t calcTimeDependantBrightness(int hour, int minute, int second);

#endif