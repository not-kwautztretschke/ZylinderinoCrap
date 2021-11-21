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

static class : zylProg{
public:
	int init(){
		Serial.println("Init Sample Program 3");
	}
	void render(){
		for(int x=0; x<X_RES; x++)
			for(int y=0; y<Y_RES; y++)
				m_FB[x][y].setHue((millis()/10)%255);
	}
} sampleProgram3;