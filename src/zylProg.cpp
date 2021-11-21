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


zylProg::zylProg()
{ // automatically add new zylProg to linked list
	zylProgManager::add(this);
};

//TODO: constructors, linked list,

//************************* Program Manager **********************
//TODO: g->s, colorarray, FG/BG
int zylProgManager::s_Count=0;
zylProg *zylProgManager::s_pHead=NULL;
zylProg *zylProgManager::s_pActive=NULL;

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

//TODO only render programs in renderlist
void zylProgManager::renderPrograms(){
	zylProg* ptr = s_pHead;
	for(int i=0; i<s_Count; i++){
		ptr->render();
		ptr = ptr->m_pNext;
	}
}

//TODO renderlist; compositemode per zylProg
void zylProgManager::composite(CRGB fb_in[X_RES][Y_RES], zylCompositeMode mode){
	zylProg* ptr = s_pHead;
	for(int i=0; i<s_Count; i++){
		//compositing
		for(int x=0;x<X_RES;x++){
			for(int y=0;y<Y_RES;y++){
				switch(mode){
				case ZCM_SINGLE:
					fb_in[x][y] = s_pActive->m_FB[x][y];
					break;
				case ZCM_OVERWRITE:
				case ZCM_ADD_SCALE:
				case ZCM_ADD_SATURATE:
				case ZCM_SUB:
				case ZCM_AVG:
					break;
				}
			}
		}
		ptr = ptr->m_pNext;
	}
	;
}

//TODO: setColor, getColor