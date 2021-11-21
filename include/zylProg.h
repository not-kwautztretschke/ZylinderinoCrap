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

//************************* Zylinder Program ************************

class zylProg{
public:
	int						m_Id;
	CRGB					m_FB[X_RES][Y_RES];
	zylProg*				m_pNext = NULL;

	;						zylProg();
	virtual int 			init(){return 0;}
	virtual void          	activate(){};
	virtual void          	render() = 0;    // * must be redefined
	virtual void			input(uint8_t x, uint8_t y, uint8_t z){};
	//TODO: composite mode, ID, renderlist pointer & methods, constructors(bool)
};

//************************* Program Manager *************************

enum zylCompositeMode{
	ZCM_SINGLE,
	ZCM_OVERWRITE,
	ZCM_ADD_SCALE,
	ZCM_ADD_SATURATE,
	ZCM_SUB,
	ZCM_AVG
};

class zylProgManager{
private:
	static int					s_Count;
	static zylProg*				s_pHead;
	static zylProg*				s_pActive;
public:
	static void					add(zylProg *ptr);
	static int					focus(int id);
	static int					initPrograms();
	static int					init();
	static void					renderPrograms();
	static void					composite(CRGB fb[X_RES][Y_RES], zylCompositeMode mode);
	static void					input(uint8_t x, uint8_t y, uint8_t z);
	//TODO: rename g->s, color array, init(), FG & BG
};

#endif
