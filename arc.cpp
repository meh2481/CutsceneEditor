/*
    CutsceneEditor source - arc.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "arc.h"

arc::arc(uint8_t number, Image* img)
{
	pos = NULL;
	if(img == NULL || number == 0) return;
	num = number;
	pos = (float*)malloc(sizeof(float)*num);
	arcSegImg = img;
	p1.x = p1.y = p2.x = p2.y = 0.0f;
	add = 15.0;
	max = 50.0;
	avg = 2;
	init();
}

arc::~arc()
{
	free(pos);
}

void arc::render()
{
	//Calculate angle between two points and offset accordingly
}

void arc::update(float dt)
{
	dt *= 60.0;
	for(int i = 1; i < num-1; i++)
	{
		pos[i] += dt*randFloat(-add, add);
		if(pos[i] > max)
			pos[i] = max;
		if(pos[i] < -max)
			pos[i] = -max;
	}
	average();
}

void arc::init()
{
	//Initialize values of array to sane defaults, so we don't start with a flat arc for one frame
	for(int i = 0; i < num; i++)
		pos[i] += randFloat(-max, max);
	average();
}

void arc::average()
{
	float* temp = (float*) malloc(sizeof(float)*num);
	
	//Copy our array values into temporary storage to work off of
	memcpy(temp, pos, sizeof(float)*num);
	
	//Center two end values
	temp[0] = temp[num-1] = 0.0f;
	
	//Loop through, averaging values of all but two end values
	for(int i = 1; i < num-1; i++)
    {
      float fTot = 0.0;
	  float fNum = 0.0;
      for(int j = i-avg; j < i+avg+1; j++)
      {
		if(j > 0 && j < num)
		{
			fTot += pos[j];
			fNum++;
		}
	  }
      temp[i] = fTot / fNum;
    }
	
	//Copy back over
	memcpy(pos, temp, sizeof(float)*num);
	free(temp);
}









