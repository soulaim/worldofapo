/*
 * File:   stats_names.h
 * Author: urtela
 *
 * Created on October 25, 2011, 2:31 PM
 */

#ifndef STATS_NAMES_H
#define	STATS_NAMES_H

#include <string>
#include <vector>

class StatsNames {

    std::vector<std::string> names;
    std::vector<std::string> keys;

public:
    StatsNames();
    const std::string& getKey(int i) const;
    const std::string& getName(int i) const;
    size_t size() const;
};


#endif	/* STATS_NAMES_H */

