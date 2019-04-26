#ifndef main_h
#define main_h

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiUdp.h>

//Debug Defines
#define DEBUG
#define WIFIDEBUG
#define LEDDEBUG
#define CLOCKDEBUG


//Simple Setup for full setup during init?
//#define SIMPLE_SETUP

//Global Defines
#define BUTTON_PIN  14
#define NUM_PIXELS  60
#define MAX_SRV_CLIENTS 4
#define MAX_LIST_LENGTH 10

//Button Stuff
#define LONG_PRESS_MS 800
#define VERY_LONG_PRESS_MS 5000
typedef enum{
    NO_PRESS,
    DOWN,
    SHORT_PRESS,
    LONG_PRESS,
    VERY_LONG_PRESS,
}mode_button_e;



typedef enum{
    CONNECT_INIT,
    CONNECT_CONNECT,
    CONNECT_CONNECTED,
    CONNECT_READY,
    CONNECT_TIMEOUT
}t_connect_state;

//Global Neopixel Object to control LED's
extern NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip;
extern NeoGamma<NeoGammaTableMethod> colorGamma; // for any fade animations, best to correct gamma
extern DNSServer dnsServer;
extern ESP8266WebServer server;
extern WiFiClient  ServerClients[MAX_SRV_CLIENTS];
extern WiFiUDP Udp;
extern char tempstr[2048];

#endif