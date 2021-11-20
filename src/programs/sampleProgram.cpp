/**
 * @file sampleProgram.cpp
 * @author Bernhard Stöffler
 * @brief 
 * @version 0.1
 * @date 2021-11-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "zylProg.h"
#pragma message: Obnoxious

class : zylProg{
public:
	int init(){
		m_FB = zylProgManager::getColor(0);
		return 0;
	}
	void render(){
		for(int x=0; x<X_RES; x++)
			for(int y=2; y<Y_RES-3; y++)
				m_FB(x,y).setHue((millis()/100)%255);
	}
} sampleProgram;