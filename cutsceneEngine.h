/*
    CutsceneEditor source - cutsceneEngine.h
    Copyright (c) 2013 Mark Hutcheson
*/
#ifndef CUTSCENEENGINE_H
#define CUTSCENEENGINE_H

#include "Engine.h"
#include <vector>

#define SELECT_MIN_DISTANCE	0.3

class CutsceneEngine : public Engine
{
private:
  list<obj*> m_lActors;
  ttvfs::VFSHelper vfs;
  Vec3 CameraPos;
  list<obj*>::iterator m_CurSelectedActor;
  Color selectionPulse;
  Object3D* m_centerDraw;	//For drawing objects' centers
  
  //Editing helpers
  Point m_ptOldPos;
  float32 m_fOldRot;
  bool m_bDragPos;
  bool m_bDragRot;
  bool m_bConstrainX;
  bool m_bConstrainY;

protected:
    void frame();
    void draw();
    void init(list<commandlineArg> sArgs);
    void handleEvent(SDL_Event event);

public:
    CutsceneEngine(uint16_t iWidth, uint16_t iHeight, string sTitle);
    ~CutsceneEngine();

    void hudSignalHandler(string sSignal);  //For handling signals that come from the HUD
    
    //Program-specific functions
    void loadActors(string sFolderPath);
    void drawActors();
	list<obj*>::iterator findClosestObject(Vec3 pos);	//Find object closest to given point
	
	//Save and load XML detailing the layout
	void save(string sFilename);
	void load(string sFilename);
	void writeObject(obj* object, XMLElement* parent, XMLDocument* doc);
	void readObject(obj* object, XMLElement* actor);
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to myEngine

//XML helper functions
void writeVec2(XMLElement* elem, string sAttributeName, Point vec);	//Write out a 2D point as one XML attribute
void readVec2(XMLElement* elem, string sAttributeName, Point* vec);	//Read a 2D point from one XML attribute


#endif
