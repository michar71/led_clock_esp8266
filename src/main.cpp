// NeoPixelFunLoop
// This example will move a trail of light around a series of pixels.  
// A ring formation of pixels looks best.  
// The trail will have a slowly fading tail.
// 
// This will demonstrate the use of the NeoPixelAnimator.
// It shows the advanced use an animation to control the modification and 
// starting of other animations.
// It also shows the normal use of animating colors.
// It also demonstrates the ability to share an animation channel rather than
// hard code them to pixels.
//

//Button is GPIO15 / D8


#include "main.h"
#include "clock_mode.h"
#include "config.h"
#include "web_setup.h"
#include "filesys.h"
#include "web_server.h"
#include "web_control.h"

//Do we want to execute the wifi setup?
bool wifisetup = false;

//Network Components
DNSServer dnsServer;
ESP8266WebServer server (80);
WiFiClient  ServerClients[MAX_SRV_CLIENTS];
WiFiUDP Udp;

//Temprary string to assemble HTML responses
char tempstr[2048];

//Connection state
t_connect_state connect_state = CONNECT_INIT;

//Global Neopixel Object to control LED's
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUM_PIXELS);
NeoGamma<NeoGammaTableMethod> colorGamma; // for any fade animations, best to correct gamma


void SetRandomSeed()
{
    uint32_t seed;

    // random works best with a seed that can use 31 bits
    // analogRead on a unconnected pin tends toward less than four bits
    seed = analogRead(0);
    delay(1);

    for (int shifts = 3; shifts < 31; shifts += 3)
    {
        seed ^= analogRead(0) << shifts;
        delay(1);
    }

    // Serial.println(seed);
    randomSeed(seed);
}

mode_button_e check_button(void)
{
    static bool button_pressed = false;
    static unsigned long button_time = 0;

    //New Button Press
    if ((LOW == digitalRead(BUTTON_PIN)) && (button_pressed == false))
    {
        button_pressed = true;
        button_time = millis();     
        return DOWN; 
    }
    else if ((HIGH == digitalRead(BUTTON_PIN)) && (button_pressed == true))
    {
        button_pressed = false;
        if ((millis() - button_time) > VERY_LONG_PRESS_MS)
        {        
            return VERY_LONG_PRESS;
        }
        if ((millis() - button_time) > LONG_PRESS_MS)
        {         
            return LONG_PRESS;
        }
        else
        {            
            return SHORT_PRESS;
        }
    }
    return NO_PRESS;
}

void setup()
{
    //Setup Serial Port
    Serial.begin(115200);
    Serial.println("");
    Serial.println("");
    Serial.println("Startup...");

    //Setup Button
    pinMode(BUTTON_PIN,INPUT_PULLUP);

    //Setup LED Strip
    strip.Begin();
    strip.Show();

    //Setup File System
    init_FS();

    //We 'll need Random Somewhere...
    SetRandomSeed();

    //Init EEPROM/Config
    init_config();
    //Check Button and record state
    if (LOW == digitalRead(BUTTON_PIN))
    {
#ifdef DEBUG  
        Serial.println("Button Pressed");      
#endif    
      wifisetup = true;  
    }
    
    //Try loading config Data or intialize.... No Data also means we jumpt to setup...
    if (false == load_config())
    {
#ifdef DEBUG  
        Serial.println("No Config");      
#endif  
      init_config();
      wifisetup = true;
    }

#ifdef DEBUG  
    if (wifisetup == true)    
        Serial.println("Activating Network Setup");      
#endif      
}


bool wifi_state(void)
{
    config_t* pConfig = get_config_ptr();
    static uint8_t pixel = 0;
    static uint16_t toc = 0;

    switch(connect_state)
    {
        case CONNECT_INIT:
#ifdef DEBUG  
            Serial.println("Wifi: Init");    
            Serial.print("Connecting to ");
            Serial.println(pConfig->current_ap);              
#endif      

            WiFi.begin(pConfig->current_ap, pConfig->current_pw);   
            connect_state = CONNECT_CONNECT;
            return false;
        case CONNECT_CONNECT:
#ifdef DEBUG  
        Serial.println("Wifi: Connecting");      
#endif         
            if (WiFi.status() == WL_CONNECTED) 
            {
#ifdef DEBUG                  
                Serial.print("IP number assigned by DHCP is ");
                Serial.println(WiFi.localIP());
#endif                
                connect_state = CONNECT_CONNECTED;
            }
            else
            {
                delay(50);
                Serial.print(".");
                strip.ClearTo(RgbColor(0,0,0));
                strip.SetPixelColor(pixel,RgbColor(0,80 + pixel,0));
                strip.Show();

                pixel++;
                if (pixel == 60)
                {
                    pixel = 0;
                    toc++;

                    if (toc == 100)
                    {
                        toc = 0;
                        connect_state = CONNECT_TIMEOUT;
                        return false;
                    }
                }
            }  
            return false;
        case CONNECT_CONNECTED:
#ifdef DEBUG  
            Serial.println("Wifi: Connected");      
#endif 
            //If we are connected to AP set up NTP 
            open_NTP_Port();
#ifdef DEBUG  
        Serial.println("Wifi: NTP Connected");      
#endif 
            //Setup Webserver
            webserver_init();
            register_Setup_pages();
            server.begin();
#ifdef DEBUG  
            Serial.println("Wifi: Webserver Connected");      
#endif 

            connect_state = CONNECT_READY;
            return false;
        case CONNECT_READY:
            //We are connected... Jut fall through for LED Display
            return true;
        case CONNECT_TIMEOUT:
            //What can we do if we have no Network Connection?
            //Just blink nervously?
            //What if we loose connection? Cn we retry?
            strip.ClearTo(RgbColor(0,0,0));
            strip.Show();
            delay(500);
            strip.ClearTo(RgbColor(120,0,0));
            strip.Show();
            delay(500);
            connect_state = CONNECT_INIT;

            return false;
    }
    return false;
}

void run_led()
{
    //Only One thing we have right now....
    //Add a seector here for different modes later
    run_clock();

    //Other modes:
    //- Color Chasing Circe Blobs (At different speeds, with overlaps)
    //- Full Circe Rainbow
    //- Pendulum/Ball
    //- Fire Sim
    //- searchlight
    //- Alarm/Police
}

//Master State machine
void system_state()
{
  if(wifisetup)
  {
    wifisetup = setup_state();
  }
  else
  {
      if (true == wifi_state())
      {
          //Do LED Stuff
          run_led();
          //Do Network Stuff
          server.handleClient();
      }

  }
  
}

void loop()
{
    system_state();
}