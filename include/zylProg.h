/**
 * @file zylProg.h
 * @author Bernhard Stoeffler
 * @brief 
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __ZYLPROG_H_
#define __ZYLPROG_H_

#ifdef ARDUINO
#include <FastLED.h>
#else
typedef char CRGB;
#include	<stdlib.h>
#include	<stdint.h>
#endif
#include "config.h"

//************************* Pixel and Framebuffer *******************

class framebuffer {
private:
	class pixel {
		CRGB				m_Rgb;
		uint8_t				m_Alpha;
	};
	pixel 					fb[X_RES][Y_RES];
public:
	CRGB*					xy(int x, int y);
	uint8_t					xy_a(int x, int y);
	void					setColorXY(int x, int y, CRGB c);
	void					setAlphaXY(int x, int y, uint8_t a);
	void					setColorX(int x, CRGB c);
	void					setAlphaX(int x, uint8_t a);
	void					setColorY(int y, CRGB c);
	void					setAlphaY(int y, uint8_t a);
	void					setColorAll(CRGB c);
	void					setAlphaAll(uint8_t a);
	framebuffer&			operator+= (CRGB& c); //i have no idea what i'm doing or why these are references
	framebuffer&			operator-= (CRGB& c);
	framebuffer&			operator/= (uint8_t d);
	framebuffer&			operator*= (uint8_t d);
};


//************************* Zylinder Program ************************

class zylProg{
protected:
	framebuffer				m_FB;
public:
	zylProg*				m_pNext = NULL;

	;						zylProg();
	virtual int 			init(){return 0;}
	virtual void          	activate(){};
	virtual void          	render() = 0;    // * must be redefined
	virtual void			input(uint8_t x, uint8_t y, uint8_t z){};
};

//************************* Program Manager *************************

enum zylCompositeMode{
	ZCM_OVERWRITE,
	ZCM_ADD_SCALE,
	ZCM_ADD_SATURATE,
	ZCM_SUB,
	ZCM_AVG
};

class zylProgManager{
private:
	static int					g_Count;
	static zylProg*				g_pHead;
	static zylProg*				g_pActive;
public:
	static void					add(zylProg *ptr);
	static void					focus(int index);
	static int					initPrograms();
	static void					renderPrograms();
	static void					composite(framebuffer fb, zylCompositeMode mode);
	static void					input(uint8_t x, uint8_t y, uint8_t z);
};

#endif
