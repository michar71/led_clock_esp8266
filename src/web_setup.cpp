#include "web_setup.h"
#include "main.h"
#include "config.h"
#include "citylist.h"
#include <listObj.h>

//Global Object
const byte DNS_PORT = 53;
const IPAddress apIp(192, 168, 0, 1);
listObj wifiList;
t_web_Setup_states web_setup_state = WESETUP_INIT;
bool configMode  = true;



//LED Control
RgbColor currentCol = 0;
const uint16_t AnimCount = NUM_PIXELS / 5 * 2 + 1; // we only need enough animations for the tail and one extra
const uint16_t PixelFadeDuration = 300; // third of a second
const uint16_t NextPixelMoveDuration = 1000 / NUM_PIXELS; // how fast we move through the pixels (one second divide by the number of pixels = loop once a second)
// what is stored for state is specific to the need, in this case, the colors and
// the pixel to animate;
// basically what ever you need inside the animation update function
struct MyAnimationState
{
    RgbColor StartingColor;
    RgbColor EndingColor;
    uint16_t IndexPixel; // which pixel this animation is effecting
};

NeoPixelAnimator setup_animations(AnimCount); // NeoPixel animation management object
MyAnimationState animationState[AnimCount];
uint16_t frontPixel = 0;  // the front of the loop
RgbColor frontColor;  // the color at the front of the loop


//LED Control During Setup...
//----------------------------
void setLED(RgbColor col)
{
  currentCol = col;
}


void FadeOutAnimUpdate(const AnimationParam& param)
{
    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        param.progress);
    // apply the color to the strip
    strip.SetPixelColor(animationState[param.index].IndexPixel, 
        colorGamma.Correct(updatedColor));
}

void LoopAnimUpdate(const AnimationParam& param)
{
    // wait for this animation to complete,
    // we are using it as a timer of sorts
    if (param.state == AnimationState_Completed)
    {
        // done, time to restart this position tracking animation/timer
        setup_animations.RestartAnimation(param.index);

        // pick the next pixel inline to start animating
        // 
        frontPixel = (frontPixel + 1) % NUM_PIXELS; // increment and wrap
        if (frontPixel == 0)
        {
            // we looped, lets pick a new front color
            frontColor = currentCol;
        }

        uint16_t indexAnim;
        // do we have an animation available to use to animate the next front pixel?
        // if you see skipping, then either you are going to fast or need to increase
        // the number of animation channels
        if (setup_animations.NextAvailableAnimation(&indexAnim, 1))
        {
            animationState[indexAnim].StartingColor = frontColor;
            animationState[indexAnim].EndingColor = RgbColor(0, 0, 0);
            animationState[indexAnim].IndexPixel = frontPixel;

            setup_animations.StartAnimation(indexAnim, PixelFadeDuration, FadeOutAnimUpdate);
        }
    }
}



void wifiStaConnect(const WiFiEventSoftAPModeStationConnected& evt)
{
#ifdef WIFIDEBUG
  Serial.println("Assoc");
#endif
}
WiFiEventHandler wifiStaConnectHandler;

void wifiStaDisconnect(const WiFiEventSoftAPModeStationDisconnected& evt)
{
#ifdef WIFIDEBUG
  Serial.println("Deassoc");
#endif
}
WiFiEventHandler wifiStaDisconnectHandler;


#ifdef SIMPLE_SETUP
void handleSetupSave() 
{
 listObj::pEntry pEntry;
 int ii;
 config_t* pConfig = get_config_ptr();

  
  String message = "Setup Saved\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 200, "text/plain", message );

  //Parse Servar args and store
  if (server.args() != 2)
    return;

  //Get AP
  ii = atoi(server.arg(0).c_str());
  wifiList.getByIndex(ii,&pEntry);
  strncpy(pConfig->current_ap,pEntry->pStr,sizeof(pConfig->current_ap)); 

  //Get PW
  strncpy(pConfig->current_pw,server.arg(1).c_str(),sizeof(pConfig->current_pw)); 


#ifdef WIFIDEBUG
  Serial.println("Config parsed:");
  Serial.print("Access Point:");
  Serial.println(pConfig->current_ap);
  Serial.print("Password:");
  Serial.println(pConfig->current_pw);
#endif 

#ifdef WIFIDEBUG  
  Serial.println("Saving Config");
#endif  
  save_config();

  //Go to passthrough operation
  configMode = false;
}

