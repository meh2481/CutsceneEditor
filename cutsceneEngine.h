/*
    CutsceneEditor source - cutsceneEngine.h
    Copyright (c) 2012 Mark Hutcheson
*/
#ifndef CUTSCENEENGINE_H
#define CUTSCENEENGINE_H

#include "Engine.h"
#include <vector>

class CutsceneEngine : public Engine
{
private:

protected:
    void frame();
    void draw();
    void init();
    void handleEvent(SDL_Event event);

public:
    CutsceneEngine(uint16_t iWidth, uint16_t iHeight, string sTitle);
    ~CutsceneEngine();

    void loadImages(string sListFilename);  //Loads all images listed in this file into memory, so we can batch load easily
    void loadSounds(string sListFilename);  //Loads all sounds listed in this file into memory
    void playSound(string sName);   //Plays a sound, with pitch shifting depending on framerate
    void hudSignalHandler(string sSignal);  //For handling signals that come from the HUD
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to myEngine


#endif
