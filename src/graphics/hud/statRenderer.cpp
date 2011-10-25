
#include "graphics/opengl.h"
#include "graphics/hud/statRenderer.h"
#include "world/objects/unit.h"

#include "graphics/texturehandler.h"

#include <sstream>

using std::stringstream;
using std::vector;
using std::string;

void clamp(float& val, float min, float max) {
    if(val < min)
        val = min;
    if(val > max)
        val = max;
}

int StatRenderer::input(int dx, int dy, int mousePress, int) {
    cursor_x += dx * 0.000001f; clamp(cursor_x, -0.5f, +0.5f);
    cursor_y -= dy * 0.000001f; clamp(cursor_y, -0.5f, +0.5f);
    if(mousePress == 0)
        mouseButtons = 0;
    else
        mouseButtons += 1;

    if(mouseButtons == 1 && selected != -1) {
        return selected;
    }

    return -1;
}

StatRenderer::StatRenderer() {
    cursor_x = 0;
    cursor_y = 0;
    mouseButtons = 0;
    selected = -1;
}

void StatRenderer::drawStatSheet(Unit& unit) {
    const HasProperties& stats = unit.getStats();

    glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float w = 0.5f;
    float h = 0.5f;
    float ho = -0.3f;
    float cw = 0.06f;
    float ch = 0.06f;

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    TextureHandler::getSingleton().bindTexture(0, "stats_bg");
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f(-w, +h+ho, -1.0f);
        glTexCoord2f(0, 0); glVertex3f(-w, -h+ho, -1.0f);
        glTexCoord2f(1, 0); glVertex3f(+w, -h+ho, -1.0f);
        glTexCoord2f(1, 1); glVertex3f(+w, +h+ho, -1.0f);
    glEnd();

    selected = -1;
    for(size_t i=0; i<statsNames.size(); ++i) {
        auto it = stats.intVals.find(statsNames.getKey(i));
        stringstream ss; ss << "^Y" << statsNames.getName(i) << ": ^G" << it->second;
        float y_diff = (cursor_y + ho - 0.035f) + 0.07f * i;
        if(y_diff * y_diff < 0.03f * 0.03f) {
            ss << "^R++";
            selected = i;
        }
        textRenderer.drawString(ss.str(), -0.4f, 0.0f - 0.07f * i, 1.6f);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
    TextureHandler::getSingleton().bindTexture(0, "stats_cursor");
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f(cursor_x-cw, cursor_y+ch+ho, -1.0f);
        glTexCoord2f(0, 0); glVertex3f(cursor_x-cw, cursor_y-ch+ho, -1.0f);
        glTexCoord2f(1, 0); glVertex3f(cursor_x+cw, cursor_y-ch+ho, -1.0f);
        glTexCoord2f(1, 1); glVertex3f(cursor_x+cw, cursor_y+ch+ho, -1.0f);
    glEnd();


    glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
    glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

