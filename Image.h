/*
 CutsceneEditor source - Image.h
 Copyright (c) 2013 Mark Hutcheson
*/
#ifndef IMAGE_H
#define IMAGE_H

#include "globaldefs.h"

//TODO: Figure out what causes this and calculate mathematically? Or just live with it?
#define MAGIC_ZOOM_NUMBER -2.414213562373095

class Image
{
private:
    Image(){};  //Default constructor is uncallable

    GLuint   	m_hTex;
    string     	m_sFilename;
    uint32_t 	m_iWidth, m_iHeight;
//#ifdef __APPLE__
#if 1
    uint32_t 	m_iRealWidth, m_iRealHeight;
#endif
    uint32_t 	m_iID;  //TODO: Remove/isolate/optimize

    void _load(string sFilename);

public:    
    //Constructor/destructor
    Image(string sFilename);
    ~Image();
    
	//Engine use functions
    void _reload();  //Reload memory associated with this image
	void _setFilename(string s) {m_sFilename = s;};
    uint32_t _getID()    {return m_iID;};    //For engine use
    void _setID(uint32_t id) {m_iID = id;};

    //Accessor methods
    uint32_t getWidth()     {return m_iWidth;};
    uint32_t getHeight()    {return m_iHeight;};
    string getFilename()    {return m_sFilename;};

    //public methods
    void draw(Rect rcDrawPos);
    void draw(Rect rcDrawPos, Rect rcImgPos);
    void draw(float32 x, float32 y);    //draw the entire image with upper left corner at x,y
    void draw(Point pt);                //draw the entire image with upper left corner at pt
    void draw(float32 x, float32 y, Rect rcImgPos); //draw part of the image at x, y
    void draw(Point pt, Rect rcImgPos); //draw part of the image at pt
    void draw4V(Point ul, Point ur, Point bl, Point br);  //render the image at arbitrary quad
    void drawCentered(float32 x, float32 y, float32 rotation = 0.0, float32 stretchFactorx = 1.0, float32 stretchFactory = 1.0);    //Center the entire image centered at x,y
    void drawCentered(Point pt, float32 rotation = 0.0, float32 stretchFactorx = 1.0, float32 stretchFactory = 1.0);    //Center the entire image centered at pt
    void drawCentered(float32 x, float32 y, Rect rcImgPos, float32 rotation = 0.0, float32 stretchFactorx = 1.0, float32 stretchFactory = 1.0);    //Center part of the image at x,y
    void drawCentered(Point pt, Rect rcImgPos, float32 rotation = 0.0, float32 stretchFactorx = 1.0, float32 stretchFactory = 1.0);    //Center part of the image at pt
};

//Image reloading handler functions
void reloadImages();
void _addImgReload(Image* img);
void _removeImgReload(Image* img);

//Misc image functions
void setImageBlurred();		//Draw images blurred
void setImagePixellated();	//Draw images pixellated




#endif



