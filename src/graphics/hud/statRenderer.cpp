
#include "graphics/opengl.h"
#include "graphics/hud/statRenderer.h"
#include "world/objects/unit.h"

#include <sstream>

using std::stringstream;
using std::vector;
using std::string;

StatRenderer::StatRenderer() {
    names.push_back("Hyperbolic\\sMovement");
    keys.push_back("MOVEMENT");

    names.push_back("Telepathic\\ssenses");
    keys.push_back("TELEPATHIC");

    names.push_back("Zen\\smind");
    keys.push_back("ZEN");

    names.push_back("Biometal\\sEndoskeleton");
    keys.push_back("CONSTITUTION");

    names.push_back("Master\\sEngineer");
    keys.push_back("ENGINEER");

    names.push_back("Perception");
    keys.push_back("PERCEPTION");

    names.push_back("Dark\\svision");
    keys.push_back("DARKVISION");

    names.push_back("Ballistic\\saptitude");
    keys.push_back("BALLISTIC");

    names.push_back("Particle\\sprodigy");
    keys.push_back("BEAM");

    names.push_back("Sneak\\s&\\sHide");
    keys.push_back("SNEAK");
}

void StatRenderer::drawStatSheet(Unit& unit) {
    const HasProperties& stats = unit.getStats();
    for(size_t i=0; i<keys.size(); ++i) {
        auto it = stats.intVals.find(keys[i]);
        stringstream ss; ss << "^Y" << names[i] << ": ^G" << it->second;
        textRenderer.drawString(ss.str(), -0.3f, 0.5f - 0.07f * i, 1.6f);
    }
}
