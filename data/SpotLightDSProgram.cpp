#include "SpotLightDSProgram.hpp"

#include <iostream>

using namespace Vancom;

SpotLightDSProgram::SpotLightDSProgram(){

}

bool SpotLightDSProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/light.vs"))
        return false;

    if (!addShader(GL_FRAGMENT_SHADER, "../shaders/spotLight.fs"))
        return false;

    if (!finalize())
        return false;

    // get uniforms
    locColor = getUniformLocation("spotLight.base.base.color");
    locAmbient = getUniformLocation("spotLight.base.base.ambientIntensity");
    locDiffuse = getUniformLocation("spotLight.base.base.diffuseIntensity");
    locPos = getUniformLocation("spotLight.base.pos");
    locDir = getUniformLocation("spotLight.direction");
    locAttenConst = getUniformLocation("spotLight.base.atten.constant");
    locAttenLinear = getUniformLocation("spotLight.base.atten.linear");
    locAttenExp = getUniformLocation("spotLight.base.atten.ex");
    locCutoff = getUniformLocation("spotLight.cutoff");

    return LightDSProgram::init();
}

void SpotLightDSProgram::setSpotLight(const SpotLight& light) const{

    glUniform3fv(locColor, 1, glm::value_ptr(light.color));
    glUniform1f(locAmbient, light.ambientIntensity);
    glUniform1f(locDiffuse, light.diffuseIntensity);
    glUniform3fv(locPos, 1, glm::value_ptr(light.pos));
    glUniform3fv(locDir, 1, glm::value_ptr(light.direction));
    glUniform1f(locAttenConst, light.atten.constant);
    glUniform1f(locAttenLinear, light.atten.linear);
    glUniform1f(locAttenExp, light.atten.exp);
    glUniform1f(locCutoff, light.cutoff);
}