/**
 * @file zylFB.h
 * @author Bernhard Stoeffler
 * @brief 
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __ZYLFB_H_
#define __ZYLFB_H_

#include <FastLED.h>
#include "config.h"


//*************** Zylinder Pixel, inherited from FastLED *************************
//				  (Syntax is also inherited, hence the inline functions)
struct zylPel : CRGB {
	union{
		uint8_t     a;
		uint8_t     alpha;
		uint8_t     m_Alpha;
	};
	zylPel& setARGB (uint8_t na, uint8_t nr, uint8_t ng, uint8_t nb){
		a = na; r = nr; g = ng; b = nb;
        return *this;
    }
	inline zylPel& setAlpha (uint8_t na) {
		a = na;
		return *this;
	}

	//? constructors dont get inherited so here we go
	inline zylPel(uint32_t colorcode) : CRGB(colorcode)	
		{a = 255;}
	inline zylPel(const CRGB& rhs) : CRGB(rhs)			
		{a = 255;}
	inline zylPel(uint8_t ir, uint8_t ig, uint8_t ib, uint8_t ia = 255) : CRGB(ir, ig, ib)	
		{a = ia;}
	inline zylPel()	: CRGB()			
		{a = 255;}
	inline zylPel(zylPel& rhs) = default;
};

class zylFB {
private:
	zylPel				m_FB[X_RES][Y_RES];
public:
	zylPel&				xy(int x, int y);
	void				setAll(zylPel c);
	void				setAlpha(uint8_t na);
	zylPel&				operator()(int x, int y);
	void				operator=(zylPel c);
	;					zylFB(zylPel c);
	;					zylFB();
};

#endif