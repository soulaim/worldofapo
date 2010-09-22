

#include "model.h"
#include <fstream>

using namespace std;


void Model::tick()
{
  animation_time++;
  return;
}


void Model::setAction(string name)
{
  animation_name = name;
  animation_time = 0;
}


void Model::load(string filename)
{
  ifstream in_file(filename.c_str());
  while(in_file.good() && !in_file.eof())
  {
    string cmd, father, child, wirename;
    float osx, osy, osz;
    in_file >> cmd;
    
    if(cmd == "CHILD")
    {
      in_file >> father >> child >> wirename >> osx >> osy >> osz;
      for(int i=0; i<parts.size(); i++)
      {
	  if(parts[i].name == father)
	  {
	    parts[i].children.push_back(parts.size());
	    break;
	  }
      }
      
      ModelNode node;
      node.name = child;
      node.wireframe = wirename;
      node.offset_x = osx;
      node.offset_y = osy;
      node.offset_z = osz;
      parts.push_back(node);
      
    }
    else if(cmd == "ROOT")
    {
      in_file >> father >> wirename;
      root = parts.size();
      
      ModelNode node;
      node.name = father;
      node.wireframe = wirename;
      parts.push_back(node);
    }
  }
}

