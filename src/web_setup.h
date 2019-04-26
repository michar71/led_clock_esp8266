#ifndef web_setup_h
#define web_setup_h

#include <arduino.h>

typedef enum{
    WESETUP_INIT,
    WSETUP_SCAN,
    WSETUP_ENABLE_AP,
    WSETUP_SETUP_WAITCONN,
    WSETUP_SETUP_WEBSERVER,
}t_web_Setup_states;

bool setup_state(void);


#endif

