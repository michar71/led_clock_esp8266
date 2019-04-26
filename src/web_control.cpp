#include "web_control.h"
#include "filesys.h"
#include "citylist.h"
#include "main.h"
#include "config.h"
#include <listObj.h>

void httpClockSetup();
void httpClockSetupSave();

void httpClockSetup()
{
#ifdef WIFIDEBUG
  Serial.println("Handle Clock Setup");
#endif

  char tempstr[1024*3];  //Is that enough? How big is the stack actually???
  char tmp2[1024];  //Is that enough? How big is the stack actually???
  uint16_t citycnt = 0;
// Items to Setup
// - City
// - Night Brighness
// - Day Brightness
// - NTP polling Interval

//Header
  snprintf ( tempstr, sizeof(tempstr),
  "<!DOCTYPE html>\
   <title>LED Clock Setup</title>\
   <meta name=\"viewport\" content=\"initial-scale=1.0\">\
   <link rel=\"stylesheet\" href=\"pure-min.css\">\
  <form action=\"/saveledsetup\" class=\"pure-form pure-form-aligned\">\
  <fieldset>\
   <div class=\"pure-control-group\">\
    <label for=\"city\">City</label>\
       <select name=\"city\">");

//Insert City List
#ifdef WIFIDEBUG
  Serial.println("Inserting City List");
#endif
   while(cities[citycnt].latitude != 0)
   {
      snprintf(tmp2,1024,"<option value=\"%d\">%s</option>",citycnt,cities[citycnt].cityname);
      strncat(tempstr,tmp2,2048);  
      citycnt++;
      yield();
   }
  
 //Other Parameters/Footer...  
  snprintf(tmp2,1024,
   "</select>\
   <div class=\"pure-control-group\">\
    <label for=\"bday\">Brightness Day [%%]</label>\
    <input name=\"bday\" id=\"bday\" type=\"number\" min=\"005\" max=\"100\" required placeholder=\"80\">\
   </div>\
   <div class=\"pure-control-group\">\
    <label for=\"bnight\">Brightness Night [%%]</label>\
    <input name=\"bnight\" id=\"bnight\" type=\"number\" min=\"005\" max=\"100\" required placeholder=\"15\">\
   </div>\
   <div class=\"pure-control-group\">\
    <label for=\"ntpupd\">NTP Update Interval [min]</label>\
    <input name=\"ntpupd\" id=\"ntpupd\" type=\"number\" min=\"0001\" max=\"1440\" required placeholder=\"15\">\
   </div>\
   <div class=\"pure-control-group\">\
    <input type=\"submit\" value=\"Save\">\
   </div>\
</fieldset>\
</form>");
 strncat(tempstr,tmp2,2048);

#ifdef WIFIDEBUG
  Serial.println("Serving Clock Setup Page");
  Serial.println(tempstr);
  Serial.println("");
#endif
 server.send(200, "text/html", tempstr);
}

void httpClockSetupSave()
{ 
  config_t* pConfig = get_config_ptr();
  logline << server.client().remoteIP() << " connect ";
  appendLog();
  for (int ii=0;ii<server.args();ii++)
  {
     logline << ' ' << server.argName(ii) << ':' << server.arg(ii); 
  }
  appendLog();

  //Store Values sent from Webpage
  pConfig->city_index = atoi(server.arg("city").c_str());
  pConfig->day_brightness = atoi(server.arg("bday").c_str());
  pConfig->night_brightness = atoi(server.arg("bnight").c_str());
  pConfig->NTP_Sync_interval = atoi(server.arg("ntpupd").c_str());

#ifdef WIFIDEBUG  
  Serial.println("Saving Clock Config");
#endif  
  save_config();

  //TODO 
  //- Do we need to force a refresh of the clock settings?
  //- Do Refresh with a new webpage?

}

void register_Setup_pages(void)
{
    server.on("/", httpClockSetup);
    server.on("/saveledsetup", httpClockSetupSave);
}