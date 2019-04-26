#ifndef citylist_h
#define citylist_h

#include <arduino.h>

typedef struct{
    char cityname[48];
    int8_t EST_Offset;
    uint8_t DST_Start_Day;
    uint8_t DST_Start_Month;
    uint8_t DST_End_Day;
    uint8_t DST_End_Month;
    uint8_t DST_Hour_Start;
    uint8_t DST_Horu_End;
    float latitude;
    float longitude;
}city_t; 
#endif

const city_t cities[] {
 {"Hamburg",1,31,3,27,10,2,3,53.551,9.9937},
 {"Los Angeles",-8,10,3,3,11,2,2,34.0522,-118.2437},
 {"EOL",0,0,0,0,0,0}
};