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
//#include <WiFi.h>
//#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
//#include "WiFiAutoSelector.h"

#include "config.h"
#include "zylHW.h"
#include "zylWifi.h"
#include "zylProg.h"
#include "opcodes.h"

#include "programs/sampleProgram.h"


//*************************** Static Variables **************************
static zylHW_Bernie 	s_HW;
static zylWifi			s_Wifi;
static zylUdp			s_Udp;
static CRGB				s_FB[X_RES][Y_RES];
zylProgManager			g_Zpm;

//*************************** Timer Interrupt ************************
volatile bool 			g_vRenderFrame = 	false;
volatile long 			g_vCurrentFrame = 	0;
volatile int  			g_vMissedFrames = 	0;

hw_timer_t * 			g_pTimer = 			NULL;  							//timer pointer
portMUX_TYPE 			g_TimerMux = 		portMUX_INITIALIZER_UNLOCKED; 	//var to share portmux state 

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

void initTimer(){
	g_pTimer = timerBegin(0, 80, true);    							//timer ticks at 80Mhz/divider, so 1Mhz here. divider can be 2-65536 (0->65536, 1->2)
	timerAttachInterrupt(g_pTimer, &onTimer, true);
	timerAlarmWrite(g_pTimer, 1000000/TARGET_FRAME_RATE, true);    	//interruptAt sets ticks it takes between interrupts
	timerAlarmEnable(g_pTimer);
}

void(* resetFunc) (void) = 0;
//*************************** Main Program ******************************
void setup(){
	Serial.begin(115200);
	initTimer();
	initOTA();
	s_HW.init();				//TODO all these have a return value, use it.
	s_Wifi.init(s_HW.getDipSwitch(0) ? ZWM_HOST : ZWM_CLIENT);
	s_Udp.init(PUP_PORT);
	zylProgManager::initPrograms();
}

void loop(){
	(void) s_Wifi.handle();
	if (g_vMissedFrames){
		Serial.printf("MISSED %d FRAMES, SOMETHING IS WRONG\n", g_vMissedFrames);
		portENTER_CRITICAL(&g_TimerMux);
		g_vMissedFrames = 0;
		portEXIT_CRITICAL(&g_TimerMux);
	}
	if (g_vRenderFrame == true){
		g_Zpm.renderPrograms();
		g_Zpm.composite(s_FB, ZCM_OVERWRITE);
		s_HW.show(s_FB);
		portENTER_CRITICAL(&g_TimerMux);
		g_vRenderFrame = false;
		portEXIT_CRITICAL(&g_TimerMux);
	}

	uint8_t opcode, x, y, z;
	if(s_Udp.read(&opcode, &x, &y, &z)){
		switch(opcode){
			case OP_REBOOT:
				resetFunc();
			case OP_CHANGEPROGRAM:
				g_Zpm.focus(x);
			case OP_PROGRAMINPUT:
				g_Zpm.input(x, y, z);
			default:
			Serial.println("Invalid Opcode");
		}
	}


	ArduinoOTA.handle();
}