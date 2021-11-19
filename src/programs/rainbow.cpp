/**
 * @file rainbow.cpp
 * @author  Bernhard Stoeffler
 * @brief 
 * @version 0.1
 * @date 2021-11-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "zylProg.h"

class rainbowClass : zylProg{
private:
	int 		m_Speed = 5;
	int 		m_Slant = 0;
	int			m_Dir	= 1;
public:
	void render(){
		for (int x = 0; x < X_RES; x++){
			for (int y = 0; y < Y_RES; y++){
				m_FB(x,y).setHue(  
					((255*x)/X_RES)  
					+y*(m_Slant/5) 
					+ (m_Dir)?
						 -(millis()*(m_Speed/10))%255 
						: (millis()*(m_Speed/10))%255    
				); 
			} 
    	}
	}
	void input(uint8_t x, uint8_t y, uint8_t z){
		switch(x){
		case 0:
			m_Speed = y;
			break;
		case 1:
			m_Slant = y;
			break;
		case 2:
			m_Dir = !m_Dir;
		}
	}
};

static rainbowClass(g_Zpm);