void handleSetupRoot() 
{
  char tmp2[1024];

  setLED(RgbColor(0,0,100));
  snprintf ( tempstr, sizeof(tempstr),
  "<html>\
    <head>\
      <title>Led Light Network Setup</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>\
    <h1>ESP8266 Serial Passthrough Configuration</h1>\
    <form action=\"setup\">\
       <br>Access Points:\
       <select name=\"AP\">");

    for (uint16_t ii=1;ii<=wifiList.getSize();ii++)
    {
      listObj::pEntry pEntry;
      wifiList.getByIndex(ii,&pEntry);
      snprintf(tmp2,1024,"<option value=\"%d\">%s (%d dB)  Mode:%d</option>",ii,pEntry->pStr,pEntry->num,(unsigned int)pEntry->pPtr);
      strncat(tempstr,tmp2,2048);   
    }

  snprintf(tmp2,1024,
  "       </select>\
           <br>Password:\
          <input type=\"text\" name=\"pw\">\
        <br><br>\
       <input type=\"submit\" value=\"Save\">\
      </form>\
     </body>\
   </html>");
  strncat(tempstr,tmp2,2048);

  server.send(200, "text/html", tempstr);
}
#else
void handleSetupSave() 
{
 listObj::pEntry pEntry;
 int ii;
 config_t* pConfig = get_config_ptr();

  
  String message = "Setup Saved\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 200, "text/plain", message );

  //Parse Servar args and store
  if (server.args() != 6)
    return;

  //Get AP
  ii = atoi(server.arg("ap").c_str());
  wifiList.getByIndex(ii,&pEntry);
  strncpy(pConfig->current_ap,pEntry->pStr,sizeof(pConfig->current_ap)); 

  //Get PW
  strncpy(pConfig->current_pw,server.arg("pw").c_str(),sizeof(pConfig->current_pw)); 

  //Get Other Values
  pConfig->city_index = atoi(server.arg("city").c_str());
  pConfig->day_brightness = atoi(server.arg("bday").c_str());
  pConfig->night_brightness = atoi(server.arg("bnight").c_str());
  pConfig->NTP_Sync_interval = atoi(server.arg("ntpupd").c_str());
#ifdef WIFIDEBUG
  Serial.println("Config parsed:");
  Serial.print("Access Point:");
  Serial.println(pConfig->current_ap);
  Serial.print("Password:");
  Serial.println(pConfig->current_pw);
  Serial.print("City Index:");
  Serial.println(pConfig->city_index);
  Serial.print("Day Brightness:");
  Serial.println(pConfig->day_brightness);
  Serial.print("Night Brightness:");
  Serial.println(pConfig->night_brightness);
  Serial.print("NTP Sync Interval:");
  Serial.println(pConfig->NTP_Sync_interval);    
#endif 

#ifdef WIFIDEBUG  
  Serial.println("Saving Config");
#endif  
  save_config();

  //Go to passthrough operation
  configMode = false;
}


void sortListBySignalStrength()
{
   uint16_t i, j; 
   listObj::pEntry pEntryA;
   listObj::pEntry pEntryB;
   uint16_t size = wifiList.getSize();

   for (i = 0; i < size-1; i++)       
   {
       // Last i elements are already in place    
       for (j = 0; j < size-i-1; j++)  
       {  
          wifiList.getByIndex(j,&pEntryA); 
          wifiList.getByIndex(j+1,&pEntryB); 
           if (pEntryA->num < pEntryB->num) 
              wifiList.swap(j, j+1); 
       }
   }
}

void handleSetupRoot() 
{
  char tmp2[1024];

  sortListBySignalStrength();
  setLED(RgbColor(0,0,100));
  snprintf ( tempstr, sizeof(tempstr),
  "<html>\
    <head>\
      <title>Led Light Network Setup</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>\
    <h1>ESP8266 Serial Passthrough Configuration</h1>\
    <form action=\"setup\">\
       <br>Access Points:\
       <select name=\"ap\">");

#ifdef WIFIDEBUG
    Serial.print("Accesspoints found: ");
    Serial.println(wifiList.getSize());
#endif
    for (uint16_t ii=1;ii<=wifiList.getSize();ii++)
    {
      //Only show first n strongest access points to reduce size of webpage...
      if (ii>MAX_LIST_LENGTH)
      {
        break;
      }
      listObj::pEntry pEntry;
      wifiList.getByIndex(ii,&pEntry);
      
#ifdef WIFIDEBUG
    Serial.print("Index:");
    Serial.print(ii);
    Serial.print(" Name:");
    Serial.print(pEntry->pStr);
    Serial.print(" dB:");
    Serial.print(pEntry->num);
    Serial.print(" Mode:");
    Serial.println((unsigned int)pEntry->pPtr);            

#endif
      snprintf(tmp2,1024,"<option value=\"%d\">%s (%d dB)  Mode:%d</option>",ii,pEntry->pStr,pEntry->num,(unsigned int)pEntry->pPtr);
      strncat(tempstr,tmp2,2048);   
    }

  snprintf(tmp2,1024,
  "       </select>\
           <br>Password:\
          <input type=\"text\" name=\"pw\">\
        <br><br>\
       <br>City:\
       <select name=\"city\">");
  strncat(tempstr,tmp2,2048);   

   uint8_t citycnt = 0;
   while(cities[citycnt].latitude != 0)
   {
      snprintf(tmp2,1024,"<option value=\"%d\">%s</option>",citycnt,cities[citycnt].cityname);
      strncat(tempstr,tmp2,2048);  
      citycnt++;
      yield();
   }

  snprintf(tmp2,1024,
  "    </select>\
    <br>Brightness Day [%%]:\
          <input name=\"bday\" type=\"number\" min=\"005\" max=\"100\" required placeholder=\"80\">\
        <br><br>\
    <br>Brightness Night [%%]:\
          <input name=\"bnight\" type=\"number\" min=\"005\" max=\"100\" required placeholder=\"15\">\
        <br><br>\
    <br>Time Server Update Interval [min]:\
          <input name=\"ntpupd\" type=\"number\" min=\"0001\" max=\"1440\" required placeholder=\"10\">\
        <br><br>\
       <input type=\"submit\" value=\"Save\">\
      </form>\
     </body>\
   </html>");
  strncat(tempstr,tmp2,2048);

  //Serial.println("");
  //Serial.println(tempstr);
  //Serial.println("");

  server.send(200, "text/html", tempstr);
}
#endif

