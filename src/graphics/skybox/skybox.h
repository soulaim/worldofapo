

#ifndef SKYBOX_H
#define	SKYBOX_H

#include "graphics/shaders.h"
#include <string>

class Shader;
class Camera;

class Skybox
{
public:

    const std::string space;
    const std::string sky;
    const std::string night;

    Skybox();

    void draw(Shader& shader, Camera* camera_p);
    void setSkyBox();
private:
    std::string skyboxTexture;
};


#endif	/* SKYBOX_H */

