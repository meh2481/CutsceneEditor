/*
    CutsceneEditor source - cutsceneEngine.h
    Copyright (c) 2013 Mark Hutcheson
*/
#ifndef CUTSCENEENGINE_H
#define CUTSCENEENGINE_H

#include "Engine.h"
#include <vector>

class CutsceneEngine : public Engine
{
private:
  list<obj*> m_lActors;
  ttvfs::VFSHelper vfs;
  Vec3 CameraPos;
  list<obj*>::iterator m_CurSelectedActor;
  Color selectionPulse;

protected:
    void frame();
    void draw();
    void init();
    void handleEvent(SDL_Event event);

public:
    CutsceneEngine(uint16_t iWidth, uint16_t iHeight, string sTitle);
    ~CutsceneEngine();

    void hudSignalHandler(string sSignal);  //For handling signals that come from the HUD
    
    //Program-specific functions
    void loadActors(string sFolderPath);
    void drawActors();
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to myEngine


#endif