void handleNotFound() 
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
#ifdef WIFIDEBUG  
  Serial.println(message);
#endif  
}

void scanForAP()
{
  //Set LED to Yellow
  setLED(RgbColor(100,100,0));
  wifiList.clear();

  //Scan
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    
#ifdef WIFIDEBUG
    Serial.println("Couldn't get a wifi connection");
#endif
    setLED(RgbColor(100,0,0));
    while (true);
  }
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    listObj::tEntry entry;

    strncpy(entry.pStr,WiFi.SSID(thisNet).c_str(),sizeof(entry.pStr));
    entry.num = WiFi.RSSI(thisNet);
    entry.pPtr = (void*)(unsigned int)WiFi.encryptionType(thisNet);
    wifiList.add(entry);

#ifdef WIFIDEBUG
  Serial.print(entry.pStr);
  Serial.print("  ");
  Serial.print(entry.num);
  Serial.print("dB  Mode:");
  Serial.println((unsigned int)entry.pPtr);        
#endif   
    
  }
}


void enableSTAmode()
{
#ifdef WIFIDEBUG
  Serial.println("Enable STA");
#endif   
  WiFi.persistent(false);
  WiFi.disconnect(true);
  wifiStaConnectHandler = WiFi.onSoftAPModeStationConnected(wifiStaConnect);
  wifiStaDisconnectHandler = WiFi.onSoftAPModeStationDisconnected(wifiStaDisconnect);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIp, apIp, IPAddress(255, 255, 255, 0));
  WiFi.softAP("LEDLIGHT_SETUP", nullptr, 1);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
 #ifndef WIFIDEBUG   
    Serial.println("Start DNS");
 #endif  
  dnsServer.start(DNS_PORT, "*", apIp);
  
    //Set LED to Green
  setLED(RgbColor(0,100,0));
#ifdef WIFIDEBUG
  Serial.println("STA enabled");
#endif     
}

bool setup_state(void)
{
    static bool ls = false;
    static uint16_t col = 0;

    //Handle States
    switch(web_setup_state)
    {
        case WESETUP_INIT:
#ifdef WIFIDEBUG
          Serial.println("STATE: INIT");
#endif        
          setLED(RgbColor(0,0,0));
          // we use the index 0 animation to time how often we move to the next
          // pixel in the strip
          setup_animations.StartAnimation(0, NextPixelMoveDuration, LoopAnimUpdate);
          web_setup_state = WSETUP_SCAN;
          break;
        case WSETUP_SCAN:
#ifdef WIFIDEBUG
          Serial.println("STATE: SETUP_SCAN");
#endif         
          //Scan for Available Access Points
          scanForAP();  
          web_setup_state = WSETUP_ENABLE_AP;         
          break;  
        case WSETUP_ENABLE_AP:
#ifdef WIFIDEBUG
          Serial.println("STATE: SETUP_ENABLE_AP");
#endif         
          //Enable STA mode with No Password to access setup Webpage
          enableSTAmode();        
          web_setup_state = WSETUP_SETUP_WAITCONN;    
          break;
        case WSETUP_SETUP_WAITCONN:
#ifdef WIFIDEBUG
          Serial.println("STATE: SETUP_WAITCONN");
#endif        
          server.on("/", handleSetupRoot);
          server.on("/hotspot-detect.html", handleSetupRoot);
          server.on("/setup", handleSetupSave);
          server.onNotFound ( handleNotFound );

          server.begin();
          web_setup_state = WSETUP_SETUP_WEBSERVER;
          break;
        case WSETUP_SETUP_WEBSERVER:
#ifdef WIFIDEBUG
          Serial.println("STATE: SETUP_WEBSERVER");
#endif     
         dnsServer.processNextRequest();   
         server.handleClient();
         if(col == 100)
         {
           col = 0;
           ls =! ls;
         } 

         if (ls)
         {
            setLED(RgbColor(0,0,100));
         }
         else
         {
            setLED(RgbColor(0,100,0));
         }
         col++;

         if (configMode == false)
         {
            server.close();
            setup_animations.StopAnimation(0);
            web_setup_state = WESETUP_INIT; 
#ifdef WIFIDEBUG
          Serial.println("STATE: Network Config Done");
#endif  
            return false;
         }
         break;
    }   
    //During Setup we show the circle animation
    // this is all that is needed to keep it running
    // and avoiding using delay() is always a good thing for
    // any timing related routines
    setup_animations.UpdateAnimations();
    strip.Show();     
    return true;
}