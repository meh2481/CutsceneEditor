/*
    CutsceneEditor source - arc.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "arc.h"

arc::arc(uint32_t number, Image* img)
{
	segmentPos = NULL;
	if(img == NULL || number == 0) return;
	numSegments = number;
	segmentPos = (float*)malloc(sizeof(float)*numSegments);
	arcSegImg = img;
	obj1 = obj2 = NULL;
	add = max = 0.0f;
	avg = 2;
	height = 0.1;
	init();
}

arc::~arc()
{
	free(segmentPos);
}

void arc::render()
{
	Point p1, p2;
	if(obj1 == NULL)
		p1.x = p1.y = 1;
	else
		p1 = obj1->getPos();
	if(obj2 == NULL)
		p2.x = p2.y = -1;
	else
		p2 = obj2->getPos();
	glColor4f(col.r,col.g,col.b,col.a);
	//Offset according to depth
	glPushMatrix();
	glTranslatef(p1.x, -p1.y - height / 2.0, depth);
	
	//TODO: Calculate angle between two points and offset accordingly
	float32 fDistance = sqrt((p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y));	//Grahh slow
	float32 fAngle = -atan2((p2.y-p1.y),(p2.x-p1.x));
	glRotatef(RAD2DEG*fAngle,0.0f,0.0f,1.0f);
	float32 fSegWidth = fDistance / (float32)(numSegments-1);
    for(int i = 0; i < numSegments-1; i++)
    {
      Point ul, ur, bl, br;
      ul.x = bl.x = (float32)i*fSegWidth;
      ur.x = br.x = bl.x + fSegWidth;
      ul.y = segmentPos[i];
      bl.y = ul.y + height;
      ur.y = segmentPos[i+1];
      br.y = ur.y + height;
      arcSegImg->draw4V(ul, ur, bl, br);
    }
	
	glPopMatrix();
	glColor4f(1.0,1.0,1.0,1.0);
}

void arc::update(float dt)
{
	dt *= 60.0;
	for(int i = 1; i < numSegments-1; i++)
	{
		segmentPos[i] += dt*randFloat(-add, add);
		if(segmentPos[i] > max)
			segmentPos[i] = max;
		if(segmentPos[i] < -max)
			segmentPos[i] = -max;
	}
	average();
}

void arc::init()
{
	//Initialize values of array to sane defaults, so we don't start with a flat arc for one frame
	for(int i = 0; i < numSegments; i++)
		segmentPos[i] += randFloat(-max, max);
	average();
}

void arc::average()
{
	float* temp = (float*) malloc(sizeof(float)*numSegments);
	
	//Copy our array values into temporary storage to work off of
	memcpy(temp, segmentPos, sizeof(float)*numSegments);
	
	//Center two end values
	temp[0] = temp[numSegments-1] = 0.0f;
	
	//Loop through, averaging values of all but two end values
	for(int i = 1; i < numSegments-1; i++)
    {
      float fTot = 0.0;
      for(int j = i-avg; j < i+avg+1; j++)
      {
		if(j > 0 && j < numSegments)
			fTot += segmentPos[j];
	  }
      temp[i] = fTot / (float32)(avg*2+1);
    }
	
	//Copy back over
	memcpy(segmentPos, temp, sizeof(float)*numSegments);
	free(temp);
}









