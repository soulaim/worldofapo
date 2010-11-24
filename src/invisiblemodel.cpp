#include "invisiblemodel.h"

using namespace std;

InvisibleModel::InvisibleModel():
	Model()
{
}

InvisibleModel::InvisibleModel(const InvisibleModel& model):
	Model(model)
{
}

void InvisibleModel::rotate_y(float)
{
}

bool InvisibleModel::load(const string&)
{
	return false;
}

bool InvisibleModel::save(const string&) const
{
	return false;
}

void InvisibleModel::draw() const
{
}

