
#include "graphics/hud/itempickrenderer.h"
#include "graphics/opengl.h"
#include "graphics/texturehandler.h"
#include "world/objects/itempicker.h"
#include "world/objects/world_item.h"

void ItemPickRenderer::draw(const Inventory&, const ItemPicker& picker) {

    WorldItem* item = picker.get();
    if(item == 0)
        return;

    glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    TextureHandler::getSingleton().unbindTexture(0);

    float pos_x =  0.0f, width = 1.0f;
    float pos_y = -0.5f, height = 0.5f;

    glColor4f(0.3f, 0.3f, 0.3f, 0.7f);

    glBegin(GL_QUADS);
    		glVertex3f(pos_x - width*.5f  , pos_y - height*.5f, -1);
            glVertex3f(pos_x + width*.5f  , pos_y - height*.5f, -1);
            glVertex3f(pos_x + width*.5f  , pos_y + height*.5f, -1);
            glVertex3f(pos_x - width*.5f  , pos_y + height*.5f, -1);
    glEnd();

    std::vector<std::string> details;
    item->getDetails(details);

    // draw information in two columns
    for(size_t i=0; i<details.size(); ++i) {
        if(i < 8) {
            float y = pos_y - i * 0.06f + height * 0.35f;
            textRenderer.drawString(details[i], pos_x - width * 0.5f, y, 1.3f);
        } else {
            float y = pos_y - (i - 7) * 0.06f + height * 0.35f;
            textRenderer.drawString(details[i], pos_x, y, 1.3f);
        }
    }

    glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
