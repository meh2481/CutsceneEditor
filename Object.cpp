/*
    Heartlight++ source - Object.cpp
    Class for objects within the game
    Copyright (c) 2012 Mark Hutcheson
*/

#include "Object.h"

//-----------------------------------------------------------------------------------------------------------------------
// obj class

obj::obj()
{
  pos.x = pos.y = 0.0f;
  rot = 0.0f;
  usr = NULL;
}

obj::~obj()
{
    for(list<physSegment*>::iterator i = segments.begin(); i != segments.end(); i++)
        delete (*i);
}

void obj::draw()
{
    for(list<physSegment*>::iterator i = segments.begin(); i != segments.end(); i++)
    {
        //Point ptPos(0,0);
        //float32 rot = 0.0;
        if((*i)->body != NULL)
        {
            pos = (*i)->body->GetPosition();
			pos *= SCALE_UP_FACTOR;
            rot = (*i)->body->GetAngle();
			//if((*i)->layer != NULL && (*i)->layer->image != NULL)
			//	(*i)->layer->image->drawCentered(ptPos.x + (*i)->layer->pos.x*cos(rot) - (*i)->layer->pos.y*sin(rot),
      //                                ptPos.y + (*i)->layer->pos.y*cos(rot) + (*i)->layer->pos.x*sin(rot),
      //                                (*i)->layer->rot + rot, (*i)->layer->scale.x, (*i)->layer->scale.y);
        }
        glPushMatrix();
        glTranslatef(pos.x, pos.y, 0.0f);
        glRotatef(rot*RAD2DEG, 0.0f, 0.0f, 1.0f);
        if((*i)->obj3D != NULL)
        {
          (*i)->obj3D->render();
        }
        if((*i)->layer != NULL)
        {
          (*i)->layer->draw();
        }
        glPopMatrix();
        
    }
}

void obj::addSegment(physSegment* seg)
{
    segments.push_back(seg);
}


//----------------------------------------------------------------------------------------------------
// physSegment class
physSegment::physSegment()
{
    body = NULL;
    layer = NULL;
    obj3D = NULL;
}

physSegment::~physSegment()
{
  if(body != NULL)
    ; //TODO: Free Box2D body
  if(layer != NULL)
    delete layer;
  if(obj3D != NULL)
    delete obj3D;
}


















