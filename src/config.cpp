#include "config.h"
#include <EEPROM.h>

#define FORMAT_VER 1
config_t config;
const char magic_header[] = "CONF";


void print_config(void)
{

}

bool init_config(void)
{
    EEPROM.begin(512);
    config.config_format = FORMAT_VER;
    config.magic_header[0] = magic_header[0];
    config.magic_header[1] = magic_header[1];
    config.magic_header[2] = magic_header[2];
    config.magic_header[3] = magic_header[3];
    config.city_index = 0;
    config.current_ap[0] = 0;
    config.current_pw[0] = 0;
    config.day_brightness = 85;  //85%
    config.night_brightness = 15; //15%
    config.NTP_Sync_interval = 10;
    return true;
}



bool load_config(void)
{
    uint16_t ii;
    uint8_t* pData;
    pData = (uint8_t*)&config;

    for (ii=0;ii<sizeof(config_t);ii++)
    {
        *pData = EEPROM.read(ii);
        pData++;
    }

    //Check if we have valid data
    if ((config.magic_header[0] != magic_header[0]) 
        || (config.magic_header[0] != magic_header[0]) 
        || (config.magic_header[1] != magic_header[1]) 
        || (config.magic_header[2] != magic_header[2]) 
        || (config.magic_header[3] != magic_header[3]) 
        || (config.config_format != FORMAT_VER))
    {
        return false;
    }
    return true;                                

}

bool save_config(void)
{
    uint16_t ii;
    uint8_t* pData;
    pData = (uint8_t*)&config;

    for (ii=0;ii<sizeof(config_t);ii++)
    {
        EEPROM.write(ii,*pData);
        pData++;
    }
    EEPROM.commit();
    return true;
}

 config_t* get_config_ptr(void)
{
    return &config;
}
