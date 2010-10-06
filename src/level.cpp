

#include "level.h"
#include <cstdlib>

using namespace std;

vector<vector<FixedPoint> > pointheight_info;
//vector<vector<int> > walls_info;

void Level::generate(int seed)
{
	srand(seed);
	pointheight_info.resize(101);
	for(size_t i = 0; i < pointheight_info.size(); ++i)
	{
		pointheight_info[i].resize( 101 );
		for(size_t k = 0; k < pointheight_info[i].size(); ++k)
		{
			pointheight_info[i][k].number = 3000;
		}
	}
	
	// create long walls
	for(int i=0; i<150; i++)
	{
		
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		
		for(int k=0; k<15; k++)
		{
			pointheight_info[x_p][y_p].number = 28000;
			x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
			
			if(x_p < 0 || x_p >= pointheight_info.size())
				break;
			if(y_p < 0 || y_p >= pointheight_info[x_p].size())
				break;
		}
	}
	
	// create some accessible higher ground
	for(int i=0; i<150; i++)
	{
		
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		int height = 3000;
		
		for(int k=0; k<10; k++)
		{
			height += 1000;
			pointheight_info[x_p][y_p].number = height;
			x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
			
			if(x_p < 0 || x_p >= pointheight_info.size())
				break;
			if(y_p < 0 || y_p >= pointheight_info[x_p].size())
				break;
		}
	}
	
	
	/*
	pointheight_info[10][11].number = 18000;
	pointheight_info[11][10].number = 18000;
	pointheight_info[11][11].number = 18000;
	pointheight_info[10][10].number = 18000;
	
	pointheight_info[15][15].number = 40000;
	pointheight_info[15][16].number = 40000;
	pointheight_info[16][16].number = 40000;
	pointheight_info[17][17].number = 40000;
	*/
}

float Level::estimateHeightDifference(int x, int y)
{
	float min = 10000000;
	float max = 0;
	
	if(pointheight_info[x][y].number < min)
		min = pointheight_info[x][y].number;
	if(pointheight_info[x][y].number > max)
		max = pointheight_info[x][y].number;
	
	x++;
	if(pointheight_info[x][y].number < min)
		min = pointheight_info[x][y].number;
	if(pointheight_info[x][y].number > max)
		max = pointheight_info[x][y].number;
	
	y++;
	if(pointheight_info[x][y].number < min)
		min = pointheight_info[x][y].number;
	if(pointheight_info[x][y].number > max)
		max = pointheight_info[x][y].number;
	
	x--;
	if(pointheight_info[x][y].number < min)
		min = pointheight_info[x][y].number;
	if(pointheight_info[x][y].number > max)
		max = pointheight_info[x][y].number;
	
	return max - min;
}



FixedPoint Level::getHeight(const FixedPoint& x, const FixedPoint& y)
{
	int x_index = x.getInteger() / 8;
	int y_index = y.getInteger() / 8;
	
	int x_desimal = x.getDesimal() + (x.getInteger() & 7) * 1000;
	int y_desimal = y.getDesimal() + (y.getInteger() & 7) * 1000;
	
	if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) -2)
		return FixedPoint(0);
	if(y_index < 0 || y_index > static_cast<int>(pointheight_info.size()) -2)
		return FixedPoint(0);
	
	FixedPoint th1 = pointheight_info[x_index][y_index];
	FixedPoint th2 = pointheight_info[x_index+1][y_index];
	
	FixedPoint bh1 = pointheight_info[x_index][y_index+1];
	FixedPoint bh2 = pointheight_info[x_index+1][y_index+1];
	
	int bot_val = th1.number + x_desimal * (th2.number - th1.number) / 8000;
	int top_val = bh1.number + x_desimal * (bh2.number - bh1.number) / 8000;
	
	FixedPoint height_value;
	height_value.number = bot_val + y_desimal * (top_val - bot_val) / 8000;
	return height_value;
}



int Level::max_x() const
{
	return pointheight_info.size() * 8 - 8;
}

int Level::max_z() const
{
	return pointheight_info.front().size() * 8 - 8;
}

