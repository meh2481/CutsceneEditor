/*
    CutsceneEditor source - myEngine.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "cutsceneEngine.h"
#include <float.h>

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
	selectionPulse.g = selectionPulse.b = 0.2f;
	Interpolate* inter = new Interpolate(&(selectionPulse.g));
    inter->setMinVal(0.2f, false);
    inter->setMaxVal(0.4f, false);
    inter->calculateIncrement(0.4f, 0.5f);
    addInterpolation(inter);
	inter = new Interpolate(&(selectionPulse.b));
    inter->setMinVal(0.2f, false);
    inter->setMaxVal(0.4f, false);
    inter->calculateIncrement(0.4f, 0.5f);
    addInterpolation(inter);
	showCursor();
	m_centerDraw = new Object3D("res/selectball.obj", NO_TEXTURE);
	m_centerDraw->wireframe = true;
}

CutsceneEngine::~CutsceneEngine()
{
	errlog << "~CutsceneEngine" << endl;
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
		delete (*i);
		
	delete m_centerDraw;
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
  
	//GFX hello world stuff
	/*glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, MAGIC_ZOOM_NUMBER);
	glDisable(GL_LIGHTING);
	Rect m_rcViewScreen;
	m_rcViewScreen.set(0,0,getWidth()/2,getHeight()/2);
	fillRect(m_rcViewScreen, 255, 0, 0, 100);
	m_rcViewScreen.offset(0,getHeight()/2);
	fillRect(m_rcViewScreen, 0, 255, 0, 100);
	m_rcViewScreen.offset(getWidth()/2,0);
	fillRect(m_rcViewScreen, 0, 0, 255, 100);*/
        
}

void CutsceneEngine::init(list<commandlineArg> sArgs)
{
	//Run through list for arguments we recognize
	for(list<commandlineArg>::iterator i = sArgs.begin(); i != sArgs.end(); i++)
	{
		errlog << "Commandline argument. Switch: " << i->sSwitch << ", value: " << i->sValue << endl;
		if(!(i->sSwitch.size()))	//No switch; treat as folder or something
			loadActors(i->sValue);
		else
		{
			if(i->sSwitch == "d" ||
			   i->sSwitch == "dir")
			{
				loadActors(i->sValue);
			}
		}
	}
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
		
		/*case SDL_MOUSEBUTTONDOWN:
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
            break;*/

        case SDL_MOUSEMOTION:
            if(getCursorDown(LMB) && m_CurSelectedActor != m_lActors.end())
            {
				(*m_CurSelectedActor)->pos.x += event.motion.xrel/(180.0);
				(*m_CurSelectedActor)->pos.y += event.motion.yrel/(180.0);
            }
            else if(getCursorDown(RMB) && m_CurSelectedActor != m_lActors.end())
            {
				(*m_CurSelectedActor)->rot += event.motion.xrel/170.0;
            }
			else //No button down; choose object to select
			{
				Vec3 pos;
				Point cursorpos = getCursorPos();
				pos.x = event.motion.x - getWidth()/2.0;
				pos.y = event.motion.y - getHeight()/2.0;
				pos.z = 0;
				
				pos.x /= 180.0;
				pos.y /= 180.0;
				m_CurSelectedActor = findClosestObject(pos);
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
		//Support .tiny3d files as well as .obj, because why not
		bool bIs3D = false;
		string sPNG = *i;
		if((*i).find(".tiny3d", (*i).size() - 7) != string::npos)
		{
			//Use PNG with this filename as texture
			sPNG.erase((*i).size() - 7);
			sPNG.append(".png");
			bIs3D = true;
		}
		else if((*i).find(".obj", (*i).size() - 4) != string::npos)
		{
			sPNG.replace((*i).size() - 4, 4, ".png");
			bIs3D = true;
		}
		
		if(bIs3D)
		{
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
	glEnable(GL_LIGHTING);
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		if(i == m_CurSelectedActor)	//Selected actor pulses red
			glColor4f(selectionPulse.r, selectionPulse.g, selectionPulse.b, selectionPulse.a);
		(*i)->draw();
		//Draw center
		glPushMatrix();
		glColor4f(0.0,1.0,0.0,1.0);
		glTranslatef((*i)->pos.x, 0.0f, (*i)->pos.y);
		glRotatef((*i)->rot*RAD2DEG, 0.0f, 1.0f, 0.0f);
		glScalef(0.07f, 0.07f, 0.07f);
		glDisable(GL_LIGHTING);
		m_centerDraw->render();
		glEnable(GL_LIGHTING);
		glPopMatrix();
		glColor4f(1.0f,1.0f,1.0f,1.0f);
	}
	glDisable(GL_LIGHTING);
	
	/* DEBUG: Draw cursor sort of thing where cursor is
	Vertex pos;
	Point cursorpos = getCursorPos();
	pos.x = cursorpos.x - getWidth()/2.0;
	pos.y = cursorpos.y - getHeight()/2.0;
	pos.z = 0;
	
	pos.x /= 180.0;
	pos.y /= 180.0;
	
	glPushMatrix();
	glColor4f(0.0,0.0,1.0,1.0);
	glTranslatef(pos.x, pos.z, pos.y);
	glScalef(0.01f, 0.01f, 0.01f);
	m_centerDraw->render();
	glPopMatrix();
	glColor4f(1.0f,1.0f,1.0f,1.0f);*/
}

list<obj*>::iterator CutsceneEngine::findClosestObject(Vec3 pos)
{
	list<obj*>::iterator ret = m_lActors.end();
	float32 closestPos = FLT_MAX;
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		Vec3 objPos;
		objPos.x = (*i)->pos.x;
		objPos.y = (*i)->pos.y;
		objPos.z = 0.0f;
		
		float32 dist = distanceSquared(objPos, pos);
		if(dist < closestPos)
		{
			closestPos = dist;
			ret = i;
		}
	}
	return ret;
}







