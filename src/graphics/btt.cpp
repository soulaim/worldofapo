#include "graphics/btt.h"
#include "graphics/frustum/frustumr.h"
#include "misc/fixed_point.h"
#include "world/level.h"

#include <iostream>
#include <vector>

using namespace std;

struct BTT_Node
{
	BTT_Node(): left_child(0), right_child(0), left(0), right(0), down(0), myLod(0), active(true)
	{
	}
	
	bool hasChildren() const
	{
		if(left_child == 0)
			return false;
		return left_child->active;
	}
	
	void createChildren()
	{
		if(left_child != 0)
		{
			left_child->activate();
			right_child->activate();
		}
		else
		{
			right_child = new BTT_Node();
			left_child  = new BTT_Node();
		}
	}
	
	void trivialSplit()
	{
		// trivial split. first create a new node
		createChildren();
		
		// if there are neighbours, update them accordingly.
		updateNeighbours();
		
		
		right_child->down = right;
		right_child->right = left_child;
		right_child->left = down;
		
		left_child->down = left;
		left_child->left = right_child;
		left_child->right = down;
		
		right_child->myLod = myLod+1;
		left_child->myLod = myLod+1;
		
		// finally update triangle vertices.
		right_child->p_top = p_left + p_right;
		right_child->p_top /= 2;
		right_child->p_left = p_right;
		right_child->p_right = p_top;
		
		left_child->p_right = p_left;
		left_child->p_left = p_top;
		left_child->p_top = right_child->p_top;
		return;
	}
	
	
	
	
	void updateNeighbours()
	{
		if(right != 0)
		{
			if(right->myLod == myLod)
				right->left = right_child;
			else
				right->down = right_child;
		}
		
		if(left != 0)
		{
			if(left->myLod == myLod)
				left->right = left_child;
			else
				left->down = left_child;
		}
	}
	
	
	
	void split()
	{
		if(hasChildren())
		{
			return;
		}
		
		if(down == 0)
		{
			trivialSplit();
			return;
		}
		
		// if levels of detail differ (meaning that  down->myLod == myLod - 1), just split down first so its easy to split me
		if(down->myLod != myLod)
		{
			down->split();
		}
		
		// first split myself!
		createChildren();
		
		// split down triangle as well
		down->createChildren();
		
		right_child->myLod = myLod+1;
		left_child->myLod  = myLod+1;
		
		down->left_child->myLod  = down->myLod+1;
		down->right_child->myLod = down->myLod+1;
		
		
		// this is ok, since the down will remain constant for new part (but not for *this)
		right_child->down = right;
		right_child->right = left_child;
		right_child->left = down->left_child;
		
		left_child->down = left;
		left_child->left = right_child;
		left_child->right = down->right_child;
		
		down->right_child->left  = this->left_child;
		down->right_child->right = down->left_child;
		down->right_child->down  = down->right;
		
		down->left_child->right = this->right_child;
		down->left_child->left  = down->right_child;
		down->left_child->down  = down->left;
		
		updateNeighbours();
		down->updateNeighbours();
		
		
		// finally update triangle vertices.
		right_child->p_top = p_left + p_right;
		right_child->p_top /= 2;
		right_child->p_left = p_right;
		right_child->p_right = p_top;
		
		left_child->p_right = p_left;
		left_child->p_left = p_top;
		left_child->p_top = right_child->p_top;
		
		// finally update triangle vertices.
		down->right_child->p_top   = down->p_left + down->p_right;
		down->right_child->p_top  /= 2;
		down->right_child->p_left  = down->p_right;
		down->right_child->p_right = down->p_top;
		
		down->left_child->p_right = down->p_left;
		down->left_child->p_left  = down->p_top;
		down->left_child->p_top   = down->right_child->p_top;
	}
	
