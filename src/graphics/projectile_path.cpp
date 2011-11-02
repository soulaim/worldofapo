
#include "graphics/projectile_path.h"
#include "graphics/opengl.h"

ProjectilePath::ProjectilePath(const Location& d, const Location& c): start(d), end(c)
{
    r = g = b = 0.7f;
    a = 1.0f;
    scale = 1.0f;

    time_to_live = 3;
}

void ProjectilePath::tick(int amount) {
    time_to_live -= amount;
}

bool ProjectilePath::alive() const {
    return time_to_live > 0;
}

void ProjectilePath::draw() const {
    glColor4f(r, g, b, a * time_to_live / 3);
    glBegin(GL_LINES);
        glVertex3f(start.x.getFloat(), start.y.getFloat(), start.z.getFloat());
        glVertex3f(end.x.getFloat(), end.y.getFloat(), end.z.getFloat());
    glEnd();
}