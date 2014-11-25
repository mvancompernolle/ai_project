#include "ShadowVolumeProgram.hpp"

#include <iostream>

using namespace Vancom;

ShadowVolumeProgram::ShadowVolumeProgram(){

}

bool ShadowVolumeProgram::init()
{

    if (!ShaderProgram::init()) {
        return false;
    }

    if (!addShader(GL_VERTEX_SHADER, "../shaders/silhouette.vs")) {
        return false;
    }

    if (!addShader(GL_GEOMETRY_SHADER, "../shaders/shadowVolume.gs")) {
        return false;
    }

    if (!addShader(GL_FRAGMENT_SHADER, "../shaders/stencil.fs")) {
        return false;
    }

    if (!finalize()) {
        return false;
    }

    locVP = getUniformLocation("vp");
    locModel = getUniformLocation("model");
    locLightPos = getUniformLocation("lightPos");

    return true;
}

void ShadowVolumeProgram::setVP(const glm::mat4 vp) const{

    glUniformMatrix4fv(locVP, 1, GL_FALSE, glm::value_ptr(vp));   
}

void ShadowVolumeProgram::setModel(const glm::mat4 model) const{

    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model)); 
}

void ShadowVolumeProgram::setLightPosition(const glm::vec3 pos) const{

    glUniform3f(locLightPos, pos.x, pos.y, pos.z);
}