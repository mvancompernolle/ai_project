#include "PointLightDSProgram.hpp"

#include <iostream>

using namespace Vancom;

PointLightDSProgram::PointLightDSProgram(){

}

bool PointLightDSProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/light.vs"))
        return false;

    if (!addShader(GL_FRAGMENT_SHADER, "../shaders/pointLight.fs"))
        return false;

    if (!finalize())
        return false;

    // get uniforms
    locColor = getUniformLocation("pLight.base.color");
    locAmbient = getUniformLocation("pLight.base.ambientIntensity");
    locDiffuse = getUniformLocation("pLight.base.diffuseIntensity");
    locPos = getUniformLocation("pLight.pos");
    locAttenConst = getUniformLocation("pLight.atten.constant");
    locAttenLinear = getUniformLocation("pLight.atten.linear");
    locAttenExp = getUniformLocation("pLight.atten.ex");

    return LightDSProgram::init();
}

void PointLightDSProgram::setPointLight(const PointLight& light) const{

    glUniform3fv(locColor, 1, glm::value_ptr(light.color));
    glUniform1f(locAmbient, light.ambientIntensity);
    glUniform1f(locDiffuse, light.diffuseIntensity);
    glUniform3fv(locPos, 1, glm::value_ptr(light.pos));
    glUniform1f(locAttenConst, light.atten.constant);
    glUniform1f(locAttenLinear, light.atten.linear);
    glUniform1f(locAttenExp, light.atten.exp);
}