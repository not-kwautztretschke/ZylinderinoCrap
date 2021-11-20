/**
 * @file zylProg.cpp
 * @author Bernhard Stöffler
 * @brief 
 * @version 0.1
 * @date 2021-11-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "zylProg.h"

//***************** ZylProg base class functions ************************
zylProg::zylProg(bool include)
{ // automatically add new zylProg to linked list
	if(include)
		zylProgManager::add(this);
};

zylProg::zylProg()
{
	zylProg(true);
}

void zylProg::push()
{	//pushes program to the top of the render queue
	m_pAbove = 						&zylProgManager::s_FG;	//set own pointers
	m_pBelow = 						zylProgManager::s_FG.m_pBelow;
	zylProgManager::s_FG.m_pBelow = this;					//redirect other's pointers
	m_pBelow->m_pAbove = 			this;
}

int zylProg::pop()
{	//removes program from render queue, if it's in there
	if(m_pAbove && m_pBelow){
		m_pAbove->m_pBelow = 	m_pBelow; 	//redirect other's pointers
		m_pBelow->m_pAbove = 	m_pAbove;
		m_pAbove = 				NULL;		//remove own pointers
		m_pBelow = 				NULL;
		return 0;
	}else
		return 1;
}

int zylProg::move(bool up)
{	//swaps program with one above(/below) if in render queue
	if((m_pAbove == &zylProgManager::s_FG && up)
	 ||(m_pAbove == &zylProgManager::s_BG && !up)){
		return 1;
	}else{
		if(up){		//get a whiteboard and draw some arrows if unclear
			zylProg* oldAbove = 	m_pAbove;
			m_pAbove = 				oldAbove->m_pAbove;
			oldAbove->m_pAbove =	this;
			oldAbove->m_pBelow = 	m_pBelow;
			m_pBelow->m_pAbove = 	oldAbove;
			m_pBelow = 				oldAbove;
			m_pAbove->m_pBelow = 	this;
			return 0;
		}else{
			zylProg* oldBelow = 	m_pBelow;
			m_pBelow = 				oldBelow->m_pBelow;
			oldBelow->m_pBelow = 	this;
			oldBelow->m_pAbove = 	m_pAbove;
			m_pAbove->m_pBelow = 	oldBelow;
			m_pAbove = 				oldBelow;
			m_pBelow->m_pAbove = 	this;
			return 0;
		}
	}
}

//************************* Program Manager **********************

int 		zylProgManager::s_Count =				0;
zylProg*	zylProgManager::s_pHead =				NULL;
zylProg*	zylProgManager::s_pActive =				NULL;
zylPel 		zylProgManager::s_aColors[MAX_COLORS] = {zylPel::Black};
int 		zylProgManager::s_ActiveColorIndex = 	0;
zylProg		zylProgManager::s_FG(false);
zylProg		zylProgManager::s_BG(false);

void zylProgManager::add(zylProg* ptr)
{	//pushes newly installed program on the list of all programs
	ptr->m_pNext = 	s_pHead;
	s_pHead = 		ptr;
}

void zylProgManager::focus(int id)
{	//finds program by id and activates it
	zylProg* ptr = s_pHead;
	while(ptr->m_Id != id && ptr->m_pNext!=NULL)
		ptr = ptr->m_pNext;
	s_pActive = ptr;
	s_pActive->activate();
}

int zylProgManager::init()
{	
	s_aColors[0] = 		zylPel::Green;
	s_FG.m_pAbove = 	&s_FG;
	s_FG.m_pBelow = 	&s_BG;
	s_BG.m_pAbove = 	&s_FG;
	s_BG.m_pBelow = 	&s_BG;
	s_pActive = 		s_pHead;	//focus first program and push it on the render list
	s_pActive->activate();
	s_pActive->push();
	return 0;
}

int zylProgManager::initPrograms()
{	//calls init() on all installed programs
	int error=0;
	zylProg* ptr = s_pHead;
	for(int i=0; i<s_Count; i++){
		error += ptr->init();
		ptr = ptr->m_pNext;
	}
	return error;
}

void zylProgManager::input(uint8_t x, uint8_t y, uint8_t z)
{
	s_pActive->input(x, y, z);
}

void zylProgManager::renderPrograms()
{
	#ifdef ZPM_RENDER_ALL
	zylProg* ptr = s_pHead;
	for(int i=0; i<s_Count; i++){
		ptr->render();
		ptr = ptr->m_pNext;
	}
	#else
	#endif
}

void zylProgManager::composite(zylFB fb_in)
{
	fb_in = zylPel::Black;
	zylProg* ptr = &zylProgManager::s_BG;
	while(ptr != &zylProgManager::s_FG){
		for(int x=0;x<X_RES;x++){
			for(int y=0;y<Y_RES;y++){
				uint8_t a = ptr->m_FB.xy(x,y).m_Alpha;
				switch(ptr->m_CompositeMode){
					case ZCM_SOLID:
						fb_in(x,y) = 
							(fb_in(x,y)*(255-a))
							+(ptr->m_FB(x,y)*a);
						break;
					case ZCM_ADD:
						fb_in(x,y) = 
							fb_in(x,y)
							+(ptr->m_FB(x,y)*a);
						break;
					case ZCM_SUB:
						fb_in(x,y) = 
							fb_in(x,y)
							-(ptr->m_FB(x,y)*a);
						break;
				}
			}
		}
		ptr = ptr->m_pAbove;
	}
}

void zylProgManager::setColor(zylPel c, int i)
{	//changes a specific color and leaves index on it
	if(i>=0 && i<MAX_COLORS){
		s_ActiveColorIndex = i;
		setColor(c);
	}
}

void zylProgManager::setColor(zylPel c)
{	//changes the currently active color
	s_aColors[s_ActiveColorIndex] = c;
}

zylPel zylProgManager::getColor(int i)
{	//returns a specific color without activating it
	return s_aColors[i];
}

zylPel zylProgManager::getColor()
{	//returns the currently selected color
	return s_aColors[s_ActiveColorIndex];
}