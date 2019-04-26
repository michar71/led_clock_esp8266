/*
 * Time_NTP.pde
 * Example showing time sync to NTP time source
 *
 * This sketch uses the ESP8266WiFi library
 */
 
#include <TimeLib.h> 
#include "network_time.h"
#include "main.h"
#include "config.h"
#include "clock_mode.h"

IPAddress timeServerIP;          // time.nist.gov NTP server address
const char* NTPServerName = "time.nist.gov";
unsigned int localTimePort = 8888;  // local port to listen for UDP packets


void sendNTPpacket(IPAddress &address);
time_t getNtpTime();

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  config_t* pConfig = get_config_ptr();
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  #ifdef CLOCKDEBUG
  Serial.println("Resolve NIST Server IP");
  #endif
  if(!WiFi.hostByName(NTPServerName, timeServerIP)) 
  { 
#ifdef CLOCKDEBUG
    Serial.println("DNS lookup failed. Rebooting.");
    Serial.flush();
#endif
    ESP.reset();
  }
#ifdef CLOCKDEBUG
  Serial.print("Time server IP:\t");
  Serial.println(timeServerIP);
  Serial.println("Transmit NTP Request");
#endif
  sendNTPpacket(timeServerIP);

  uint16_t size = 0;
  uint32_t startms = millis();
  do
  {
    size = Udp.parsePacket();
    yield();
#ifdef CLOCKDEBUG
    Serial.print(".");
#endif
  }
  while((size == 0) && ((startms+2000) > millis()));

  if ((startms+2000) < millis())
  {
#ifdef CLOCKDEBUG
    Serial.println("Time Packet Timeout");
    Serial.print("NTP Response Time: ");
    Serial.println(millis()-startms);
#endif
    //try again in 60 sec
    setSyncInterval(60);
    return 0;  
  }
#ifdef CLOCKDEBUG
  Serial.print("NTP Response Time: ");
  Serial.println(millis()-startms);
  Serial.println("");
  Serial.println("Time Packet Received");  
#endif

#ifdef CLOCKDEBUG 
  Serial.print("Time Packet Size:");  
  Serial.println(size);
#endif
  if (size >= NTP_PACKET_SIZE) 
  {
#ifdef CLOCKDEBUG      
    Serial.println("Receive NTP Response");
    setSyncInterval(60 * pConfig->NTP_Sync_interval);
#endif      
    Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
    unsigned long secsSince1900;
    // convert four bytes starting at location 40 to a long integer
    secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
    secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
    secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
    secsSince1900 |= (unsigned long)packetBuffer[43];
    return secsSince1900 - 2208988800UL;
  }

#ifdef CLOCKDEBUG
  Serial.println("Wrong NTP Response :-(");
#endif
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void open_NTP_Port() 
{
  config_t* pConfig = get_config_ptr();
#ifdef CLOCKDEBUG
  Serial.println("Starting UDP");
#endif  
  Udp.begin(localTimePort);
#ifdef CLOCKDEBUG
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
#endif
  setSyncInterval(60 * pConfig->NTP_Sync_interval);
  setSyncProvider(getNtpTime);
#ifdef CLOCKDEBUG
  Serial.println("Inital Sync successful");
  Serial.print("Sync Interval [sec]: ");
  Serial.println(60 * pConfig->NTP_Sync_interval);
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.println(second());
#endif  
}