	void activate()
	{
		active = true;
		updateNeighbours();
	}
	
	
	void getTriangles(std::vector<BTT_Triangle>& triangles) const
	{
		if(!hasChildren())
		{
			triangles.push_back(BTT_Triangle(p_top, p_left, p_right));
		}
		else
		{
			left_child->getTriangles(triangles);
			right_child->getTriangles(triangles);
		}
	}
	
	void print()
	{
		if(!hasChildren())
		{
			cerr << "TRIANGLE: ";
			p_top.print();
			p_left.print();
			p_right.print();
			cerr << endl;
		}
		else
		{
			left_child->print();
			right_child->print();
		}
	}
	
	void doSplitting(const std::vector<FixedPoint>& var_tree, const std::vector<std::vector<FixedPoint> >& h_diffs, unsigned myIndex, const Location& position, const FrustumR& frustum, const Level& lvl)
	{
		// lets just try something simple first.
		
		static int taulukko1[] = {0, 1, 2, 0, 3, 0, 0, 0, 4};
		static int taulukko2[] = {0, 5, 6, 0, 7, 0, 0, 0, 8};
		static int taulukko3[] = {0, 9, 10, 0, 11, 0, 0, 0, 12};
		static int taulukko4[] = {0, 13, 14, 0, 15, 0, 0, 0, 16};
		
		int number = (h_diffs.size() - 1);
		
		int bitti = 0;
		bitti += taulukko1[number & (1 | 2 | 4 | 8)];
		bitti += taulukko2[(number >> 4) & (1 | 2 | 4 | 8)];
		bitti += taulukko3[(number >> 8) & (1 | 2 | 4 | 8)];
		bitti += taulukko4[(number >> 12) & (1 | 2 | 4 | 8)];
		
		assert(bitti < 12);
		
		// I have ABSOLUTELY no clue whether this is correct or not.
		if(myLod > ((bitti - 1) * 2) - 1)
		{
			return;
		}
		
		if(hasChildren())
		{
			left_child->doSplitting(var_tree, h_diffs, myIndex * 2, position, frustum, lvl);
			right_child->doSplitting(var_tree, h_diffs, myIndex * 2 + 1, position, frustum, lvl);
			return;
		}
		else
		{
			// ok, so when do I need to split?
			
			// TODO ALERT: heights of points are not zero. need to fix this.
			vec3<float> points[3];
			points[0].x = p_left.x * Level::BLOCK_SIZE;
			points[0].z = p_left.z * Level::BLOCK_SIZE;
			points[0].y = lvl.getVertexHeight(p_left.x, p_left.z).getFloat();
			
			points[1].x = p_right.x * Level::BLOCK_SIZE;
			points[1].z = p_right.z * Level::BLOCK_SIZE;
			points[1].y = lvl.getVertexHeight(p_right.x, p_right.z).getFloat();
			
			points[2].x = p_top.x * Level::BLOCK_SIZE;
			points[2].z = p_top.z * Level::BLOCK_SIZE;
			points[2].y = lvl.getVertexHeight(p_top.x, p_top.z).getFloat();
			
			vec3<float> semiAverage = (points[0] + points[1] + points[2]) / 3.0f;
			float r = (semiAverage - points[0]).length();
			
			
			// if not in frustum -> can disable triangle (set neighbour's appropriate neighbour pointer to zero.
			if(frustum.sphereInFrustum(semiAverage, r) == FrustumR::OUTSIDE)
			{
				// well, lets just not split it at this time.
				// TODO ALERT: SHOULD COMPLETELY DESTROY THIS TRIANGLE, SO IT WONT BE HANDLED LATER ON
				return;
			}
			
			
			// if variance error too high for distance -> split
			FixedPoint error;
			if(myIndex >= var_tree.size())
			{
				BTT_Point mid = p_left + p_right;
				mid /= 2;
				error = (h_diffs[mid.x][mid.z] - (h_diffs[p_left.x][p_left.z] + h_diffs[p_right.x][p_right.z]) / FixedPoint(2)).abs();
			}
			else
			{
				error = var_tree[myIndex];
			}
			
			// if(error > FixedPoint(0))
			{
				split();
				left_child->doSplitting(var_tree, h_diffs, myIndex * 2, position, frustum, lvl);
				right_child->doSplitting(var_tree, h_diffs, myIndex * 2 + 1, position, frustum, lvl);
			}
			
			/*
			float x_diff = (semiAverage.x - position.x.getFloat());
			float y_diff = (semiAverage.y - position.y.getFloat());
			float z_diff = (semiAverage.z - position.z.getFloat());
			float sqrDist = x_diff * x_diff + y_diff * y_diff + z_diff * z_diff;
			
			// if error compared to distance is great enough, split
			if(error > sqrDist / 100.f)
			{
				split();
				left_child->doSplitting(var_tree, h_diffs, myIndex * 2, position, frustum);
				right_child->doSplitting(var_tree, h_diffs, myIndex * 2 + 1, position, frustum);
			}
			*/
			
			/*
			else if()
			{
				// if player character in triangle, split
				// TODO:
				
				return;
			}
			*/
			
		}
	}
	
