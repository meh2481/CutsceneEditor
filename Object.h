/*
    CutsceneEditor source - Object.h
    Copyright (c) 2013 Mark Hutcheson
*/

#ifndef OBJECT_H
#define OBJECT_H

#include "globaldefs.h"
#include "Image.h"
#include "3DObject.h"


#define VELOCITY_ITERATIONS 8
#define PHYSICS_ITERATIONS 3
#define SCALE_UP_FACTOR 16.0
#define SCALE_DOWN_FACTOR 0.0625

class physSegment
{
public:
    b2Body*         body;
    Image*  		img;
    Object3D*       obj3D;
	
	Point pos;
	float32 rot;
	Point scale;

    physSegment();
    ~physSegment();
	
	void draw();

};

class obj
{
public:
	bool bIsChild;	//If this is a child of another object
	
    list<physSegment*> segments;
    list<obj*> children;
    
    obj();
    ~obj();
    
    Point pos;
    float32 rot;
	Color col;
    void* usr;
    obj* parent;

    void draw(bool bChildDraw = false);
    void addSegment(physSegment* seg);
    void addChild(obj* object, bool bOffset = true);	//bOffset = factor in location of this object and calculate offset
	Point getPos();	//Get position with parents' positions factored in
    

};









#endif
