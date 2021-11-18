/**
 * @file sampleProgram.h
 * @author Bernhard Stöffler
 * @brief 
 * @version 0.1
 * @date 2021-11-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "zylProg.h"

class sampleProgram : zylProg {
private:
	CRGB 					m_Color;
public:
	int				init();
	void			input(uint8_t x, uint8_t y, uint8_t z);
	void          	render();
};