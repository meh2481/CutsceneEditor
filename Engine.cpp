/*
    CutsceneEditor source - Engine.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "Engine.h"
//#include <SDL/SDL_syswm.h>
ofstream errlog("err.log");


GLfloat LightAmbient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
// Diffuse Light Values
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
// Light Position 
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };


extern int screenDrawWidth;
extern int screenDrawHeight;

void PrintEvent(const SDL_Event * event)
{
    if (event->type == SDL_WINDOWEVENT) {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            printf("Window %d shown\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            printf("Window %d hidden\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            printf("Window %d exposed\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            printf("Window %d moved to %d,%d\n",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            printf("Window %d resized to %dx%d\n",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            printf("Window %d minimized\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            printf("Window %d maximized\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            printf("Window %d restored\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            printf("Mouse entered window %d\n",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            printf("Mouse left window %d\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            printf("Window %d gained keyboard focus\n",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            printf("Window %d lost keyboard focus\n",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            printf("Window %d closed\n", event->window.windowID);
            break;
        default:
            printf("Window %d got unknown event %d\n",
                    event->window.windowID, event->window.event);
            break;
        }
    }
}

bool Engine::_frame()
{
    //Handle input events from SDL
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
		//PrintEvent(&event);
        //Update internal cursor position if cursor has moved
        if(event.type == SDL_MOUSEMOTION)
        {
            m_ptCursorPos.x = event.motion.x;
            m_ptCursorPos.y = event.motion.y;
        }
		else if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
		{
			if(m_bResizable)
				changeScreenResolution(event.window.data1, event.window.data2);
			else
				errlog << "Error! Resize event generated, but resizable flag not set." << endl;
		}
        handleEvent(event);
        if(event.type == SDL_QUIT)
            return true;
    }

    //Get current key state
    m_iKeystates = SDL_GetKeyboardState(NULL);

    float32 fCurTime = (float32)SDL_GetTicks()/1000.0;
    if(m_fAccumulatedTime <= fCurTime)
    {
        m_fAccumulatedTime += m_fTargetTime;
        //Box2D wants fixed timestep, so we use target framerate here instead of actual elapsed time
        m_physicsWorld->Step(m_fTargetTime, VELOCITY_ITERATIONS, PHYSICS_ITERATIONS);
        //Use cycle time for everything else
        _interpolations(m_fTargetTime);
        frame();
        _render();
    }

    if(m_fAccumulatedTime + m_fTargetTime * 3.0 < fCurTime)    //We've gotten far too behind; we could have a huge FPS jump if the load lessens
        m_fAccumulatedTime = fCurTime;   //Drop any frames past this

    m_fLastCycle = getTime();
    return m_bQuitting;
}

void Engine::_render()
{
    // Begin rendering by clearing the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Game-specific drawing
    draw();
	
	//Draw cursor at absolute location
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, MAGIC_ZOOM_NUMBER);
	//glClear(GL_DEPTH_BUFFER_BIT); //TODO Draw cursor over everything
	glPopMatrix();
	
    // End rendering and update the screen
     SDL_GL_SwapWindow(m_Window);
}

Engine::Engine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sIcon, bool bResizable)
{
	m_sTitle = sTitle;
	m_sIcon = sIcon;
	m_bResizable = bResizable;
    b2Vec2 gravity(0.0, 9.8);  //Vector for our world's gravity
    m_physicsWorld = new b2World(gravity);
    m_ptCursorPos.SetZero();
    m_physicsWorld->SetAllowSleeping(true);
    m_iWidth = iWidth;
    m_iHeight = iHeight;
    m_iKeystates = NULL;
    m_bShowCursor = true;
    setup_sdl();
    m_bFullscreen = false;  //TODO: Start in fullscreen mode
    setup_opengl();

    //Initialize engine stuff
    setFramerate(60);   //60 fps default
    m_fAccumulatedTime = 0.0;
    //m_bFirstMusic = true;
    m_bQuitting = false;
    srand(SDL_GetTicks());  //Not as random as it could be... narf
    m_iImgScaleFac = 0;
    screenDrawWidth = iWidth;
    screenDrawHeight = iHeight;
    m_fLastCycle = getTime();
}

Engine::~Engine()
{
	SDL_DestroyWindow(m_Window);

    //Clean up our image map
	errlog << "Clearing images" << endl;
    clearImages();
    
    //Clean up interpolations that are currently going on
	errlog << "Clearing interps" << endl;
    clearInterpolations();

    //TODO: Clean up our sound effects

    // Clean up and shutdown
	errlog << "Deleting phys world" << endl;
	delete m_physicsWorld;
	errlog << "Quit SDL" << endl;
	SDL_Quit();
}

void Engine::clearImages()
{
    for(map<string, Image*>::iterator i = m_mImages.begin(); i != m_mImages.end(); i++)
        delete (i->second);    //Delete each image
    m_mImages.clear();
}

void Engine::start()
{
    // Load all that we need to
    init(lCommandLine);
    // Let's rock now!
    while(!_frame());
}

void Engine::fillRect(Point p1, Point p2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    fillRect(p1.x, p1.y, p2.x, p2.y, red, green, blue, alpha);
}

void Engine::fillRect(float32 x1, float32 y1, float32 x2, float32 y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    Color col;
    col.from256(red, green, blue, alpha);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_QUADS);
    glColor4f(col.r,col.g,col.b,col.a);	//Colorize
    //Draw
    glVertex3f((2.0*(float32)m_iWidth/(float32)m_iHeight)*((GLfloat)x1/(GLfloat)m_iWidth-0.5), -2.0*(GLfloat)y1/(GLfloat)m_iHeight + 1.0, 0.0);
    glVertex3f((2.0*(float32)m_iWidth/(float32)m_iHeight)*((GLfloat)x1/(GLfloat)m_iWidth-0.5), -2.0*(GLfloat)y2/(GLfloat)m_iHeight+1.0, 0.0);
    glVertex3f((2.0*(float32)m_iWidth/(float32)m_iHeight)*((GLfloat)x2/(GLfloat)m_iWidth-0.5), -2.0*(GLfloat)y2/(GLfloat)m_iHeight+1.0, 0.0);
    glVertex3f((2.0*(float32)m_iWidth/(float32)m_iHeight)*((GLfloat)x2/(GLfloat)m_iWidth-0.5), -2.0*(GLfloat)y1/(GLfloat)m_iHeight+1.0, 0.0);
    glEnd();
    glColor4f(1.0,1.0,1.0,1.0);	//Back to normal
}

void Engine::fillRect(Rect rc, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    fillRect(rc.left, rc.top, rc.right, rc.bottom, red, green, blue, alpha);
}

Image* Engine::getImage(string sFilename)
{
    map<string, Image*>::iterator i = m_mImages.find(sFilename);
    if(i == m_mImages.end())   //This image isn't here; load it
    {
        Image* img = new Image(sFilename);   //Create this image
        m_mImages[sFilename] = img; //Add to the map
        //img->_setID(m_mImages.size());   //For now, just numbering 0...n will work for an ID
		//TODO: What is an ID even good for?
        return img;
    }
    return i->second; //Return this image
}

void Engine::createSound(string sPath, string sName)
{
    //TODO
}

void Engine::playSound(string sName, int volume, int pan, float32 pitch)
{
    // TODO
}

void Engine::pauseMusic()
{
    // TODO
}

void Engine::playMusic(string sName, int volume, int pan, float32 pitch)
{
	//TODO
}

bool Engine::keyDown(int32_t keyCode)
{
    return(m_iKeystates[keyCode]);
}

void Engine::setFramerate(float32 fFramerate)
{
    if(m_fFramerate == 0.0)
        m_fAccumulatedTime = (float32)SDL_GetTicks()/1000.0;   //If we're stuck at 0fps for a while, this number could be huge, which would cause unlimited fps for a bit
    m_fFramerate = fFramerate;
    if(m_fFramerate == 0.0)
        m_fTargetTime = FLT_MAX;    //Avoid division by 0
    else
        m_fTargetTime = 1.0 / m_fFramerate;
}

#include "opengl-api.h"

void Engine::setup_sdl()
{

	if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
		errlog << "SDL_InitSubSystem Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    errlog << "Loading OpenGL..." << std::endl;

    if (SDL_GL_LoadLibrary(NULL) == -1)
    {
		errlog << "SDL_GL_LoadLibrary Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

  // Quit SDL properly on exit
  atexit(SDL_Quit);
  
  // Create SDL window
  Uint32 flags = SDL_WINDOW_OPENGL;
  if(m_bResizable)
	flags |= SDL_WINDOW_RESIZABLE;
  m_Window = SDL_CreateWindow(m_sTitle.c_str(),
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             m_iWidth, 
							 m_iHeight,
                             flags);

  if(m_Window == NULL)
  {
  	errlog << "Couldn't set video mode: " << SDL_GetError() << endl;
    exit(1);
  }
  SDL_GLContext glcontext = SDL_GL_CreateContext(m_Window);

  
  // Set the minimum requirements for the OpenGL window
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  
  //Hide system cursor for SDL, so we can use our own
  SDL_ShowCursor(0);
  
  OpenGLAPI::LoadSymbols();	//Load our OpenGL symbols to use
  
  _loadicon();	//Load our window icon
  
}

//Set up OpenGL
void Engine::setup_opengl()
{
	// Make the viewport
    glViewport(0, 0, m_iWidth, m_iHeight);

    // set the clear color to grey
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    glEnable(GL_TEXTURE_2D);

    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
  
    //Enable image transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set the camera projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective( 45.0f, (GLfloat)m_iWidth/(GLfloat)m_iHeight, 0.1f, 100.0f );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, MAGIC_ZOOM_NUMBER);
	glPushMatrix();
	
	//Set up lighting
    glShadeModel(GL_SMOOTH);
    glEnable( GL_LIGHT0 );
    glEnable( GL_LIGHT1 );
    glEnable( GL_COLOR_MATERIAL );

    // Setup The Ambient Light
    glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient );

    // Setup The Diffuse Light
    glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse );

    // Position The Light
    glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );

    // Enable Light One
    glEnable( GL_LIGHT1 ); 
    
}

void Engine::_loadicon()	//Load icon into SDL window
{
	errlog << "Load icon " << m_sIcon << endl;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP *dib(0);
	BYTE* bits(0);
	unsigned int width(0), height(0);
	
	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(m_sIcon.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(m_sIcon.c_str());
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
	{
		errlog << "Unknown image type for file " << m_sIcon << endl;
		return;
	}
  
	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, m_sIcon.c_str());
	else
		errlog << "File " << m_sIcon << " doesn't support reading." << endl;
	//if the image failed to load, return failure
	if(!dib)
	{
		errlog << "Error loading image " << m_sIcon.c_str() << endl;
		return;
	}  
	//retrieve the image data
  
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	FreeImage_FlipVertical(dib);
  
	bits = FreeImage_GetBits(dib);	//if this somehow one of these failed (they shouldn't), return failure
	if((bits == 0) || (width == 0) || (height == 0))
	{
		errlog << "Something went terribly horribly wrong with getting image bits; just sit and wait for the singularity" << endl;
		return;
	}
	
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(bits, width, height, FreeImage_GetBPP(dib), FreeImage_GetPitch(dib), 
													0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if(surface == NULL)
		errlog << "NULL surface for icon " << m_sIcon << endl;
	else
	{
		SDL_SetWindowIcon(m_Window, surface);
		SDL_FreeSurface(surface);
	}
  
	FreeImage_Unload(dib);
}

void Engine::changeScreenResolution(float32 w, float32 h)
{
//In Windoze, we can copy the graphics memory to a new context, so we don't have to reload all our images and stuff
#if false//#ifdef _WIN32
	SDL_SysWMinfo info;
 
	//get window handle from SDL
	SDL_VERSION(&info.version);
	if(SDL_GetWindowWMInfo(m_Window, &info) == -1) 
	{
		errlog << "SDL_GetWMInfo #1 failed" << endl;
		return;
	}

	//get device context handle
	HDC tempDC = GetDC(info.info.win.window);

	// create temporary context
	HGLRC tempRC = wglCreateContext(tempDC);
	if(tempRC == NULL) 
	{
		errlog << "wglCreateContext failed" << endl;
		return;
	}
	
	//share resources to temporary context
	SetLastError(0);
	if(!wglShareLists(wglGetCurrentContext(), tempRC))
	{
		errlog << "wglShareLists #1 failed" << endl;
		return;
	}
#endif
	
	screenDrawWidth = m_iWidth = w;
	screenDrawHeight = m_iHeight = h;
	
	//Create SDL window
	int flags = SDL_WINDOW_OPENGL;
	if(m_bFullscreen)
		SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if(m_bResizable)
		flags |= SDL_WINDOW_RESIZABLE;
	
	SDL_SetWindowSize(m_Window, m_iWidth, m_iHeight);

	SDL_GLContext glcontext = SDL_GL_CreateContext(m_Window);
	
	//Set OpenGL back up
	setup_opengl();
	
#if false//#ifdef _WIN32
	//previously used structure may possibly be invalid, to be sure we get it again
	SDL_VERSION(&info.version);
	if(SDL_GetWindowWMInfo(m_Window, &info) == -1) 
	{
		errlog << "SDL_GetWMInfo #2 failed" << endl;
		return;
	}
 
	//share resources to new SDL-created context
	if(!wglShareLists(tempRC, wglGetCurrentContext()))
	{
		errlog << "wglShareLists #2 failed" << endl;
		return;
	}
 
	//we no longer need our temporary context
	if(!wglDeleteContext(tempRC))
	{
		errlog << "wglDeleteContext failed" << endl;
		return;
	}
#else
	//TODO: Linux supposedly does this for us. Does Mac as well?
	//Reload images
	reloadImages();
	//Reload 3D models
	reload3DObjects();
#endif
}

void Engine::toggleFullscreen()
{
  m_bFullscreen = !m_bFullscreen;
  if(m_bFullscreen)
	SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  else
	SDL_SetWindowFullscreen(m_Window, 0);
}

void Engine::setFullscreen(bool bFullscreen)
{
	if(m_bFullscreen == bFullscreen) 
		return;
	toggleFullscreen();
}

Point Engine::getWindowPos()
{
	Point p;
	int x, y;
	SDL_GetWindowPosition(m_Window, &x, &y);
	p.x = x;
	p.y = y;
	return p;
}

void Engine::setWindowPos(Point pos)
{
	SDL_SetWindowPosition(m_Window, pos.x, pos.y);
}

void Engine::maximizeWindow()
{
	SDL_MaximizeWindow(m_Window);
}

list<SDL_DisplayMode> Engine::getAvailableResolutions()
{
	//TODO: SDL 2.0 way
	list<SDL_DisplayMode> lResolutions;
	for(int i = 0; i < SDL_GetNumDisplayModes(0); i++)
	{
		SDL_DisplayMode mode;
		if(SDL_GetDisplayMode(0, i, &mode) == 0)
		{
			lResolutions.push_back(mode);
		}
	}
	return lResolutions;  
}

void Engine::setCursorPos(int32_t x, int32_t y)
{
    SDL_WarpMouseInWindow(m_Window, x, y);
//#ifdef __APPLE__
//    hideCursor(); //TODO: Warping the mouse shows it again in Mac, and this doesn't work. Hermph.
//#endif
}

void Engine::addInterpolation(Interpolate* inter)
{
  m_lInterpolations.push_back(inter);
}

void Engine::_interpolations(float32 dt)
{
  for(list<Interpolate*>::iterator i = m_lInterpolations.begin(); i != m_lInterpolations.end(); i++)
  {
    if((*i)->update(dt))
    {
      list<Interpolate*>::iterator j = i;
      j--;
      delete (*i);
      m_lInterpolations.erase(i);
      i = j;
    }
  }
}

void Engine::clearInterpolations()
{
  for(list<Interpolate*>::iterator i = m_lInterpolations.begin(); i != m_lInterpolations.end(); i++)
  {
    delete (*i);
  }
  m_lInterpolations.clear();
}

bool Engine::getCursorDown(int iButtonCode)
{
	Uint8 ms = SDL_GetMouseState(NULL, NULL);
	switch(iButtonCode)
	{
		case LMB:
			return(ms & SDL_BUTTON(SDL_BUTTON_LEFT));
		case RMB:
			return(ms & SDL_BUTTON(SDL_BUTTON_RIGHT));
		case MMB:
			return(ms & SDL_BUTTON(SDL_BUTTON_MIDDLE));
		default:
			errlog << "Unsupported mouse code: " << iButtonCode << endl;
			break;
	}
	return false;
}

void Engine::commandline(int argc, char** argv)
{
	//Step through intelligently
	for(int i = 1; i < argc; i++)
	{
		commandlineArg cla;
		string sSwitch = argv[i];
		if(sSwitch.find('-') == 0)
		{
			if(sSwitch.find("--") == 0)
				sSwitch.erase(0,1);
			sSwitch.erase(0,1);
			
			cla.sSwitch = sSwitch;
			if(i+1 < argc)	//Switch with a value
			{
				i++;
				cla.sValue = argv[i];
			}
			
		}
		else	//No switch for this value
			cla.sValue = sSwitch;
		lCommandLine.push_back(cla);
	}
}














