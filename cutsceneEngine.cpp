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
	vfs.Prepare();
	CameraPos.x = CameraPos.y = 0;
	CameraPos.z = -4;
	m_CurSelectedActor = m_lActors.end();
	selectionPulse.r = 1.0f;
	selectionPulse.g = selectionPulse.b = 0.0f;
	Interpolate* inter = new Interpolate(&(selectionPulse.g));
    inter->setMinVal(0.0f, false);
    inter->setMaxVal(1.0f, false);
    inter->calculateIncrement(1.0f, 1.0f);
    addInterpolation(inter);
	inter = new Interpolate(&(selectionPulse.b));
    inter->setMinVal(0.0f, false);
    inter->setMaxVal(1.0f, false);
    inter->calculateIncrement(1.0f, 1.0f);
    addInterpolation(inter);
}

CutsceneEngine::~CutsceneEngine()
{
	errlog << "~CutsceneEngine" << endl;
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
		delete (*i);
}

void CutsceneEngine::frame()
{
    //updateObjects();    //Update the objects in the game
}

void CutsceneEngine::draw()
{
	glLoadIdentity();
	glTranslatef(CameraPos.x, CameraPos.y, CameraPos.z);
	glRotatef(90.0f,1.0f,0.0f,0.0f);
  drawActors();
  
  glLoadIdentity();
  glDisable(GL_LIGHTING);
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

bool isMouseDown = false;

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
					
				case SDLK_EQUALS:
					if(m_CurSelectedActor == m_lActors.end())
						m_CurSelectedActor = m_lActors.begin();
					else
						m_CurSelectedActor++;
					break;
				
				case SDLK_MINUS:
					if(m_CurSelectedActor == m_lActors.begin())
						m_CurSelectedActor = m_lActors.end();
					else
						m_CurSelectedActor--;
					break;
                    
              }
            break;

        //Key released
        case SDL_KEYUP:
            switch(event.key.keysym.sym)
            {

            }
            break;
		
		case SDL_MOUSEBUTTONDOWN:
            if(event.button.button == SDL_BUTTON_LEFT)
            {
				isMouseDown = true;
            }
			//RMB zooms in/out
            else if(event.button.button == SDL_BUTTON_RIGHT)
            {
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT)
            {
				isMouseDown = false;
            }
            break;

        case SDL_MOUSEMOTION:
            if(isMouseDown && m_CurSelectedActor != m_lActors.end())
            {
				(*m_CurSelectedActor)->pos.x += event.motion.xrel/(1000.0);
				(*m_CurSelectedActor)->pos.y += event.motion.yrel/(1000.0);
              //glTranslatef(event.motion.xrel/(scale_amt*1000.0), -event.motion.yrel/(scale_amt*1000.0), 0.0);
			  
              //CameraPos.x += event.motion.xrel/(1000.0);
              //CameraPos.y -= event.motion.yrel/(1000.0);
              //cout << "pos: " << fXpos << ", " << fYpos << endl;
            }
            break;
	}
		
}

void CutsceneEngine::loadActors(string sFolderPath)
{
	ttvfs::StringList lFiles;
	ttvfs::GetFileListRecursive(sFolderPath, lFiles);
	for(ttvfs::StringList::iterator i = lFiles.begin(); i != lFiles.end(); i++)
	{
		if((*i).find(".obj", (*i).size() - 4) != string::npos)
		{
			//Use PNG with this filename as texture
			string sPNG = *i;
			sPNG.replace((*i).size() - 4, 4, ".png");
			Object3D* o3d = new Object3D((*i), sPNG);
			obj* o = new obj();
			physSegment* seg = new physSegment();
			seg->obj3D = o3d;
			o->addSegment(seg);
			m_lActors.push_back(o);
		}
	}
}

void CutsceneEngine::drawActors()
{
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		if(i == m_CurSelectedActor)
			glColor4f(selectionPulse.r, selectionPulse.g, selectionPulse.b, selectionPulse.a);
		else
			glColor4f(1.0f,1.0f,1.0f,1.0f);
		(*i)->draw();
	}
}
