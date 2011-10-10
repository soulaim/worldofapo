
#include "graphics/hud/inventoryrenderer.h"
#include "world/objects/inventory.h"
#include "graphics/opengl.h"
#include "graphics/texturehandler.h"

InventoryRenderer::InventoryRenderer() {
    x_pos = -0.7f;
    y_pos = +0.5f;
}

void InventoryRenderer::drawArmorItem(const Inventory& inventory, int slot, float dx, float dy, float hw, float hh, int tex_x) {

    WorldItem* item = inventory.getItemSlot(slot);

    int itemLevel = 0;
    if(item != 0)
        itemLevel = item->intVals["ITEM_LVL"];

    // draw
    glColor4f(1, 1, 1, 0.5f);
    int itemType = slot;

    float xp = (itemLevel+1.0f-tex_x) / 8.0f;
    float xn = (itemLevel+tex_x) / 8.0f;
    float yp = 1.0f - itemType / 8.0f - 0.003f;
    float yn = 1.0f - (itemType+1.0f) / 8.0f + 0.003;

    glBegin(GL_QUADS);
        glTexCoord2f(xn, yn); glVertex3f(x_pos + dx - hw, y_pos + dy - hh, -1);
        glTexCoord2f(xp, yn); glVertex3f(x_pos + dx + hw, y_pos + dy - hh, -1);
        glTexCoord2f(xp, yp); glVertex3f(x_pos + dx + hw, y_pos + dy + hh, -1);
        glTexCoord2f(xn, yp); glVertex3f(x_pos + dx - hw, y_pos + dy + hh, -1);
    glEnd();
}


void InventoryRenderer::drawWeaponItem(const Inventory& inventory, int slot, int itemType) {
    WorldItem* item = inventory.getItemSlot(slot);
    int itemLevel = 0;
    if(item != 0)
        itemLevel = item->intVals["ITEM_LVL"];

    if(inventory.active_item == (unsigned)slot)
        glColor4f(1, 1, 1, 1);
    else
        glColor4f(0.8f, 0.8f, 0.8f, 0.5f);

    float xp = (itemLevel+1.0f) / 8.0f;
    float xn = itemLevel / 8.0f;
    float yp = 1.0f - itemType / 8.0f - 0.003f;
    float yn = 1.0f - (itemType+1.0f) / 8.0f + 0.003f;

    float x = (slot - 7) * 0.1f - 0.5f;
    float y = -0.9f;
    float hw = 0.05f;
    float hh = 0.05f;

    glBegin(GL_QUADS);
        glTexCoord2f(xn, yn); glVertex3f(x - hw, y - hh, -1);
        glTexCoord2f(xp, yn); glVertex3f(x + hw, y - hh, -1);
        glTexCoord2f(xp, yp); glVertex3f(x + hw, y + hh, -1);
        glTexCoord2f(xn, yp); glVertex3f(x - hw, y + hh, -1);
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

    TextureHandler::getSingleton().bindTexture(0, "item_texture");

    drawArmorItem(inventory, inventory.HEAD_SLOT, 0.0f, +0.19f, 0.07f * 0.5f, 0.10f * 0.5f);
    drawArmorItem(inventory, inventory.TORSO_SLOT, 0.0f, 0.02f, 0.14f * 0.5f, 0.22f * 0.5f);
    drawArmorItem(inventory, inventory.LEGS_SLOT, 0.0f, -0.21f, 0.15f * 0.5f, 0.24f * 0.5f);
    drawArmorItem(inventory, inventory.BELT_SLOT, 0.0f, -0.07f, 0.1f * 0.5f, 0.1f * 0.5f);
    drawArmorItem(inventory, inventory.ARMS_SLOT, +0.135f, +0.08f, 0.15f * 0.5f, 0.13f * 0.5f);
    drawArmorItem(inventory, inventory.ARMS_SLOT, -0.135f, +0.08f, 0.15f * 0.5f, 0.13f * 0.5f, +1);
    drawArmorItem(inventory, inventory.AMULET_SLOT, 0.0f, +0.08f, 0.1f * 0.5f, 0.1f * 0.5f);

    drawWeaponItem(inventory, 6, 6);
    drawWeaponItem(inventory, 7, 6);
    drawWeaponItem(inventory, 8, 7);
    drawWeaponItem(inventory, 9, 7);
    drawWeaponItem(inventory, 10, 7);
    //drawWeaponItem(inventory, inventory.getItemSlot(10), 7);
    //drawWeaponItem(inventory, inventory.getItemSlot(11), 7);

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}
