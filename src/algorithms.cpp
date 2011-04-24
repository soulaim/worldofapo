#include "algorithms.h"

#include <algorithm>
#include <cmath>

using namespace std;

FixedPoint pointInLinesegment(const Location& a, const Location& b, const Location& p)
{
	FixedPoint segmentLength = (b - a).dotProduct(b - a);
	return (a-p).dotProduct(b-p) / segmentLength;
}

void interpolate(const Location& A, const Location& B, const Location& C, Location& p)
{
	Location direction1 = B - A;
	Location direction2 = C - A;

	// A + t*(direction1) + u*(direction2) = p;
	//
	// or in other words:
	//
	// t*(direction11.x) + u*(direction2.x) = p.x - A.x
	// t*(direction11.z) + u*(direction2.z) = p.z - A.z
	//
	// Solution for t = t_top/t_bot and u = u_top / u_bot is:
	FixedPoint t_top = direction2.x * (p.z - A.z) - direction2.z * (p.x - A.x);
	FixedPoint u_top = direction1.x * (p.z - A.z) - direction1.z * (p.x - A.x);
	FixedPoint t_bot = direction2.x * direction1.z - direction1.x * direction2.z;
	FixedPoint u_bot = direction1.x * direction2.z - direction2.x * direction1.z;

	p = A + direction1 * t_top / t_bot + direction2 * u_top / u_bot;
}

int light_scissor(const vec3<float>& lightpos, float r, int screen_width, int screen_height, std::array<int,4>& rect )
{
	rect[0] = 0;
	rect[1] = 0;
	rect[2] = screen_width;
	rect[3] = screen_height;

	float r2 = r*r;

	vec3<float> l2 = vec3<float>(lightpos.x*lightpos.x, lightpos.y*lightpos.y, lightpos.z*lightpos.z);

	float aspect_ratio = screen_width / screen_height;
	
	float e1 = 1.2f;
	float e2 = 1.2f*aspect_ratio;

	// Calculate left and right plane.
	float d1 = r2*l2.x - (l2.x+l2.z)*(r2-l2.z);
	if(d1 >= 0)
	{
		d1 = sqrt(d1);

		float nx1 = (r*lightpos.x + d1)/(l2.x+l2.z);
		float nx2 = (r*lightpos.x - d1)/(l2.x+l2.z);

		float nz1 = (r-nx1*lightpos.x) / lightpos.z;
		float nz2 = (r-nx2*lightpos.x) / lightpos.z;

		float pz1 = (l2.x+l2.z-r2) / (lightpos.z-(nz1/nx1)*lightpos.x);
		float pz2 = (l2.x+l2.z-r2) / (lightpos.z-(nz2/nx2)*lightpos.x);

		if(pz1 < 0)
		{
			float fx = nz1*e1 / nx1;
			int ix = (int)((fx+1.0f)*screen_width*0.5f);

			float px = -pz1*nz1 / nx1;
			if(px < lightpos.x)
				rect[0] = max(rect[0],ix);
			else
				rect[2] = min(rect[2],ix);
		}

		if(pz2 < 0)
		{
			float fx = nz2*e1/nx2;
			int ix = (int)((fx+1.0f)*screen_width*0.5f);

			float px = -pz2*nz2 / nx2;
			if(px < lightpos.x)
				rect[0] = max(rect[0],ix);
			else
				rect[2] = min(rect[2],ix);
		}
	}

	// Calculate upper and lower plane.
	float d2 = r2*l2.y - (l2.y+l2.z)*(r2-l2.z);
	if(d2 >= 0)
	{
		d2 = sqrt(d2);

		float ny1 = (r*lightpos.y + d2) / (l2.y+l2.z);
		float ny2 = (r*lightpos.y - d2) / (l2.y+l2.z);

		float nz1 = (r-ny1*lightpos.y) / lightpos.z;
		float nz2 = (r-ny2*lightpos.y) / lightpos.z;

		float pz1 = (l2.y+l2.z-r2) / (lightpos.z-(nz1/ny1)*lightpos.y);
		float pz2 = (l2.y+l2.z-r2) / (lightpos.z-(nz2/ny2)*lightpos.y);

		if(pz1 < 0)
		{
			float fy = nz1*e2 / ny1;
			int iy = (int)((fy+1.0f)*screen_height*0.5f);

			float py = -pz1*nz1 / ny1;
			if(py < lightpos.y)
				rect[1] = max(rect[1],iy);
			else
				rect[3] = min(rect[3],iy);
		}

		if(pz2 < 0)
		{
			float fy = nz2*e2/ny2;
			int iy = (int)((fy+1.0f)*screen_height*0.5f);

			float py = -pz2*nz2 / ny2;
			if(py < lightpos.y)
				rect[1] = max(rect[1],iy);
			else
				rect[3] = min(rect[3],iy);
		}
	}

	rect[0] = max(0, rect[0]);
	rect[1] = max(0, rect[1]);
	rect[2] = max(0, rect[2]);
	rect[3] = max(0, rect[3]);
	rect[0] = min(screen_width, rect[0]);
	rect[1] = min(screen_height, rect[1]);
	rect[2] = min(screen_width, rect[2]);
	rect[3] = min(screen_height, rect[3]);

	int n = (rect[2]-rect[0]) * (rect[3]-rect[1]);

	if(n <= 0)
	{
		return 0;
	}
	else if(n >= screen_width*screen_height)
	{
		// Disable scissor test.
		return screen_width*screen_height;
	}
	else
	{
		// Enable scissor test with rect.
		return n;
	}
}

float getXangle(const vec3<float>& vec)
{
	vec3<float> v1 = vec;
	v1.y = 0.0;
	v1.normalize();
	vec3<float> v2(-1, 0, 0);
	float angle = atan2(v2.z, v2.x) - atan2(v1.z, v1.x);
	return fmod(angle / 3.14159265f * 180.0f + 90.0f, 360.0f);
}

float getYangle(const vec3<float>& vec)
{
	vec3<float> v = vec;
	v.normalize();
	float angle = acos(v.dotProduct(vec3<float>(0,1,0)));
	return -angle / 3.14159265f * 180.0f + 180.0f;
}

