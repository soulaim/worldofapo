
#include "editor.h"
#include "../texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;


Editor::Editor()
{
	init();
}

void Editor::init()
{
	view.loadObjects("data/parts.dat");
	view.megaFuck(); // blah..
	
	userio.init();
	
	// load some textures.
	// should not be done here. FIX
	TextureHandler::getSingleton().createTexture("grass", "data/grass.png");
	TextureHandler::getSingleton().createTexture("highground", "data/highground.png");
	TextureHandler::getSingleton().createTexture("mountain", "data/hill.png");
}

void Editor::start()
{
	tick();

	view.setTime( SDL_GetTicks() );

	view.tick();
	view.draw(models);
}

void Editor::tick()
{

}

