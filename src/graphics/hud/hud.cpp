#include "graphics/hud/hud.h"
#include "graphics/texturehandler.h"
#include "graphics/font.h"
#include "graphics/frustum/matrix4.h"
#include "graphics/string2rgb.h"

#include "misc/timer.h"
#include "misc/apomath.h"

#include "graphics/opengl.h"

#include <string>
#include <sstream>
#include <map>
#include <iomanip>
#include <cmath>

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;
extern int QUADS_DRAWN_THIS_FRAME;


Hud::PerformanceData::PerformanceData()
{
	reset();
}

void Hud::PerformanceData::reset()
{
	last_time = Timer::time_now();
	fps = 0;
	world_fps = 0;
	frames = 0;
	world_ticks = 0;
}

float Hud::getFPS()
{
	return perfData.fps;
}

Hud::Hud():
	showStats(false),
	currentTime(0)
{
	cerr << "Loading config file for HUD.." << endl;
	load("configs/hud.conf");

	area_name = "World of Apo";
}

void Hud::reset()
{
	perfData.reset();
}

void Hud::setAreaName(const string& areaName)
{
	area_name = areaName;
}

void Hud::insertDebugString(const std::string& str)
{
	core_info.push_back(str);
}

void Hud::setLevelSize(const FixedPoint& x, const FixedPoint& z)
{
	level_max_x = x;
	level_max_z = z;
}

// TODO: These are pointless. HUD has units map and local player ID stored, so doesnt need function calls to deduce these things..
void Hud::setLocalPlayerName(const string& name)
{
	plr_name = name;
}

void Hud::setLocalPlayerHP(const int life)
{
	stringstream ss;
	ss << life;
	health = ss.str();
}

void Hud::setLocalPlayerKills(const int k)
{
	stringstream ss;
	ss << k;
	kills = ss.str();
}

void Hud::setLocalPlayerDeaths(const int d)
{
	stringstream ss;
	ss << d;
	deaths = ss.str();
}

void Hud::handle(const ChatMessage& msg)
{
	viewMessages.push_back(ViewMessage(msg.line, currentTime));
}

void Hud::setTime(unsigned time)
{
	currentTime = time;
}

void Hud::setCurrentClientCommand(const string& cmd)
{
	currentClientCommand = cmd;
}

void Hud::world_tick()
{
	++perfData.world_ticks;

	long long time_now = Timer::time_now();
	long long time_since_last = time_now - perfData.last_time;

	perfData.fps = 1000.0f * perfData.frames / time_since_last;
	perfData.world_fps = 1000.0f * perfData.world_ticks/ time_since_last;
}

void Hud::setUnitsMap(std::map<int, Unit>* _units)
{
	units = _units;
}

void Hud::setLocalPlayerID(int _myID)
{
	myID = _myID;
}

void Hud::setPlayerInfo(map<int,PlayerInfo> *pInfo)
{
	Players = pInfo;
}

void Hud::drawStatusBar() const
{
	stringstream ss1;
    stringstream ss2;
    stringstream ss3;
    std::map<int, Unit>::iterator it = units->find(this->myID);
    if(it == units->end())
        return;

	ss1 << "^YLife:     ^G" << health << "/" << it->second.getMaxHP();
    ss2 << "^YSanity: ^B" << it->second.intVals["SANITY"] << " %";
    ss3 << "^YArmor:   ^W" << it->second.getInventory().getArmorClass();
	textRenderer.drawString(ss1.str(), -0.9f, 0.90f, 2.0f, true);
    textRenderer.drawString(ss2.str(), -0.9f, 0.83f, 2.0f, true);
    textRenderer.drawString(ss3.str(), -0.9f, 0.76f, 2.0f, true);
}

void Hud::drawBanner() const
{
	textRenderer.drawString("^YArea: ^G" + area_name, -0.1f, +0.9f, 1.3f);
}

