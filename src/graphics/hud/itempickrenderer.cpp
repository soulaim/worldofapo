
#include "graphics/hud/itempickrenderer.h"
#include "graphics/opengl.h"
#include "graphics/texturehandler.h"
#include "world/objects/itempicker.h"

void ItemPickRenderer::draw(const Inventory&, const ItemPicker& picker) {

    if(picker.get() == 0)
        return;

    TextureHandler::getSingleton().unbindTexture(0);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float pos_x = 0.5f;
    float pos_y = 0.5f;

    glColor4f(1.0f, 0.0f, 0.0f, 0.7f);

    glBegin(GL_QUADS);
    		glVertex3f(pos_x - 0.2f  , pos_y - 0.2f, -1);
            glVertex3f(pos_x + 0.2f  , pos_y - 0.2f, -1);
            glVertex3f(pos_x + 0.2f  , pos_y + 0.2f, -1);
            glVertex3f(pos_x - 0.2f  , pos_y + 0.2f, -1);
    glEnd();

    glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}
