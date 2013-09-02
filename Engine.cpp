/*
    CutsceneEditor source - Engine.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "Engine.h"
ofstream errlog("err.log");


GLfloat LightAmbient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
// Diffuse Light Values
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
// Light Position 
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };


extern int screenDrawWidth;
extern int screenDrawHeight;

bool Engine::_frame()
{
    //Handle input events from SDL
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        handleEvent(event);
        //See if cursor has moved
        if(event.type == SDL_MOUSEMOTION)
        {
            m_ptCursorPos.x = event.motion.x;
            m_ptCursorPos.y = event.motion.y;
        }
        if(event.type == SDL_QUIT)
            return true;
    }

    //Get current key state
    m_iKeystates = SDL_GetKeyState(NULL);

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
    SDL_GL_SwapBuffers();
}

Engine::Engine(uint16_t iWidth, uint16_t iHeight, string sTitle)
{
	m_sTitle = sTitle;
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
    m_bFirstMusic = true;
    m_bQuitting = false;
    srand(SDL_GetTicks());  //Not as random as it could be... narf
    m_iImgScaleFac = 0;
    screenDrawWidth = iWidth;
    screenDrawHeight = iHeight;
    m_fLastCycle = getTime();
}

Engine::~Engine()
{
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

Image* Engine::getImage(string sName)
{
    map<string, Image*>::iterator i = m_mImages.find(sName);
    if(i == m_mImages.end())   //This image isn't here; load it
    {
        Image* img = new Image(m_mImageNames[sName]);   //Create this image
        m_mImages[sName] = img; //Add to the map
        img->_setID(m_mImages.size());   //For now, just numbering 0...n will work for an ID
		//TODO: What is an ID even good for?
        return img;
    }
    return i->second; //Return this image
}

void Engine::createImage(string sPath, string sName)
{
    m_mImageNames[sName] = sPath;
	//Set to new path
	map<string, Image*>::iterator i = m_mImages.find(sName);
    if(i != m_mImages.end())
		m_mImages[sName]->_setFilename(sPath);
}

void Engine::createSound(string sPath, string sName)
{
    m_mSoundNames[sName] = sPath;
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

void Engine::setup_sdl()
{
  const SDL_VideoInfo* video;

  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
  	fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  // Quit SDL properly on exit
  atexit(SDL_Quit);

  // Get the current video information
  video = SDL_GetVideoInfo();
  if(video == NULL)
  {
  	fprintf(stderr, "Couldn't get video information: %s\n", SDL_GetError());
    exit(1);
  }

  // Set the minimum requirements for the OpenGL window
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  //Set icon for window
  SDL_Surface *image;
#ifdef __APPLE__
  image = SDL_LoadBMP("res/icon.bmp");
#else
  image = IMG_Load("res/icon.png");
#endif
  SDL_WM_SetCaption(m_sTitle.c_str(), NULL);
  SDL_WM_SetIcon(image, NULL);
  SDL_FreeSurface(image);

  
  //Get supported screen resolutions (TODO: SDL 2.0)
  m_iNumScreenModes = 0;
  
  // Get available fullscreen/hardware modes 
  m_rcScreenModes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
  
  // Check if there are any modes available 
  if (m_rcScreenModes == (SDL_Rect**)0) 
  {
    errlog << "No SDL screen modes available!" << endl;
    m_iWidth = 800;
    m_iHeight = 600;
  }
   
  // Check if our resolution is restricted 
  if (m_rcScreenModes == (SDL_Rect**)-1) 
  {
    errlog << "All fullscreen resolutions available." << endl;
    m_iWidth = 800;
    m_iHeight = 600;
  }
  else
  {
       // Print valid modes 
       errlog << "Available fullscreen modes:" << endl;
       for (int i = 0; m_rcScreenModes[i]; ++i)
       {
         errlog << "  " << m_rcScreenModes[i]->w << " x " << m_rcScreenModes[i]->h << endl;
         m_iNumScreenModes++;
       }
  }
  
  // Create SDL window
  if(SDL_SetVideoMode(m_iWidth, m_iHeight, video->vfmt->BitsPerPixel, SDL_OPENGL) == 0)
  {
  	fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
    exit(1);
  }
  
  //Hide system cursor for SDL, so we can use our own
  SDL_ShowCursor(0);
  
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
    glEnable(GL_CULL_FACE);	//Only draw the front faces of objects (faster)
	
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

void Engine::changeScreenResolution(float32 w, float32 h)
{
  //TODO: Windoze has stuff for this, Mac/Linux ought to as well
  screenDrawWidth = m_iWidth = w;
  screenDrawHeight = m_iHeight = h;
  const SDL_VideoInfo* video = SDL_GetVideoInfo();
  if(video == NULL)
  {
  	fprintf(stderr, "Couldn't get video information: %s\n", SDL_GetError());
    exit(1);
  }
  // Create SDL window
  int flags = SDL_OPENGL;
  if(m_bFullscreen)
    flags |= SDL_FULLSCREEN;
  if(SDL_SetVideoMode(m_iWidth, m_iHeight, video->vfmt->BitsPerPixel, flags) == 0)
  {
  	fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
    exit(1);
  }
  //Set OpenGL back up
  setup_opengl();
  //Reload images
  reloadImages();
  //Reload 3D models
  reload3DObjects();
}

void Engine::toggleFullscreen()
{
  m_bFullscreen = !m_bFullscreen;
  const SDL_VideoInfo* video = SDL_GetVideoInfo();
  if(video == NULL)
  {
  	fprintf(stderr, "Couldn't get video information: %s\n", SDL_GetError());
    exit(1);
  }
  // Create SDL window
  int flags = SDL_OPENGL;
  if(m_bFullscreen)
    flags |= SDL_FULLSCREEN;
  if(SDL_SetVideoMode(m_iWidth, m_iHeight, video->vfmt->BitsPerPixel, flags) == 0)
  {
  	fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
    exit(1);
  }
  //Set OpenGL back up
  setup_opengl();
  //Reload images
  reloadImages();
  //Reload 3D object memory
  reload3DObjects();
}

list<resolution> Engine::getAvailableResolutions()
{
  list<resolution> lResolutions;
  for (int i = 0; m_rcScreenModes[i]; ++i)
  {
    resolution r;
    r.w = m_rcScreenModes[i]->w;
    r.h = m_rcScreenModes[i]->h;
    lResolutions.push_back(r);
  }
  return lResolutions;  
}

void Engine::setCursorPos(int32_t x, int32_t y)
{
    SDL_WarpMouse(x,y);
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
			cout << "Unsupported mouse code: " << iButtonCode << endl;
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














