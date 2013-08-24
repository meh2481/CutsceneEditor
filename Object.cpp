/*
    CutsceneEditor source - Object.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "Object.h"

//-----------------------------------------------------------------------------------------------------------------------
// obj class

obj::obj()
{
  pos.x = pos.y = 0.0f;
  rot = 0.0f;
  usr = NULL;
  parent = NULL;
}

obj::~obj()
{
    for(list<physSegment*>::iterator i = segments.begin(); i != segments.end(); i++)
        delete (*i);
}

void obj::draw()
{
	glPushMatrix();
	glTranslatef(pos.x, 0.0f, pos.y);	//X and Y are messed up for us. Ah, well
	glRotatef(rot*RAD2DEG, 0.0f, 1.0f, 0.0f);
	//Draw segments of this object
    for(list<physSegment*>::iterator i = segments.begin(); i != segments.end(); i++)
        (*i)->draw();
	//Draw children of this object translated/rotated
	for(list<obj*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->draw();
	glPopMatrix();
}

void obj::addSegment(physSegment* seg)
{
    segments.push_back(seg);
}

void obj::addChild(obj* object)
{
  children.push_back(object);
}


//----------------------------------------------------------------------------------------------------
// physSegment class
physSegment::physSegment()
{
    body = NULL;
    img = NULL;
    obj3D = NULL;
	
	pos.x = pos.y = 0.0f;
	rot = 0.0f;
	scale.x = scale.y = 1.0f;
}

physSegment::~physSegment()
{
  if(body != NULL)
    ; //TODO: Free Box2D body
  //if(img != NULL)	//NOTE: Potential memory leak danger if used incorrectly
  //  delete img;
  if(obj3D != NULL)
    delete obj3D;
}

void physSegment::draw()
{
	glPushMatrix();
	glTranslatef(pos.x, 0.0f, pos.y);
	glRotatef(rot*RAD2DEG, 0.0f, 1.0f, 0.0f);
	if(obj3D != NULL)
	{
		obj3D->render();
	}
	if(img != NULL)
	{
		Rect rcImgPos;
		rcImgPos.set(0,0,img->getWidth(),img->getHeight());
		img->drawCentered(pos, rcImgPos, rot, scale.x, scale.y);
	}
	glPopMatrix();
}

















