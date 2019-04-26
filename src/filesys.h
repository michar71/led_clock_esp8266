#ifndef filesys_h
#define filesys_h
#include <FS.h>
#include <PString.h>
#include <Streaming.h>
#include <arduino.h>

struct PrintMac
{
  const uint8_t* mac;
};

extern File logfile;
extern PString logline;

void appendLog();
void init_FS(void);

#endif