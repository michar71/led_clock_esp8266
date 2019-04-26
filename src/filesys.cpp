#include "filesys.h"


File logfile;
char logbuf[256];
PString logline(logbuf, sizeof(logbuf));


Print& operator<<(Print& p, const PrintMac& pmac)
{
  const uint8_t* mac = pmac.mac;
  for (int i = 0; i < 6; ++i) {
    if (i > 0) {
      p << ':';
    }
    int b = mac[i];
    if (b < 0x10) {
      p << '0';
    }
    p << _HEX(b);
  }
  return p;
}

void appendLog()
{
#ifndef DEBUG     
  Serial << millis() << ' ' << logline << '\n';
#endif  
  logfile << millis() << ' ' << logline << '\n';
  logline.begin();
}


void init_FS(void)
{
    SPIFFS.begin();
    logfile = SPIFFS.open("/log.txt", "a+");
}