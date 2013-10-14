/*
 CutsceneEditor source - globaldefs.cpp
 Copyright (c) 2013 Mark Hutcheson
*/

#include "globaldefs.h"
#include <sstream>


Color::Color()
{
    clear();
}

void Color::from256(int ir, int ig, int ib, int ia)
{
	r = (float32)ir/255.0;
	g = (float32)ig/255.0;
	b = (float32)ib/255.0;
	a = (float32)ia/255.0;
}

string stripCommas(string s)
{
    //Replace all ',' characters with ' '
    for(int i = 0; ; i++)
    {
        size_t iPos = s.find(',', i);
        if(iPos == s.npos)
            break;  //Done

        s.replace(iPos, 1, " ");
    }
    return s;
}

Rect rectFromString(string s)
{
    s = stripCommas(s);

    //Now, parse
    istringstream iss(s);
    Rect rc;
    if(!(iss >> rc.left >> rc.top >> rc.right >> rc.bottom))
		rc.set(0,0,0,0);
    return rc;
}

Point pointFromString(string s)
{
    s = stripCommas(s);

    //Now, parse
    istringstream iss(s);
    Point pt;
    if(!(iss >> pt.x >> pt.y))
		pt.SetZero();
    return pt;
}

Color colorFromString(string s)
{
    s = stripCommas(s);

    //Now, parse
    Color c;
    istringstream iss(s);
    if(!(iss >> c.r >> c.g >> c.b >> c.a))
		c.clear();
    return c;
}

string colorToString(Color c)
{
	ostringstream oss;
	oss << c.r << ", " << c.g << ", " << c.b << ", " << c.a;
	return oss.str();
}

string vec3ToString(Vec3 vec)
{
	ostringstream oss;
	oss << vec.x << ", " << vec.y << ", " << vec.z;
	return oss.str();
}

Vec3 vec3FromString(string s)
{
	s = stripCommas(s);

    Vec3 vec;
    istringstream iss(s);
    if(!(iss >> vec.x >> vec.y >> vec.z))
		vec.setZero();
    return vec;
}

//TODO Use actual random number generator (Mersenne Twister or such)
int32_t randInt(int32_t min, int32_t max)
{
    if(min == max)
        return min;
    int32_t diff = max-min+1;
    return(rand()%diff + min);
}

float32 randFloat(float32 min, float32 max)
{
    if(min == max)
        return min;
    float32 scale = rand() % 1001;
    return((float32)scale/1000.0*(max-min) + min);
}

Vec3::Vec3()
{
	setZero();
}

void Vec3::normalize()
{
    float32 fMag;
    fMag = sqrt(x*x + y*y + z*z);

    x = x / fMag;
    y = y / fMag;
    z = z / fMag;
}

Vec3 Vec3::normalized()
{
    Vec3 ret;
    float32 fMag = sqrt(x*x + y*y + z*z);

    ret.x = x / fMag;
    ret.y = y / fMag;
    ret.z = z / fMag;
    return ret;
}

//Test for inequality between vectors
bool Vec3::operator!=(const Vec3& v)
{
    float32 xdiff = abs(v.x - x);
    float32 ydiff = abs(v.y - y);
    float32 zdiff = abs(v.z - z);

    if(xdiff > DIFF_EPSILON || ydiff > DIFF_EPSILON || zdiff > DIFF_EPSILON)    //I call hacks. But it works.
        return true;
    return false;
}

Vec3 crossProduct(Vec3 vec1, Vec3 vec2)
{
    Vec3 ret;
    ret.x = ((vec1.y * vec2.z) - (vec1.z * vec2.y));
    ret.y = -((vec1.z * vec2.z) - (vec1.z * vec2.x));
    ret.z = ((vec1.x * vec2.y) - (vec1.y * vec2.x));
    return ret;
}

float32 dotProduct(Vec3 vec1, Vec3 vec2)
{
    return (vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z);
}

//Rotate the vector vecToRot around the vector rotVec
Vec3 rotateAroundVector(Vec3 vecToRot, Vec3 rotVec, float32 fAngle)
{
    Vec3 result;
    float32 ux = rotVec.x * vecToRot.x;
    float32 uy = rotVec.x * vecToRot.y;
    float32 uz = rotVec.x * vecToRot.z;
    float32 vx = rotVec.y * vecToRot.x;
    float32 vy = rotVec.y * vecToRot.y;
    float32 vz = rotVec.y * vecToRot.z;
    float32 wx = rotVec.z * vecToRot.x;
    float32 wy = rotVec.z * vecToRot.y;
    float32 wz = rotVec.z * vecToRot.z;
    float32 sa = sin(DEG2RAD*fAngle);
    float32 ca = cos(DEG2RAD*fAngle);
    //Matrix math without actual matrices woo
    result.x = rotVec.x*(ux+vy+wz)+(vecToRot.x*(rotVec.y*rotVec.y+rotVec.z*rotVec.z)-rotVec.x*(vy+wz))*ca+(-wy+vz)*sa;
    result.y = rotVec.y*(ux+vy+wz)+(vecToRot.y*(rotVec.x*rotVec.x+rotVec.z*rotVec.z)-rotVec.y*(ux+wz))*ca+(wx-uz)*sa;
    result.z = rotVec.z*(ux+vy+wz)+(vecToRot.z*(rotVec.x*rotVec.x+rotVec.y*rotVec.y)-rotVec.z*(ux+vy))*ca+(-vx+uy)*sa;
    return result;
}

float32 distanceSquared(Vec3 vec1, Vec3 vec2)
{
	Vec3 diff;
	diff.x = vec1.x - vec2.x;
	diff.y = vec1.y - vec2.y;
	diff.z = vec1.z - vec2.z;
	return (diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);
}

float32 distanceBetween(Vec3 vec1, Vec3 vec2)
{
	return sqrt(distanceSquared(vec1, vec2));
}

#ifdef __APPLE__
//returns the closest power of two value
int power_of_two(int input)
{
	int value = 1;
	while ( value < input ) {
		value <<= 1;
	}
	return value;
}
#endif


