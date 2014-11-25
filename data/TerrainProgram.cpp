#include "TerrainProgram.hpp"

#include <iostream>

using namespace Vancom;

TerrainProgram::TerrainProgram(){

}

bool TerrainProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/terrain.vs"))
        return false;

	if (!addShader(GL_FRAGMENT_SHADER, "../shaders/terrain.fs"))
		return false;

	if (!finalize())
		return false;

    // get uniform and attribute positions in shader
    locMVP = getUniformLocation("mvp");
    locModel = getUniformLocation("model");
    locSampler = getUniformLocation("gSampler");
    locLightDir = getUniformLocation("lightDir");
    locSpotLightDir = getUniformLocation("spotLightDir");
    locSpotLightPos = getUniformLocation("spotLightPos");
    locSpecularPower = getUniformLocation("specularPower");
    locSpecularIntensity = getUniformLocation("specularIntensity");
    locCameraPos = getUniformLocation("cameraPos");
    locSpecularOn = getUniformLocation("isSpecularOn");

    locPos = glGetAttribLocation(program, "vs_pos");
    locTex = glGetAttribLocation(program, "vs_tex");
    locNormal = glGetAttribLocation(program, "vs_normal");

    if(locMVP == -1 || locPos == -1 || locLightDir == -1 || locTex == -1 || locNormal == -1)
        std::cout << "Unable to find location in TerrainProgram" << std::endl;

    return true;
}

void TerrainProgram::setMVP(const glm::mat4 MVP) const{

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(MVP));    
}

void TerrainProgram::setModelPos(const glm::mat4 model) const{

    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));    
}

void TerrainProgram::setSampler(const int sampler) const{

    glUniform1i(locSampler, 0);    
}

void TerrainProgram::setLightDirection(const glm::vec3 direction) const{

    glUniform3fv(locLightDir, 1, glm::value_ptr(direction));
}

void TerrainProgram::setSpotLightDirection(const glm::vec3 direction) const{

    glUniform3fv(locSpotLightDir, 1, glm::value_ptr(direction));
}

void TerrainProgram::setSpotLightPosition(const glm::vec3 pos) const{

    glUniform3fv(locSpotLightPos, 1, glm::value_ptr(pos));
}

void TerrainProgram::setSpecularPower(const float power) const{

    glUniform1f(locSpecularPower, power);
}

void TerrainProgram::setSpecularIntensity(const float intensity) const{

    glUniform1f(locSpecularIntensity, intensity);
}

void TerrainProgram::setCameraPosition(const glm::vec3 pos) const{

    glUniform3fv(locCameraPos, 1, glm::value_ptr(pos));
}

void TerrainProgram::setSpecularFlag(const bool isOn) const{

    int val = 0;
    if(isOn) { val = 1; }
    glUniform1i(locSpecularOn, val);
}
