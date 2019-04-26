#ifndef comnfig_h
#define comnfig_h

#include <stdint.h>

typedef struct{
    //Network Setup
    char magic_header[4];
    uint8_t config_format; 
    char current_ap[128];
    char current_pw[32];

    //Clock Setup
    int8_t city_index;
    uint8_t day_brightness;
    uint8_t night_brightness;
    uint16_t NTP_Sync_interval;
}config_t;


bool init_config(void);
bool load_config(void);
bool save_config(void);
config_t* get_config_ptr(void);


#endif