void Hud::drawAmmo() const
{
	if(units->find(myID) == units->end())
		return;

	Unit& myUnit = units->find(myID)->second;
    const Inventory& inventory = myUnit.getInventory();
    WorldItem* item = inventory.getItemActive();
    if(item == 0)
        return;

    // info display for ballistic weapons.
    if(item->intVals["TYPE"] == 1)
    {
        float reloading = 1.0f * item->intVals["RLTIME"] / item->intVals["RELOAD_TIME"];
        int clip_ammo = item->intVals["CLIP"];
        int clip_size = item->intVals["CLIPSIZE"];

        stringstream ammo;
        string colorCode;

        if(clip_ammo > 0) {
            colorCode = "^G";
        }
        else {
            colorCode = "^R";
        }

        ammo << "^YAmmo: " << colorCode << clip_ammo << "/" << clip_size;

        textRenderer.drawString(ammo.str(), 0.f, -0.9f, 2.0f, true);

        /*
        if(oncooldown > 0.1f) {
            textRenderer.drawString("cooldown");
        }
        */

        if(reloading > 0.00001f)
        {
            // this should be centered
            textRenderer.drawString("^RRELOADING", -0.2, -0.7f, 2.0f, true);
            barRenderer.drawBar(reloading, "GREEN", "GREEN", -0.2, +0.2, -0.77f, -0.72f);
        }
    }
}


void Hud::drawLevelUpInfo()
{
    if(units->find(myID) == units->end())
	return;

    Unit& myUnit = units->find(myID)->second;
    if (myUnit.intVals["STAT_POINTS"] > 0)
    {
        stringstream ss0;
        ss0 << "^RUnused statpoints left: " << myUnit.intVals["STAT_POINTS"];
        textRenderer.drawString(ss0.str(), -0.95f, -0.4f, 2.0f, true);
    }

}



void Hud::drawMessages()
{
	for(size_t i = 0; i < viewMessages.size(); ++i)
	{
		if(viewMessages[i].endTime < currentTime)
		{
			for(size_t k = i+1; k < viewMessages.size(); ++k)
				viewMessages[k-1] = viewMessages[k];
			viewMessages.pop_back();

			i--;
			continue;
		}

		int reverse_index = viewMessages.size() - (i+1);
		float pos_x = -0.9;
		float pos_y = -0.82 + 0.05 * reverse_index;

		float location_alpha = 1.0f - reverse_index * 0.045f;
		float age_alpha      = float(viewMessages[i].endTime - currentTime) / ViewMessage::VIEW_MESSAGE_LIFE;
		float alpha          = age_alpha * location_alpha;

		if(alpha < 0.f)
			alpha = 0.f;
		textRenderer.drawString(viewMessages[i].msgContent, pos_x, pos_y, viewMessages[i].scale * 2.0f, viewMessages[i].hilight, alpha);
	}

	if(currentClientCommand.size() > 0)
		textRenderer.drawString(currentClientCommand, -0.9, -0.9, 1.3, true);
}

void Hud::drawFPS()
{

	++perfData.frames;
	long long time_now = Timer::time_now();
	long long time_since_last = time_now - perfData.last_time;

	if(time_since_last > 500)
	{
		perfData.fps = 1000.0f * perfData.frames / time_since_last;
		perfData.world_fps = 1000.0f * perfData.world_ticks/ time_since_last;
		perfData.frames = 1;
		perfData.world_ticks = 1;
		perfData.last_time = time_now;
	}

	// TODO: these should probably not be set here.
	stringstream ss0;
	ss0 << "FPS: " << fixed << setprecision(2) << perfData.fps;
	stringstream ss1;
	ss1 << "SPF: " << fixed << setprecision(3) << 1.0/perfData.fps;
	stringstream ss2;
	ss2 << "TPS: " << fixed << setprecision(2) << perfData.world_fps;
	stringstream ss3;
	ss3 << "TRIS: " << fixed << setprecision(2) << TRIANGLES_DRAWN_THIS_FRAME;
	stringstream ss4;
	ss4 << "QUADS: " << fixed << setprecision(2) << QUADS_DRAWN_THIS_FRAME;

	core_info.push_back(ss0.str());
	core_info.push_back(ss1.str());
	core_info.push_back(ss2.str());
	core_info.push_back(ss3.str());
	core_info.push_back(ss4.str());

	for(size_t i = 0; i < core_info.size(); i++)
	{
		textRenderer.drawString(core_info[i], 0.6f, 0.9f - 0.1f * i, 1.5, true);
	}

}

void Hud::draw(bool firstPerson)
{
	if(firstPerson)
	{
		drawCrossHair();
	}

    // These should be separate components, would be about 1000 times more clear
	drawMessages();
	drawStatusBar();
	drawMinimap();
	drawBanner();
	drawFPS();
	drawAmmo();
        drawLevelUpInfo();

    if(this->units != 0) {
        map<int, Unit>::iterator plr_it = this->units->find(this->myID);
        if(plr_it != this->units->end()) {
            inventoryRenderer.draw(plr_it->second.getInventory());
            itemPickRenderer.draw(plr_it->second.getInventory(), plr_it->second.getItemPicker());
            if(showStats)
                statRenderer.drawStatSheet(plr_it->second);
        }
    }

	// clear per-visual-frame info messages
	core_info.clear();
}

