#include "DirLightDSProgram.hpp"

#include <iostream>

using namespace Vancom;

DirLightDSProgram::DirLightDSProgram(){

}

bool DirLightDSProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/light.vs"))
        return false;

    if (!addShader(GL_FRAGMENT_SHADER, "../shaders/dirLight.fs"))
        return false;

    if (!finalize())
        return false;

    // get uniforms
    locColor = getUniformLocation("dirLight.base.color");
    locAmbient = getUniformLocation("dirLight.base.ambientIntensity");
    locDiffuse = getUniformLocation("dirLight.base.diffuseIntensity");
    locDir = getUniformLocation("dirLight.direction");

    return LightDSProgram::init();
}

void DirLightDSProgram::setDirLight(const DirectionalLight& light) const{

    glUniform3fv(locColor, 1, glm::value_ptr(light.color));
    glUniform1f(locAmbient, light.ambientIntensity);
    glUniform1f(locDiffuse, light.diffuseIntensity);
    glUniform3fv(locDir, 1, glm::value_ptr(light.direction));
}