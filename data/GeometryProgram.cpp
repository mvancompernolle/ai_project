#include "GeometryProgram.hpp"

#include <iostream>

using namespace Vancom;

GeometryProgram::GeometryProgram(){

}

bool GeometryProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/geometry.vs"))
        return false;

	if (!addShader(GL_FRAGMENT_SHADER, "../shaders/geometry.fs"))
		return false;

	if (!finalize())
		return false;

    // get uniform and attribute positions in shader
    locMVP = getUniformLocation("mvp");
    locModel = getUniformLocation("model");
    locSampler = getUniformLocation("colorMap");
    locSpecularPower = getUniformLocation("specularPower");
    locSpecularIntensity = getUniformLocation("specularIntensity");

    return true;
}

void GeometryProgram::setMVP(const glm::mat4 MVP) const{

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(MVP));    
}

void GeometryProgram::setModelPos(const glm::mat4 model) const{

    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));    
}

void GeometryProgram::setColorTextureUnit(const int textureUnit) const{

    glUniform1i(locSampler, textureUnit);    
}

void GeometryProgram::setSpecularPower(const float power) const{

    glUniform1f(locSpecularPower, power);
}

void GeometryProgram::setSpecularIntensity(const float intensity) const{

    glUniform1f(locSpecularIntensity, intensity);
}