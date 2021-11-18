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

#include "programs/sampleProgram.h"

int sampleProgram::init(){
	m_Color = CRGB::Red;
	return 0;
}

void sampleProgram::render(){
	for(int x=0; x<X_RES; x++)
		for(int y=0; y<Y_RES; y++)
			m_FB = m_Color;
}

void sampleProgram::input(uint8_t x, uint8_t y, uint8_t z){
	m_Color.setRGB(x, y, z);
}

static sampleProgram(g_Zpm); //add itself to global program manager