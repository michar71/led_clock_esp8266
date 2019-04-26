//Deal with auto dimming.
//We need:
//1) Brightness during the day
//2) Brightness during the night
//3) lat/long
//4) Day
//5) Local time offset from GMT
//6) Daylight Savings on/off


//Ideally we want to have the elevation of the sun above the horizon so we can use that to dim the lights...

//See discussion of simpler method here:
//https://www.instructables.com/id/Calculating-Sunset-and-Sunrise-for-a-Microcontroll/

//1) Calculate global brightness based on time of day and sunset/sunrise for given position
//2) Scale to min/max brightness
//3) How do we get the location? Enter lat/long, IP geo-location lookup, enter city?  (we need the timezone anyway, might just provide enough large cities people can select from which will give us approx location..)


#include <math.h>
#include "config.h"
#include "citylist.h"
#include "SunSet.h"

SunSet sun;

#define ZENITH -.83

float todaysSunrise = 0;
float todaysSunset = 0;

void calculateSunriseSunset(float* pSunrise,float*pSunset,int year,int month,int day,float lat, float lng,int localOffset);
void printSunriseSunset(void);


void calculateSunriseSunset(float* pSunrise,float*pSunset,int year,int month,int day,float lat, float lng,int localOffset) 
{
    sun.setPosition(lat, lng, localOffset);
	sun.setCurrentDate(year, month, day);
    *pSunrise = sun.calcSunrise();
	*pSunset = sun.calcSunset();
}

void printSunriseSunset(void) 
{
   double hours;
   float minutes;
   Serial.print("Raw Sunrise: ");
   Serial.println(todaysSunrise);
   Serial.print("Raw Sunset: ");
   Serial.println(todaysSunset);
   hours = todaysSunrise / 60;
   minutes = modf(hours,&hours)*60;
   hours = (int) hours;
   minutes = (int)minutes;
   Serial.print("Sunrise: ");
   Serial.print(hours);
   Serial.print(":");
   Serial.println(minutes);
   hours = todaysSunset / 60;
   minutes = modf(hours,&hours)*60;
   hours = (int) hours;
   minutes = (int)minutes;
   Serial.print("Sunset: ");
   Serial.print(hours);
   Serial.print(":");
   Serial.println(minutes);
}

void determineSunriseSunset(int year, int month, int day, int offset)
{
    config_t* pConfig = get_config_ptr();
    calculateSunriseSunset(&todaysSunrise,&todaysSunset,year,month,day,cities[pConfig->city_index].latitude, cities[pConfig->city_index].longitude,offset);
    printSunriseSunset(); 
}

//Start transition 15 minutes before sunset until 15 minutes after....
#define SECONDS_TRANSITION 900
uint8_t calcTimeDependantBrightness(int hour, int minute, int second)
{
    uint8_t brightness = 0;
    config_t* pConfig = get_config_ptr();

    //Calculate current second of day
    long currsec = second + (minute * 60) + (hour * 60 * 60);
    double hours;
    float minutes;
    hours = todaysSunrise / 60;
    minutes = modf(hours,&hours)*60;
    hours = (int) hours;
    minutes = (int)minutes;
    long srs = ((long)minutes * 60) + ((long)hours * 60 * 60);
    hours = todaysSunset / 60;
    minutes = modf(hours,&hours)*60;
    hours = (int) hours;
    minutes = (int)minutes;
    long sss = ((long)minutes * 60) + ((long)hours * 60 * 60);   

    if ((currsec < (srs-SECONDS_TRANSITION)) || (currsec > (sss+SECONDS_TRANSITION)))
        brightness = pConfig->night_brightness;

    if ((currsec > (srs+SECONDS_TRANSITION)) && (currsec < (sss-SECONDS_TRANSITION)))
        brightness = pConfig->day_brightness;

    if ((currsec >= (srs-SECONDS_TRANSITION)) && (currsec <= (srs+SECONDS_TRANSITION)))
        brightness = map(currsec,srs-SECONDS_TRANSITION,srs+SECONDS_TRANSITION,pConfig->night_brightness,pConfig->day_brightness);

    if ((currsec >= (sss-SECONDS_TRANSITION)) && (currsec <= (sss+SECONDS_TRANSITION)))
        brightness = map(currsec,sss-SECONDS_TRANSITION,sss+SECONDS_TRANSITION,pConfig->day_brightness,pConfig->night_brightness);    

    return (uint8_t)map(brightness,0,100,0,255);
}