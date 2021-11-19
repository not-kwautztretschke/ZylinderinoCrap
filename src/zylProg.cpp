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

//************************* Program Manager **********************

int zylProgManager::g_Count=0;
zylProg *zylProgManager::g_pHead=NULL;
zylProg *zylProgManager::g_pActive=NULL;

// implement "static methods" (i.e. global functions)

void zylProgManager::add(zylProg* ptr)
{
	if (g_pHead == NULL){ //first program
		g_pHead = ptr;
		g_pActive = ptr;
	}else{
		zylProg* cur = g_pHead;
		while(cur->m_pNext != NULL)
			cur = cur->m_pNext;
		cur->m_pNext = ptr;
	}
	g_Count++;
}

void zylProgManager::focus(int index)
{
	zylProg* ptr = g_pHead;
	for(int i=0; i<index; i++)
		ptr = ptr->m_pNext;
	g_pActive = ptr;
	g_pActive->activate();
}

void zylProgManager::input(uint8_t x, uint8_t y, uint8_t z){
	g_pActive->input(x, y, z);
}

int zylProgManager::initPrograms(){
	int error=0;
	zylProg* ptr = g_pHead;
	for(int i=0; i<g_Count; i++){
		error += ptr->init();
		ptr = ptr->m_pNext;
	}
	return error;
}

void zylProgManager::renderPrograms(){
	zylProg* ptr = g_pHead;
	for(int i=0; i<g_Count; i++){
		ptr->render();
		ptr = ptr->m_pNext;
	}
}

void zylProgManager::composite(zylFB fb_in, zylCompositeMode mode){
	zylProg* ptr = g_pHead;
	for(int i=0; i<g_Count; i++){
		//compositing
		for(int x=0;x<X_RES;x++){
			for(int y=0;y<Y_RES;y++){
				switch(mode){
				case ZCM_SINGLE:
					fb_in(x,y) = g_pActive->m_FB(x,y);
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

