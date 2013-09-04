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
  parent = NULL;
}

obj::~obj()
{
    for(list<physSegment*>::iterator i = segments.begin(); i != segments.end(); i++)
        delete (*i);
}

void obj::draw(bool bChildDraw)
{
	if(parent != NULL && !bChildDraw)
		return;
	glColor4f(col.r,col.g,col.b,col.a);
	glPushMatrix();
	glTranslatef(pos.x, 0.0f, pos.y);	//X and Y are messed up for us. Ah, well
	glRotatef(rot*RAD2DEG, 0.0f, 1.0f, 0.0f);
	//Draw segments of this object
    for(list<physSegment*>::iterator i = segments.begin(); i != segments.end(); i++)
        (*i)->draw();
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	//Draw children of this object translated/rotated
	for(list<obj*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->draw(true);
	glPopMatrix();
}

void obj::addSegment(physSegment* seg)
{
    segments.push_back(seg);
}

void obj::addChild(obj* object, bool bOffset)
{
	if(object->parent != NULL)
		return;	//Don't make this a child if it already is a child of another object
	object->parent = this;
	if(bOffset)
	{
		//obj* toplevel;
		//for(toplevel = object; toplevel->parent != NULL; toplevel = toplevel->parent)
		//	;
		for(obj* o = this; o != NULL; o = o->parent)
		{
			object->pos.x -= o->pos.x;
			object->pos.y -= o->pos.y;	//Keep same position as before by factoring in parent locations
		}
	}
	//TODO: Take rotation into account
	children.push_back(object);
}

Point obj::getPos()
{
	Point p = pos;
	for(obj* o = parent; o != NULL; o = o->parent)
	{
		//Factor rotation of parents into finding out where this is in world space
		float32 xnew = p.x * cos(o->rot) + p.y * sin(o->rot);	//Reverse a standard Cartesian Coordinate transform, since -y is up
		float32 ynew = -p.x * sin(o->rot) + p.y * cos(o->rot);
		
		p.x = xnew + o->pos.x;
		p.y = ynew + o->pos.y;	//Keep same position as before by factoring in parent locations
	}
	return p;
}

float32 obj::getRot()
{
	float32 f = 0;
	for(obj* o = this; o != NULL; o = o->parent)
		f += o->rot;
	return f;
}

void obj::removeParenting()
{
	if(parent == NULL)
		return;	//No parents already
	pos = getPos();	//Reset position
	rot = getRot();
	
	for(list<obj*>::iterator i = parent->children.begin(); i != parent->children.end(); i++)
	{
		if(*i == this)
		{
			parent->children.erase(i);
			break;	//Done
		}
	}
	parent = NULL;
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

















