/*
    CutsceneEditor source - arc.h
    Class for creating electric-looking arcs between two points
    Copyright (c) 2013 Mark Hutcheson
*/
#ifndef ARC_H
#define ARC_H

#include "globaldefs.h"
#include "Image.h"

class arc
{
protected:
	float* pos;
	Image* arcSegImg;
	uint8_t num;
	
	arc(){};
	void average();	//Helper function to average the values for a less jittery arc
	
public:
	Point p1, p2;
	Color col;
	float add;
	float max;
	uint8_t avg;
	
	arc(uint8_t number, Image* img);
	~arc();
	
	void init();
	void render();
	void update(float dt);
	

};
















#endif