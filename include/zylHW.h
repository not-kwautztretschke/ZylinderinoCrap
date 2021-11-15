/**
 * @file zylHW.h
 * @author Bernhard Stöffler
 * @brief 
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __ZYLHW_H_
#define __ZYLHW_H_

#include <FastLED.h>
#include "config.h"

#define LED_VOLTAGE 			5                   //Led voltage
#define LED_MAX_MILLIAMPS 		1000                //Maximum current provided by power source
#define GLOBAL_BRIGHTNESS 		10                  //Scales down overall brightness
#ifdef Z_LEO
  #define NUM_LEDS 				336				    //Total number of Leds
  #define DATA_PIN 				13                  //Led data pin
#endif
#ifdef Z_BERNIE
  #define NUM_LEDS 				270                 //Total number of Leds
  #define DATA_PIN 				26                  //Led data pin
  #define DIP1_PIN				33					//Dipswitches
  #define DIP2_PIN				32
  #define DIP3_PIN				35
  #define DIP4_PIN				34
#endif

/*					belongs somewhere else
enum zylHWMode{
	ZHW_OVERWRITE,
	ZHW_ADD_SCALE,
	ZHW_ADD_SATURATE,
	ZHW_SUB,
	ZHW_AVG
};*/ 

class zylHW{
private:
	CRGB			m_DummyLed;
	//CRGB 			m_aLeds[]; 		//? don't know if this is smart to have..
									//? it needs to be redefined anyway
public:
	virtual int 	init();
	virtual void	show(CRGB* fb);
	//virtual void	composite(zylHWMode mode, CRGB** fb);
};

class zylHW_Bernie : zylHW{		//? does this need to be in the header or can we just define it in zylHW.cpp?
private:
	const int 		m_aRes[];
	const int 		m_aOff[];
	const int 		g_aRes[8] = {32, 33, 33, 34, 34, 34, 35, 35};		// Number of LEDs in each row
	const int 		g_aOff[8] = {0, 32, 65, 98, 132, 166, 200, 235};	// indices of first LED in row
	CRGB 			m_aLeds[NUM_LEDS];
public:
	int 			init();
	void			show(CRGB* fb);
	//void			composite(zylHWMode mode, CRGB** fb);
	int				getDipSwitch(int num);
};


#endif