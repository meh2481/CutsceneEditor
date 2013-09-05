/*
    CutsceneEditor source - arc.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "arc.h"

arc::arc(uint8_t number, Image* img)
{
	segmentPos = NULL;
	if(img == NULL || number == 0) return;
	numSegments = number;
	segmentPos = (float*)malloc(sizeof(float)*numSegments);
	arcSegImg = img;
	p1.x = p1.y = p2.x = p2.y = 0.0f;
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
	//Offset according to depth
	glPushMatrix();
	glTranslatef(p1.x, -p1.y - height / 2.0, depth);
	
	//TODO: Calculate angle between two points and offset accordingly
	float32 fDistance = sqrt((p2.x-p1.x)*(p2.x-p1.x) - (p2.y-p1.y)*(p2.y-p1.y));	//Grahh slow
	float32 fAngle = -atan2((p2.y-p1.y),(p2.x-p1.x));
	glRotatef(RAD2DEG*fAngle,0.0f,0.0f,1.0f);
	//glScalef(1.0f,1.0f,1.0f);
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
	  float fNum = 0.0;
      for(int j = i-avg; j < i+avg+1; j++)
      {
		if(j > 0 && j < numSegments)
		{
			fTot += segmentPos[j];
			fNum++;
		}
	  }
      temp[i] = fTot / fNum;
    }
	
	//Copy back over
	memcpy(segmentPos, temp, sizeof(float)*numSegments);
	free(temp);
}