	void shutDown()
	{
		// intentionally not using hasChildren()
		if(left_child != 0)
		{
			left_child->shutDown();
			right_child->shutDown();
			
			delete left_child;
			delete right_child;
			
			left_child = 0;
			right_child = 0;
		}
	}
	
	
	FixedPoint buildVarianceTree(const std::vector<std::vector<FixedPoint> >& h_diffs, std::vector<FixedPoint>& var_tree, unsigned myIndex)
	{
		if(myIndex >= var_tree.size())
			return FixedPoint(0);
		
		BTT_Point mid = p_left + p_right;
		mid /= 2;
		FixedPoint error = h_diffs[mid.x][mid.z] - ((h_diffs[p_left.x][p_left.z] + h_diffs[p_right.x][p_right.z])) / FixedPoint(2);
		var_tree[myIndex] = error;
		
		if(var_tree[myIndex] < FixedPoint(0))
			var_tree[myIndex] *= -1;
		
		split();
		FixedPoint v_left  = left_child->buildVarianceTree(h_diffs, var_tree, myIndex * 2);
		FixedPoint v_right = right_child->buildVarianceTree(h_diffs, var_tree, myIndex * 2 + 1);
		
		if(v_left > var_tree[myIndex])
			var_tree[myIndex] = v_left;
		if(v_right > var_tree[myIndex])
			var_tree[myIndex] = v_right;
		
		return var_tree[myIndex];
	}
	
	/*
	void kill()
	{
		active = false;
		killChildren();
	}
	
	void killChildren()
	{
		// still no clue whether its possible to upkeep the structure with deletions. so don't call these functions.
		if(hasChildren())
		{
			left_child->kill();
			right_child->kill();
		}
	}
	*/
	
	BTT_Node *left_child, *right_child;
	BTT_Node *left, *right, *down;
	BTT_Point p_top, p_left, p_right;
	int myLod;
	bool active;
};

BinaryTriangleTree::BinaryTriangleTree(int max_x, int max_z):
	upper_left(new BTT_Node),
	lower_right(new BTT_Node)
{
	reset(max_x, max_z);
}

BinaryTriangleTree::~BinaryTriangleTree()
{
	// release memory.
	upper_left->shutDown();
	lower_right->shutDown();
}


FixedPoint BinaryTriangleTree::buildVarianceTree(const std::vector<std::vector<FixedPoint> >& h_diffs, std::vector<FixedPoint>& var_tree)
{
	FixedPoint max(0);
	
	FixedPoint a = upper_left->buildVarianceTree(h_diffs, var_tree, 1);
	FixedPoint b = lower_right->buildVarianceTree(h_diffs, var_tree, 2);
	
	upper_left->shutDown();
	lower_right->shutDown();
	
	if(a > max)
		max = a;
	if(b > max)
		max = b;
	return max;
}


