
#include "graphics/terrain/roof_renderer.h"
#include "world/world.h"
#include "world/level/level.h"

RoofRenderer::RoofRenderer(): roofTexture1("roof1"), roofTexture2("roof2") {
    roofTexture = roofTexture1;
    enabled = true;
}

void RoofRenderer::draw(Level* level) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    int max_x = level->max_x().getInteger();
    int max_z = level->max_z().getInteger();

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max_x, 10, max_z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max_x, 10, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0, 10, 0);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(0, 10, max_z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max_x, 10, max_z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0, 10, 0);
    glEnd();
}

void RoofRenderer::setTexture(const std::string& texName) {
    roofTexture = texName;
}

void RoofRenderer::enable() {
    enabled = true;
}

void RoofRenderer::disable() {
    enabled = false;
}
