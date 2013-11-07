/*
    CutsceneEditor source - myEngine.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "cutsceneEngine.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>

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

CutsceneEngine::CutsceneEngine(uint16_t iWidth, uint16_t iHeight, string sTitle, bool bResizable) : Engine(iWidth, iHeight, sTitle, bResizable)
{
	g_pGlobalEngine = this;
	vfs.Prepare();
	CameraPos.x = CameraPos.y = 0;
	CameraPos.z = -4;
	
	//Set up selection variables
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
	
	//Set up parenting variables
	m_CurSelectedParent = m_lActors.end();
	parentPulse.b = 1.0f;
	parentPulse.g = parentPulse.r = 0.2f;
	inter = new Interpolate(&(parentPulse.g));
    inter->setMinVal(0.2f, false);
    inter->setMaxVal(0.4f, false);
    inter->calculateIncrement(0.4f, 0.5f);
    addInterpolation(inter);
	inter = new Interpolate(&(parentPulse.r));
    inter->setMinVal(0.2f, false);
    inter->setMaxVal(0.4f, false);
    inter->calculateIncrement(0.4f, 0.5f);
    addInterpolation(inter);
	
	showCursor();
	m_centerDraw = new Object3D("res/selectball.tiny3d", NO_TEXTURE);
	m_centerDraw->wireframe = true;
	
	m_bDragPos = m_bDragRot = m_bPanScreen = false;
	m_bConstrainX = m_bConstrainY = false;
	m_bShowArcs = false;
		
	m_CurSelectedArc = m_lArcs.end();
	
	//m_text = new Text("res/font.xml");
	m_fCurrentFrameTime = 0.0f;
	m_bIsPlaying = false;
	
	m_hud = new HUD("hud");
	m_hud->create("res/hud.xml");
}

CutsceneEngine::~CutsceneEngine()
{
	errlog << "~CutsceneEngine" << endl;
	//save("res/autosave.cutscene");	//Save our cutscene //DEBUG: Not for now, while I'm tinkering with stuff
	saveConfig("res/editor.config");
	for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
		delete i->o;
	for(list<arc*>::iterator i = m_lArcs.begin(); i != m_lArcs.end(); i++)
		delete (*i);
	errlog << "delete center draw" << endl;
	delete m_centerDraw;
	errlog << "delete hud" << endl;
	delete m_hud;
	//errlog << "delete text" << endl;
	//delete m_text;
}

float32 CutsceneEngine::mouseScaleFac()
{
	float32 diff = (float32)(getHeight())/(float32)(DEFAULT_HEIGHT);	//Because of the way gluPerspective() calculates aspect ratios...
	return ((-4.0/CameraPos.z) * 180.0 * diff);	//Magic numbers ftw
}

Point CutsceneEngine::getPannedMousePos()
{
	Point cursorpos = getCursorPos();
	cursorpos.x -= CameraPos.x*mouseScaleFac();
	cursorpos.y += CameraPos.y*mouseScaleFac();
	return cursorpos;
}

void CutsceneEngine::frame()
{
    //updateObjects();    //Update the objects in the game
	for(list<arc*>::iterator i = m_lArcs.begin(); i != m_lArcs.end(); i++)
		(*i)->update(1.0/30.0);
		
	if(m_bIsPlaying)
	{
		m_fPlayingTime += KEYFRAME_SIZE;
		changeFrame(m_fPlayingTime);
	}
}

void CutsceneEngine::draw()
{
	//Draw 3D stuff
	glEnable(GL_CULL_FACE);	//Only draw the front faces of 3D objects (faster)
	glEnable(GL_LIGHTING);
	glLoadIdentity();
	glTranslatef(CameraPos.x, CameraPos.y, CameraPos.z);
	glRotatef(90.0f,1.0f,0.0f,0.0f);	//Look down at objects
	drawActors();
	
	//Draw 2D stuff
	glDisable(GL_CULL_FACE);	//Draw both sides of 2D objects (So we can flip images for free)
	glDisable(GL_LIGHTING);
	glLoadIdentity();
	glTranslatef(CameraPos.x, CameraPos.y, CameraPos.z);
	for(list<arc*>::iterator i = m_lArcs.begin(); i != m_lArcs.end(); i++)
	{
		if(m_CurSelectedArc == i)
			(*i)->col.set(selectionPulse.r, selectionPulse.g, selectionPulse.b, selectionPulse.a);
		(*i)->render();
		if(m_CurSelectedArc == i)
			(*i)->col.clear();
	}
		
	//Draw HUD and such
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, MAGIC_ZOOM_NUMBER);
	//Point texPos;
	//texPos.SetZero();
	//ostringstream oss;
	//if(!m_bIsPlaying)
	//	oss << "frame " << (int)(m_fCurrentFrameTime / KEYFRAME_SIZE);
	//else
	//	oss << "frame " << (int)(m_fPlayingTime / KEYFRAME_SIZE);
	//m_text->render(oss.str(), texPos);
	//...
	m_hud->draw(0);
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
	
	//Load our editor defaults
	loadConfig("res/editor.config");
	
	//Load our XML file
	load("res/autosave.cutscene");
}


void CutsceneEngine::hudSignalHandler(string sSignal)
{
}

void CutsceneEngine::handleEvent(SDL_Event event)
{
    m_hud->event(event);    //Let our HUD handle any events it needs to
    switch(event.type)
    {
        //Key pressed
        case SDL_KEYDOWN:
            switch(event.key.keysym.sym)
            {

                case SDL_SCANCODE_ESCAPE:
                    quit();
                    break;
                
                case SDL_SCANCODE_RETURN:
                    if(keyDown(SDL_SCANCODE_LALT) || keyDown(SDL_SCANCODE_RALT))
                    {
                      toggleFullscreen();
                    }
                    break;
					
				case SDL_SCANCODE_S:
					if(keyDown(SDL_SCANCODE_LCTRL) || keyDown(SDL_SCANCODE_RCTRL))
					{
						//TODO Specify file to save, Save As with Ctrl-Shift-S
						save("res/autosave.cutscene");
					}
					break;
					
				case SDL_SCANCODE_DELETE:	//Erase objects
					//TODO: Broken. Why?
					if(m_CurSelectedActor != m_lActors.end())
					{
						delete m_CurSelectedActor->o;
						m_lActors.erase(m_CurSelectedActor);
					}
					m_CurSelectedActor = m_lActors.end();
					m_CurSelectedParent = m_lActors.end();
					break;
					
				case SDL_SCANCODE_X: //Constrain to only move along X axis
					if(m_CurSelectedActor != m_lActors.end() && m_bDragPos)
					{
						if(!m_bConstrainX && !m_bConstrainY)	//Snap Y back to 0 difference
						{
							m_CurSelectedActor->o->pos.y = m_ptOldPos.y;
							m_bConstrainX = true;
						}
					}
					break;
					
				case SDL_SCANCODE_Y: //Constrain to only move along Y axis
					if(m_CurSelectedActor != m_lActors.end() && m_bDragPos)
					{
						if(!m_bConstrainY && !m_bConstrainX)	//Snap X back to 0 difference
						{
							m_CurSelectedActor->o->pos.x = m_ptOldPos.x;
							m_bConstrainY = true;
						}
					}
					break;
				
				case SDL_SCANCODE_P:	//Parent objects to other objects
					if(keyDown(SDL_SCANCODE_LCTRL) || keyDown(SDL_SCANCODE_RCTRL))	
					{
						//Parent
						if(m_CurSelectedParent != m_lActors.end() && m_CurSelectedActor != m_lActors.end() && m_CurSelectedParent != m_CurSelectedActor)
						{
							m_CurSelectedParent->o->addChild(m_CurSelectedActor->o);	//This automatically tests for duplication
						}
					}
					else if(keyDown(SDL_SCANCODE_LALT) || keyDown(SDL_SCANCODE_RALT))
					{
						//Remove parent
						if(m_CurSelectedActor != m_lActors.end())
						{
							m_CurSelectedActor->o->removeParenting();
						}
					}
					else if(keyDown(SDL_SCANCODE_LSHIFT) || keyDown(SDL_SCANCODE_RSHIFT))	//Play animation
					{
						if(m_bIsPlaying)
						{
							changeFrame(m_fCurrentFrameTime);
							m_bIsPlaying = false;
						}
						else
						{
							//Play animation
							m_bIsPlaying = true;
							m_fPlayingTime = m_fCurrentFrameTime;	//Start at current frame
						}
					}
					else	//Set to be parent
					{
						if(m_CurSelectedParent == m_CurSelectedActor)
							m_CurSelectedParent = m_lActors.end();
						else
							m_CurSelectedParent = m_CurSelectedActor;
					}
					break;
				
				//Dealing with arcs
				case SDL_SCANCODE_EQUALS:
					if(m_CurSelectedArc == m_lArcs.end())
						m_CurSelectedArc = m_lArcs.begin();
					else
						m_CurSelectedArc++;
					break;
				
				case SDL_SCANCODE_MINUS:
					if(m_CurSelectedArc == m_lArcs.begin())
						m_CurSelectedArc = m_lArcs.end();
					else
						m_CurSelectedArc--;
					break;
				
				case SDL_SCANCODE_1:
					if(m_CurSelectedArc != m_lArcs.end() && m_CurSelectedActor != m_lActors.end())
					{
						(*m_CurSelectedArc)->obj1 = m_CurSelectedActor->o;
					}
					break;
					
				case SDL_SCANCODE_2:
					if(m_CurSelectedArc != m_lArcs.end() && m_CurSelectedActor != m_lActors.end())
					{
						(*m_CurSelectedArc)->obj2 = m_CurSelectedActor->o;
					}
					break;
				
				case SDL_SCANCODE_A:
					if(m_bShowArcs)
					{
						m_bShowArcs = false;
						for(list<arc*>::iterator i = m_lArcs.begin(); i != m_lArcs.end(); i++)
						{
							Interpolate* inter = new Interpolate(&(*i)->col.a);
							inter->setMinVal(0.0);
							inter->calculateIncrement(0.0f,0.5f);
							addInterpolation(inter);
						}
					}
					else
					{
						m_bShowArcs = true;
						for(list<arc*>::iterator i = m_lArcs.begin(); i != m_lArcs.end(); i++)
						{
							Interpolate* inter = new Interpolate(&(*i)->col.a);
							inter->setMaxVal(1.0);
							inter->calculateIncrement(1.0f,0.5f);
							addInterpolation(inter);
						}
					}
					break;
					
				case SDL_SCANCODE_RIGHT:
					if(m_bIsPlaying)
					{
						m_fCurrentFrameTime = m_fPlayingTime;
						m_bIsPlaying = false;
					}
					m_fCurrentFrameTime += KEYFRAME_SIZE;
					changeFrame(m_fCurrentFrameTime);
					break;
					
				case SDL_SCANCODE_LEFT:
					if(m_bIsPlaying)
					{
						m_fCurrentFrameTime = m_fPlayingTime;
						m_bIsPlaying = false;
					}
					m_fCurrentFrameTime -= KEYFRAME_SIZE;
					if(m_fCurrentFrameTime < 0)
						m_fCurrentFrameTime = 0;
					changeFrame(m_fCurrentFrameTime);
					break;
					
				case SDL_SCANCODE_UP:
					if(m_bIsPlaying)
					{
						m_fCurrentFrameTime = m_fPlayingTime;
						m_bIsPlaying = false;
					}
					m_fCurrentFrameTime += KEYFRAME_SIZE * 10;
					changeFrame(m_fCurrentFrameTime);
					break;
					
				case SDL_SCANCODE_DOWN:
					if(m_bIsPlaying)
					{
						m_fCurrentFrameTime = m_fPlayingTime;
						m_bIsPlaying = false;
					}
					m_fCurrentFrameTime -= KEYFRAME_SIZE * 10;
					if(m_fCurrentFrameTime < 0)
						m_fCurrentFrameTime = 0;
					changeFrame(m_fCurrentFrameTime);
					break;
					
				case SDL_SCANCODE_0:	//Reset camera pos
					CameraPos.x = CameraPos.y = 0;
					CameraPos.z = -4;
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
            if(event.button.button == SDL_BUTTON_LEFT)	//Left mouse button: Drag
            {
				if(keyDown(SDL_SCANCODE_LCTRL) || keyDown(SDL_SCANCODE_RCTRL))	//Ctrl-clicking is equivalent to MMB
				{
					if(!m_bDragPos && !m_bDragRot)
						m_bPanScreen = true;
					else if(m_bDragRot)
					{
						m_bDragRot = false;
						m_CurSelectedActor->o->rot = m_fOldRot;
					}
				}
				else if(m_CurSelectedActor != m_lActors.end())
				{
					if(m_bDragRot)
					{
						m_bDragRot = false;
						m_CurSelectedActor->o->rot = m_fOldRot;
					}
					else	//Start dragging
					{
						m_ptOldPos = m_CurSelectedActor->o->pos;
						m_bDragPos = true;
					}
				}
            }
            else if(event.button.button == SDL_BUTTON_RIGHT)	//Right mouse button: Rotate
            {
				if(m_CurSelectedActor != m_lActors.end())
				{
					if(m_bDragPos)
					{
						m_bDragPos = false;
						m_CurSelectedActor->o->pos = m_ptOldPos;
					}
					else	//Start rotating
					{
						m_fOldRot = m_CurSelectedActor->o->rot;
						m_bDragRot = true;
					}
				}
            }
			else if(event.button.button == SDL_BUTTON_MIDDLE)
			{
				if(!m_bDragPos && !m_bDragRot)
					m_bPanScreen = true;
				else if(m_bDragPos)
				{
					m_bDragPos = false;
					m_CurSelectedActor->o->pos = m_ptOldPos;
				}
				else if(m_bDragRot)
				{
					m_bDragRot = false;
					m_CurSelectedActor->o->rot = m_fOldRot;
				}
			}
            break;
			
		case SDL_MOUSEWHEEL:
			if(event.wheel.y < 0)//== SDL_BUTTON_WHEELUP)
			{
				if(!m_bDragPos && !m_bDragRot && !m_bPanScreen)
					CameraPos.z = min(CameraPos.z + 0.2, 0.0);
			}
			else// if(event.button.button == SDL_BUTTON_WHEELDOWN)
			{
				if(!m_bDragPos && !m_bDragRot && !m_bPanScreen)
					CameraPos.z = max(CameraPos.z - 0.2, -20.0);
			}
			break;

        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT)
            {
				m_bConstrainX = m_bConstrainY = false;
				if(m_bDragPos)
				{
					m_bDragPos = false;
					addFrame(&(*m_CurSelectedActor), &(m_CurSelectedActor->o->pos.x), m_CurSelectedActor->o->pos.x, m_fCurrentFrameTime);
					addFrame(&(*m_CurSelectedActor), &(m_CurSelectedActor->o->pos.y), m_CurSelectedActor->o->pos.y, m_fCurrentFrameTime);
				}
				m_bPanScreen = false;
            }
			else if(event.button.button == SDL_BUTTON_RIGHT)
			{
				if(m_bDragRot)
				{
					m_bDragRot = false;
					addFrame(&(*m_CurSelectedActor), &(m_CurSelectedActor->o->rot), m_CurSelectedActor->o->rot, m_fCurrentFrameTime);
				}
			}
			else if(event.button.button == SDL_BUTTON_MIDDLE)
			{
				m_bPanScreen = false;
			}
            break;

        case SDL_MOUSEMOTION:
            if(m_bDragPos && m_CurSelectedActor != m_lActors.end())
            {
				if(!m_bConstrainY)
					m_CurSelectedActor->o->pos.x += event.motion.xrel/(mouseScaleFac());
				if(!m_bConstrainX)
					m_CurSelectedActor->o->pos.y += event.motion.yrel/(mouseScaleFac());
            }
            else if(m_bDragRot && m_CurSelectedActor != m_lActors.end())
            {
				Point center = m_CurSelectedActor->o->getPos();
				center *= mouseScaleFac();
				center.x += getWidth()/2.0;
				center.y += getHeight()/2.0;
				Point oldPos;
				oldPos.x = getPannedMousePos().x - event.motion.xrel;
				oldPos.y = getPannedMousePos().y - event.motion.yrel;
				Point newPos;
				newPos.x = getPannedMousePos().x;
				newPos.y = getPannedMousePos().y;
				float32 startAngle, endAngle;
				startAngle = atan2((oldPos.y - center.y),(oldPos.x - center.x));
				endAngle = atan2((newPos.y - center.y),(newPos.x - center.x));
				m_CurSelectedActor->o->rot += startAngle - endAngle;
            }
			else if(m_bPanScreen)
			{
				CameraPos.x += event.motion.xrel/(mouseScaleFac());
				CameraPos.y -= event.motion.yrel/(mouseScaleFac());
			}
			else //No button down; choose object to select
			{
				Vec3 pos;
				pos.x = getPannedMousePos().x - getWidth()/2.0;
				pos.y = getPannedMousePos().y - getHeight()/2.0;
				pos.z = 0;
				
				pos.x /= mouseScaleFac();
				pos.y /= mouseScaleFac();
				m_CurSelectedActor = findClosestObject(pos);
			}
            break;
	}
		
}

void CutsceneEngine::loadActors(string sFolderPath)
{
	ttvfs::StringList lFiles;
	ttvfs::GetFileList(sFolderPath.c_str(), lFiles);
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
			string sObj = sFolderPath + *i;	//Append folder path to file to load
			sPNG = sFolderPath + sPNG;
			Object3D* o3d = new Object3D(sObj, sPNG);
			obj* o = new obj();
			physSegment* seg = new physSegment();
			seg->obj3D = o3d;
			o->addSegment(seg);
			keyobj ko;
			ko.o = o;
			m_lActors.push_back(ko);
		}
	}
}

void CutsceneEngine::drawActors()
{
	glEnable(GL_LIGHTING);
	for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		if(i == m_CurSelectedParent)
			i->o->col.set(parentPulse.r, parentPulse.g, parentPulse.b, parentPulse.a);
		else if(i == m_CurSelectedActor)	//Selected actor pulses red
			i->o->col.set(selectionPulse.r, selectionPulse.g, selectionPulse.b, selectionPulse.a);
		i->o->draw();
		//Draw center
		glPushMatrix();
		if(i == m_CurSelectedParent)
			glColor4f(0.0,0.0,1.0,1.0);	//Center of parent is drawn blue
		else if(i == m_CurSelectedActor)
			glColor4f(1.0,0.0,0.0,1.0);	//Center of current actor is drawn red
		else
			glColor4f(0.0,1.0,0.0,1.0);
		glTranslatef(i->o->getPos().x, 0.0f, i->o->getPos().y);
		glRotatef(i->o->getRot()*RAD2DEG, 0.0f, 1.0f, 0.0f);
		glScalef(0.07f, 0.07f, 0.07f);
		glDisable(GL_LIGHTING);
		m_centerDraw->render();
		glEnable(GL_LIGHTING);
		glPopMatrix();
		glColor4f(1.0f,1.0f,1.0f,1.0f);
	}
	
	//Loop back through and clear colors, so we aren't dependent on list order for when colors are set or cleared
	for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
		i->o->col.clear();
	
	glDisable(GL_LIGHTING);
}

list<keyobj>::iterator CutsceneEngine::findClosestObject(Vec3 pos)
{
	list<keyobj>::iterator ret = m_lActors.end();
	float32 closestPos = FLT_MAX;
	for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		Vec3 objPos;
		objPos.x = i->o->getPos().x;
		objPos.y = i->o->getPos().y;
		objPos.z = 0.0f;
		
		float32 dist = distanceSquared(objPos, pos);
		if(dist < closestPos)
		{
			closestPos = dist;
			ret = i;
		}
	}
	
	if(closestPos > SELECT_MIN_DISTANCE)	//If no object is close enough, ignore
		return m_lActors.end();
	return ret;
}

void CutsceneEngine::save(string sFilename)
{
	errlog << "Saving cutscene XML " << sFilename << endl;
	XMLDocument* doc = new XMLDocument;
	XMLElement* root = doc->NewElement("cutscene");
	
	//Write arcs (First because they have to point to objects)
	int CurArcLink = 0;
	for(list<arc*>::iterator i = m_lArcs.begin(); i != m_lArcs.end(); i++)
	{
		XMLElement* arc = doc->NewElement("arc");
		arc->SetAttribute("num", (*i)->getNumber());
		arc->SetAttribute("depth", (*i)->depth);
		arc->SetAttribute("col", colorToString((*i)->col).c_str());
		arc->SetAttribute("add", (*i)->add);
		arc->SetAttribute("max", (*i)->max);
		arc->SetAttribute("height", (*i)->height);
		arc->SetAttribute("avg", (*i)->avg);
		arc->SetAttribute("img", (*i)->getImageFilename().c_str());
		if((*i)->obj1 != NULL)
		{
			if((*i)->obj1->usr == NULL)	//If this has a name already, use that
			{
				(*i)->obj1->usr = malloc(64);
				sprintf((char*)((*i)->obj1->usr), "arc%d", ++CurArcLink);
			}
			arc->SetAttribute("obj1", (const char*)((*i)->obj1->usr));
		}
		if((*i)->obj2 != NULL)
		{
			if((*i)->obj2->usr == NULL)	//If this has a name already, use that
			{
				(*i)->obj2->usr = malloc(64);
				sprintf((char*)((*i)->obj2->usr), "arc%d", ++CurArcLink);
			}
			arc->SetAttribute("obj2", (const char*)((*i)->obj2->usr));
		}
		root->InsertEndChild(arc);
	}
	
	//Write actors
	for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		if(i->o->parent == NULL)	//Only write toplevel if not a child of another actor
			writeObject(i->o, root, doc);
	}
	
	//Clear leftover user data from objects
	for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		if(i->o->usr != NULL)
			free(i->o->usr);
		i->o->usr = NULL;
	}
	
	doc->InsertFirstChild(root);
	doc->SaveFile(sFilename.c_str());
	delete doc;
}

void CutsceneEngine::load(string sFilename)
{
	errlog << "Loading cutscene XML " << sFilename << endl;
	XMLDocument* doc = new XMLDocument;
	int iErr = doc->LoadFile(sFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing XML file " << sFilename << ": Error " << iErr << endl;
		delete doc;
		return;
	}
	
	//Grab root element
	XMLElement* root = doc->RootElement();
	if(root == NULL)
	{
		errlog << "Error: Root element NULL in XML file " << sFilename << endl;
		delete doc;
		return;
	}
	
	//Load actors
	for(XMLElement* actor = root->FirstChildElement("actor"); actor != NULL; actor = actor->NextSiblingElement("actor"))
	{
		obj* object = new obj();
		readObject(object, actor);
		keyobj ko;
		ko.o = object;
		m_lActors.push_back(ko);
	}
	
	//Load arcs (After we read in actors, so we know what objects these arcs are between)
	for(XMLElement* arcelem = root->FirstChildElement("arc"); arcelem != NULL; arcelem = arcelem->NextSiblingElement("arc"))
	{
		uint32_t number = 0;
		
		if(arcelem->QueryUnsignedAttribute("num", &number) != XML_NO_ERROR)
		{
			errlog << "Arc has no num" << endl;
			continue;	//Ignore
		}
		if(number < 2)
		{
			errlog << "Arc num < 2" << endl;
			continue;	//Ignore if ludicrously low resolution
		}
		const char* cImg = arcelem->Attribute("img");
		if(cImg == NULL)
		{
			errlog << "Arc img null" << endl;
			continue;	//Ignore
		}
		
		arc* newarc = new arc(number, getImage(cImg));	//Create new arc from this filename and resolution
		
		//Get rest of arc info from XML, ignoring errors
		arcelem->QueryFloatAttribute("depth", &newarc->depth);
		arcelem->QueryFloatAttribute("add", &newarc->add);
		arcelem->QueryFloatAttribute("max", &newarc->max);
		arcelem->QueryFloatAttribute("height", &newarc->height);
		arcelem->QueryUnsignedAttribute("avg", &newarc->avg);
		const char* cCol = arcelem->Attribute("col");
		if(cCol != NULL)
			newarc->col = colorFromString(cCol);
		
		const char* cObj1 = arcelem->Attribute("obj1");
		const char* cObj2 = arcelem->Attribute("obj2");
		//Link up objects to arc elements if we can
		if(cObj1 != NULL)	//See if we can find object with this name
		{
			string sObj1 = cObj1;
			for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)	//O(n), I know
			{
				if(i->o->usr != NULL)
				{
					string s = (char*)(i->o->usr);
					if(s == sObj1)
					{
						newarc->obj1 = i->o;
						break;
					}
				}
			}
		}
		if(cObj2 != NULL)
		{
			string sObj2 = cObj2;
			for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)	//O(n), I know
			{
				if(i->o->usr != NULL)
				{
					string s = (char*)(i->o->usr);
					if(s == sObj2)
					{
						newarc->obj2 = i->o;
						break;
					}
				}
			}
		}
		
		m_lArcs.push_back(newarc);
	}
	
	//Clear user data from objects
	for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
		i->o->usr = NULL;
	
	delete doc;
}

void writeVec2(XMLElement* elem, string sAttributeName, Point vec)
{
	ostringstream oss;
	oss << vec.x << ", " << vec.y;
	elem->SetAttribute(sAttributeName.c_str(), oss.str().c_str());
}

void readVec2(XMLElement* elem, string sAttributeName, Point* vec)
{
	const char* cPos = elem->Attribute(sAttributeName.c_str());
	if(cPos == NULL) return;
	*vec = pointFromString(cPos);
}

void CutsceneEngine::writeObject(obj* object, XMLElement* parent, XMLDocument* doc)
{
	XMLElement* actor = doc->NewElement("actor");
	if(object->usr != NULL)
		actor->SetAttribute("id", (const char*)(object->usr));
	writeVec2(actor, "pos", object->pos);
	actor->SetAttribute("rot", object->rot);
	
	//Write physics segments
	for(list<physSegment*>::iterator i = object->segments.begin(); i != object->segments.end(); i++)
	{
		XMLElement* segment = doc->NewElement("segment");
		writeVec2(segment, "offset", (*i)->pos);
		segment->SetAttribute("rot", (*i)->rot);
		
		if((*i)->body != NULL)
			;	//TODO: Box2D body stuff
		if((*i)->img != NULL)
			;	//TODO: Write image stuff
		if((*i)->obj3D != NULL)
		{
			//Write object3D stuff
			XMLElement* o3d = doc->NewElement("mesh");
			o3d->SetAttribute("mesh", (*i)->obj3D->getObjFilename().c_str());
			o3d->SetAttribute("texture", (*i)->obj3D->getTexFilename().c_str());
			if(!((*i)->obj3D->shaded))
				o3d->SetAttribute("shaded", false);
			segment->InsertEndChild(o3d);
		}
		
		actor->InsertEndChild(segment);
	}
	
	//Write children
	for(list<obj*>::iterator i = object->children.begin(); i != object->children.end(); i++)
	{
		writeObject((*i), actor, doc);	//Recursive call
	}
		
	parent->InsertEndChild(actor);
}

void CutsceneEngine::readObject(obj* object, XMLElement* actor)
{
	readVec2(actor, "pos", &(object->pos));
	actor->QueryFloatAttribute("rot", &(object->rot));
	
	//Read user data
	object->usr = (void*)actor->Attribute("id");
	
	//Read physics segments
	for(XMLElement* segment = actor->FirstChildElement("segment"); segment != NULL; segment = segment->NextSiblingElement("segment"))
	{
		physSegment* ps = new physSegment();
		readVec2(segment, "offset", &(ps->pos));
		segment->QueryFloatAttribute("rot", &(ps->rot));
		
		//TODO: Read Box2D and image stuff
		
		//Read Object3D stuff
		XMLElement* o3d = segment->FirstChildElement("mesh");
		if(o3d != NULL)
		{
			string sMesh, sTex;
			const char* cdata = o3d->Attribute("mesh");
			if(cdata != NULL) sMesh = cdata;
			cdata = o3d->Attribute("texture");
			if(cdata != NULL) sTex = cdata;
			
			Object3D* o = new Object3D(sMesh, sTex);
			ps->obj3D = o;
			
			//Don't care if this works or not...
			o3d->QueryBoolAttribute("shaded", &o->shaded);
		}
		
		object->addSegment(ps);
	}
	
	//Read in child actors
	for(XMLElement* actor2 = actor->FirstChildElement("actor"); actor2 != NULL; actor2 = actor2->NextSiblingElement("actor"))
	{
		obj* object2 = new obj();
		readObject(object2, actor2);	//Recursive call for child objects
		keyobj ko;
		ko.o = object2;
		m_lActors.push_back(ko);
		object->addChild(object2, false);
	}
}

void CutsceneEngine::changeFrame(float32 fTime)
{
	for(list<keyobj>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		for(list<itemkeys*>::iterator j = i->items.begin(); j != i->items.end(); j++)
		{
			//Find the keyframes we're currently between or on
			list<keyframe*>::iterator prev = (*j)->keyframes.end();
			list<keyframe*>::iterator next = (*j)->keyframes.end();
			for(list<keyframe*>::iterator k = (*j)->keyframes.begin(); k != (*j)->keyframes.end(); k++)
			{
				list<keyframe*>::iterator nextit = k;
				nextit++;
				if(nextit == (*j)->keyframes.end())
				{
					prev = k;
					break;	//Done; hit end of list
				}
				if((*nextit)->time >= fTime)	//if the second keyframe is past where we are
				{
					prev = k;
					if((*prev)->time >= fTime)	//If the first keyframe is also past where we are, this is the beginning of the list
					{
						next = k;
						prev = (*j)->keyframes.end();
						break;
					}
					prev = k;
					next = nextit;
					break;
				}
			}
			if(prev == (*j)->keyframes.end() && next == (*j)->keyframes.end())
			{
				continue;	//No keyframes here; no interpolation to be done
			}
			else if(prev == (*j)->keyframes.end())	//Before first keyframe
			{
				*((*j)->item) = (*next)->value;
			}
			else if(next == (*j)->keyframes.end())	//After last keyframe
			{
				*((*j)->item) = (*prev)->value;
			}
			else	//Between two keyframes
			{
				//Linear interpolate: Grab slope of interpolation
				float32 fTimeDiff = (*next)->time - (*prev)->time;
				float32 fValDiff = (*next)->value - (*prev)->value;
				//Multiply slope by time offset
				*((*j)->item) = (fValDiff/fTimeDiff)*(fTime - (*prev)->time) + (*prev)->value;
			}
		}
	}
}

void CutsceneEngine::addFrame(keyobj* o, float32* pointer, float32 value, float32 fTime)
{
	list<itemkeys*>::iterator key = o->items.end();
	for(key = o->items.begin(); key != o->items.end(); key++)
	{
		if((*key)->item == pointer)	//If we've keyed this already
			break;
	}

	if(key == o->items.end())
	{
		//We didn't find a keyframe list for this item; add it
		itemkeys* ik = new itemkeys;
		ik->item = pointer;
		o->items.push_back(ik);	//IKR? HAHAHAHAHAHAHAHAHAHAHHA
		key = o->items.end();
		key--;
	}
	
	//Insert this keyframe into the right spot
	keyframe* kf = new keyframe;
	kf->value = value;
	kf->time = fTime;
	
	for(list<keyframe*>::iterator i = (*key)->keyframes.begin(); i != (*key)->keyframes.end(); i++)
	{
		if((*i)->time > kf->time)	//Went past last keyframe
		{
			(*key)->keyframes.insert(i, kf);
			return;
		}
		else if((*i)->time == kf->time)	//Already a keyframe here; overwrite
		{
			(*i)->value = kf->value;
			return;
		}
	}
	
	//This list has no keyframes; add this one
	(*key)->keyframes.push_back(kf);
	
}

void CutsceneEngine::loadConfig(string sFilename)
{
	//Open file
	XMLDocument* doc = new XMLDocument;
	int iErr = doc->LoadFile(sFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		cout << "Error parsing editor file " << sFilename << ": Error " << iErr << ". Ignoring..." << endl;
		delete doc;
		return;	//No file; ignore
	}
	
	//Grab root element
	XMLElement* root = doc->RootElement();
	if(root == NULL)
	{
		cout << "Error: Root element NULL in XML file " << sFilename << endl;
		delete doc;
		return;
	}
	
	XMLElement* window = root->FirstChildElement("window");
	if(window != NULL)
	{
		bool bFullscreen = isFullscreen();
		bool bMaximized = isMaximized();
		uint32_t width = getWidth();
		uint32_t height = getHeight();
		
		window->QueryUnsignedAttribute("width", &width);
		window->QueryUnsignedAttribute("height", &height);
		window->QueryBoolAttribute("fullscreen", &bFullscreen);
		window->QueryBoolAttribute("maximized", &bMaximized);
		
		changeScreenResolution(width, height);
		setFullscreen(bFullscreen);
		if(bMaximized && !isMaximized() && !bFullscreen)
			maximizeWindow();
	}
	
	XMLElement* camera = root->FirstChildElement("camera");
	if(camera != NULL)
	{
		const char* cCameraPos = camera->Attribute("pos");
		if(cCameraPos != NULL)
		{
			CameraPos = vec3FromString(cCameraPos);
			if(CameraPos.z == 0)
				CameraPos.z = -4;
		}
	}
	
	delete doc;
}

void CutsceneEngine::saveConfig(string sFilename)
{
	errlog << "Saving editor config XML " << sFilename << endl;
	XMLDocument* doc = new XMLDocument;
	XMLElement* root = doc->NewElement("config");
	
	XMLElement* window = doc->NewElement("window");
	window->SetAttribute("width", getWidth());
	window->SetAttribute("height", getHeight());
	window->SetAttribute("fullscreen", isFullscreen());
	window->SetAttribute("maximized", isMaximized());
	//window->SetAttribute("", );
	root->InsertEndChild(window);
	
	XMLElement* camera = doc->NewElement("camera");
	camera->SetAttribute("pos", vec3ToString(CameraPos).c_str());
	root->InsertEndChild(camera);
	
	doc->InsertFirstChild(root);
	doc->SaveFile(sFilename.c_str());
	delete doc;
}





















