
#include "world/level/levelgenerator.h"
#include "world/random.h"

void LevelGenerator::generate(int seed) {

    for(int x=0; x<129; ++x) {
        for(int y=0; y<129; ++y) {
            this->levelMap[x][y] = 'x';
        }
    }

    RandomMachine random;
    random.setSeed(seed);

    // placement of rooms.
    for(int i=0; i<2000; ++i) {
        int w = random.getInt() % 10 + 4;
        int h = random.getInt() % 10 + 4;

        int x = random.getInt() % 129;
        int y = random.getInt() % 129;

        bool ok = true;
        for(int xx = x - w/2 -1; xx < x + w/2+1; ++xx) {
        for(int yy = y - h/2 -1; yy < y + h/2+1; ++yy) {
            if(xx > 0 && yy > 0 && xx < 128 && yy < 128 && this->levelMap[xx][yy] == 'x' && ok);
            else
            {
                ok = false;
                break;
            }
        }
        }

        if(ok) {
            for(int xx = x - w/2; xx < x + w/2; ++xx) {
            for(int yy = y - h/2; yy < y + h/2; ++yy) {
                this->levelMap[xx][yy] = '.';
            }
            }
        }
    }


    // build corridors to connect rooms

    // place start and end

    // place doors and buttons

}
