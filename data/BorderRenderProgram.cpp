#include "BorderRenderProgram.hpp"

#include <iostream>

using namespace Vancom;

BorderRenderProgram::BorderRenderProgram(){

}

bool BorderRenderProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/borderRender.vs"))
        return false;

    if (!addShader(GL_GEOMETRY_SHADER, "../shaders/borderRender.gs"))
        return false;

	if (!addShader(GL_FRAGMENT_SHADER, "../shaders/borderRender.fs"))
		return false;

	if (!finalize())
		return false;

	// get uniform locations
    locMVP = getUniformLocation("mvp");
    locCameraPos = getUniformLocation("cameraPos");
    locWidth = getUniformLocation("width");
    locHeight = getUniformLocation("height");
    locMixVal = getUniformLocation("mixVal");

    return true;
}

void BorderRenderProgram::setMVP(const glm::mat4 MVP){

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(MVP));    
}


void BorderRenderProgram::setCameraPos(const glm::vec3& pos){

	glUniform3fv(locCameraPos, 1, glm::value_ptr(pos));
}

void BorderRenderProgram::setParticleSize(float width, float height){

	// set width and height of border particles
    glUniform1f(locWidth, width);
    glUniform1f(locHeight, height);
}

void BorderRenderProgram::setMixVal(float mixVal){

    glUniform1f(locMixVal, mixVal);
}