#include "clock_mode.h"
#include "main.h"
#include "citylist.h"
#include "config.h"
#include "auto_dim.h"

int8_t timeoffset = 0;
uint8_t clock_brightness = 0;
time_t local_time;

void show_clock(time_t ct);
void print_clock(time_t ct);

time_t calculate_local_time(time_t utc)
{
  return utc + (timeoffset * 60 * 60);
}

void set_timeoffset(int8_t offset)
{
    timeoffset = offset;
}


void calculate_offset()
{
  int8_t offset;
  config_t* pConfig = get_config_ptr(); 
  city_t citydata = cities[pConfig->city_index];

  offset = citydata.EST_Offset;
  time_t corrtime = now() + (offset * 60);

#ifdef OFFSETCALCDEBUG
   Serial.print("Timezone Offset");
   Serial.println(citydata.EST_Offset);
   Serial.print("DST Starts ");
   Serial.print(citydata.DST_Start_Month);
   Serial.print("/");
   Serial.println(citydata.DST_Start_Day);
   Serial.print("DST Ends ");
   Serial.print(citydata.DST_End_Month);
   Serial.print("/");
   Serial.println(citydata.DST_End_Day);
   Serial.print("Today is ");
   Serial.print(month(corrtime));
   Serial.print("/");
   Serial.println(day(corrtime));
#endif
  //Correct for DST... Does not take the hour DST switches into account... Too lazy...
  //This should be pulled from the web somehow. I mean, there must be lists and data and city 
  //Location out there....

  bool isdst = false;

  //Check if this place has DST....
  if (citydata.DST_Start_Month != citydata.DST_End_Month)
  {
    //First check the month...
    if ((month(corrtime)>=citydata.DST_Start_Month) && (month(corrtime)<=citydata.DST_End_Month))
    {
      //OK... So DST should happen in this month... Now we just need to check if it's also happening on this date
      if ((month(corrtime)==citydata.DST_Start_Month) && (day(corrtime)>=citydata.DST_Start_Day))
      {
        isdst = true;
      }
      if ((month(corrtime)==citydata.DST_End_Month) && (day(corrtime)<=citydata.DST_End_Day))
      {
        isdst = true;
      }
      if ((month(corrtime)>citydata.DST_Start_Month) && (month(corrtime)<citydata.DST_End_Month))
      {
        isdst = true;
      }  
      
      if (isdst)
      {
        offset = offset + 1;
#ifdef OFFSETCALCDEBUG
        Serial.println("Adding 1 hour for DST to Offset");
#endif
      }
    }
  }

#ifdef OFFSETCALCDEBUG
   Serial.print("Final Offset: ");
   Serial.println(offset);
#endif
  set_timeoffset(offset);
}


void show_clock(time_t ct)
{
    uint8_t hr = hour(ct);
    if (hr > 11)
        hr = hr - 12;
    uint8_t hr_pos = (hr*5);
    hr_pos = hr_pos + (minute(ct) / 12);  //Fractional hours....
    hr_pos = 59 - hr_pos;
    uint8_t min_pos = 59 - minute(ct);
    uint8_t sec_pos = 59 - second(ct);
    //turn off all NeoPixels
    strip.ClearTo(RgbColor(0, 0, 0));

    //Set pixel color. This blends colors together if hr/min/sec are on the same position....
    //Would be easier with readback but global brightness control doesn't work with that....
    
    strip.SetPixelColor(hr_pos,RgbColor(180, 0, 0));

    if (min_pos == hr_pos)
      strip.SetPixelColor(min_pos,RgbColor(180, 128, 0));
    else 
      strip.SetPixelColor(min_pos,RgbColor(0, 128, 0));

    if (sec_pos == min_pos)
      strip.SetPixelColor(sec_pos,RgbColor(0, 128, 250));
    else if (sec_pos == hr_pos)
      strip.SetPixelColor(sec_pos,RgbColor(180, 0, 250));
    else 
      strip.SetPixelColor(sec_pos,RgbColor(0, 0, 250));

    if ((sec_pos == hr_pos) &&  (sec_pos == min_pos))
      strip.SetPixelColor(hr_pos,RgbColor(180, 128, 250));

    strip.Show();
}

void print_clock(time_t ct)
{
    Serial.print("Offs:");
    Serial.print(timeoffset);
    Serial.print(" Br:");
    Serial.print(clock_brightness);
    Serial.print(" Local ");
    Serial.print(hour(ct));
    Serial.print(":");
    Serial.print(minute(ct));
    Serial.print(":");
    Serial.print(second(ct));
    Serial.print(" UTC ");
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());
}


void run_clock()
{
  static uint8_t lastsec = 0;
  static uint16_t lastday = 0;

  //Apply Offset to internal UTC Time
  local_time = calculate_local_time(now());
  uint8_t nowsec = second(local_time); 


  //It's a new day! Lets calulate Sunset/Sunrise for auto dimming function
  if (day(local_time) != lastday)
  {
        //Calulate Timzone/DST offset
        calculate_offset();
        //Calulate Sunrise/Sunset
        determineSunriseSunset(year(local_time), month(local_time), day(local_time), timeoffset);
        lastday = day(local_time);
  }

  if (nowsec != lastsec)
  {
    //Calulate Timzone/DST offset
    calculate_offset();
    //Calulate the current brightness based on time/location
    clock_brightness = calcTimeDependantBrightness(hour(local_time),minute(local_time),second(local_time));
    strip.SetBrightness(clock_brightness);
    //Draw the clock
    

    show_clock(local_time);
 #ifdef CLOCKDEBUG   
    print_clock(local_time);
 #endif   
    lastsec = nowsec;
  }
}