void BinaryTriangleTree::reset(int max_x, int max_z)
{
	// release memory.
	upper_left->shutDown();
	lower_right->shutDown();
	
	upper_left->down = lower_right.get();
	lower_right->down = upper_left.get();
	
	upper_left->p_top.x = 0;
	upper_left->p_top.z = max_z;
	
	upper_left->p_left.x = 0;
	upper_left->p_left.z = 0;
	
	upper_left->p_right.x = max_x;
	upper_left->p_right.z = max_z;
	
	
	lower_right->p_top.x = max_x;
	lower_right->p_top.z = 0;
	
	lower_right->p_left.x = max_x;
	lower_right->p_left.z = max_z;
	
	lower_right->p_right.x = 0;
	lower_right->p_right.z = 0;
}

void BinaryTriangleTree::doSplit(const std::vector<std::vector<FixedPoint> >& h_diffs, const std::vector<FixedPoint>& var_tree, const Location& position, const FrustumR& frustum, const Level& lvl)
{
	/*
	upper_left->killChildren();
	lower_right->killChildren();
	*/
	
	upper_left->shutDown();
	lower_right->shutDown();
	
	upper_left->down = lower_right.get();
	lower_right->down = upper_left.get();
	
	upper_left->left = 0;
	upper_left->right = 0;
	lower_right->left = 0;
	lower_right->right = 0;
	
	upper_left->doSplitting(var_tree, h_diffs, 1, position, frustum, lvl);
	lower_right->doSplitting(var_tree, h_diffs, 2, position, frustum, lvl);
}

void BinaryTriangleTree::print() const
{
	upper_left->print();
	lower_right->print();
}


void BinaryTriangleTree::getTriangles(std::vector<BTT_Triangle>& tris) const
{
	upper_left->getTriangles(tris);
	lower_right->getTriangles(tris);
}

void BinaryTriangleTree::draw(int x, int z)
{
	std::vector<BTT_Triangle> tris;
	getTriangles(tris);
	
	char painting[129][129];
	
	for(int i=0; i<129; i++)
		for(int k=0; k<129; k++)
			painting[i][k] = ' ';
	
	for(size_t i=0; i<tris.size(); i++)
	{
		int x1 = tris[i].points[0].x;
		int z1 = tris[i].points[0].z;
		int x2 = tris[i].points[1].x;
		int z2 = tris[i].points[1].z;
		int x3 = tris[i].points[2].x;
		int z3 = tris[i].points[2].z;
		
		for(int k=0; k<100; k++)
		{
			int p1x = x1 + k * (x2 - x1) / 100;
			int p1z = z1 + k * (z2 - z1) / 100;
			
			int p2x = x2 + k * (x3 - x2) / 100;
			int p2z = z2 + k * (z3 - z2) / 100;
			
			int p3x = x3 + k * (x1 - x3) / 100;
			int p3z = z3 + k * (z1 - z3) / 100;
			
			painting[p1x][p1z] = '+';
			painting[p2x][p2z] = '+';
			painting[p3x][p3z] = '+';
		}
	}
	
	painting[x][z] = '0';
	painting[x-1][z] = 'l';
	painting[x-1][z-1] = '/';
	painting[x][z-1] = '"';
	painting[x+1][z-1] = '\\';
	painting[x+1][z] = 'l';
	painting[x+1][z+1] = '/';
	painting[x][z+1] = '_';
	painting[x-1][z+1] = '\\';
	
	for(int i=0; i<129; i++)
	{
		for(int k=0; k<129; k++)
			cerr << painting[k][i];
		cerr << endl;
	}
}

void BinaryTriangleTree::buildVarianceTree(const Level& lvl)
{
	variance_tree.resize(2048, FixedPoint(0));
	FixedPoint max_error = buildVarianceTree(lvl.pointheight_info, variance_tree);
	cerr << "Maximum encountered variance error: " << max_error << endl;
}

void BinaryTriangleTree::splitBTT(const Level& lvl, const Location& position, const FrustumR& frustum)
{
	doSplit(lvl.pointheight_info, variance_tree, position, frustum, lvl);
}

