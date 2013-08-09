/*
    CutsceneEditor source - Object.h
    Copyright (c) 2013 Mark Hutcheson
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
public:
    list<physSegment*> segments;
    list<obj*> children;
    
    obj();
    ~obj();
    
    Point pos;
    float32 rot;
    void* usr;
    obj* parent;

    void draw();
    void addSegment(physSegment* seg);
    void addChild(obj* object);
    

};









#endif
