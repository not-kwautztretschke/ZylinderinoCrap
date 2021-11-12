/**
 * @file main.cpp
 * @author Bernhard Stoeffler
 * @brief 
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiAutoSelector.h>


//************************** Defines **********************************
#define BAUD_RATE				115200
#define WIFI_CONNECT_TIMEOUT 	8000
#define PUP_PORT 				26091				//UDP port for receiving PUP commands
#define TEXT_PORT 				26092				//UDP port for receiving plaintext
#define LED_VOLTAGE 			5                   //Led voltage
#define LED_MAX_MILLIAMPS 		1000                //Maximum current provided by power source
#define GLOBAL_BRIGHTNESS 		10                  //Scales down overall brightness
#define AP_NAME 				"Zylinder"			//SoftAP SSID name
#define AP_PSWD 				"papatastisch"		//SoftAP Password
#define TEXTBUFFER_SIZE 		4096
#ifdef Z_LEO
  #define X_RES 				48                  //Resolution in x direction
  #define Y_RES 				7                   //Resolution in y direction
  #define NUM_LEDS 				(X_RES * Y_RES)     //Total number of Leds
  #define DATA_PIN 				13                  //Led data pin
  #define TARGET_FRAME_RATE 	60                  //global hat frame rate
  #define PUPBUFFER_SIZE		5					//PUP size, 5 -> with Signature
  #define 
#else
  #ifndef Z_BERNIE
    #define Z_BERNIE
  #endif
  #define X_RES 				35                  //Resolution in x direction (maximum)
  #define Y_RES 				8                   //Resolution in y direction
  #define NUM_LEDS 				270                 //Total number of Leds
  #define DATA_PIN 				26                  //Led data pin
  #define DIP1_PIN				33					//Dipswitches
  #define DIP2_PIN				32
  #define DIP3_PIN				35
  #define DIP4_PIN				34
  #define TARGET_FRAME_RATE 	30                  //global hat frame rate
  #define PUPBUFFER_SIZE		4					//PUP size, 4 -> without Signature
#endif

//*************************** Global Variables ***********************
bool g_WifiMode; 									//Dip1, Host or Client

CRGB g_aFrameBuffer[X_RES][Y_RES];					//global Framebuffer of CRGBs //TODO: refactor into class
//*************************** Hardware *******************************
CRGB g_aHWLeds[NUM_LEDS]; 				// Actual physical Framebuffer to be pushed out
CRGB g_DummyLed;   						// dummy to point to instead of NULL (avoids segfault)

int initHW(){
	//* dipswitches
	pinMode(DIP1_PIN, INPUT);
	pinMode(DIP2_PIN, INPUT);
	pinMode(DIP3_PIN, INPUT);
	pinMode(DIP4_PIN, INPUT);
	g_WifiMode = (digitalRead(DIP1_PIN)==HIGH) ? true : false;
	//* initialize LEDs
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(g_aHWLeds, NUM_LEDS);
	FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTAGE, LED_MAX_MILLIAMPS);
	FastLED.setBrightness(GLOBAL_BRIGHTNESS);
	for(int j=0;j<NUM_LEDS; j++)
		g_aHWLeds[j] = CRGB::Black;           // initialize all LEDs to Off
	FastLED.show();
}
#ifdef Z_LEO
	//! deprecated, see below
	static CRGB* coords(int x, int y, bool xWrap = false, bool yWrap = false){         // returns pointer to the CRGB at coordinates
		if(xWrap==false && (x<0 || x>=X_RES))
			return &g_DummyLed;
		else
		x = modulo(x,X_RES);
		
		if(yWrap==false && (y<0 || y>=Y_RES))
			return &g_DummyLed;
		else
		y = modulo(y,Y_RES);
		
		return &g_aLeds[x*Y_RES + abs(6*((x+1)%2)-y)];
	}
#else
	displayLEDs(){  //converts g_aFrameBuffer[][] to g_aHWLeds[] and displays it
		//todo: implement :)
		FastLED.show();
	}
#endif
//*************************** WiFi ***********************************
#include "wificredentials.h"
WiFiUDP 			g_Udp;
WiFiUDP 			g_UdpText;
char 				g_aPupBuffer[PUPBUFFER_SIZE];
char 				g_aTextBuffer[TEXTBUFFER_SIZE];
WiFiAutoSelector 	g_WifiAutoSelector(WIFI_CONNECT_TIMEOUT);

int initWifi(){
	if (g_WifiMode){ 							//Client mode
		g_WifiAutoSelector.add(STASSID, STAPSWD); 		//todo add more, change wificredentials.h
	}else{			
  		WiFi.softAP(AP_NAME, AP_PSWD);					//Host mode
		IPAddress IP = WiFi.softAPIP();
		Serial.print("AP IP address: ");
		Serial.println(IP);
	}
	g_Udp.begin(PUP_PORT);
	g_UdpText.begin(TEXT_PORT);
	return 0;
}

int handleWifiConnection(){								//periodically check the wifi connection
	if(WiFi.status() != WL_CONNECTED) {					//and try to reconnect if broken
		Serial.print("Connecting wifi ");
		if(-1 < wifiAutoSelector.scanAndConnect()) {
			int connectedIndex = wifiAutoSelector.getConnectedIndex();
			Serial.print("to '");
			Serial.print(wifiAutoSelector.getSSID(connectedIndex));
			Serial.println("'. Done.");
		}else{
			Serial.println("failed.");
			return 1;
		}
	}
	return 0;
}
//*************************** Over the Air Update ********************
int initOTA(){
	#ifdef Z_LEO
		ArduinoOTA.setHostname("Zylinderino-Leo");
		ArduinoOTA.setPasswordHash("a1d237e74e5ce5c1f4584da49afeb5f7");
	#else
		ArduinoOTA.setHostname("bernie-zylinder");
		ArduinoOTA.setPassword("swag");
	#endif
	ArduinoOTA.onStart([]() {
		Serial.println("OTA: Start");
		//digitalWrite(BOARD_LED,1);
	});
	ArduinoOTA.onEnd([]() {
		//digitalWrite(BOARD_LED,1);
		Serial.println("\nOTA: End");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		static bool state = false;
		//digitalWrite(BOARD_LED,state);
		state ^= true;
		Serial.printf("OTA: Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if 		(error == OTA_AUTH_ERROR)      	{Serial.println("Auth failed");		return 1;}
		else if (error == OTA_BEGIN_ERROR)  	{Serial.println("Begin failed");	return 2;}
		else if (error == OTA_CONNECT_ERROR)	{Serial.println("Connect failed");	return 3;}
		else if (error == OTA_RECEIVE_ERROR)	{Serial.println("Receive failed");	return 4;}
		else if (error == OTA_END_ERROR)    	{Serial.println("End failed");		return 5;}
	});
	ArduinoOTA.begin();
	return 0;
}
//*************************** Timer Interrupt ************************
volatile bool 	g_vRenderFrame = 	false;
volatile long 	g_vCurrentFrame = 	0;
volatile int  	g_vMissedFrames = 	0;

hw_timer_t * 	g_pTimer = 			NULL;  							//timer pointer
portMUX_TYPE 	g_TimerMux = 		portMUX_INITIALIZER_UNLOCKED; 	//var to share portmux state 

void initTimer(){
	g_pTimer = timerBegin(0, 80, true);    							//timer ticks at 80Mhz/divider, so 1Mhz here. divider can be 2-65536 (0->65536, 1->2)
	timerAttachInterrupt(g_pTimer, &onTimer, true);
	timerAlarmWrite(g_pTimer, 1000000/TARGET_FRAME_RATE, true);    	//interruptAt sets ticks it takes between interrupts
	timerAlarmEnable(g_pTimer);
}

void IRAM_ATTR onTimer() {  										//ISR
	portENTER_CRITICAL_ISR(&g_TimerMux);
	if(g_vRenderFrame){   											//still true -> last frame was not rendered in time
		g_vMissedFrames++;
	}else{
		g_vRenderFrame = true;
		g_vCurrentFrame++;          //! don't be tempted to do animations reliant on FPS!
	}
	portEXIT_CRITICAL_ISR(&g_TimerMux);
}

//*************************** Papatastic UDP Protocol *******************

int handlePupInput(){
	while (g_Udp.parsePacket()){
		int length = g_Udp.read(g_aPupBuffer, PUPBUFFER_SIZE);
		if length {
			switch g_aPupBuffer[0]{
				default:
				Serial.println("Invalid OPcode");
				return 1;
			}
		}
	}
}

//*************************** Zylinder Programs *************************
void renderPrograms(){
	//TODO: implement lmao
}
//*************************** Main Program ******************************
void setup(){
	Serial.begin(BAUD_RATE)
	initTimer();
	initWifi();
	initOTA();
}

void loop(){
	if (g_vMissedFrames){
		Serial.printf("MISSED %d FRAMES, SOMETHING IS WRONG\n", g_vMissedFrames);
		portENTER_CRITICAL(&g_TimerMux);
		g_vMissedFrames = 0;
		portEXIT_CRITICAL(&g_TimerMux);
	}
	if (g_vRenderFrame == true){
		renderPrograms();
		FastLED.show();
		portENTER_CRITICAL(&g_TimerMux);
		g_vRenderFrame = false;
		portEXIT_CRITICAL(&g_TimerMux);
	}
	if (g_WifiMode)
		handleWifiConnection();
	handlePupInput();
	ArduinoOTA.handle();
}