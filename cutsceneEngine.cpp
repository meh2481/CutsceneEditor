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

CutsceneEngine::CutsceneEngine(uint16_t iWidth, uint16_t iHeight, string sTitle) : Engine(iWidth, iHeight, sTitle)
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
	
	m_bDragPos = m_bDragRot = false;
	m_bConstrainX = m_bConstrainY = false;
}

CutsceneEngine::~CutsceneEngine()
{
	errlog << "~CutsceneEngine" << endl;
	save("res/editor.cutscene");	//Save our cutscene
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
	
	//Load our XML file
	load("res/editor.cutscene");
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
					
				case SDLK_s:
					if(keyDown(SDLK_LCTRL) || keyDown(SDLK_RCTRL))
					{
						save("res/editor.cutscene");
					}
					break;
					
				case SDLK_DELETE:	//Erase objects
					if(m_CurSelectedActor != m_lActors.end())
					{
						delete (*m_CurSelectedActor);
						m_lActors.erase(m_CurSelectedActor);
					}
					m_CurSelectedActor = m_lActors.end();
					m_CurSelectedParent = m_lActors.end();
					break;
					
				case SDLK_x: //Constrain to only move along X axis
					if(m_CurSelectedActor != m_lActors.end() && m_bDragPos)
					{
						m_bConstrainX = !m_bConstrainX;
						if(m_bConstrainX)	//Snap Y back to 0 difference
						{
							(*m_CurSelectedActor)->pos.y = m_ptOldPos.y;
						}
						else	//Snap Y to where it would have been
						{
							Point cursorpos = getCursorPos();
							(*m_CurSelectedActor)->pos.y = (*m_CurSelectedActor)->getPos().y - (*m_CurSelectedActor)->pos.y + (cursorpos.y - getHeight()/2.0) / 180.0;
						}
					}
					break;
					
				case SDLK_y: //Constrain to only move along Y axis
					if(m_CurSelectedActor != m_lActors.end() && m_bDragPos)
					{
						m_bConstrainY = !m_bConstrainY;
						if(m_bConstrainY)	//Snap X back to 0 difference
						{
							(*m_CurSelectedActor)->pos.x = m_ptOldPos.x;
						}
						else	//Snap X to where it would have been
						{
							Point cursorpos = getCursorPos();
							(*m_CurSelectedActor)->pos.x = (*m_CurSelectedActor)->getPos().x - (*m_CurSelectedActor)->pos.x + (cursorpos.x - getWidth()/2.0) / 180.0;
						}
					}
					break;
				
				case SDLK_p:	//Parent objects to other objects
					if(keyDown(SDLK_LCTRL) || keyDown(SDLK_RCTRL))	
					{
						//Parent
						if(m_CurSelectedParent != m_lActors.end() && m_CurSelectedActor != m_lActors.end() && m_CurSelectedParent != m_CurSelectedActor)
						{
							(*m_CurSelectedParent)->addChild(*m_CurSelectedActor);	//This automatically tests for duplication
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
				if(m_CurSelectedActor != m_lActors.end())
				{
					if(m_bDragRot)
					{
						m_bDragRot = false;
						(*m_CurSelectedActor)->rot = m_fOldRot;
					}
					else	//Start dragging
					{
						m_ptOldPos = (*m_CurSelectedActor)->pos;
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
						(*m_CurSelectedActor)->pos = m_ptOldPos;
					}
					else	//Start rotating
					{
						m_fOldRot = (*m_CurSelectedActor)->rot;
						m_bDragRot = true;
					}
				}
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT)
            {
				m_bConstrainX = m_bConstrainY = false;
				if(m_bDragPos)
					m_bDragPos = false;
            }
			else if(event.button.button == SDL_BUTTON_RIGHT)
			{
				if(m_bDragRot)
					m_bDragRot = false;
			}
            break;

        case SDL_MOUSEMOTION:
            if(m_bDragPos && m_CurSelectedActor != m_lActors.end())
            {
				if(!m_bConstrainY)
					(*m_CurSelectedActor)->pos.x += event.motion.xrel/(180.0);
				if(!m_bConstrainX)
					(*m_CurSelectedActor)->pos.y += event.motion.yrel/(180.0);
            }
            else if(m_bDragRot && m_CurSelectedActor != m_lActors.end())
            {
				(*m_CurSelectedActor)->rot += event.motion.xrel/170.0;
            }
			else //No button down; choose object to select
			{
				Vec3 pos;
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
			m_lActors.push_back(o);
		}
	}
}

void CutsceneEngine::drawActors()
{
	glEnable(GL_LIGHTING);
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		if(i == m_CurSelectedParent)
			(*i)->col.set(parentPulse.r, parentPulse.g, parentPulse.b, parentPulse.a);
		else if(i == m_CurSelectedActor)	//Selected actor pulses red
			(*i)->col.set(selectionPulse.r, selectionPulse.g, selectionPulse.b, selectionPulse.a);
		(*i)->draw();
		//Draw center
		glPushMatrix();
		if(i == m_CurSelectedParent)
			glColor4f(0.0,0.0,1.0,1.0);	//Center of parent is drawn blue
		else if(i == m_CurSelectedActor)
			glColor4f(1.0,0.0,0.0,1.0);	//Center of current actor is drawn red
		else
			glColor4f(0.0,1.0,0.0,1.0);
		glTranslatef((*i)->getPos().x, 0.0f, (*i)->getPos().y);
		glRotatef((*i)->rot*RAD2DEG, 0.0f, 1.0f, 0.0f);
		glScalef(0.07f, 0.07f, 0.07f);
		glDisable(GL_LIGHTING);
		m_centerDraw->render();
		glEnable(GL_LIGHTING);
		glPopMatrix();
		glColor4f(1.0f,1.0f,1.0f,1.0f);
	}
	
	//Loop back through and clear colors, so we aren't dependent on list order for when colors are set or cleared
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
		(*i)->col.clear();
	
	glDisable(GL_LIGHTING);
}

list<obj*>::iterator CutsceneEngine::findClosestObject(Vec3 pos)
{
	list<obj*>::iterator ret = m_lActors.end();
	float32 closestPos = FLT_MAX;
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		Vec3 objPos;
		objPos.x = (*i)->getPos().x;
		objPos.y = (*i)->getPos().y;
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
	
	//Write actors
	for(list<obj*>::iterator i = m_lActors.begin(); i != m_lActors.end(); i++)
	{
		if(!((*i)->bIsChild))	//Only write toplevel if not a child of another actor
			writeObject(*i, root, doc);
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
		m_lActors.push_back(object);
	}
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
	istringstream iss(cPos);
	char cDiscard;
	if(!(iss >> vec->x >> cDiscard >> vec->y))
		vec->x = vec->y = 0.0f;
}

void CutsceneEngine::writeObject(obj* object, XMLElement* parent, XMLDocument* doc)
{
	XMLElement* actor = doc->NewElement("actor");
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
		writeObject(*i, actor, doc);	//Recursive call
	}
		
	parent->InsertEndChild(actor);
}

void CutsceneEngine::readObject(obj* object, XMLElement* actor)
{
	readVec2(actor, "pos", &(object->pos));
	actor->QueryFloatAttribute("rot", &(object->rot));
	
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
		m_lActors.push_back(object2);
		object->addChild(object2, false);
	}
}

