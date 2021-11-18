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

void sampleProgram::render(){
	for(int x=0; x<X_RES; x++)
		for(int y=0; y<Y_RES; y++)
			m_FB(x, y).setHue((millis()/10)%255);
}

static sampleProgram(g_Zpm); //add itself to global program manager