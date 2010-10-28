
#ifndef H_STRINGSYSTEM
#define H_STRINGSYSTEM

#include "frustum/Vec3.h"

struct StringNode
{
	Vec3 pos;
	Vec3 vel;
	Vec3 force;
	
	float mass;
	float u, v;
	
	std::vector<StringArc> arcs;
};

struct StringArc
{
	int target;
	float k;
	float d;
};

struct StringSystem
{
private:
	void updateForce(StringNode& a, StringNode& b, StringArc& arc)
	{
		Vec3 d_vec = b.pos - a.pos;
		float distance = d_vec.length();
		d_vec.normalize();
		
		float delta = distance - arc.d;
		float intensity = arc.k * delta;
		
		a.force += d_vec * intensity;
		b.force -= d_vec * intensity;
	}
	
public:
	void addArc(int a, int b, float k, float d)
	{
		StringArc arc;
		arc.k = k;
		arc.d = d;
		arc.target = b;
		nodes[a].arcs.push_back(arc);
		arc.target = a;
		nodes[b].arcs.push_back(arc);
	}
	
	void addNode(const Vec3& pos, float mass, float u, float v)
	{
		StringNode node;
		node.mass = mass;
		node.u = u;
		node.v = v;
		node.pos = pos;
		
		nodes.push_back(node);
	}
	
	void gatherForces()
	{
		for(size_t i=0; i<nodes.size(); i++)
		{
			nodes[i].force.x = 0.f;
			nodes[i].force.y = 0.f;
			nodes[i].force.z = 0.f;
		}
		
		for(size_t i=0; i<nodes.size(); i++)
		{
			StringNode& node = nodes[i];
			for(size_t k=0; k<node.arcs.size(); k++)
				updateForce(node, nodes[node.arcs[k].target], node.arcs[k]);
		}
	}
	
	void applyForces()
	{
		for(size_t i=0; i<nodes.size(); i++)
		{
			nodes[i].vel += nodes[i].force / nodes[i].mass;
			nodes[i].vel.y -= 0.035; // gravity. TODO: Should come from the outside.
			nodes[i].pos += nodes[i].vel;
			nodes[i].vel *= 0.995f; // entropy takes it's toll!! BWAHAHAHHA :DD
		}
	}
	
	std::vector<StringNode> nodes;
};


#endif

