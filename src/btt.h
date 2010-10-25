

#ifndef H_QUADTREE_TERRAIN
#define H_QUADTREE_TERRAIN

#include "fixed_point.h"
#include <iostream>
#include <vector>

using std::cerr;
using std::endl;


struct BTT_Point
{
	int x, z;
	
	BTT_Point operator + (const BTT_Point& a)
	{
		BTT_Point val;
		val = a;
		val.x += x;
		val.z += z;
		return val;
	}
	
	const BTT_Point& operator /=(const int a)
	{
		x /= a;
		z /= a;
		return *this;
	}
	
	void print()
	{
		cerr << "(" << x << ", " << z << ") ";
	}
};

struct BTT_Triangle
{
	BTT_Triangle(BTT_Point a, BTT_Point b, BTT_Point c)
	{
		points[0] = a;
		points[1] = b;
		points[2] = c;
	}
	
	BTT_Point points[3];
};

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
	
	void doSplitting(const std::vector<FixedPoint>& var_tree, const std::vector<std::vector<FixedPoint> >& h_diffs, unsigned myIndex, int importance_x, int importance_z)
	{
		// lets just try something simple first.
		if(myLod > 14)
		{
			return;
		}
		
		if(hasChildren())
		{
			left_child->doSplitting(var_tree, h_diffs, myIndex * 2);
			right_child->doSplitting(var_tree, h_diffs, myIndex * 2 + 1);
			return;
		}
		else
		{
			// ok, so when do I need to split?
			
			// if not in frustum -> can disable triangle (set neighbour's appropriate neighbour pointer to zero.
			
			// if variance error too high for distance -> split
			
			// if player character in triangle, split
			importance_x importance_z
			
			
			FixedPoint error = var_tree[myIndex];
			if(myIndex >= var_tree.size())
			{
				BTT_Point mid = p_left + p_right;
				mid /= 2;
				error = h_diffs[mid.x][mid.z] - (h_diffs[p_left.x][p_left.z] + h_diffs[p_right.x][p_right.z]) / FixedPoint(2);
			}
			
			if(error > FixedPoint(0))
			{
				split();
				left_child->doSplitting(var_tree, h_diffs, myIndex * 2);
				right_child->doSplitting(var_tree, h_diffs, myIndex * 2 + 1);
			}
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
		var_tree[myIndex] = h_diffs[mid.x][mid.z] - ((h_diffs[p_left.x][p_left.z] + h_diffs[p_left.x][p_left.z])) / FixedPoint(2);
		
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

class BinaryTriangleTree
{
public:
	BinaryTriangleTree(int max_x, int max_z)
	{
		reset(max_x, max_z);
	}
	
	~BinaryTriangleTree()
	{
		// release memory.
		upper_left.shutDown();
		lower_right.shutDown();
	}
	
	
	FixedPoint buildVarianceTree(const std::vector<std::vector<FixedPoint> >& h_diffs, std::vector<FixedPoint>& var_tree)
	{
		FixedPoint max(0);
		
		FixedPoint a = upper_left.buildVarianceTree(h_diffs, var_tree, 1);
		FixedPoint b = lower_right.buildVarianceTree(h_diffs, var_tree, 2);
		
		upper_left.shutDown();
		lower_right.shutDown();
		
		if(a > max)
			max = a;
		if(b > max)
			max = b;
		return max;
	}
	
	
	void reset(int max_x, int max_z)
	{
		// release memory.
		upper_left.shutDown();
		lower_right.shutDown();
		
		upper_left.down = &lower_right;
		lower_right.down = &upper_left;
		
		upper_left.p_top.x = 0;
		upper_left.p_top.z = max_z;
		
		upper_left.p_left.x = 0;
		upper_left.p_left.z = 0;
		
		upper_left.p_right.x = max_x;
		upper_left.p_right.z = max_z;
		
		
		lower_right.p_top.x = max_x;
		lower_right.p_top.z = 0;
		
		lower_right.p_left.x = max_x;
		lower_right.p_left.z = max_z;
		
		lower_right.p_right.x = 0;
		lower_right.p_right.z = 0;
	}
	
	void doSplit(const std::vector<std::vector<FixedPoint> >& h_diffs, const std::vector<FixedPoint>& var_tree)
	{
		/*
		upper_left.killChildren();
		lower_right.killChildren();
		*/
		
		upper_left.shutDown();
		lower_right.shutDown();
		
		upper_left.down = &lower_right;
		lower_right.down = &upper_left;
		
		upper_left.left = 0;
		upper_left.right = 0;
		lower_right.left = 0;
		lower_right.right = 0;
		
		upper_left.doSplitting(var_tree, h_diffs, 1);
		lower_right.doSplitting(var_tree, h_diffs, 2);
	}
	
	void print()
	{
		upper_left.print();
		lower_right.print();
	}
	
	
	void getTriangles(std::vector<BTT_Triangle>& tris) const
	{
		upper_left.getTriangles(tris);
		lower_right.getTriangles(tris);
	}
	
	void draw(int x, int z)
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
	
	BTT_Node upper_left;
	BTT_Node lower_right;
	
	
};


#endif

