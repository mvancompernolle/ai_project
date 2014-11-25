#include "SilhouetteProgram.hpp"

#include <iostream>

using namespace Vancom;

SilhouetteProgram::SilhouetteProgram(){

}

bool SilhouetteProgram::init()
{

    if (!ShaderProgram::init()) {
        return false;
    }

    if (!addShader(GL_VERTEX_SHADER, "../shaders/silhouette.vs")) {
        return false;
    }

    if (!addShader(GL_GEOMETRY_SHADER, "../shaders/silhouette.gs")) {
        return false;
    }

    if (!addShader(GL_FRAGMENT_SHADER, "../shaders/silhouette.fs")) {
        return false;
    }

    if (!finalize()) {
        return false;
    }

    locMVP = getUniformLocation("mvp");
    locModel = getUniformLocation("model");
    locCameraPos = getUniformLocation("cameraPos");

    return true;
}


void SilhouetteProgram::setMVP(const glm::mat4 mvp) const{

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(mvp));      
}


void SilhouetteProgram::setModelPos(const glm::mat4 model) const{

    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));    
}

void SilhouetteProgram::setCameraPosition(const glm::vec3 pos) const{

    glUniform3fv(locCameraPos, 1, glm::value_ptr(pos));
}