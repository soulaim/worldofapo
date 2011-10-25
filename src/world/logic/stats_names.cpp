


#include "stats_names.h"

StatsNames::StatsNames() {
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


const std::string& StatsNames::getKey(int i) const {
    return keys[i];
}

const std::string& StatsNames::getName(int i) const {
    return names[i];
}

size_t StatsNames::size() const {
    return keys.size();
}
