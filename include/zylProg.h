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

#include <FastLED.h>
#include "config.h"

class zylProgManager; // forward declaration, needed for zylProg(zpm)

class zylProg{
protected:
	CRGB					m_FB[X_RES][Y_RES];
public:
	zylProg*				m_pNext = NULL;

	;						zylProg(zylProgManager zpm);
	virtual int 			init(){}
	virtual void          	activate(){};
	virtual void          	render() = 0;    // * Virtual function, so must be redefined
	virtual void			input(uint8_t x, uint8_t y, uint8_t z);
};

//************************* Program Manager *************************

class zylProgManager{
private:
	int						m_Count = 0;
	zylProg*				m_pHead=NULL;
	zylProg*				m_pActive=NULL;
public:
	void					add(zylProg* ptr);
	void					focus(int index);
	void					renderPrograms();
	void					composite(CRGB** fb);
	void					input(uint8_t x, uint8_t y, uint8_t z);
};

void zylProgManager::add(zylProg* ptr){
	if (m_pHead == NULL){ //first program
		m_pHead = ptr;
		m_pActive = ptr;
	}else{
		zylProg* cur = m_pHead;
		while(cur->m_pNext != NULL)
			cur = cur->m_pNext;
		cur->m_pNext = ptr;
	}
	m_Count++;
}

void zylProgManager::focus(int index){
	zylProg* ptr = m_pHead;
	for(int i=0; i<index; i++)
		ptr = ptr->m_pNext;
	m_pActive = ptr;
	m_pActive->activate();
}

void zylProgManager::input(uint8_t x, uint8_t y, uint8_t z){
	m_pActive->input(x, y, z);
}
//static zylProgManager zpm;	//? I hope this works? should be only one zpm overall, the same for each program

zylProg::zylProg(zylProgManager zpm) { // automatically add new zylProg to linked list
	zpm.add(this);
};


#endif