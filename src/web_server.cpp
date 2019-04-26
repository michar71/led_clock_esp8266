#include <arduino.h>
#include "web_server.h"
#include "filesys.h"
#include "main.h"

bool loadFromSpiffs(String path);
void httpDefault();
void handleWebRequests();
void httpPureCss();
void httpLogDelete();


bool loadFromSpiffs(String path)
{
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";
 
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r"); 
  if (!dataFile)
  {
    //Try zipped versions of these files....
    path += ".gz";
    File dataFile = SPIFFS.open(path.c_str(), "r"); 
    if (!dataFile)
    {
      //No luck  
      return false;
    }  
  }
  if (server.hasArg("download")) dataType = "application/octet-stream";

 #ifdef WIFIDEBUG  
  Serial.print("XMit File: ");
  Serial.print(path);
  Serial.print(" Type: ");
  Serial.println(dataType);
#endif     
  if (server.streamFile(dataFile, dataType) != dataFile.size()) 
  {
#ifdef WIFIDEBUG  
  Serial.println("XMit File Size Missmatch");
#endif      
  }
 
  dataFile.close();
  return true;
}


void httpLog()
{
  logline << server.client().remoteIP() << " log";
  appendLog();
  logfile.seek(0, SeekSet);
  server.streamFile(logfile, "text/plain");
  logfile.seek(0, SeekEnd);
}

void httpLogDelete()
{
  logfile.close();
  SPIFFS.remove("/log.txt");
  logfile = SPIFFS.open("/log.txt", "a+");
  logline << server.client().remoteIP() << "NEW LOG";
  appendLog();
}
void httpDefault()
{
  logline << server.client().remoteIP() << " redirect";
  appendLog();
  server.sendHeader("Location", "http://freewifi.lan/", true);
  server.send(302, "text/plain", "");
  server.client().stop();
}

void httpHome()
{
  //if (server.hostHeader() != String("freewifi.lan")) 
  //{
  //  return httpDefault();
  //}
  logline << server.client().remoteIP() << " home";
  appendLog();
  File file = SPIFFS.open("/index.htm.gz", "r");
  //Handle file not existing....
  if (file)
  {
    server.streamFile(file, "text/html");
  }
  else
  {
    File file = SPIFFS.open("/index.htm.", "r");
    if (file)
    {
      server.streamFile(file, "text/html");
    }
    else
    {
      logline << server.client().remoteIP() << " missing index.htm";
      appendLog();
      handleWebRequests();
    }
  }
  file.close();
}

void httpPureCss()
{
  File file = SPIFFS.open("/pure-min.css.gz", "r");
  server.streamFile(file, "text/css");
  file.close();
}

void handleWebRequests()
{
  if(loadFromSpiffs(server.uri())) return;
  //ttpDefault();

  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
#ifdef WIFIDEBUG  
  Serial.println(message);
#endif  
}


void webserver_init(void)
{
 #ifndef WIFIDEBUG   
    Serial.println("Registering WebServer Callbacks");
 #endif   
    //server.on("/", httpHome);
    server.on("/hotspot-detect.html", httpHome);
    //server.on("/pure-min.css", httpPureCss);
    server.on("/log.txt", httpLog);
    server.on("/log.delete", httpLogDelete);
    server.onNotFound(handleWebRequests);
 #ifndef WIFIDEBUG   
    Serial.println("Starting Server");
 #endif   
    server.begin();
}
