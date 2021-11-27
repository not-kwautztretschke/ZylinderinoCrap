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

//***************** ZylProg base class functions *******************
zylProg::zylProg()
{	// automatically add new zylProg to linked list
	zylProgManager::add(this);
}

zylProg::zylProg(bool add)
{	// automatically add new zylProg to linked list
	//! to use this, derived zylProg needs "using zylProg::zylProg"
	if(add)
		zylProgManager::add(this);
}
//************************* Program Manager **********************
//TODO: FG/BG
int 		zylProgManager::s_Count=0;
zylProg*	zylProgManager::s_pHead=NULL;
zylProg*	zylProgManager::s_pActive=NULL;
CRGB 		zylProgManager::s_aColors[MAX_COLORS] = {CRGB::Black};
int 		zylProgManager::s_ActiveColorIndex = 	0;

void zylProgManager::add(zylProg* ptr)
{
	if (s_pHead == NULL)
		s_pActive = ptr;		//move to zylProgManager::init();
	ptr->m_pNext = 	s_pHead;
	s_pHead = 		ptr;
	s_Count++;
}

int zylProgManager::focus(int id)
{
	zylProg* ptr = s_pHead;
	while(ptr != NULL){
		if(ptr->m_Id==id){
			Serial.printf("Found program with ID %d\n", id);
			s_pActive = ptr;
			s_pActive->activate();
			return 0;
		}
		ptr = ptr->m_pNext;
	}
	return 1;
}

void zylProgManager::input(uint8_t x, uint8_t y, uint8_t z){
	s_pActive->input(x, y, z);
}

//TODO init(): FG/BG pointers, push/activate first, NULL handling

int zylProgManager::initPrograms(){
	Serial.printf("initializing %d programs\n", s_Count);
	int error=0;
	zylProg* ptr = s_pHead;
	for(int i=0; i<s_Count; i++){
		error += ptr->init();
		Serial.printf("Program with ID %d at %p\n", ptr->m_Id, ptr);
		ptr = ptr->m_pNext;
	}
	return error;
}

int zylProgManager::init(){
	s_aColors[0] = 		CRGB::Green;
	s_pActive = 		s_pHead;	//focus first program and push it on the render list
	if(s_pActive != NULL){
		s_pActive->activate();
	}else{
		Serial.println("NULL POINTER ERROR, no programs loaded");
		return 1;
	}
	return 0;

}

//TODO only render programs in renderlist
void zylProgManager::renderPrograms()
{
	zylProg* ptr = s_pHead;
	for(int i=0; i<s_Count; i++){
		ptr->render();
		ptr = ptr->m_pNext;
	}
}

//TODO renderlist; compositemode per zylProg
void zylProgManager::composite(CRGB fb_in[X_RES][Y_RES])
{	//currently only composite active program
	for(int x=0;x<X_RES;x++)
		for(int y=0;y<Y_RES;y++)
			fb_in[x][y] = s_pActive->m_FB[x][y];
}

void zylProgManager::selectColor(int i)
{	//activates color i
	s_ActiveColorIndex = i;
}

void zylProgManager::setColor(CRGB c, int i)
{	//changes a specific color and leaves index on it
	if(i>=0 && i<MAX_COLORS){
		s_ActiveColorIndex = i;
		setColor(c);
 	}
}
 
void zylProgManager::setColor(CRGB c)
{	//changes the currently active color
	s_aColors[s_ActiveColorIndex] = c;
}

CRGB zylProgManager::getColor(int i)
{	//returns a specific color without activating it
	return s_aColors[i];
}

CRGB zylProgManager::getColor()
{	//returns the currently selected color
	return s_aColors[s_ActiveColorIndex];
}