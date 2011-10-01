#include "opengl.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

bool& OpenGL::getGL3bit() {
    static bool gl3 = true;
    return gl3;
}

OpenGL::OpenGL() {
    init();
}


void OpenGL::init() {

    static bool haveInited = false;
    if(haveInited)
        return;
    haveInited = true;

    GLenum status = glewInit();
    if (status == GLEW_OK) {
        std::cerr << "GLEW JIHUU :DD" << std::endl;
    } else {
        std::stringstream ss;
        ss << "GLEW initialization failed: " << glewGetErrorString(status);
        throw std::runtime_error(ss.str());
    }

    if (!GLEW_VERSION_3_0) {
        // throw std::runtime_error("Hardware does not support OpenGL 3.0");
        std::cerr << "Hardware does not support OpenGL 3.0: Not using geometry shaders." << std::endl;
        bool& gl3bit = getGL3bit();
        gl3bit = false;
    }
}
