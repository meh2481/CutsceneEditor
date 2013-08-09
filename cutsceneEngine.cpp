/*
    CutsceneEditor source - myEngine.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "cutsceneEngine.h"

//For our engine functions to be able to call our Engine class functions - Note that this means there can be no more than one Engine at a time
//TODO: Think of workaround? How does everything communicate now?
static CutsceneEngine* g_pGlobalEngine;

void signalHandler(string sSignal)
{
    g_pGlobalEngine->hudSignalHandler(sSignal);
}

void fillRect(Rect rc, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    g_pGlobalEngine->fillRect(rc, red, green, blue, alpha);
}

CutsceneEngine::CutsceneEngine(uint16_t iWidth, uint16_t iHeight, string sTitle) : Engine(iWidth, iHeight, sTitle)
{
    g_pGlobalEngine = this;
}

CutsceneEngine::~CutsceneEngine()
{
}

void CutsceneEngine::frame()
{
    //updateObjects();    //Update the objects in the game
}

void CutsceneEngine::draw()
{
  Rect m_rcViewScreen;
  m_rcViewScreen.set(0,0,getWidth()/2,getHeight()/2);
  fillRect(m_rcViewScreen, 255, 0, 0, 100);
  m_rcViewScreen.offset(0,getHeight()/2);
  fillRect(m_rcViewScreen, 0, 255, 0, 100);
  m_rcViewScreen.offset(getWidth()/2,0);
  fillRect(m_rcViewScreen, 0, 0, 255, 100);
  
        
}

void CutsceneEngine::init()
{
  loadActors("./segments/");
}


void CutsceneEngine::hudSignalHandler(string sSignal)
{
}

void CutsceneEngine::handleEvent(SDL_Event event)
{
    //m_hud->event(event);    //Let our HUD handle any events it needs to
    switch(event.type)
    {
        //Key pressed
        case SDL_KEYDOWN:
            switch(event.key.keysym.sym)
            {

                case SDLK_ESCAPE:
                    quit();
                    break;

               
                
                case SDLK_RETURN:
                    if(keyDown(SDLK_LALT) || keyDown(SDLK_RALT))
                    {
                      toggleFullscreen();
                    }
                    break;
                    
              }
            break;

        //Key released
        case SDL_KEYUP:
            switch(event.key.keysym.sym)
            {

            }
            break;

            }
}

void CutsceneEngine::loadActors(string sFolderPath)
{
 
}

void CutsceneEngine::drawActors()
{
  
}
