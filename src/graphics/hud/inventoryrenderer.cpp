
#include "graphics/hud/inventoryrenderer.h"
#include "world/objects/inventory.h"
#include "graphics/opengl.h"
#include "graphics/texturehandler.h"

InventoryRenderer::InventoryRenderer() {
    x_pos = -0.7f;
    y_pos = +0.5f;
}

void InventoryRenderer::drawItem(const Inventory& inventory, int slot, float dx, float dy, float hw, float hh) {

    WorldItem* item = inventory.getItemSlot(slot);

    int itemLevel = 0;
    if(item != 0)
        itemLevel = item->intVals["ITEM_LVL"];

    TextureHandler::getSingleton().bindTexture(0, "item_texture");

    // draw
    glColor4f(1, 1, 1, 0.5f);
    int itemType = slot;

    float xp = (itemLevel+1.0f) / 8.0f;
    float xn = itemLevel / 8.0f;
    float yp = 1.0f - itemType / 8.0f;
    float yn = 1.0f - (itemType+1.0f) / 8.0f;

    glBegin(GL_QUADS);
        glTexCoord2f(xn, yn); glVertex3f(x_pos + dx - hw, y_pos + dy - hh, -1);
        glTexCoord2f(xp, yn); glVertex3f(x_pos + dx + hw, y_pos + dy - hh, -1);
        glTexCoord2f(xp, yp); glVertex3f(x_pos + dx + hw, y_pos + dy + hh, -1);
        glTexCoord2f(xn, yp); glVertex3f(x_pos + dx - hw, y_pos + dy + hh, -1);
    glEnd();
}

void InventoryRenderer::draw(const Inventory& inventory) {
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

    glEnable(GL_BLEND);

    drawItem(inventory, inventory.HEAD_SLOT, 0.0f, +0.15f, 0.11f * 0.5f, 0.11f * 0.5f);
    drawItem(inventory, inventory.TORSO_SLOT, 0.0f, 0.0f, 0.1f * 0.5f, 0.15f * 0.5f);
    drawItem(inventory, inventory.LEGS_SLOT, 0.0f, -0.24f, 0.1f * 0.5f, 0.15f * 0.5f);
    drawItem(inventory, inventory.BELT_SLOT, 0.0f, -0.12f, 0.1f * 0.5f, 0.1f * 0.5f);
    drawItem(inventory, inventory.ARMS_SLOT, +0.1f, +0.075f, 0.1f * 0.5f, 0.1f * 0.5f);
    drawItem(inventory, inventory.ARMS_SLOT, -0.1f, +0.075f, 0.1f * 0.5f, 0.1f * 0.5f);
    drawItem(inventory, inventory.AMULET_SLOT, 0.0f, +0.06f, 0.1f * 0.5f, 0.1f * 0.5f);

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}