void Hud::setShowStats(bool _showStats) {
	showStats = _showStats;
}

int Hud::statViewInput(int dx, int dy, int mousePress, int keyState) {
    return statRenderer.input(dx, dy, mousePress, keyState);
}

void Hud::drawCrossHair() const
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	auto it = strVals.find("CROSSHAIR");
	string texture = (it == strVals.end() ? "chessboard" : it->second);
	TextureHandler::getSingleton().bindTexture(0, texture);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	float scale = 1.0f;
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-0.03f * scale, -0.03f * scale, -1);
	glTexCoord2f(1.f, 0.f); glVertex3f(+0.03f * scale, -0.03f * scale, -1);
	glTexCoord2f(1.f, 1.f); glVertex3f(+0.03f * scale, +0.03f * scale, -1);
	glTexCoord2f(0.f, 1.f); glVertex3f(-0.03f * scale, +0.03f * scale, -1);
	++QUADS_DRAWN_THIS_FRAME;
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

//	if(lightsActive)
//		glEnable(GL_LIGHTING);
}

void Hud::drawMinimap() const
{
	static ApoMath apomath = ApoMath();

	auto iteratorMyUnit = (*units).find(myID);
	if(iteratorMyUnit == units->end())
		return;

	int my_team = iteratorMyUnit->second["TEAM"];

	TextureHandler::getSingleton().unbindTexture(0);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	Unit& myUnit = iteratorMyUnit->second;
    float range = myUnit.getModifier("TELEPATHIC") * 8;

	float map_r = 0.18f;
	float minimap_angle = apomath.getDegrees( myUnit.angle );

	float tx = (myUnit.position.x / level_max_x).getFloat();
	float tz = (myUnit.position.z / level_max_z).getFloat();

	float unit_x_on_minimap = -2.f * map_r * tx + map_r;
	float unit_z_on_minimap = +2.f * map_r * tz - map_r;

	// make minimap rotate along with the user.
	// glTranslatef(+0.74f - unit_x_on_minimap, -0.74f - unit_z_on_minimap, 0.0f);
	glTranslatef(+0.74f, -0.74f, 0.0f);
	glRotatef(-minimap_angle, 0.0f, 0.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/*
	glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
	glBegin(GL_QUADS);
	glVertex3f(-map_r, -map_r, 0.f);
	glVertex3f(+map_r, -map_r, 0.f);
	glVertex3f(+map_r, +map_r, 0.f);
	glVertex3f(-map_r, +map_r, 0.f);
	++QUADS_DRAWN_THIS_FRAME;
	glEnd();
	*/

	glPointSize(5.0f);
	glBegin(GL_POINTS);
	for(map<int, Unit>::iterator it = units->begin(); it != units->end(); ++it)
	{
        if( (it->second.getPosition() - myUnit.getPosition()).lengthSquared().getFloat() > range * range )
            continue;

        // telepathy does not reveal unconscious elements
        if( it->second.controllerTypeID == Unit::INANIMATE_OBJECT )
            continue;

		const int id = it->first;

		float r = 1.0f;
		float g = 0.0f;
		float b = 0.0f;

		if(myID == id)
		{
			r = 0.0f;
			g = 1.0f;
			b = 0.0f;
		}
		else if(it->second["TEAM"] == my_team)
		{
			if(it->second.human())
			{
				r = 0;
				g = 1;
				b = 0;
			}
			else
			{
				r = 173.0f / 255.0f;
				g = 255.0f / 255.0f;
				b = 47.0f / 255.0f;
			}
		}
		else
		{
			if(it->second.human())
			{
				r = 1.0f;
				g = 0.0f;
				b = 0.0f;
			}
			else
			{
				r = 1.0f;
				g = 0.5f;
				b = 0.1f;
			}
		}

		const Unit& u = it->second;
		const Location& loc = u.getPosition();

		float x = -(loc.x / level_max_x).getFloat() * 2 * map_r + map_r - unit_x_on_minimap;
		float z = +(loc.z / level_max_z).getFloat() * 2 * map_r - map_r - unit_z_on_minimap;

		glColor3f(r, g, b); glVertex3f(x, z, 0.0f);
	}
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void Hud::clearDebugStrings()
{
	core_info.clear();
}

