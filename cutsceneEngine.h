/*
    CutsceneEditor source - cutsceneEngine.h
    Copyright (c) 2013 Mark Hutcheson
*/
#ifndef CUTSCENEENGINE_H
#define CUTSCENEENGINE_H

#include "Engine.h"
#include "arc.h"
#include <vector>

#define SELECT_MIN_DISTANCE	0.3
#define KEYFRAME_SIZE	0.01666666666666666666666666666667

typedef struct
{
	list<itemkeys*> items;
	obj* o;
} keyobj;

class CutsceneEngine : public Engine
{
private:
  list<keyobj> m_lActors;
  list<arc*> m_lArcs;
  Image* arcImg;
  ttvfs::VFSHelper vfs;
  Vec3 CameraPos;
  list<keyobj>::iterator m_CurSelectedActor;
  list<keyobj>::iterator m_CurSelectedParent;
  list<arc*>::iterator m_CurSelectedArc;
  Color selectionPulse;
  Color parentPulse;
  Object3D* m_centerDraw;	//For drawing objects' centers
  float32 m_fCurrentFrameTime;
  Text* m_text;
  bool m_bIsPlaying;
  float32 m_fPlayingTime;
  
  HUD* m_hud;
  
  //Editing helpers
  Point m_ptOldPos;
  float32 m_fOldRot;
  bool m_bDragPos;
  bool m_bDragRot;
  bool m_bPanScreen;
  bool m_bConstrainX;
  bool m_bConstrainY;
  bool m_bShowArcs;

protected:
    void frame();
    void draw();
    void init(list<commandlineArg> sArgs);
    void handleEvent(SDL_Event event);
	float32 mouseScaleFac();
	Point getPannedMousePos();

public:
    CutsceneEngine(uint16_t iWidth, uint16_t iHeight, string sTitle, bool bResizable = false);
    ~CutsceneEngine();

    void hudSignalHandler(string sSignal);  //For handling signals that come from the HUD
    
    //Program-specific functions
    void loadActors(string sFolderPath);
    void drawActors();
	list<keyobj>::iterator findClosestObject(Vec3 pos);	//Find object closest to given point
	
	//Save and load XML detailing the layout
	void save(string sFilename);
	void load(string sFilename);
	void writeObject(obj* object, XMLElement* parent, XMLDocument* doc);
	void readObject(obj* object, XMLElement* actor);
	
	void changeFrame(float32 fTime);
	void addFrame(keyobj* o, float32* pointer, float32 value, float32 fTime);	//Add a frame to this keyed object
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to myEngine

//XML helper functions
void writeVec2(XMLElement* elem, string sAttributeName, Point vec);	//Write out a 2D point as one XML attribute
void readVec2(XMLElement* elem, string sAttributeName, Point* vec);	//Read a 2D point from one XML attribute


#endif
