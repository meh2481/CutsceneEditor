/*
    Heartlight++ source - Object.h
    Class for objects within the game
    Copyright (c) 2012 Mark Hutcheson
*/

#ifndef OBJECT_H
#define OBJECT_H

#include "globaldefs.h"
#include "Image.h"
#include "3DObject.h"
#include "SceneLayer.h"


#define VELOCITY_ITERATIONS 8
#define PHYSICS_ITERATIONS 3
#define SCALE_UP_FACTOR 16.0
#define SCALE_DOWN_FACTOR 0.0625

class physSegment
{
public:
    b2Body*         body;
    parallaxLayer*  layer;
    Object3D*       obj3D;

    physSegment();
    ~physSegment();

};

class obj
{
protected:
    list<physSegment*> segments;

public:
    obj();
    ~obj();
    
    Point pos;
    float32 rot;
    void* usr;

    void draw();
    void addSegment(physSegment* seg);

};









#endif
