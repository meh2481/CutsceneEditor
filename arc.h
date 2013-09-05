/*
    CutsceneEditor source - arc.h
    Class for creating electric-looking arcs between two points
    Copyright (c) 2013 Mark Hutcheson
*/
#ifndef ARC_H
#define ARC_H

#include "globaldefs.h"
#include "Image.h"
#include "Object.h"

class arc
{
protected:
	float32* segmentPos;
	Image* arcSegImg;
	uint32_t numSegments;
	
	arc(){};
	void average();	//Helper function to average the values for a less jittery arc
	
public:
	obj* obj1, *obj2;
	float32 depth;
	Color col;
	float32 add;
	float32 max;
	float32 height;
	uint32_t avg;
	
	arc(uint32_t number, Image* img);
	~arc();
	
	void init();
	void render();
	void update(float dt);
	
	//Accessor methods
	string getImageFilename()	{return arcSegImg->getFilename();};
	uint32_t getNumber()	{return numSegments;};
	

};
















#endif