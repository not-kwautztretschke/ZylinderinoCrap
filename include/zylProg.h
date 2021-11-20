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

#include "zylFB.h"
#include "config.h"

//************************* Zylinder Program ************************
enum zylCompositeMode{
	ZCM_SOLID,
	ZCM_ADD,
	ZCM_SUB,
	ZCM_AVG
};

class zylProg{
public:
	int						m_Id;
	zylFB					m_FB;
	zylProg*				m_pNext = NULL;
	zylProg*				m_pAbove = NULL;
	zylProg*				m_pBelow = NULL;
	zylCompositeMode		m_CompositeMode;

	virtual int 			init(){return 0;}
	virtual void          	activate(){};
	virtual void          	render(){};
	virtual void			input(uint8_t x, uint8_t y, uint8_t z){};
	;						zylProg(bool include);
	;						zylProg();
	void					push();
	int						pop();
	int						move(bool up);
};

//************************* Program Manager *************************

class zylProgManager{
private:		
	static int					s_Count;
	static zylProg*				s_pHead;
	static zylProg*				s_pActive;
	static zylPel				s_aColors[MAX_COLORS];
	static int					s_ActiveColorIndex;
public:
	static zylProg				s_FG;
	static zylProg				s_BG;
	static void					add(zylProg *ptr);
	static void					focus(int index);
	static int					init();
	static int					initPrograms();
	static void					input(uint8_t x, uint8_t y, uint8_t z);
	static void					renderPrograms();
	static void					composite(zylFB fb_in);
	static void					setColor(zylPel c, int i);
	static void 				setColor(zylPel c);
	static zylPel				getColor(int i);
	static zylPel				getColor();
};

#endif
