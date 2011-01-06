#ifndef H_QUADTREE_TERRAIN
#define H_QUADTREE_TERRAIN

#include "fixed_point.h"
#include <iostream>
#include <vector>
#include <memory>

class FrustumR;
class Location;
class Level;

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
	
	void print() const
	{
		std::cerr << "(" << x << ", " << z << ") ";
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

struct BTT_Node;

class BinaryTriangleTree
{
public:
	BinaryTriangleTree(int max_x, int max_z);
	void buildVarianceTree(const Level&);

	~BinaryTriangleTree();
	void reset(int max_x, int max_z);


	void print() const;
	void draw(int x, int z);

	void splitBTT(const Level& lvl, const Location& position, const FrustumR& frustum);
	void getTriangles(std::vector<BTT_Triangle>& tris) const;
	
private:
	FixedPoint buildVarianceTree(const Level&, std::vector<FixedPoint>& var_tree);
	FixedPoint buildVarianceTree(const std::vector<std::vector<FixedPoint> >& h_diffs, std::vector<FixedPoint>& var_tree);
	void doSplit(const std::vector<std::vector<FixedPoint> >& h_diffs, const std::vector<FixedPoint>& var_tree, const Location& position, const FrustumR& frustum, const Level& lvl);

	std::shared_ptr<BTT_Node> upper_left;
	std::shared_ptr<BTT_Node> lower_right;

	std::vector<FixedPoint> variance_tree;
};

#endif

